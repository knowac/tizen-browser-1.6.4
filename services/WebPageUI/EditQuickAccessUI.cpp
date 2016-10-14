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

#include "EditQuickAccessUI.h"

#include <Elementary.h>
#include <boost/format.hpp>
#include <AbstractMainWindow.h>
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "QuickAccess.h"
#include "app_i18n.h"

namespace tizen_browser{
namespace base_ui{

EditQuickAccessUI::EditQuickAccessUI()
    : m_parent(nullptr)
    , m_editState(QuickAccessState::Edit)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

EditQuickAccessUI::~EditQuickAccessUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void EditQuickAccessUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_naviframe->getLayout());
    m_naviframe->show();
    if (m_editState == QuickAccessState::Edit)
        m_naviframe->setRightButtonEnabled(true);
}

void EditQuickAccessUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_naviframe->getLayout());
    m_naviframe->hide();
}

void EditQuickAccessUI::init(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object *EditQuickAccessUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);

    auto signal = requestQuickAccessState();
    if (signal)
        m_editState = *signal;
    else
        BROWSER_LOGW("Missing signal value");

    if (!m_naviframe)
        createEditLayout();

    auto layout = elm_layout_add(m_naviframe->getLayout());
    elm_layout_theme_set(layout, "layout", "application", "default");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    auto bg = elm_bg_add(layout);
    elm_bg_color_set(bg, 255, 255, 255);
    elm_object_part_content_set(layout, "elm.swallow.bg", bg);

    if (m_editState == QuickAccessState::Edit) {
        m_naviframe->setTitle(_("IDS_BR_OPT_EDIT_QUICK_ACCESS_ABB"));
        m_naviframe->setRightButtonText(_("IDS_BR_SK_DONE"));
        auto signal = requestQuickAccessGengrid();
        if (signal)
            elm_object_part_content_set(layout, "elm.swallow.content", *signal);
        else
            BROWSER_LOGW("Missing signal value");
    } else if (m_editState == QuickAccessState::DeleteMostVisited) {
        //TODO: add translation IDS_BR_HEADER_SELECT_ITEMS_ABB2
        m_naviframe->setTitle("Select items");
        m_naviframe->setRightButtonText(_("IDS_BR_SK_DELETE_ABB"));
        auto signal = requestMostVisitedGengrid();
        if (signal)
            elm_object_part_content_set(layout, "elm.swallow.content", *signal);
        else
            BROWSER_LOGW("Missing signal value");
    } else {
        BROWSER_LOGE("No correct Edit state");
    }

    evas_object_show(layout);
    m_naviframe->setContent(layout);
    m_naviframe->setRightButtonVisible(true);
    m_naviframe->setRightButtonEnabled(false);

    return m_naviframe->getLayout();
}

void EditQuickAccessUI::backPressed()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    editingFinished();
    closeUI();
}

void EditQuickAccessUI::setMVSelectedItems(int count)
{
    if (count)
        m_naviframe->setTitle((boost::format(_("IDS_BR_HEADER_PD_SELECTED_ABB")) % count).str());
    else
        //TODO: Add translation
        m_naviframe->setTitle("Select items");
    m_naviframe->setRightButtonEnabled(count);
}

void EditQuickAccessUI::_cancel_clicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = static_cast<EditQuickAccessUI*>(data);
    self->backPressed();
}

void EditQuickAccessUI::_done_clicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = static_cast<EditQuickAccessUI*>(data);
    self->editingFinished();
    if (self->m_editState == QuickAccessState::Edit) {
        //TODO: save reordered items
    } else if (self->m_editState == QuickAccessState::DeleteMostVisited) {
        self->deleteSelectedMostVisitedItems();
    }

    self->closeUI();
}

void EditQuickAccessUI::createEditLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);

    m_naviframe = std::make_shared<NaviframeWrapper>(m_parent);

    m_naviframe->addLeftButton(_cancel_clicked, this);
    m_naviframe->setLeftButtonText(_("IDS_BR_SK_CANCEL_ABB"));
    m_naviframe->setLeftButtonVisible(true);

    m_naviframe->addRightButton(_done_clicked, this);
}

}   // namespace tizen_browser
}   // namespace base_ui

