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

#include "SettingsHomePage.h"

namespace tizen_browser{
namespace base_ui{

const std::string SettingsHomePage::DEF_HOME_PAGE = "http://www.samsung.com";

SettingsHomePage::SettingsHomePage(Evas_Object* parent)
{
    init(parent);
};

SettingsHomePage::~SettingsHomePage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

std::string SettingsHomePage::getCurrentPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_current = SettingsPrettySignalConnector::Instance().requestCurrentPage();
    BROWSER_LOGD("[%s:%s] ", __PRETTY_FUNCTION__, (*m_current).c_str());
    if(m_current && !(*m_current).empty())
        return *m_current;
    return SettingsHomePage::DEF_HOME_PAGE;
}

void SettingsHomePage::updateButtonMap()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    // TODO Missing translations
    ItemData defaultPage;
    defaultPage.buttonText = "Default";
    defaultPage.subText = DEF_HOME_PAGE;
    defaultPage.sui = this;
    defaultPage.id = DEFAULT;

    ItemData current;
    current.buttonText = "Current page";
    current.subText = getCurrentPage();
    current.sui = this;
    current.id = CURRENT;

    ItemData quick;
    quick.buttonText = "Quick access";
    quick.sui = this;
    quick.id = QUICK_ACCESS;

    ItemData most;
    most.buttonText = "Most visited websites";
    most.sui = this;
    most.id = MOST_VIS;

    ItemData other;
    other.buttonText = "Other";
    other.subText = "http://wwww.samsung.com";
    other.sui = this;
    other.id = OTHER;

    m_buttonsMap[SettingsHomePageOptions::DEFAULT] = defaultPage;
    m_buttonsMap[SettingsHomePageOptions::CURRENT] = current;
    m_buttonsMap[SettingsHomePageOptions::QUICK_ACCESS] = quick;
    m_buttonsMap[SettingsHomePageOptions::MOST_VIS] = most;
    m_buttonsMap[SettingsHomePageOptions::OTHER] = other;
}

bool SettingsHomePage::populateList(Evas_Object* genlist)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", "Home page");
    updateButtonMap();
    m_itemsMap[SettingsHomePageOptions::DEFAULT] =
        appendGenlist(genlist, m_setting_check_radio_item_class, &m_buttonsMap[SettingsHomePageOptions::DEFAULT], _default_cb);
    m_itemsMap[SettingsHomePageOptions::CURRENT] =
        appendGenlist(genlist, m_setting_check_radio_item_class, &m_buttonsMap[SettingsHomePageOptions::CURRENT], _current_cb);
    m_itemsMap[SettingsHomePageOptions::QUICK_ACCESS] =
        appendGenlist(genlist, m_setting_check_radio_item_class, &m_buttonsMap[SettingsHomePageOptions::QUICK_ACCESS], _quick_cb);
    m_itemsMap[SettingsHomePageOptions::MOST_VIS] =
        appendGenlist(genlist, m_setting_check_radio_item_class, &m_buttonsMap[SettingsHomePageOptions::MOST_VIS], _most_visited_cb);
    m_itemsMap[SettingsHomePageOptions::OTHER] =
        appendGenlist(genlist, m_setting_check_radio_item_class, &m_buttonsMap[SettingsHomePageOptions::OTHER], _other_cb);
    return true;
}

Evas_Object* SettingsHomePage::createRadioButton(Evas_Object* obj, ItemData* itd)
{
    auto radio_button = elm_radio_add(obj);
    if (radio_button) {
        elm_radio_state_value_set(radio_button, itd->id);
        elm_radio_group_add(radio_button, getRadioGroup());
        evas_object_propagate_events_set(radio_button, EINA_FALSE);
        switch (itd->id) {
            case SettingsHomePageOptions::DEFAULT:
                evas_object_smart_callback_add(radio_button, "changed", _default_cb, this);
                break;
            case SettingsHomePageOptions::CURRENT:
                evas_object_smart_callback_add(radio_button, "changed", _current_cb, this);
                break;
            case SettingsHomePageOptions::QUICK_ACCESS:
                evas_object_smart_callback_add(radio_button, "changed", _quick_cb, this);
                break;
            case SettingsHomePageOptions::MOST_VIS:
                evas_object_smart_callback_add(radio_button, "changed", _most_visited_cb, this);
                break;
            case SettingsHomePageOptions::OTHER:
                evas_object_smart_callback_add(radio_button, "changed", _other_cb, this);
                break;
        }
        elm_access_object_unregister(radio_button);
    }
    return radio_button;
}

void SettingsHomePage::_default_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SettingsPrettySignalConnector::Instance().homePageChanged(SettingsHomePage::DEF_HOME_PAGE);
}

void SettingsHomePage::_current_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    auto self = static_cast<SettingsHomePage*>(data);
    SettingsPrettySignalConnector::Instance().homePageChanged(self->getCurrentPage());
}

void SettingsHomePage::_quick_cb(void *, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    // TODO Implementation
}

void SettingsHomePage::_most_visited_cb(void *, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    // TODO Implementation
}

void SettingsHomePage::_other_cb(void *, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    // TODO Implementation
}

}
}
