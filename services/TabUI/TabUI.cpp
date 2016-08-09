/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Elementary.h>
#include <vector>
#include <string.h>
#include <AbstractMainWindow.h>

#include "app_i18n.h"
#include "TabUI.h"
#include "TabId.h"
#include "BrowserLogger.h"
#include "BrowserImage.h"
#include "Config/Config.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(TabUI, "org.tizen.browser.tabui")

TabUI::TabUI()
    : m_parent(nullptr)
    , m_content(nullptr)
    , m_gengrid(nullptr)
    , m_empty_layout(nullptr)
    , m_itemToShow(nullptr)
    , m_item_class(nullptr)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("TabUI/TabUI.edj");
    createTabItemClass();
}

TabUI::~TabUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_gengrid_item_class_free(m_item_class);
}

void TabUI::createTabItemClass()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_item_class) {
        m_item_class = elm_gengrid_item_class_new();
        m_item_class->item_style = "custom_tab_item";
        m_item_class->func.text_get = _gengrid_text_get;
        m_item_class->func.content_get =  _gengrid_content_get;
        m_item_class->func.state_get = nullptr;
        m_item_class->func.del = nullptr;
    }
}

void TabUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_naviframe->getLayout());
    m_naviframe->show();
    orientationChanged();
}

void TabUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_naviframe->getLayout());
    elm_gengrid_clear(m_gengrid);
    m_naviframe->hide();
}

void TabUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* TabUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_naviframe)
        createTabUILayout();
    return m_naviframe->getLayout();
}

void TabUI::createTabUILayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());

    m_naviframe = std::make_shared<NaviframeWrapper>(m_parent);

    createTopContent();

    m_content = elm_layout_add(m_naviframe->getLayout());
    evas_object_size_hint_weight_set(m_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_content, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_content);
    elm_layout_file_set(m_content, m_edjFilePath.c_str(), "naviframe_content");
    m_naviframe->setContent(m_content);

    createGengrid();
    createBottomContent();
    createEmptyLayout();
}

void TabUI::createTopContent()
{
    m_naviframe->setTitle("Tabs"); //TODO: _("IDS_BR_HEADER_TABS_ABB2") when it works
    m_naviframe->addPrevButton(_close_clicked, this);
    m_naviframe->setVisibilityPrevButton(true);
}

void TabUI::createBottomContent()
{
    Evas_Object *layout = elm_layout_add(m_naviframe->getLayout());
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "bottom_bar_with_margins");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(layout);

    m_naviframe->createBottomBar(layout);
    m_naviframe->setVisibilityBottomBar(true);
    //TODO: Missing translation
    m_naviframe->addButtonToBottomBar("Enable Secret", _enable_secret_clicked, this);
    m_naviframe->setEnableButtonInBottomBar("Enable Secret", true);
    //TODO: _("IDS_BR_OPT_NEW_TAB") when it works
    m_naviframe->addButtonToBottomBar("New tab", _new_tab_clicked, this);
    m_naviframe->setEnableButtonInBottomBar("New tab", true);
}

void TabUI::createEmptyLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_empty_layout = elm_layout_add(m_content);
    elm_layout_theme_set(m_empty_layout, "layout", "nocontents", "default");

    evas_object_size_hint_weight_set(m_empty_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_empty_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    //TODO: Add translations
    elm_object_translatable_part_text_set(m_empty_layout, "elm.text", "No tabs");
    //TODO: _("IDS_BR_BODY_AFTER_YOU_VIEW_WEBPAGES_THEY_WILL_BE_SHOWN_HERE") when it works
    elm_object_translatable_part_text_set(m_empty_layout, "elm.help.text",
        "After you view webpages, they will be shown here.");

    elm_layout_signal_emit(m_empty_layout, "text,disabled", "");
    elm_layout_signal_emit(m_empty_layout, "align.center", "elm");

    elm_object_part_content_set(m_content, "elm.swallow.content_overlay", m_empty_layout);
}

