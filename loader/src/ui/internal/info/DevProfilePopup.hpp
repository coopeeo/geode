#pragma once

#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

class ModListLayer;

class UselessClass {
    protected:
        std::string uselessString;
    public:
        UselessClass();
};

template <typename T>
class DevProfilePopup : public Popup<std::string const&, T*> {
protected:
    T* m_layer;
    // bool m_noElasticity;
    // CCLayer* m_mainLayer;

    virtual bool setup(std::string const& developer, T* list) override;

public:
    static DevProfilePopup* create(std::string const& developer, T* list);
};
