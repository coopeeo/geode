#pragma once

#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

class ModListLayer;

template <typename T>
class DevProfilePopup : public Popup<std::string const&, T*> {
protected:
    T* m_layer;


    virtual bool setup(std::string const& developer, T* list) override;

public:
    static DevProfilePopup* create(std::string const& developer, T* list);
};
