/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#ifndef __SIMPLE_POPUP_H__
#define __SIMPLE_POPUP_H__ 1

#include <Evas.h>
#include <Elementary.h>
#include <string>
#include <list>
#include <memory>
#include <boost/signals2/signal.hpp>

#include "PopupButtons.h"
#include "WebConfirmation.h"

namespace tizen_browser
{

namespace base_ui
{

struct PopupData {
    virtual ~PopupData() {};
};

struct AuthenticationPopupData : public PopupData {
    basic_webengine::AuthenticationConfirmationPtr auth;
    Evas_Object *loginEntry;
    Evas_Object *passwordEntry;
};

struct EntryPopupData : public PopupData {
    std::string text;
    Evas_Object * entry;
};

class SimplePopup
{
public:
    static SimplePopup* createPopup();
    static SimplePopup* createPopup(const std::string &title, const std::string &message);

    void show();
    void setTitle(const std::string &title);
    void setMessage(const std::string &message);
    void setContent(Evas_Object *content);
    void setData(std::shared_ptr<PopupData> popupData);
    void addButton(PopupButtons buttonId);

    boost::signals2::signal<void (PopupButtons, std::shared_ptr<PopupData>)> buttonClicked;

    ~SimplePopup();

private:
    SimplePopup();
    SimplePopup(const std::string &title, const std::string &message);

    Evas_Object *popup;
    Evas_Object *content;
    std::string title;
    std::string message;
    std::list<PopupButtons> buttons;
    static void _response_cb(void *data, Evas_Object *obj, void *event_info);
    std::map<Evas_Object *, PopupButtons> addedButtons;
    std::shared_ptr<PopupData> popupData;
};

}

}

#endif //__SIMPLE_POPUP_H__