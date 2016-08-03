#include "RadioPopup.h"
#include "AbstractMainWindow.h"

namespace tizen_browser
{

namespace base_ui
{

std::map<RadioButtons, std::string> RadioPopup::createTranslations()
{
    std::map<RadioButtons, std::string> m;
    m[RadioButtons::GOOGLE]  = "Google";
    m[RadioButtons::YAHOO]   = "Yahoo!";
    m[RadioButtons::BING]    = "Bing";
    // TODO Translations
    m[RadioButtons::DEVICE]  = "Device";
    m[RadioButtons::SD_CARD] = "SD card";

    return m;
}

std::map<RadioButtons, std::string> RadioPopup::s_buttonsTranslations = createTranslations();

RadioPopup* RadioPopup::createPopup(Evas_Object* parent)
{
    RadioPopup* raw_popup = new RadioPopup(parent);
    return raw_popup;
}

RadioPopup::RadioPopup(Evas_Object* parent)
    : m_parent(parent)
    , m_popup(nullptr)
    , m_radioGroup(nullptr)
    , m_box(nullptr)
{
    m_popup = elm_popup_add(m_parent);
    elm_popup_align_set(m_popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
    evas_object_size_hint_weight_set(m_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    m_box = elm_box_add(m_popup);
    elm_object_focus_set(m_box, EINA_FALSE);
    evas_object_size_hint_weight_set(m_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_box);
}

RadioPopup::~RadioPopup()
{
    radioButtonClicked.disconnect_all_slots();
    evas_object_del(m_popup);
}

void RadioPopup::setButtons(RadioButtons rb, Evas_Object* button)
{
    m_buttons[rb] = button;
}

Evas_Object* RadioPopup::createItem(Evas_Object* parent, RadioButtons button)
{
    auto item = elm_button_add(parent);
    elm_object_focus_allow_set(item, EINA_TRUE);
    elm_layout_theme_set(item, "genlist", "item", "type1/default");
    evas_object_size_hint_weight_set(item, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
    evas_object_size_hint_align_set(item, EVAS_HINT_FILL, EVAS_HINT_FILL);

    static int val = static_cast<int>(RadioButtons::GOOGLE);

    if (!m_radioGroup) {
        m_radioGroup = elm_radio_add(item);
        elm_radio_state_value_set(m_radioGroup, -1);
    }
    auto radio = elm_radio_add(item);
    elm_radio_group_add(radio, m_radioGroup);
    elm_radio_state_value_set(radio, static_cast<int>(button));
    elm_radio_value_pointer_set(radio, &val);
    elm_access_object_unregister(radio);

    elm_object_part_content_set(item, "elm.swallow.end", radio);
    evas_object_propagate_events_set(radio, EINA_FALSE);
    evas_object_smart_callback_add(radio, "changed", _response_cb, this);

    elm_object_part_text_set(item, "elm.text", s_buttonsTranslations[button].c_str());
    setButtons(button, radio);
    evas_object_show(item);
    return item;
}

void RadioPopup::addRadio(RadioButtons button)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto item = createItem(m_popup, button);
    elm_box_pack_end(m_box, item);

    elm_object_content_set(m_popup, m_box);
}

void RadioPopup::show()
{
    evas_object_show(m_popup);
    popupShown(this);
}

void RadioPopup::_response_cb(void* data, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = reinterpret_cast<RadioPopup*>(data);
    auto it = std::find_if(
        self->m_buttons.begin(),
        self->m_buttons.end(),
        [obj] (const std::pair<RadioButtons, Evas_Object*>& i) -> bool {
                return i.second == obj;
        }
    );

    if (it == self->m_buttons.end())
        BROWSER_LOGW("[%s:%d] Button not found!", __PRETTY_FUNCTION__, __LINE__);
    else
        self->radioButtonClicked(it->first);
    self->dismiss();
}

void RadioPopup::setTitle(const std::string& title)
{
    m_title = title;
    elm_object_part_text_set(m_popup, "title,text", m_title.c_str());
}}

}
