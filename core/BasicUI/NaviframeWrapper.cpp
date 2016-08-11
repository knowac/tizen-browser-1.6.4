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

/*
 * NaviframeWrapper.cpp
 *
 *  Created on: Aug 4, 2016
 *      Author: m.kawonczyk@samsung.com
 */

#include "NaviframeWrapper.h"

#include "BrowserAssert.h"
#include "BrowserLogger.h"

namespace tizen_browser{
namespace base_ui{

NaviframeWrapper::NaviframeWrapper(Evas_Object* parent)
    : m_parent(parent)
    , m_layout(nullptr)
    , m_bottom_box(nullptr)
{
    M_ASSERT(m_parent);
    m_layout = elm_layout_add(m_parent);
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_layout);
    elm_layout_theme_set(m_layout, "naviframe", "item/basic", "default");
}

NaviframeWrapper::~NaviframeWrapper()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    evas_object_del(m_layout);
}

Evas_Object* NaviframeWrapper::getLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_layout);
    return m_layout;
}

void NaviframeWrapper::setTitle(std::string title)
{
    elm_object_translatable_part_text_set(m_layout, "elm.text.title", title.c_str());
}

void NaviframeWrapper::setContent(Evas_Object *content)
{
    elm_object_part_content_set(m_layout, "elm.swallow.content", content);
}

void NaviframeWrapper::addPrevButton(Evas_Smart_Cb callback, void *data)
{
    Evas_Object* button = elm_button_add(m_layout);
    elm_object_part_content_set(m_layout, "elm.swallow.prev_btn", button);
    elm_object_style_set(button, "tizen_view/prev_btn");
    evas_object_smart_callback_add(button, "clicked", callback, data);
    evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(button, EVAS_HINT_FILL, EVAS_HINT_FILL);
}

void NaviframeWrapper::setVisibilityPrevButton(bool visible)
{
    Evas_Object* button = elm_object_part_content_get(m_layout, "elm.swallow.prev_btn");
    if (visible) {
        evas_object_show(button);
        elm_object_signal_emit(m_layout, "elm,state,prev_btn,show", "elm");
    } else {
        evas_object_hide(button);
        elm_object_signal_emit(m_layout, "elm,state,prev_btn,hide", "elm");
    }
}

void NaviframeWrapper::addButtonToBottomBar(std::string text, Evas_Smart_Cb callback, void *data)
{
    if (!m_bottom_box)
        createBottomBar();
    Evas_Object* button = elm_button_add(m_bottom_box);
    elm_object_style_set(button, "bottom");
    evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(button, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_text_set(button, "elm.text", text.c_str());
    elm_object_signal_emit(button, "elm,state,text,visible", "elm");
    evas_object_smart_callback_add(button, "clicked", callback, data);

    elm_box_pack_end(m_bottom_box, button);
    m_map_bottom_box.insert(std::pair<std::string, Evas_Object*>(text, button));
    evas_object_show(button);

}

void NaviframeWrapper::setEnableButtonInBottomBar(std::string text, bool enabled)
{
    if (enabled)
        elm_object_signal_emit(m_map_bottom_box[text], "elm,state,enabled", "elm");
    else
        elm_object_signal_emit(m_map_bottom_box[text], "elm,state,disabled", "elm");
}

void NaviframeWrapper::setVisibilityBottomBar(bool visible)
{
    if (!m_bottom_box)
        createBottomBar();
    if (visible) {
        evas_object_show(m_bottom_box);
        elm_object_signal_emit(m_layout, "elm,state,toolbar,show", "elm");
    } else {
        evas_object_hide(m_bottom_box);
        elm_object_signal_emit(m_layout, "elm,state,toolbar,hide", "elm");
    }
}

void NaviframeWrapper::createBottomBar(Evas_Object* layout, std::string swallow_name)
{
    m_bottom_box = elm_box_add(m_layout);
    elm_box_horizontal_set(m_bottom_box, EINA_TRUE);
    elm_box_padding_set(m_bottom_box, 32, 0);
    evas_object_size_hint_weight_set(m_bottom_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_bottom_box, EVAS_HINT_FILL, EVAS_HINT_FILL);

    if (layout) {
        elm_object_part_content_set(m_layout, "toolbar", layout);
        elm_object_part_content_set(layout, swallow_name.c_str(), m_bottom_box);
    } else {
        elm_object_part_content_set(m_layout, "toolbar", m_bottom_box);
    }
}


}
}
