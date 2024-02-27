#pragma once

#include "Result.hpp"
#include "MiniFunction.hpp"
#include "../loader/Event.hpp"

namespace geode {
    namespace impl {
        struct DefaultProgress {
            std::string message;
            std::optional<uint8_t> percentage;

            DefaultProgress() = default;
            DefaultProgress(auto msg) : message(msg) {}
            DefaultProgress(auto msg, uint8_t percentage) : message(msg), percentage(percentage) {}
        };
    }

    class PromiseCancellationToken final {
    private:
        std::shared_ptr<std::atomic_bool> token = std::make_shared<std::atomic_bool>(false);

        template <class T, class E, class P>
        friend class Promise;

    public:
        inline operator bool() {
            return *token;
        }
    };
    
    template <class T = impl::DefaultValue, class E = impl::DefaultError, class P = impl::DefaultProgress>
    class PromiseEventFilter;

    /**
     * Represents an asynchronous `Result`. Similar to `Future` in Rust, or 
     * `Promise` in JavaScript. May have only one of each kind of callback. Can 
     * also be used to monitor the progress of the upcoming value. All 
     * callbacks are always run in the main thread, so interacting with UI is 
     * safe
     */
    template <class T = impl::DefaultValue, class E = impl::DefaultError, class P = impl::DefaultProgress>
    class [[nodiscard]] Promise final {
    public:
        using Then = utils::MiniFunction<void(T)>;
        using Expect = utils::MiniFunction<void(E)>;
        using Progress = utils::MiniFunction<void(P)>;
        using Finally = utils::MiniFunction<void()>;
        using SimpleExecutor = utils::MiniFunction<void(
            Then resolve,
            Expect reject
        )>;
        using Executor = utils::MiniFunction<void(
            Then resolve,
            Expect reject,
            Progress progress,
            PromiseCancellationToken cancellationToken
        )>;

        /**
         * Create a Promise. Call the provided callbacks to notify the 
         * listener when the Promise is finished. Use the other constructor 
         * overloads to specify progress and handle cancellation.
         * 
         * @param threaded Whether the Promise should start executing in a new 
         * thread or not; if false, the Promise starts immediately executing in 
         * the current thread
         * 
         * @note See the class description for general information about 
         * Promises
         */
        Promise(SimpleExecutor&& executor, bool threaded = true)
          : Promise([executor](auto resolve, auto reject, auto, auto) {
                executor(resolve, reject);
            }, threaded) {}
        
        /**
         * Create a Promise. Call the provided callbacks to notify the 
         * listener when the Promise is finished. If the user cancels the 
         * Promise, this is reflected in the `cancelled` parameter; you can 
         * read from it, and if it's true, you can stop whatever you were doing 
         * and not call any of the other callbacks.
         * 
         * @param threaded Whether the Promise should start executing in a new 
         * thread or not; if false, the Promise starts immediately executing in 
         * the current thread
         * 
         * @note See the class description for general information about 
         * Promises
         */
        Promise(Executor&& executor, bool threaded = true) : m_data(std::make_shared<Data>()) {
            if (threaded) {
                std::thread([executor = std::move(executor), data = m_data]() mutable {
                    Promise::invoke_executor(std::move(executor), data);
                }).detach();
            }
            else {
                Promise::invoke_executor(std::move(executor), m_data);
            }
        }

