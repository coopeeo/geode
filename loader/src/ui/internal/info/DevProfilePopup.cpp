#include "DevProfilePopup.hpp"
#include <Geode/ui/ListView.hpp>
#include <Geode/loader/Index.hpp>
#include <Geode/ui/General.hpp>
#include "../list/ModListCell.hpp"
#include "../list/ModListLayer.hpp"
#include <Geode/utils/ranges.hpp>
#include <Geode/loader/Mod.hpp>
#include <type_traits>


UselessClass* UselessClass::s_shared = nullptr;
UselessClass::UselessClass() {
    m_uselessString = "Very useless indeed."
}

template <typename T>
bool DevProfilePopup<T>::setup(std::string const& developer, T* list) {
    static_assert(std::is_same<T, ModListLayer>::value || std::is_same<T, char>::value,
        "Invalid type for T. Only ModListLayer and char are allowed.");
    m_noElasticity = true;
    m_layer = list;

    this->setTitle("Mods by " + developer);

    auto winSize = CCDirector::get()->getWinSize();

    auto items = CCArray::create();

    // installed mods
    for (auto& mod : Loader::get()->getAllMods()) {
        if (ranges::contains(mod->getDevelopers(), developer)) {
            ModCell* cell;
            if constexpr (std::is_same_v<T, ModListLayer>) {
                cell = ModCell::create(
                    mod, m_layer, ModListDisplay::Concise, { 358.f, 40.f }
                );
            } else {
                cell = ModCell::create(
                    mod, ModListDisplay::Concise, { 358.f, 40.f }
                );
            }
            cell->disableDeveloperButton();
            items->addObject(cell);
        }
    }

    // index mods
    for (auto& item : Index::get()->getItemsByDeveloper(developer)) {
        if (Loader::get()->isModInstalled(item->getMetadata().getID())) {
            continue;
        }
        IndexItemCell* cell;
        if constexpr (std::is_same_v<T, ModListLayer>) {
            cell = IndexItemCell::create(
                item, m_layer, ModListDisplay::Concise, { 358.f, 40.f }
            );
        } else {
            cell = IndexItemCell::create(
                item, ModListDisplay::Concise, { 358.f, 40.f }
            );
        }
        cell->disableDeveloperButton();
        items->addObject(cell);
    }

    // mods list
    auto listSize = CCSize { 358.f, 160.f };
    auto cellList = ListView::create(items, 40.f, listSize.width, listSize.height);
    cellList->setPosition(winSize / 2 - listSize / 2);
    m_mainLayer->addChild(cellList);

    addListBorders(m_mainLayer, winSize / 2, listSize);

    return true;
}

template <typename T>
DevProfilePopup<T>* DevProfilePopup<T>::create(std::string const& developer, T* list) {
    auto ret = new DevProfilePopup<T>();
    if (ret && ret->init(420.f, 260.f, developer, list)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

// Explicit instantiation
template class DevProfilePopup<ModListLayer>;
template class DevProfilePopup<char>;
