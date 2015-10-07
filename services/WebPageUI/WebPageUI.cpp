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
#include <memory>
#include <boost/format.hpp>
#include "WebPageUI.h"
#include "BrowserLogger.h"
#include "ServiceManager.h"
#include "BrowserAssert.h"

namespace tizen_browser {
namespace base_ui {

EXPORT_SERVICE(WebPageUI, "org.tizen.browser.webpageui")

WebPageUI::WebPageUI()
    : m_parent(nullptr)
    , m_mainLayout(nullptr)
    , m_errorLayout(nullptr)
    , m_progressBar(nullptr)
    , m_URIEntry(new URIEntry())
    , m_homePageActive(false)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

WebPageUI::~WebPageUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void WebPageUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* WebPageUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_mainLayout) {
        createLayout();
    }
    return m_mainLayout;
}

void WebPageUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_mainLayout);
    evas_object_show(m_mainLayout);

    evas_object_show(elm_object_part_content_get(m_mainLayout, "web_view"));
    evas_object_show(m_URIEntry->getContent());
    evas_object_show(elm_object_part_content_get(m_mainLayout, "uri_bar_buttons_left"));
    evas_object_show(elm_object_part_content_get(m_mainLayout, "uri_bar_buttons_right"));

    if (m_homePageActive)
        showMainUI();
}


void WebPageUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_mainLayout);
    elm_object_focus_custom_chain_unset(m_mainLayout);
    evas_object_hide(m_mainLayout);

    if (m_homePageActive)
        hideMainUI();

    evas_object_hide(elm_object_part_content_get(m_mainLayout, "web_view"));
    m_URIEntry->editingCanceled();
    evas_object_hide(m_URIEntry->getContent());
    evas_object_hide(elm_object_part_content_get(m_mainLayout, "uri_bar_buttons_left"));
    evas_object_hide(elm_object_part_content_get(m_mainLayout, "uri_bar_buttons_right"));
}

void WebPageUI::loadStarted()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    showProgressBar();
    elm_object_signal_emit(m_URIEntry->getContent(), "shiftright_uribg", "ui");
    elm_object_signal_emit(m_mainLayout, "shiftright_uri", "ui");
    m_leftButtonBar->setActionForButton("refresh_stop_button", m_stopLoading);
}

void WebPageUI::progressChanged(double progress)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (progress == 1.0) {
        hideProgressBar();
    } else {
        elm_progressbar_value_set(m_progressBar, progress);
    }
}

void WebPageUI::loadFinished()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_mainLayout, "hide_progressbar_bg", "ui");
    m_leftButtonBar->setActionForButton("refresh_stop_button", m_reload);
}

bool WebPageUI::isErrorPageActive()
{
    return elm_object_part_content_get(m_mainLayout, "web_view") == m_errorLayout;
}

void WebPageUI::setMainContent(Evas_Object* content)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(content);
    hideWebView();
    elm_object_part_content_set(m_mainLayout, "web_view", content);
    evas_object_show(content);
}

void WebPageUI::switchViewToErrorPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_homePageActive = false;
    setMainContent(m_errorLayout);
    evas_object_show(m_leftButtonBar->getContent());
    setErrorButtons();
    refreshFocusChain();
}

void WebPageUI::switchViewToWebPage(Evas_Object* content, const std::string uri)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_homePageActive)
    {
        hideMainUI();
        m_homePageActive = false;
    }
    setMainContent(content);
    updateURIBar(uri);
    refreshFocusChain();
    evas_object_show(m_leftButtonBar->getContent());
    elm_object_focus_custom_chain_append(m_mainLayout, content, NULL);
}

void WebPageUI::switchViewToQuickAccess(Evas_Object* content)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_homePageActive = true;
    setMainContent(content);
    evas_object_hide(m_leftButtonBar->getContent());
    elm_object_signal_emit(m_mainLayout, "shiftback_uri", "ui");
    elm_object_signal_emit(m_URIEntry->getContent(), "shiftback_uribg", "ui");
    hideProgressBar();
    refreshFocusChain();
    m_URIEntry->changeUri("");
    m_URIEntry->setFocus();
    showMainUI();
}

void WebPageUI::faviconClicked(void* data, Evas_Object*, const char*, const char*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    WebPageUI* self = reinterpret_cast<WebPageUI*>(data);
    if (!self->isHomePageActive() && !self->isErrorPageActive()) {
        self->getURIEntry().clearFocus();
    }
}

void WebPageUI::setTabsNumber(int tabs)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (tabs == 0) {
        elm_object_part_text_set(m_rightButtonBar->getContent(), "tabs_number", "");
    } else {
        elm_object_part_text_set(m_rightButtonBar->getContent(), "tabs_number", (boost::format("%1%") % tabs).str().c_str());
    }
}

