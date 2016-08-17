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

#include <services/HistoryUI/HistoryUI.h>
#include <services/HistoryService/HistoryItem.h>
#include "BrowserLogger.h"
#include "HistoryDaysListManagerMob.h"
#include "HistoryDayItemData.h"
#include "mob/HistoryDayItemMob.h"
#include "mob/WebsiteHistoryItem/WebsiteHistoryItemMob.h"
#include "mob/WebsiteHistoryItem/WebsiteHistoryItemTitleMob.h"
#include "mob/WebsiteHistoryItem/WebsiteHistoryItemVisitItemsMob.h"
#include "app_i18n.h"
#include <services/HistoryUI/HistoryDeleteManager.h>


#include <GeneralTools.h>
#include <EflTools.h>

namespace tizen_browser {
namespace base_ui {

HistoryDaysListManagerMob::HistoryDaysListManagerMob()
    : m_edjeFiles(std::make_shared<HistoryDaysListManagerEdje>())
    , m_parent(nullptr)
    , m_scrollerDays(nullptr)
    , m_layoutScrollerDays(nullptr)
    , m_boxDays(nullptr)
    , m_history_day_item_class(elm_genlist_item_class_new())
    , m_history_item_item_class(elm_genlist_item_class_new())
    , m_history_download_item_class(elm_genlist_item_class_new())
{
    createGenlistItemClasses();
    connectSignals();
}

HistoryDaysListManagerMob::~HistoryDaysListManagerMob()
{
    for (auto& dayItem : m_dayItems)
        dayItem->setEflObjectsAsDeleted();

    if (m_history_day_item_class)
        elm_genlist_item_class_free(m_history_day_item_class);

    if (m_history_item_item_class)
        elm_genlist_item_class_free(m_history_item_item_class);
}

void HistoryDaysListManagerMob::createGenlistItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_history_day_item_class->item_style = "group_index";
    m_history_day_item_class->func.text_get = _genlist_history_day_text_get;
    m_history_day_item_class->func.content_get =  _genlist_history_day_content_get;
    m_history_day_item_class->func.state_get = nullptr;
    m_history_day_item_class->func.del = nullptr;
    m_history_day_item_class->decorate_all_item_style = "edit_default";

    m_history_download_item_class->item_style = "type1";
    m_history_download_item_class->func.text_get = _genlist_history_download_text_get;
    m_history_download_item_class->func.content_get =  nullptr;
    m_history_download_item_class->func.state_get = nullptr;
    m_history_download_item_class->func.del = nullptr;
    m_history_download_item_class->decorate_all_item_style = "edit_default";

    m_history_item_item_class->item_style = "type1";
    m_history_item_item_class->func.text_get = _genlist_history_item_text_get;
    m_history_item_item_class->func.content_get =  _genlist_history_item_content_get;
    m_history_item_item_class->func.state_get = nullptr;
    m_history_item_item_class->func.del = nullptr;
    m_history_item_item_class->decorate_all_item_style = "edit_default";
}

char* HistoryDaysListManagerMob::_genlist_history_day_text_get(void* data, Evas_Object *, const char *part)
{
    if (data && part) {
        auto item = static_cast<HistoryDayItemData*>(data);
        if (!strcmp(part, "elm.text"))
            return strdup(item->day.c_str());
    }
    return nullptr;
}

char* HistoryDaysListManagerMob::_genlist_history_download_text_get(void* data, Evas_Object*, const char *part)
{
    if (data && part && !strcmp(part, "elm.text")) {
        auto str = static_cast<const char*>(data);
        return strdup(str);
    }
    return nullptr;
}

char* HistoryDaysListManagerMob::_genlist_history_item_text_get(void* data, Evas_Object *, const char *part)
{
    if (data && part) {
        auto item = static_cast<WebsiteHistoryItemData*>(data);
        if (!strcmp(part, "elm.text"))
            return strdup(item->websiteVisitItem->historyItem->getTitle().c_str());
        if (!strcmp(part, "elm.text.sub"))
            return strdup(item->websiteVisitItem->historyItem->getUrl().c_str());
    }
    return nullptr;
}

Evas_Object* HistoryDaysListManagerMob::_genlist_history_day_content_get(void* data, Evas_Object* obj, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && part && !strcmp(part, "elm.swallow.end")) {
        auto dayData = static_cast<HistoryDayItemData*>(data);

        auto arrow_layout = elm_layout_add(obj);
        auto edjeDir = EDJE_DIR + std::string("HistoryUI/HistoryDaysList.edj");
        elm_layout_file_set(arrow_layout, edjeDir.c_str(), "arrow-layout");

        auto edje = elm_layout_edje_get(arrow_layout);
        edje_object_signal_emit(edje, "state,contracted,signal", "");
        if(dayData->expanded)
            edje_object_signal_emit(edje, "state,expanded,signal", "");

        return arrow_layout;
    }
    return nullptr;
}