void TabUI::createGengrid()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);

    m_gengrid = elm_gengrid_add(m_content);
    evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_gengrid_horizontal_set(m_gengrid, EINA_FALSE);
    elm_scroller_page_size_set(m_gengrid, ELM_SCALE_SIZE(720), ELM_SCALE_SIZE(0));
    elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(GENGRID_ITEM_WIDTH),
        ELM_SCALE_SIZE(GENGRID_ITEM_HEIGHT));
    elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_bounce_set(m_gengrid, EINA_FALSE, EINA_FALSE);
    elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
    elm_gengrid_highlight_mode_set(m_gengrid, EINA_TRUE);
    elm_gengrid_align_set(m_gengrid, 0.5, 0.0);
    evas_object_smart_callback_add(m_gengrid, "pressed", _gengrid_tab_pressed, this);
    evas_object_smart_callback_add(m_gengrid, "released", _gengrid_tab_released, this);

    elm_object_part_content_set(m_content, "elm.swallow.content", m_gengrid);
    evas_object_show(m_gengrid);
}

void TabUI::_close_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI * tabUI = static_cast<TabUI*>(data);
        tabUI->closeTabUIClicked();
    } else {
        BROWSER_LOGW("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
    }
}

void TabUI::orientationChanged()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<bool> landscape = isLandscape();
    if (landscape) {
        if (*landscape) {
            elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(GENGRID_ITEM_WIDTH_LANDSCAPE), ELM_SCALE_SIZE(GENGRID_ITEM_HEIGHT_LANDSCAPE));
            elm_object_signal_emit(m_content, "switch_landscape", "ui");
        } else {
            elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(GENGRID_ITEM_WIDTH), ELM_SCALE_SIZE(GENGRID_ITEM_HEIGHT));
            elm_object_signal_emit(m_content, "switch_vertical", "ui");
        }
    } else {
        BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
    }
}

void TabUI::showContextMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    boost::optional<Evas_Object*> window = getWindow();
    if (window) {
        createContextMenu(*window);

        elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_BODY_SYNC_OPEN_PAGES"), nullptr, _cm_sync_clicked, this);
        elm_ctxpopup_item_append(m_ctxpopup, "Secret mode security", nullptr, _cm_secret_clicked, this);
        if (elm_gengrid_items_count(m_gengrid) != 0)
            elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_OPT_CLOSE_ALL"), nullptr, _cm_close_clicked, this);
        alignContextMenu(*window);
    } else {
        BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
    }
}

void TabUI::_cm_sync_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI* tabUI = static_cast<TabUI*>(data);
        _cm_dismissed(nullptr, tabUI->m_ctxpopup, nullptr);
    } else {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
    }
}

void TabUI::_cm_secret_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI* tabUI = static_cast<TabUI*>(data);
        _cm_dismissed(nullptr, tabUI->m_ctxpopup, nullptr);
    } else {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
    }
}

void TabUI::_cm_close_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI* tabUI = static_cast<TabUI*>(data);
        _cm_dismissed(nullptr, tabUI->m_ctxpopup, nullptr);
        Elm_Object_Item* it = elm_gengrid_first_item_get(tabUI->m_gengrid);
        while (it) {
            TabData *item = (TabData *)elm_object_item_data_get(it);
            tabUI->closeTabsClicked(item->item->getId());
            it = elm_gengrid_item_next_get(it);
        }
        //Todo: set empty state
    } else {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
    }
}

void TabUI::_new_tab_clicked(void * data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI* tabUI = static_cast<TabUI*>(data);
        tabUI->newTabClicked();
    } else {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
    }
}

void TabUI::_enable_secret_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
    } else {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
    }
}

void TabUI::_close_all_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI* tabUI = static_cast<TabUI*>(data);
        tabUI->closeAllTabs();
        elm_gengrid_realized_items_update(tabUI->m_gengrid);
    } else {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
    }
}

void TabUI::addTabItem(basic_webengine::TabContentPtr hi)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    TabData *itemData = new TabData();
    itemData->item = hi;
    itemData->tabUI.reset(this);
    Elm_Object_Item* tab = elm_gengrid_item_append(m_gengrid, m_item_class, itemData,
        nullptr, nullptr);
    // Check if item_object was created successfully
    if (tab)
        elm_gengrid_item_selected_set(tab, EINA_FALSE);
    else
        BROWSER_LOGW("GengridItem wasn't created successfully");
}

void TabUI::addTabItems(std::vector<basic_webengine::TabContentPtr>& items)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    for (auto it = items.begin(); it < items.end(); ++it)
        addTabItem(*it);
    updateNoTabsText();
}