void WebPageUI::createLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    // create web layout
    m_mainLayout = elm_layout_add(m_parent);
    evas_object_size_hint_weight_set(m_mainLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_layout_file_set(m_mainLayout, edjePath("WebPageUI/WebPageUI.edj").c_str(), "main_layout");

    createErrorLayout();
    createActions();

    // left buttons
    m_leftButtonBar = std::unique_ptr<ButtonBar>(new ButtonBar(m_mainLayout, "WebPageUI/LeftButtonBar.edj", "left_button_bar"));
    m_leftButtonBar->addAction(m_back, "prev_button");
    m_leftButtonBar->addAction(m_forward, "next_button");
    m_leftButtonBar->addAction(m_reload, "refresh_stop_button");

    //register action that will be used later by buttons"
    m_leftButtonBar->registerEnabledChangedCallback(m_stopLoading, "refresh_stop_button");

    // right buttons
    m_rightButtonBar = std::unique_ptr<ButtonBar>(new ButtonBar(m_mainLayout, "WebPageUI/RightButtonBar.edj", "right_button_bar"));
    m_rightButtonBar->addAction(m_tab, "tab_button");
    m_rightButtonBar->addAction(m_showMoreMenu, "setting_button");

    // progress bar
    m_progressBar = elm_progressbar_add(m_mainLayout);
    elm_object_style_set(m_progressBar, "play_buffer");

    //URL bar (Evas Object is shipped by URIEntry object)
    m_URIEntry->init(m_mainLayout);
    elm_object_part_content_set(m_mainLayout, "uri_entry", m_URIEntry->getContent());
    elm_object_part_content_set(m_mainLayout, "uri_bar_buttons_left", m_leftButtonBar->getContent());
    elm_object_part_content_set(m_mainLayout, "uri_bar_buttons_right", m_rightButtonBar->getContent());

    elm_layout_signal_callback_add(m_URIEntry->getContent(), "slide_websearch", "elm", faviconClicked, this);

    connectActions();
}

void WebPageUI::createErrorLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_errorLayout =  elm_layout_add(m_mainLayout);
    evas_object_size_hint_weight_set(m_errorLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_errorLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_layout_file_set(m_errorLayout, edjePath("WebPageUI/ErrorMessage.edj").c_str(), "error_message");
}

void WebPageUI::createActions()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_back = sharedAction(new Action("Back"));
    m_back->setToolTip("Previous");
    m_back->setIcon("browser/toolbar_prev");

    m_forward = sharedAction(new Action("Next"));
    m_forward->setToolTip("Next");
    m_forward->setIcon("browser/toolbar_next");

    m_stopLoading = sharedAction(new Action("Stop"));
    m_stopLoading->setToolTip("Stop");
    m_stopLoading->setIcon("browser/toolbar_stop");

    m_reload = sharedAction(new Action("Reload"));
    m_reload->setToolTip("Reload");
    m_reload->setIcon("browser/toolbar_reload");
    m_tab = sharedAction(new Action("Tabs"));
    m_tab->setToolTip("Tab Manager");
    m_tab->setIcon("browser/toolbar_tab");

    m_showMoreMenu = sharedAction(new Action("More_Menu"));
    m_showMoreMenu->setToolTip("More Menu");
    m_showMoreMenu->setIcon("browser/toolbar_setting");
}

void WebPageUI::connectActions()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    //left bar
    m_back->triggered.connect(boost::bind(&WebPageUI::backPageConnect, this));
    m_forward->triggered.connect(boost::bind(&WebPageUI::forwardPageConnect, this));
    m_stopLoading->triggered.connect(boost::bind(&WebPageUI::stopLoadingPageConnect, this));
    m_reload->triggered.connect(boost::bind(&WebPageUI::reloadPageConnect, this));

    //right bar
    m_tab->triggered.connect(boost::bind(&WebPageUI::showTabUIConnect, this));
    m_showMoreMenu->triggered.connect(boost::bind(&WebPageUI::showMoreMenuConnect, this));
}

void WebPageUI::showProgressBar()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_mainLayout, "show_progressbar_bg", "ui");
    elm_object_part_content_set(m_mainLayout, "progress_bar", m_progressBar);
}

void WebPageUI::hideProgressBar()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_mainLayout, "hide_progressbar_bg", "ui");
    elm_progressbar_value_set(m_progressBar, 0.0);
    elm_object_part_content_unset(m_mainLayout, "progress_bar");
    evas_object_hide(m_progressBar);
}

void WebPageUI::hideWebView()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    evas_object_hide(elm_object_part_content_get(m_mainLayout, "web_view"));
    elm_object_part_content_unset(m_mainLayout, "web_view");
}

void WebPageUI::setErrorButtons()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_leftButtonBar->setActionForButton("refresh_stop_button", m_reload);
    m_stopLoading->setEnabled(false);
    m_reload->setEnabled(true);
    m_forward->setEnabled(false);
    evas_object_hide(m_progressBar);
}

void WebPageUI::updateURIBar(const std::string& uri)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_URIEntry->changeUri(uri);
    m_leftButtonBar->setActionForButton("refresh_stop_button", m_reload);

    m_stopLoading->setEnabled(true);
    m_reload->setEnabled(true);
    hideProgressBar();
}

std::string WebPageUI::edjePath(const std::string& file)
{
    return std::string(EDJE_DIR) + file;
}

void WebPageUI::showTabUIConnect()
{
    hideUI();
    showTabUI();
}
void WebPageUI::showMoreMenuConnect()
{
    hideUI();
    showMoreMenu();
}

void WebPageUI::refreshFocusChain()
{
    // set custom focus chain
    elm_object_focus_custom_chain_unset(m_mainLayout);
    elm_object_focus_custom_chain_append(m_mainLayout, m_rightButtonBar->getContent(), NULL);
    if (!m_homePageActive) {
        elm_object_focus_custom_chain_append(m_mainLayout, m_leftButtonBar->getContent(), NULL);
    } else {
        m_reload->setEnabled(false);
    }
    elm_object_focus_custom_chain_append(m_mainLayout, m_URIEntry->getContent(), NULL);
}

}   // namespace tizen_browser
}   // namespace base_ui