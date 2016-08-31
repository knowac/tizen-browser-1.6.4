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

#include "SettingsAdvanced.h"

#include "Config.h"
#include "app_preference.h"

namespace tizen_browser{
namespace base_ui{

SettingsAdvanced::SettingsAdvanced(Evas_Object* parent)
{
    init(parent);
    updateButtonMap();
    vconf_notify_key_changed("memory/sysman/mmc", notifyStorageChange, this);
};

SettingsAdvanced::~SettingsAdvanced()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsAdvanced::updateButtonMap()
{
    ItemData enable_js;
    enable_js.buttonText = Translations::SettingsAdvancedEnableJavaScript;
    enable_js.subText = Translations::SettingsAdvancedEnableJavaScriptSub;
    enable_js.sui = this;
    enable_js.id = ENABLE_JS;

    ItemData block_popups;
    block_popups.buttonText = Translations::SettingsAdvancedBlockPopups;
    block_popups.subText = Translations::SettingsAdvancedBlockPopupsSub;
    block_popups.sui = this;
    block_popups.id = BLOCK_POPUPS;

    ItemData save_content;
    save_content.buttonText = Translations::SettingsAdvancedSaveContent;
    save_content.subText =  []() {
        auto sig =
            SPSC.getWebEngineSettingsParamString(
                basic_webengine::WebEngineSettings::SAVE_CONTENT_LOCATION);
        return (sig && !sig->empty()) ?
            *sig :
            Translations::Device;
    }();
    save_content.sui = this;
    save_content.id = SAVE_CONTENT;

    ItemData manage_web_data;
    manage_web_data.buttonText = Translations::SettingsAdvancedManageWebsiteData;
    manage_web_data.subText = Translations::SettingsAdvancedManageWebsiteDataSub;
    manage_web_data.sui = this;
    manage_web_data.id = MANAGE_WEB_DATA;

    m_buttonsMap[SettingsAdvancedOptions::ENABLE_JS] = enable_js;
    m_buttonsMap[SettingsAdvancedOptions::BLOCK_POPUPS] = block_popups;
    m_buttonsMap[SettingsAdvancedOptions::SAVE_CONTENT] = save_content;
    m_buttonsMap[SettingsAdvancedOptions::MANAGE_WEB_DATA] = manage_web_data;

    SPSC.setContentDestination.connect(
        boost::bind(&SettingsAdvanced::setContentDestination, this, _1));

    changeGenlistStorage();
}

void SettingsAdvanced::changeGenlistStorage()
{
    int mmc_mode = VCONFKEY_SYSMAN_MMC_REMOVED;
    if (vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS, &mmc_mode) != 0)
        BROWSER_LOGE("Fail to get vconf_get_int : VCONFKEY_SYSMAN_MMC_STATUS");

    if (mmc_mode == -1) /* This values also means unmounted mmc */
        mmc_mode = VCONFKEY_SYSMAN_MMC_REMOVED;

    if (mmc_mode == VCONFKEY_SYSMAN_MMC_REMOVED) {
        setContentDestination(
            static_cast<std::underlying_type_t<RadioButtons>>(RadioButtons::DEVICE));
        elm_object_item_disabled_set(m_genlistItems[SettingsAdvancedOptions::SAVE_CONTENT], EINA_TRUE);
    } else
        elm_object_item_disabled_set(m_genlistItems[SettingsAdvancedOptions::SAVE_CONTENT], EINA_FALSE);
    elm_genlist_realized_items_update(m_genlist);
}

void SettingsAdvanced::notifyStorageChange(keynode_t* /*key*/, void* data)
{
    if (!data) {
        BROWSER_LOGD("[no data] ");
        return;
    }

    auto self = static_cast<SettingsAdvanced*>(data);
    self->changeGenlistStorage();
}

bool SettingsAdvanced::populateList(Evas_Object* genlist)
{
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", Translations::SettingsAdvancedTitle.c_str());

    m_genlistItems[SettingsAdvancedOptions::ENABLE_JS] =
        appendGenlist(genlist, m_setting_check_on_of_item_class, &m_buttonsMap[SettingsAdvancedOptions::ENABLE_JS], _enable_js_cb);
    m_genlistItems[SettingsAdvancedOptions::BLOCK_POPUPS] =
        appendGenlist(genlist, m_setting_check_on_of_item_class, &m_buttonsMap[SettingsAdvancedOptions::BLOCK_POPUPS], _block_popups_cb);
    m_genlistItems[SettingsAdvancedOptions::SAVE_CONTENT] =
        appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[SettingsAdvancedOptions::SAVE_CONTENT], _save_content_cb);
    m_genlistItems[SettingsAdvancedOptions::MANAGE_WEB_DATA] =
        appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[SettingsAdvancedOptions::MANAGE_WEB_DATA], _manage_web_data_cb);

    elm_object_item_disabled_set(m_genlistItems[SettingsAdvancedOptions::MANAGE_WEB_DATA], EINA_TRUE);

    return true;
}

