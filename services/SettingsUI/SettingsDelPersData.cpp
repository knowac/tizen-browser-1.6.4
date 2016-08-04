/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#include "SettingsDelPersData.h"

namespace tizen_browser{
namespace base_ui{

SettingsDelPersData::SettingsDelPersData(Evas_Object* parent)
{
    init(parent);
    updateButtonMap();
};

SettingsDelPersData::~SettingsDelPersData()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsDelPersData::updateButtonMap()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    // TODO Missing translations
    ItemData selectAll;
    selectAll.buttonText = Translations::SettingsDelPersDataSelectAll;
    selectAll.sui = this;
    selectAll.id = SELECT_ALL;

    ItemData browsHistory;
    browsHistory.buttonText = Translations::SettingsDelPersDataBrowsingHistory;
    browsHistory.sui = this;
    browsHistory.id = BROWSING_HISTORY;

    ItemData cache;
    cache.buttonText = Translations::SettingsDelPersDataCache;
    cache.sui = this;
    cache.id = CACHE;

    ItemData cookies;
    cookies.buttonText = Translations::SettingsDelPersDataCookies;
    cookies.sui = this;
    cookies.id = COOKIES_AND_SITE;

    ItemData pass;
    pass.buttonText = Translations::SettingsDelPersDataPasswords;
    pass.sui = this;
    pass.id = PASSWORDS;

    ItemData autofill;
    autofill.buttonText = Translations::SettingsDelPersDataAutoFillData;
    autofill.sui = this;
    autofill.id = DEL_PERS_AUTO_FILL;

    ItemData loc;
    loc.buttonText = Translations::SettingsDelPersDataLocationData;
    loc.sui = this;
    loc.id = LOCATION;

    m_buttonsMap[SettingsDelPersDataOptions::SELECT_ALL] = selectAll;
    m_buttonsMap[SettingsDelPersDataOptions::BROWSING_HISTORY] = browsHistory;
    m_buttonsMap[SettingsDelPersDataOptions::CACHE] = cache;
    m_buttonsMap[SettingsDelPersDataOptions::COOKIES_AND_SITE] = cookies;
    m_buttonsMap[SettingsDelPersDataOptions::PASSWORDS] = pass;
    m_buttonsMap[SettingsDelPersDataOptions::DEL_PERS_AUTO_FILL] = autofill;
    m_buttonsMap[SettingsDelPersDataOptions::LOCATION] = loc;
}

bool SettingsDelPersData::populateList(Evas_Object* genlist)
{
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", Translations::SettingsDelPersDataTitle.c_str());

    elm_object_signal_emit(m_actionBar, "show,buttons,signal", "but_vis");
    elm_object_signal_emit(m_actionBar, "hide,close,icon", "del_but");

    m_cancelButton = elm_button_add(m_actionBar);
    if (!m_cancelButton) {
        BROWSER_LOGE("Failed to create m_cancelButton");
        return false;
    }
    elm_object_style_set(m_cancelButton, "basic_button");
    evas_object_smart_callback_add(m_cancelButton, "clicked", __cancel_button_cb, this);
    elm_object_translatable_part_text_set(m_actionBar, "cancel_text", Translations::SettingsDelPersDataCancel.c_str());
    elm_object_part_content_set(m_actionBar, "cancel_button", m_cancelButton);

    m_deleteButton = elm_button_add(m_actionBar);
    if (!m_deleteButton) {
        BROWSER_LOGE("Failed to create m_doneButton");
        return false;
    }
    elm_object_style_set(m_deleteButton, "basic_button");
    evas_object_smart_callback_add(m_deleteButton, "clicked", __delete_button_cb, this);
    elm_object_part_content_set(m_actionBar, "done_button", m_deleteButton);
    elm_object_translatable_part_text_set(m_actionBar, "done_text", Translations::SettingsDelPersDataDelete.c_str());
    elm_object_signal_emit(m_actionBar, "dim,done,button,signal", "");

    appendGenlist(genlist, m_setting_check_normal_item_class, &m_buttonsMap[SettingsDelPersDataOptions::SELECT_ALL], nullptr);
    appendGenlist(genlist, m_setting_check_normal_item_class, &m_buttonsMap[SettingsDelPersDataOptions::BROWSING_HISTORY], nullptr);
    appendGenlist(genlist, m_setting_check_normal_item_class, &m_buttonsMap[SettingsDelPersDataOptions::CACHE], nullptr);
    appendGenlist(genlist, m_setting_check_normal_item_class, &m_buttonsMap[SettingsDelPersDataOptions::COOKIES_AND_SITE], nullptr);
    appendGenlist(genlist, m_setting_check_normal_item_class, &m_buttonsMap[SettingsDelPersDataOptions::PASSWORDS], nullptr);
    appendGenlist(genlist, m_setting_check_normal_item_class, &m_buttonsMap[SettingsDelPersDataOptions::DEL_PERS_AUTO_FILL], nullptr);
    appendGenlist(genlist, m_setting_check_normal_item_class, &m_buttonsMap[SettingsDelPersDataOptions::LOCATION], nullptr);
    return true;
}

void SettingsDelPersData::__cancel_button_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.closeSettingsUIClicked();
}

void SettingsDelPersData::__delete_button_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    auto self = static_cast<SettingsDelPersData*>(data);
    SPSC.deleteSelectedDataClicked(self->m_option);
    SPSC.closeSettingsUIClicked();
}

Evas_Object* SettingsDelPersData::createNormalCheckBox(Evas_Object* obj, ItemData* itd)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto check = elm_check_add(obj);
    elm_object_style_set(check, "default");
    evas_object_smart_callback_add(check, "changed", grid_item_check_changed, itd);
    if (static_cast<SettingsDelPersDataOptions>(itd->id) == BROWSING_HISTORY ||
        static_cast<SettingsDelPersDataOptions>(itd->id) == CACHE ||
        static_cast<SettingsDelPersDataOptions>(itd->id) == COOKIES_AND_SITE) {
        elm_check_state_set(check, EINA_TRUE);
        setOption(static_cast<SettingsDelPersDataOptions>(itd->id), true);
    } else {
        elm_check_state_set(check, EINA_FALSE);
        setOption(static_cast<SettingsDelPersDataOptions>(itd->id), false);
    }
    setCheckboxes(static_cast<SettingsDelPersDataOptions>(itd->id), check);
    return check;
}

void SettingsDelPersData::grid_item_check_changed(void* data, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    auto itd = static_cast<ItemData*>(data);
    auto value = elm_check_state_get(obj);
    auto self = itd->sui;
    if (static_cast<SettingsDelPersDataOptions>(itd->id) == SELECT_ALL) {
        for (auto& it : self->getCheckboxes()) {
            elm_check_state_set(it.second, value);
            itd->sui->setOption(it.first, static_cast<bool>(value));
        }
    } else {
        elm_check_state_set(obj, value);
        self->setOption(
            static_cast<SettingsDelPersDataOptions>(itd->id),
            static_cast<bool>(value));
    }
    bool val = self->getOption(SELECT_ALL);
    if (self->getOption(LOCATION) != val &&
        self->getOption(BROWSING_HISTORY) != val &&
        self->getOption(CACHE) != val &&
        self->getOption(COOKIES_AND_SITE) != val &&
        self->getOption(PASSWORDS) != val &&
        self->getOption(DEL_PERS_AUTO_FILL) != val) {
        elm_check_state_set(self->getCheckboxes()[SELECT_ALL], !val);
        self->setOption(SELECT_ALL, !val);
    }
}

}
}