Evas_Object* HistoryDaysListManagerMob::_genlist_history_item_content_get(void *data, Evas_Object* obj, const char *part)
{
    if (data && !strcmp(part, "elm.swallow.icon")) {
        auto item = static_cast<WebsiteHistoryItemData*>(data);
        auto icon = item->favIcon->getEvasImage(obj);
        elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
        evas_object_size_hint_min_set(icon,
            ELM_SCALE_SIZE(64),
            ELM_SCALE_SIZE(64));
        evas_object_size_hint_max_set(icon,
            ELM_SCALE_SIZE(64),
            ELM_SCALE_SIZE(64));
        return icon;
    } else if (!data && !strcmp(part, "elm.swallow.icon")) {
        auto icon = elm_icon_add(obj);
        elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
        evas_object_size_hint_min_set(icon,
            ELM_SCALE_SIZE(64),
            ELM_SCALE_SIZE(64));
        evas_object_size_hint_max_set(icon,
            ELM_SCALE_SIZE(64),
            ELM_SCALE_SIZE(64));
        return icon;
    }
    return nullptr;
}

Evas_Object* HistoryDaysListManagerMob::createDaysList(
    Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_parent = parent;
    m_genlist = elm_genlist_add(m_parent);
    tools::EflTools::setExpandHints(m_genlist);
    elm_scroller_policy_set(m_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_genlist_multi_select_set(m_genlist, EINA_FALSE);
    elm_genlist_select_mode_set(m_genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genlist, ELM_LIST_COMPRESS);
    evas_object_smart_callback_add(m_genlist, "expanded", _tree_item_expanded, this);
    evas_object_smart_callback_add(m_genlist, "contracted", _tree_item_contracted, this);
    evas_object_smart_callback_add(m_genlist, "pressed", _tree_item_pressed, this);

    // TODO Download history is planed for the 2nd phase
    auto el = elm_genlist_item_append(
        m_genlist, m_history_download_item_class,
        _("IDS_BR_BODY_DOWNLOAD_HISTORY"),
        nullptr, ELM_GENLIST_ITEM_NONE,
        nullptr, nullptr);
    elm_object_item_disabled_set(el, EINA_TRUE);

    evas_object_show(m_genlist);

    return m_genlist;
}

void HistoryDaysListManagerMob::addHistoryItems(
    const std::shared_ptr<services::HistoryItemVector>& items,
    HistoryPeriod period)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::vector<WebsiteHistoryItemDataPtr> historyItems;
    for (auto& item : *items) {
        auto pageViewItem = std::make_shared<WebsiteVisitItemData>(item);
        std::shared_ptr<tools::BrowserImage> websiteFavicon = item->getFavIcon();
        if (!websiteFavicon || websiteFavicon->getSize() == 0)
            websiteFavicon = nullptr;
        historyItems.push_back(
            std::make_shared<WebsiteHistoryItemData>(
                item->getTitle(),
                item->getUrl(),
                websiteFavicon,
                pageViewItem));
    }
    auto dayItem(std::make_shared<HistoryDayItemData>(toString(period), historyItems));
    appendDayItem(dayItem);
    showNoHistoryMessage(isHistoryDayListEmpty());
}

