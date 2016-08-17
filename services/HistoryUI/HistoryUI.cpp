/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <boost/concept_check.hpp>
#include <vector>
#include <string>
#include <string.h>
#include <AbstractMainWindow.h>

#include "app_i18n.h"
#include "HistoryUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "Tools/GeneralTools.h"
#include "HistoryDaysListManager/HistoryDaysListManagerMob.h"
#include "services/HistoryService/HistoryItem.h"
#include "HistoryDeleteManager.h"

namespace tizen_browser{
namespace base_ui{

using namespace services;
EXPORT_SERVICE(HistoryUI, "org.tizen.browser.historyui")

using HistoryItemData = struct _HistoryItemData
{
    std::shared_ptr<tizen_browser::services::HistoryItem> item;
    std::shared_ptr<tizen_browser::base_ui::HistoryUI> historyUI;
};

struct ItemData{
    tizen_browser::base_ui::HistoryUI* historyUI;
    Elm_Object_Item * e_item;
};

HistoryUI::HistoryUI()
    : m_parent(nullptr)
    , m_main_layout(nullptr)
    , m_buttonClose(nullptr)
    , m_buttonClear(nullptr)
    , m_daysList(nullptr)
    , m_historyDeleteManager(std::make_shared<HistoryDeleteManager>())
    , m_naviframe(nullptr)
    , m_modulesToolbar(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("HistoryUI/History.edj");

    m_historyDaysListManager = std::make_shared<HistoryDaysListManagerMob>();

    m_historyDaysListManager->signalHistoryItemClicked.connect(signalHistoryItemClicked);
    m_historyDaysListManager->signalDeleteHistoryItems.connect(signalDeleteHistoryItems);
}

HistoryUI::~HistoryUI()
{
}

void HistoryUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_main_layout);
    m_naviframe->show();
    evas_object_show(m_main_layout);
}

void HistoryUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_main_layout);
    evas_object_hide(m_main_layout);
    clearItems();
    m_historyDeleteManager->setDeleteMode(false);
    m_naviframe->hide();
}

void HistoryUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* HistoryUI::getContent()
{
    M_ASSERT(m_parent);
    if (!m_naviframe)
        createHistoryUILayout();
    return m_naviframe->getLayout();
}

void HistoryUI::createHistoryUILayout()
{
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_naviframe = std::make_shared<NaviframeWrapper>(m_parent);

    m_main_layout = elm_layout_add(m_naviframe->getLayout());
    m_naviframe->setContent(m_main_layout);

    elm_layout_file_set(m_main_layout, m_edjFilePath.c_str(), "history-layout");
    evas_object_size_hint_weight_set(m_main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_main_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_daysList = createDaysList(m_main_layout);
    clearItems();
    createTopContent();

    elm_object_signal_emit(m_naviframe->getLayout(), "show_toolbars", "ui");
}

void HistoryUI::createTopContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_naviframe->getLayout());

    m_naviframe->addPrevButton(_close_clicked_cb, this);
    m_naviframe->setPrevButtonVisible(true);
    m_naviframe->setTitle(_("IDS_BR_TAB2_HISTORY"));
}

void HistoryUI::createModulesToolbar()
{
    m_modulesToolbar = elm_toolbar_add(m_naviframe->getLayout());

    elm_object_style_set(m_modulesToolbar, "tabbar/notitle");
    elm_toolbar_shrink_mode_set(m_modulesToolbar, ELM_TOOLBAR_SHRINK_EXPAND);
    elm_toolbar_select_mode_set(m_modulesToolbar, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_toolbar_transverse_expanded_set(m_modulesToolbar, EINA_TRUE);
    elm_object_part_content_set(m_naviframe->getLayout(), "action_bar_history", m_modulesToolbar);
    evas_object_show(m_modulesToolbar);

    elm_toolbar_item_append(m_modulesToolbar, nullptr, _("IDS_BR_BODY_BOOKMARKS"), nullptr, this);
    elm_toolbar_item_append(m_modulesToolbar, nullptr, _("IDS_BR_MBODY_HISTORY"), nullptr, this);
}

HistoryItemVectorMap
HistoryUI::groupItemsByDomain(const HistoryItemVector& historyItems)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    HistoryItemVectorMap groupedMap;
    for(auto& item : historyItems) {
        auto domain = tools::extractDomain(item->getUrl());
        if(groupedMap.find(domain) == groupedMap.end()) {
            groupedMap.insert(std::pair<std::string, HistoryItemVector>(domain, {}));
        }
        groupedMap.find(domain)->second.push_back(item);
    }
    return groupedMap;
}

Evas_Object* HistoryUI::createDaysList(Evas_Object* parent)
{
    M_ASSERT(history_layout);

    auto list = m_historyDaysListManager->createDaysList(parent);

    evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);

    return list;
}

void HistoryUI::_close_clicked_cb(void * data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
        return;
    }
    auto self = static_cast<HistoryUI*>(data);
    self->closeHistoryUIClicked();
}

void HistoryUI::_clearHistory_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
        return;
    }
    auto self = static_cast<HistoryUI*>(data);
    self->clearItems();
    self->clearHistoryClicked();
}

void HistoryUI::addHistoryItems(
    std::shared_ptr<HistoryItemVector> items,
    HistoryPeriod period)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (items->size() == 0)
        return;
    m_historyDaysListManager->addHistoryItems(items, period);
}

void HistoryUI::removeHistoryItem(const std::string& uri)
{
    BROWSER_LOGD("[%s] uri=%s", __func__, uri.c_str());
}

void HistoryUI::clearItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_historyDaysListManager->clear();
}

void HistoryUI::showContextMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto window = getWindow();
    if (window) {
        createContextMenu(*window);
        elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_OPT_REMOVE"), nullptr, _cm_delete_clicked, this);
        alignContextMenu(*window);
    } else
        BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
}

void HistoryUI::_cm_delete_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
        return;
    }
    auto self = static_cast<HistoryUI*>(data);
    _cm_dismissed(nullptr, self->m_ctxpopup, nullptr);
}

}
}