char* TabUI::_gengrid_text_get(void *data, Evas_Object*, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && part) {
        TabData* itemData = static_cast<TabData*>(data);
        if (!strcmp(part, "elm.text"))
            return strdup(itemData->item->getTitle().c_str());
    } else {
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    }
    return nullptr;
}

Evas_Object * TabUI::_gengrid_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("[%s:%d] part=%s", __PRETTY_FUNCTION__, __LINE__, part);
    if (data && obj && part) {
        TabData *itemData = static_cast<TabData*>(data);
        if (!strcmp(part, "elm.icon")) {
            auto layout = elm_layout_add(obj);
            elm_layout_file_set(layout, itemData->tabUI->m_edjFilePath.c_str(), "favicon_image");
            return layout;
        }
        if (!strcmp(part, "elm.button")) {
            auto button = elm_button_add(obj);
            elm_object_style_set(button, "delete_button");
            evas_object_smart_callback_add(button, "clicked", _close_tab_clicked, data);
            return button;
        }
        if (!strcmp(part, "elm.thumbnail")) {
            auto thumbnail = itemData->item->getThumbnail();
            if (thumbnail)
                return thumbnail->getEvasImage(obj);
        }
        if (!strcmp(part, "elm.overlay")) {
            auto button = elm_button_add(obj);
            elm_object_style_set(button, "invisible_button");
            evas_object_smart_callback_add(button, "clicked", _gengrid_tab_clicked, data);
            return button;
        }
    } else {
        BROWSER_LOGE("[%s:%d] Data or obj or part is null", __PRETTY_FUNCTION__, __LINE__);
    }
    return nullptr;
}

void TabUI::_gengrid_tab_pressed(void *, Evas_Object *, void *event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (event_info) {
        auto object_item = static_cast<Elm_Object_Item*>(event_info);
        elm_object_signal_emit(
            elm_object_item_part_content_get(object_item, "elm.icon"),
            "on_mouse_down", "ui");
    } else {
        BROWSER_LOGW("[%s] event_info = nullptr", __PRETTY_FUNCTION__);
    }
}

void TabUI::_gengrid_tab_released(void *, Evas_Object *, void *event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (event_info) {
        auto object_item = static_cast<Elm_Object_Item*>(event_info);
        elm_object_signal_emit(
            elm_object_item_part_content_get(object_item, "elm.icon"),
            "on_mouse_up", "ui");
    } else {
        BROWSER_LOGW("[%s] event_info = nullptr", __PRETTY_FUNCTION__);
    }
}

void TabUI::_gengrid_tab_clicked(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabData *itemData = static_cast<TabData*>(data);
        itemData->tabUI->tabClicked(itemData->item->getId());
    } else {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
    }
}

void TabUI::_close_tab_clicked(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabData* itemData = static_cast<TabData*>(data);

        Elm_Object_Item* it = elm_gengrid_selected_item_get(itemData->tabUI->m_gengrid);
        auto prev = elm_gengrid_item_prev_get(it);
        auto next = elm_gengrid_item_next_get(it);
        elm_object_item_del(it);

        if (prev || next) {
            itemData->tabUI->m_itemToShow = prev ? prev : next;
            elm_gengrid_item_selected_set(itemData->tabUI->m_itemToShow, EINA_TRUE);
            elm_gengrid_item_show(itemData->tabUI->m_itemToShow, ELM_GENGRID_ITEM_SCROLLTO_TOP);
            itemData->tabUI->m_itemToShow = nullptr;
        }

        elm_gengrid_realized_items_update(itemData->tabUI->m_gengrid);

        itemData->tabUI->closeTabsClicked(itemData->item->getId());
        itemData->tabUI->updateNoTabsText();
    } else {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
    }
}

void TabUI::closeAllTabs()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* it = elm_gengrid_first_item_get(m_gengrid);
    while (it) {
        TabData *item = (TabData *)elm_object_item_data_get(it);
        item->tabUI->closeTabsClicked(item->item->getId());
        it = elm_gengrid_item_next_get(it);
    }
    closeTabUIClicked();
}

void TabUI::updateNoTabsText()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (elm_gengrid_items_count(m_gengrid)) {
        evas_object_hide(m_empty_layout);
        elm_object_signal_emit(m_content, "hide_overlay", "ui");
    } else {
        evas_object_show(m_empty_layout);
        elm_object_signal_emit(m_content, "show_overlay", "ui");
    }
}


}
}