        /**
         * Add a listener for when the Promise finishes. There may only be one 
         * listener at a time. If the Promise has already been resolved, the 
         * callback is immediately queued in the main thread
         */
        Promise& then(Then&& handler) {
            if (m_data->cancellationToken) return *this;
            std::unique_lock<std::mutex> _(m_data->mutex);
            if (m_data->result.has_value()) {
                auto v = std::move(m_data->result).value();
                if (v.index() == 0) {
                    Loader::get()->queueInMainThread([handler = std::move(handler), ok = std::move(std::get<0>(v))] {
                        handler(ok);
                    });
                }
            }
            else {
                m_data->thenHandler = handler;
            }
            return *this;
        }
        /**
         * Add a listener for when the Promise fails. There may only be one 
         * listener at a time. If the Promise has already been resolved, the 
         * callback is immediately queued in the main thread
         */
        Promise& expect(Expect&& handler) {
            if (m_data->cancellationToken) return *this;
            std::unique_lock<std::mutex> _(m_data->mutex);
            if (m_data->result.has_value()) {
                auto v = std::move(m_data->result).value();
                if (v.index() == 1) {
                    Loader::get()->queueInMainThread([handler = std::move(handler), err = std::move(std::get<1>(v))] {
                        handler(err);
                    });
                }
            }
            else {
                m_data->expectHandler = handler;
            }
            return *this;
        }
        /**
         * Add a listener for when the Promise's progress is updated. There may 
         * only be one listener at a time. If the Promise has already been 
         * resolved, nothing happens
         */
        Promise& progress(Progress&& handler) {
            if (m_data->cancellationToken) return *this;
            std::unique_lock<std::mutex> _(m_data->mutex);
            if (!m_data->result.has_value()) {
                m_data->progressHandler = handler;
            }
            return *this;
        }
        /**
         * Add a listener for when the Promise is finished, regardless of if 
         * it was succesful or not. There may only be one listener at a time. 
         * If the Promise has already been resolved, the callback is 
         * immediately queued in the main thread
         */
        Promise& finally(Finally&& handler) {
            if (m_data->cancellationToken) return *this;
            std::unique_lock<std::mutex> _(m_data->mutex);
            if (m_data->result.has_value()) {
                Loader::get()->queueInMainThread([handler = std::move(handler)] {
                    handler();
                });
            }
            else {
                m_data->finallyHandler = handler;
            }
            return *this;
        }

        /**
         * Cancel the Promise. Removes all listeners and sets the signal for 
         * cancelling. Whether or not the promise actually can interrupt its 
         * operation depends on the Promise; as such, this is not guaranteed to 
         * actually stop the operation that created the Promise, but it is 
         * guaranteed that the listener will not be notified after a call to 
         * cancel
         */
        Promise& cancel() {
            if (m_data->cancellationToken) return *this;
            std::unique_lock<std::mutex> _(m_data->mutex);
            m_data->thenHandler = nullptr;
            m_data->expectHandler = nullptr;
            m_data->progressHandler = nullptr;
            m_data->finallyHandler = nullptr;
            m_data->cancellationToken.token = true;
            return *this;
        }

        /**
         * Link this Promise to be cancelled alongside another Promise
         */
        Promise& link(PromiseCancellationToken otherCancellationToken) {
            if (m_data->cancellationToken) return *this;
            m_data->cancellationToken = otherCancellationToken;
            return *this;
        }

        /**
         * Returns a filter for listening to this `Promise` through the Geode 
         * Events system. Useful for example for using `Promise`s on layers, 
         * which may be removed from the node tree before the `Promise` 
         * finishes and as such calling a `then` callback that captures the 
         * layer would then read undefined memory
         */
        PromiseEventFilter<T, E, P> listen();

    private:
        struct Data {
            // Mutex for handlers & result
            std::mutex mutex;
            Then thenHandler;
            Expect expectHandler;
            Progress progressHandler;
            Finally finallyHandler;
            std::optional<std::variant<T, E>> result;
            PromiseCancellationToken cancellationToken;
        };
        // This has to be a shared_ptr so that the data can persist even after 
        // the future is destroyed, as well as to share it between resolve, reject, and the likes
        std::shared_ptr<Data> m_data;

