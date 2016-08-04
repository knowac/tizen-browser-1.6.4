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

#include "SettingsMain.h"

#include "Config.h"

namespace tizen_browser{
namespace base_ui{

SettingsMain::SettingsMain(Evas_Object* parent)
{
    init(parent);
};

SettingsMain::~SettingsMain()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsMain::updateButtonMap()
{
    ItemData homePage;
    homePage.buttonText = Translations::SettingsMainHomePage;
    homePage.subText = Translations::SettingsMainHomePageDefault;
    homePage.sui = this;
    homePage.id = HOME;

    ItemData search;
    search.buttonText = Translations::SettingsMainDefaultSearchEngine;
    search.subText =  [this]() -> std::string {
        auto sig =
            SPSC.getWebEngineSettingsParamString(
                basic_webengine::WebEngineSettings::DEFAULT_SEARCH_ENGINE);
        return (sig && !sig->empty()) ?
            *sig :
            Translations::Google;
    }();
    search.sui = this;
    search.id = SEARCH;

    ItemData autofill;
    autofill.buttonText = Translations::SettingsMainAutoFillProfiles;
    autofill.subText = Translations::SettingsMainAutoFillProfilesSub;
    autofill.sui = this;
    autofill.id = AUTO_FILL_PROFILE;

    ItemData zoom;
    zoom.buttonText = Translations::SettingsMainManualZoom;
    zoom.subText = Translations::SettingsMainManualZoomSub;
    zoom.sui = this;
    zoom.id = ZOOM;

    ItemData privacy;
    privacy.buttonText = Translations::SettingsMainPrivacy;
    privacy.sui = this;
    privacy.id = PRIVACY;

    ItemData advanced;
    advanced.buttonText = Translations::SettingsMainAdvanced;
    advanced.sui = this;
    advanced.id = ADVANCED;

    m_buttonsMap[SettingsMainOptions::HOME] = homePage;
    m_buttonsMap[SettingsMainOptions::SEARCH] = search;
    m_buttonsMap[SettingsMainOptions::AUTO_FILL_PROFILE] = autofill;
    m_buttonsMap[SettingsMainOptions::ZOOM] = zoom;
    m_buttonsMap[SettingsMainOptions::PRIVACY] = privacy;
    m_buttonsMap[SettingsMainOptions::ADVANCED] = advanced;

    SPSC.setSearchEngineSubText.connect(
        boost::bind(&SettingsMain::setSearchEngineSubText, this, _1));
}

bool SettingsMain::populateList(Evas_Object* genlist)
{
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", Translations::SettingsMainTitle.c_str());
    updateButtonMap();
    m_genlistItems[SettingsMainOptions::HOME] =
        appendGenlist(genlist, m_setting_double_item_class, &m_buttonsMap[SettingsMainOptions::HOME], _home_page_cb);
    m_genlistItems[SettingsMainOptions::SEARCH] =
        appendGenlist(genlist, m_setting_double_item_class, &m_buttonsMap[SettingsMainOptions::SEARCH], _search_engine_cb);
    m_genlistItems[SettingsMainOptions::AUTO_FILL_PROFILE] =
        appendGenlist(genlist, m_setting_double_item_class, &m_buttonsMap[SettingsMainOptions::AUTO_FILL_PROFILE], _auto_fill_cb);
    m_genlistItems[SettingsMainOptions::ZOOM] =
        appendGenlist(genlist, m_setting_check_on_of_item_class, &m_buttonsMap[SettingsMainOptions::ZOOM], _zoom_cb);
    m_genlistItems[SettingsMainOptions::PRIVACY] =
        appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[SettingsMainOptions::PRIVACY], _privacy_cb);
    m_genlistItems[SettingsMainOptions::ADVANCED] =
        appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[SettingsMainOptions::ADVANCED], _advanced_cb);
    return true;
}

Evas_Object* SettingsMain::createOnOffCheckBox(Evas_Object* obj, ItemData* itd)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto check = elm_check_add(obj);
    elm_object_style_set(check, "on&off");
    elm_check_state_set(check, getOriginalZoomState());
    evas_object_smart_callback_add(check, "changed", grid_item_check_changed, itd);
    return check;
}

Eina_Bool SettingsMain::getOriginalZoomState()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<bool> sig =
        SPSC.getWebEngineSettingsParam(
            basic_webengine::WebEngineSettings::PAGE_OVERVIEW);

    return (sig && *sig) ? EINA_TRUE : EINA_FALSE;
}

void SettingsMain::_home_page_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.settingsHomePageClicked();
}

void SettingsMain::_search_engine_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.settingsBaseShowRadioPopup();
}

void SettingsMain::_zoom_cb(void *, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto el = elm_genlist_selected_item_get(obj);
    auto check = elm_object_item_part_content_get(el, "elm.swallow.end");
    auto value = !elm_check_state_get(check);

    elm_check_state_set(check, value);
    SPSC.setWebEngineSettingsParam(
        basic_webengine::WebEngineSettings::PAGE_OVERVIEW,
        static_cast<bool>(value));
}

void SettingsMain::_advanced_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.settingsAdvancedClicked();
}

void SettingsMain::_auto_fill_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.settingsAutofillClicked();
}

void SettingsMain::_privacy_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.settingsPrivacyClicked();
}

void SettingsMain::grid_item_check_changed(void*, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto value = !elm_check_state_get(obj);

    elm_check_state_set(obj, value);
    SPSC.setWebEngineSettingsParam(
        basic_webengine::WebEngineSettings::PAGE_OVERVIEW,
        static_cast<bool>(value));
}

void SettingsMain::setSearchEngineSubText(int button)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    switch (static_cast<RadioButtons>(button)) {
        case RadioButtons::GOOGLE:
            m_buttonsMap[SettingsMainOptions::SEARCH].subText = Translations::Google;
            break;
        case RadioButtons::YAHOO:
            m_buttonsMap[SettingsMainOptions::SEARCH].subText = Translations::Yahoo;
            break;
        case RadioButtons::BING:
            m_buttonsMap[SettingsMainOptions::SEARCH].subText = Translations::Bing;
            break;
        default:
            return;
    }
    SPSC.setWebEngineSettingsParamString(
        basic_webengine::WebEngineSettings::DEFAULT_SEARCH_ENGINE,
        m_buttonsMap[SettingsMainOptions::SEARCH].subText);
    elm_genlist_realized_items_update(m_genlist);
}
}
}