Evas_Object* SettingsAdvanced::createOnOffCheckBox(Evas_Object* obj, ItemData* itd)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto check = elm_check_add(obj);
    elm_object_style_set(check, "on&off");
    elm_check_state_set(check, getOriginalState(itd->id));
    evas_object_smart_callback_add(check, "changed", grid_item_check_changed, itd);
    return check;
}

Eina_Bool SettingsAdvanced::getOriginalState(int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<bool> sig;
    switch (id){
        case ENABLE_JS:
            sig = SPSC.getWebEngineSettingsParam(
                basic_webengine::WebEngineSettings::ENABLE_JAVASCRIPT);
            break;
        case BLOCK_POPUPS:
            sig = SPSC.getWebEngineSettingsParam(
                basic_webengine::WebEngineSettings::SCRIPTS_CAN_OPEN_PAGES);
            break;
        default:
            sig = false;
            break;
    }
    return (sig && *sig) ? EINA_TRUE : EINA_FALSE;
}

void SettingsAdvanced::_enable_js_cb(void *, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto el = elm_genlist_selected_item_get(obj);
    auto check = elm_object_item_part_content_get(el, "elm.swallow.end");
    auto value = !elm_check_state_get(check);

    elm_check_state_set(check, value);
    SPSC.setWebEngineSettingsParam(
        basic_webengine::WebEngineSettings::ENABLE_JAVASCRIPT,
        static_cast<bool>(value));
}

void SettingsAdvanced::_block_popups_cb(void *, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto el = elm_genlist_selected_item_get(obj);
    auto check = elm_object_item_part_content_get(el, "elm.swallow.end");
    auto value = !elm_check_state_get(check);

    elm_check_state_set(check, value);
    SPSC.setWebEngineSettingsParam(
        basic_webengine::WebEngineSettings::SCRIPTS_CAN_OPEN_PAGES,
        static_cast<bool>(value));
}

void SettingsAdvanced::_save_content_cb(void *, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SPSC.settingsSaveContentToRadioPopup();
}

void SettingsAdvanced::_manage_web_data_cb(void *, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    // TODO Implementation
}

void SettingsAdvanced::grid_item_check_changed(void* data, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    auto itd = static_cast<ItemData*>(data);
    auto value = !elm_check_state_get(obj);
    switch (itd->id){
        case ENABLE_JS:
            elm_check_state_set(obj, value);
            SPSC.setWebEngineSettingsParam(
                basic_webengine::WebEngineSettings::ENABLE_JAVASCRIPT,
                static_cast<bool>(value));
            break;
        case BLOCK_POPUPS:
            elm_check_state_set(obj, value);
            SPSC.setWebEngineSettingsParam(
                basic_webengine::WebEngineSettings::SCRIPTS_CAN_OPEN_PAGES,
                static_cast<bool>(value));
            break;
        default:
            break;
    }
}

bool SettingsAdvanced::setStorageType(SettingsStorageType type)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (preference_set_int("DefaultStorage", static_cast<int>(type)) != PREFERENCE_ERROR_NONE) {
        BROWSER_LOGD("[%s:%d] setStorageType error", __PRETTY_FUNCTION__, __LINE__);
        return true;
    }
    return false;
}

void SettingsAdvanced::setContentDestination(int button)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    switch (static_cast<RadioButtons>(button)) {
        case RadioButtons::DEVICE:
            m_buttonsMap[SettingsAdvancedOptions::SAVE_CONTENT].subText = Translations::Device;
            setStorageType(SettingsStorageType::DEVICE);
            break;
        case RadioButtons::SD_CARD:
            m_buttonsMap[SettingsAdvancedOptions::SAVE_CONTENT].subText = Translations::SDCard;
            setStorageType(SettingsStorageType::SD_CARD);
            break;
        default:
            return;
    }

    SPSC.setWebEngineSettingsParamString(
        basic_webengine::WebEngineSettings::SAVE_CONTENT_LOCATION,
        m_buttonsMap[SettingsAdvancedOptions::SAVE_CONTENT].subText);
    elm_genlist_item_update(m_genlistItems[SAVE_CONTENT]);
}
}
}