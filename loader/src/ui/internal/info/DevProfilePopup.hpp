#pragma once

#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

class ModListLayer;
template <typename T>
class DevProfilePopup : public Popup<std::string const&, T*> {
protected:
    ModListLayer* m_layer;

    template <typename T>
    bool setup(std::string const& developer, T* list) override;

public:
    static DevProfilePopup* create(std::string const& developer, T* list);
    static DevProfilePopup* create(std::string const& developer);
};
