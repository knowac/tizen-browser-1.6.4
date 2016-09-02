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

#ifndef PASWORDUI_H
#define PASWORDUI_H

#include <Evas.h>
#include <memory>
#include "AbstractInterfaces/AbstractUIComponent.h"
#include "NaviframeWrapper.h"

namespace tizen_browser {
namespace base_ui {

class PasswordUI : public interfaces::AbstractUIComponent
{
public:
    PasswordUI();
    virtual ~PasswordUI();

    void init(Evas_Object* parent) override;
    Evas_Object* getContent() override;
    void showUI() override;
    void hideUI() override;

    void createLayout();
    boost::signals2::signal<void (std::string, std::string)> setDBStringParamValue;

    static const std::string PASSWORD_FIELD;

private:

    static void _close_clicked(void *data, Evas_Object *obj, void *event_info);
    static void _right_button_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _left_button_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _show_password_state_changed(void* data, Evas_Object*, void*);

    Evas_Object* m_parent;
    Evas_Object* m_content;
    Evas_Object* m_entry;
    Evas_Object* m_checkbox;
    std::unique_ptr<NaviframeWrapper> m_naviframe;
    std::string m_edjFilePath;
};

}
}

#endif // PASWORDUI_H