void HistoryDaysListManagerMob::clear()
{
    elm_box_clear(m_boxDays);
    m_dayItems.clear();
    elm_genlist_clear(m_genlist);
    showNoHistoryMessage(isHistoryDayListEmpty());
}

HistoryDayItemMobPtr HistoryDaysListManagerMob::getItem(
    HistoryDayItemDataPtrConst historyDayItemData)
{
    for (auto& historyDayItem : m_dayItems) {
        if (historyDayItem->getData() == historyDayItemData)
            return historyDayItem;
    }
    return nullptr;
}

void HistoryDaysListManagerMob::connectSignals()
{
    HistoryDayItemMob::signaButtonClicked.connect(
        boost::bind(&HistoryDaysListManagerMob::onHistoryDayItemButtonClicked,
            this, _1, _2));
    WebsiteHistoryItemTitleMob::signalButtonClicked.connect(
        boost::bind(&HistoryDaysListManagerMob::onWebsiteHistoryItemClicked,
            this, _1, _2));
    WebsiteHistoryItemVisitItemsMob::signalButtonClicked.connect(
        boost::bind(
            &HistoryDaysListManagerMob::onWebsiteHistoryItemVisitItemClicked,
            this, _1, _2));
}

void HistoryDaysListManagerMob::_tree_item_expanded(void* data, Evas_Object* genlist, void * event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!(data && event_info)) {
        BROWSER_LOGD("[%s:%d] data is null", __PRETTY_FUNCTION__, __LINE__);
        return;
    }
    auto it = static_cast<Elm_Object_Item*>(event_info);
    auto self = static_cast<HistoryDaysListManagerMob*>(data);

    for (auto& el : self->m_itemData[it]->websiteHistoryItems) {
        auto itData = new ItemData;
        itData->historyDaysListManager = self;
        itData->websiteVisitItem = el->websiteVisitItem;
        elm_genlist_item_append(
            genlist,
            self->m_history_item_item_class,
            el.get(),
            it,
            ELM_GENLIST_ITEM_NONE,
            _item_selected,
            itData);
    }
    self->m_itemData[it]->expanded = true;
    auto arrow_layout = elm_object_item_part_content_get(it, "elm.swallow.end");
    auto edje = elm_layout_edje_get(arrow_layout);
    edje_object_signal_emit(edje, "state,expanded,signal", "");
    elm_genlist_realized_items_update(genlist);
}

void HistoryDaysListManagerMob::_item_selected(void* data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto iD = static_cast<ItemData*>(data);
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, iD->websiteVisitItem->historyItem->getUrl().c_str());
    iD->historyDaysListManager->signalHistoryItemClicked(
        iD->websiteVisitItem->historyItem->getUrl().c_str(),
        iD->websiteVisitItem->historyItem->getTitle().c_str());
    delete iD;
}

void HistoryDaysListManagerMob::_tree_item_pressed(void* data, Evas_Object*, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto it = static_cast<Elm_Object_Item*>(event_info);
    auto self = static_cast<HistoryDaysListManagerMob*>(data);

    if (self->m_itemState[it] == EINA_FALSE) {
        elm_genlist_item_expanded_set(it, EINA_TRUE);
        self->m_itemState[it] = EINA_TRUE;
    } else if (self->m_itemState[it] == EINA_TRUE) {
        elm_genlist_item_expanded_set(it, EINA_FALSE);
        self->m_itemState[it] = EINA_FALSE;
    }
    return;
}

void HistoryDaysListManagerMob::_tree_item_contracted(void* data, Evas_Object*, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto it = static_cast<Elm_Object_Item*>(event_info);
    auto self = static_cast<HistoryDaysListManagerMob*>(data);
    elm_genlist_item_subitems_clear(it);
    self->m_itemData[it]->expanded = false;

    auto arrow_layout = elm_object_item_part_content_get(it, "elm.swallow.end");
    auto edje = elm_layout_edje_get(arrow_layout);
    edje_object_signal_emit(edje, "state,contracted,signal", "");
}