        static void invoke_executor(Executor&& executor, std::shared_ptr<Data> data) {
            executor(
                [data](auto&& value) {
                    if (data->cancellationToken) return;
                    std::unique_lock<std::mutex> _(data->mutex);
                    bool handled = false;
                    if (data->thenHandler) {
                        Loader::get()->queueInMainThread([fun = std::move(data->thenHandler), v = std::move(value)] {
                            fun(v);
                        });
                        handled = true;
                    }
                    if (data->finallyHandler) {
                        Loader::get()->queueInMainThread([fun = std::move(data->finallyHandler)] {
                            fun();
                        });
                        handled = true;
                    }
                    if (!handled) {
                        data->result = std::variant<T, E>(std::in_place_index<0>, std::move(value));
                    }
                },
                [data](auto&& error) {
                    if (data->cancellationToken) return;
                    std::unique_lock<std::mutex> _(data->mutex);
                    bool handled = false;
                    if (data->expectHandler) {
                        Loader::get()->queueInMainThread([fun = std::move(data->expectHandler), v = std::move(error)] {
                            fun(v);
                        });
                        handled = true;
                    }
                    if (data->finallyHandler) {
                        Loader::get()->queueInMainThread([fun = std::move(data->finallyHandler)] {
                            fun();
                        });
                        handled = true;
                    }
                    if (!handled) {
                        data->result = std::variant<T, E>(std::in_place_index<1>, std::move(error));
                    }
                },
                [data](auto&& p) {
                    if (data->cancellationToken) return;
                    std::unique_lock<std::mutex> _(data->mutex);
                    if (auto handler = data->progressHandler) {
                        Loader::get()->queueInMainThread([p = std::move(p), handler]() mutable {
                            handler(std::move(p));
                        });
                    }
                },
                data->cancellationToken
            );
        }
    };

    /**
     * Wraps a `Promise` in the Geode Event system for easier consumption. 
     * Useful for example for layers, where just regularly waiting for the 
     * `Promise` could run into issues if the layer is freed from memory; 
     * whereas with event listeners being RAII, they are automatically 
     * removed from layers, avoiding use-after-free errors
     */
    template <class T = impl::DefaultValue, class E = impl::DefaultError, class P = impl::DefaultProgress>
    class PromiseEvent : public Event {
    protected:
        size_t m_id;
        std::variant<T, E, P> m_value;

        PromiseEvent(size_t id, std::variant<T, E, P>&& value) : m_id(id), m_value(value) {}

        friend class Promise<T, E, P>;
        friend class PromiseEventFilter<T, E, P>;

    public:
        T const* getResolve()  const { return std::get_if<0>(&m_value); }
        E const* getReject()   const { return std::get_if<1>(&m_value); }
        P const* getProgress() const { return std::get_if<2>(&m_value); }
        bool isFinally() const { return m_value.index() != 2; }
    };

    template <class T, class E, class P>
    class PromiseEventFilter : public EventFilter<PromiseEvent<T, E, P>> {
    public:
        using Callback = void(PromiseEvent<T, E, P>*);

    protected:
        size_t m_id;

        friend class Promise<T, E, P>;

        PromiseEventFilter(size_t id) : m_id(id) {}

    public:
        PromiseEventFilter() : m_id(0) {}

        ListenerResult handle(utils::MiniFunction<Callback> fn, PromiseEvent<T, E, P>* event) {
            // log::debug("Event mod filter: {}, {}, {}, {}", m_mod, static_cast<int>(m_type), event->getMod(), static_cast<int>(event->getType()));
            if (m_id == event->m_id) {
                fn(event);
            }
            return ListenerResult::Propagate;
        }
    };

    template <class T, class E, class P>
    PromiseEventFilter<T, E, P> Promise<T, E, P>::listen() {
        // After 4 billion promises this will overflow and start producing 
        // the same IDs again, so technically if some promise takes 
        // literally forever then this could cause issues later on
        static size_t ID_COUNTER = 0;
        // Reserve 0 for PromiseEventFilter not listening to anything
        if (ID_COUNTER == 0) {
            ID_COUNTER += 1;
        }
        size_t id = ++ID_COUNTER;
        this
            ->then([id](auto&& value) { 
                PromiseEvent<T, E, P>(id, std::variant<T, E, P> { std::in_place_index<0>, std::forward<T>(value) }).post();
            })
            .expect([id](auto&& error) {
                PromiseEvent<T, E, P>(id, std::variant<T, E, P> { std::in_place_index<1>, std::forward<E>(error) }).post();
            })
            .progress([id](auto&& prog) {
                PromiseEvent<T, E, P>(id, std::variant<T, E, P> { std::in_place_index<2>, std::forward<P>(prog) }).post();
            });
        return PromiseEventFilter<T, E, P>(id);
    }
}