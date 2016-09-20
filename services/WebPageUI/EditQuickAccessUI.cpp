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
#include <AbstractMainWindow.h>
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "QuickAccess.h"

namespace tizen_browser{
namespace base_ui{

EditQuickAccessUI::EditQuickAccessUI()
    : m_parent(nullptr)
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
    if (!m_naviframe)
        createEditQuickAccessLayout();

    auto signal = requestQuickAccessGengrid();
    if (signal)
        m_naviframe->setContent(*signal);
    else
        BROWSER_LOGW("Missing signal value");

    return m_naviframe->getLayout();
}

void EditQuickAccessUI::backPressed()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    editingFinished();
    closeUI();
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
    //TODO: save reordered items
    self->closeUI();
}

void EditQuickAccessUI::createEditQuickAccessLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);

    m_naviframe = std::make_shared<NaviframeWrapper>(m_parent);
    m_naviframe->setTitle("Edit quick access");

    m_naviframe->addLeftButton(_cancel_clicked, this);
    m_naviframe->setLeftButtonText("Cancel");
    m_naviframe->setLeftButtonVisible(true);

    m_naviframe->addRightButton(_done_clicked, this);
    m_naviframe->setRightButtonText("Done");
    m_naviframe->setRightButtonVisible(true);
}

}   // namespace tizen_browser
}   // namespace base_ui