void HistoryDaysListManagerMob::appendDayItem(HistoryDayItemDataPtr dayItemData)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto item = std::make_shared<HistoryDayItemMob>(dayItemData);
    m_dayItems.push_back(item);
    dayItemData->expanded = false;
    auto el = elm_genlist_item_append(
        m_genlist, m_history_day_item_class,
        dayItemData.get(),
        nullptr, ELM_GENLIST_ITEM_TREE,
        nullptr, nullptr);

    m_itemData[el] = dayItemData;
    m_itemState[el] = EINA_FALSE;
}

void HistoryDaysListManagerMob::showNoHistoryMessage(bool show)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (show)
        elm_object_signal_emit(m_layoutScrollerDays, "show_empty_message", "ui");
    else
        elm_object_signal_emit(m_layoutScrollerDays, "hide_empty_message", "ui");
}

void HistoryDaysListManagerMob::onHistoryDayItemButtonClicked(
    const HistoryDayItemDataPtrConst clickedItem, bool remove)
{
    if (remove)
        removeItem(clickedItem);
}

void HistoryDaysListManagerMob::onWebsiteHistoryItemClicked(
    const WebsiteHistoryItemDataPtrConst clickedItem, bool remove)
{
    if (remove)
        removeItem(clickedItem);
    else
        signalHistoryItemClicked(
            tools::PROTOCOL_DEFAULT + clickedItem->websiteDomain,
            clickedItem->websiteTitle);
}

void HistoryDaysListManagerMob::onWebsiteHistoryItemVisitItemClicked(
    const WebsiteVisitItemDataPtrConst clickedItem, bool remove)
{
    if (remove) {
        removeItem(clickedItem);
        signalDeleteHistoryItems(
            std::make_shared<std::vector<int>>(std::initializer_list<int> {
            clickedItem->historyItem->getId() }));
    } else
        signalHistoryItemClicked(clickedItem->historyItem->getUrl(),
            clickedItem->historyItem->getTitle());
}

void HistoryDaysListManagerMob::removeItem(
    HistoryDayItemDataPtrConst historyDayItemData)
{
    if (!historyDayItemData) {
        BROWSER_LOGE("%s remove error", __PRETTY_FUNCTION__);
        return;
    }
    auto item = getItem(historyDayItemData);
    if (!item)
        return;
    // remove day item from vector, destructor will clear efl objects
    remove(item);
    elm_box_unpack(m_boxDays, item->getLayoutMain());
    showNoHistoryMessage(isHistoryDayListEmpty());
}

void HistoryDaysListManagerMob::removeItem(
    WebsiteHistoryItemDataPtrConst websiteHistoryItemData)
{
    if (!websiteHistoryItemData) {
        BROWSER_LOGE("%s remove error", __PRETTY_FUNCTION__);
        return;
    }
    for (auto& dayItem : m_dayItems) {
        auto websiteHistoryItem = dayItem->getItem(websiteHistoryItemData);
        if (websiteHistoryItem) {
            signalDeleteHistoryItems(websiteHistoryItem->getVisitItemsIds());
            dayItem->removeItem(websiteHistoryItemData);
            return;
        }
    }
}

void HistoryDaysListManagerMob::removeItem(
    WebsiteVisitItemDataPtrConst websiteVisitItemData)
{
    if (!websiteVisitItemData) {
        BROWSER_LOGE("%s remove error", __PRETTY_FUNCTION__);
        return;
    }
    for (auto& dayItem : m_dayItems) {
        if (dayItem->getItem(websiteVisitItemData)) {
            dayItem->removeItem(websiteVisitItemData);
            return;
        }
    }
}

void HistoryDaysListManagerMob::remove(HistoryDayItemMobPtr historyDayItem)
{
    for (auto it = m_dayItems.begin(); it != m_dayItems.end();) {
        if ((*it) == historyDayItem) {
            m_dayItems.erase(it);
            return;
        } else {
            ++it;
        }
    }
}

} /* namespace base_ui */
} /* namespace tizen_browser */
