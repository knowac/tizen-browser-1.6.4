#include "BrowserAssert.h"
#include "PasswordUI.h"
#include "BrowserLogger.h"

namespace tizen_browser{
namespace base_ui{

const std::string PasswordUI::PASSWORD_FIELD = "secret_password";

PasswordUI::PasswordUI()
    : m_parent(nullptr)
    , m_content(nullptr)
    , m_entry(nullptr)
    , m_checkbox(nullptr)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("TabUI/PasswordUI.edj");
}

PasswordUI::~PasswordUI()
{
}

void PasswordUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_naviframe->getLayout());
    m_naviframe->show();
}

void PasswordUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_naviframe->getLayout());
    m_naviframe->hide();
}

void PasswordUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* PasswordUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_naviframe)
        createLayout();
    return m_naviframe->getLayout();
}

void PasswordUI::createLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());

    m_naviframe = std::make_unique<NaviframeWrapper>(m_parent);

    m_content = elm_layout_add(m_naviframe->getLayout());
    evas_object_size_hint_weight_set(m_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_content, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_content);
    elm_layout_file_set(m_content, m_edjFilePath.c_str(), "main_layout");

    elm_layout_text_set(m_content, "instruction", "Your password must contain at "
        "least 6 characters, including at least one letter.");

    m_entry = elm_entry_add(m_content);
    elm_entry_password_set(m_entry, EINA_TRUE);
    evas_object_show(m_entry);
    elm_layout_content_set(m_content, "password_field", m_entry);

    m_checkbox = elm_check_add(m_content);
    elm_check_state_set(m_checkbox, EINA_FALSE);
    evas_object_smart_callback_add(m_checkbox, "changed", _show_password_state_changed, this);
    evas_object_show(m_checkbox);
    elm_layout_content_set(m_content, "show_password", m_checkbox);
    elm_layout_text_set(m_content, "chb_label", "Show password");

    m_naviframe->setContent(m_content);
    m_naviframe->setTitle("Create password");
    m_naviframe->addPrevButton(_close_clicked, this);
    m_naviframe->setPrevButtonVisible(true);


    m_naviframe->setVisibleBottomBar(true);
    m_naviframe->addButtonToBottomBar("Cancel", _left_button_clicked, this);
    m_naviframe->setEnableButtonInBottomBar(0, true);
    m_naviframe->addButtonToBottomBar("Save", _right_button_clicked, this);
    m_naviframe->setEnableButtonInBottomBar(1, true);


}

void PasswordUI::_close_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = static_cast<PasswordUI*>(data);
    self->closeUI();
}

void PasswordUI::_right_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = static_cast<PasswordUI*>(data);
    std::string hash = std::to_string(
        static_cast<int>(
            std::hash<std::string>()(std::string(elm_object_text_get(self->m_entry)))
        )
    );
    self->setDBStringParamValue(PASSWORD_FIELD, hash);

    // TODO: set PASSWORD_DECISION_MADE flag to true
}

void PasswordUI::_left_button_clicked(void* data, Evas_Object*, void*)
{
    auto self = static_cast<PasswordUI*>(data);
    elm_entry_entry_set(self->m_entry, "");
    self->closeUI();
}

void PasswordUI::_show_password_state_changed(void* data, Evas_Object*, void*)
{
    auto self = static_cast<PasswordUI*>(data);
    elm_entry_password_set(self->m_entry, !elm_check_state_get(self->m_checkbox));
}

}
}