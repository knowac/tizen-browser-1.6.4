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

#ifndef TABUI_H
#define TABUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractContextMenu.h"
#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "TabIdTypedef.h"
#include "AbstractRotatable.h"
#include "NaviframeWrapper.h"

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT TabUI
        : public tizen_browser::interfaces::AbstractContextMenu
        , public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
        , public tizen_browser::interfaces::AbstractRotatable
{
public:
    TabUI();
    ~TabUI();
    //AbstractUIComponent interface implementation
    void showUI();
    void hideUI();
    void init(Evas_Object *parent);
    Evas_Object* getContent();

    virtual std::string getName();

    void addTabItems(std::vector<basic_webengine::TabContentPtr>& items);
    virtual void orientationChanged() override;

    //AbstractContextMenu interface implementation
    virtual void showContextMenu() override;

    boost::signals2::signal<void (const tizen_browser::basic_webengine::TabId&)> tabClicked;
    boost::signals2::signal<void ()> newTabClicked;
    boost::signals2::signal<void (const tizen_browser::basic_webengine::TabId&)> closeTabsClicked;
    boost::signals2::signal<void ()> closeTabUIClicked;

private:
    struct TabData
    {
        basic_webengine::TabContentPtr item;
        std::shared_ptr<tizen_browser::base_ui::TabUI> tabUI;
    };

    static char* _gengrid_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _gengrid_content_get(void *data, Evas_Object *obj, const char *part);

    static void _gengrid_tab_pressed(void * data, Evas_Object * obj, void * event_info);
    static void _gengrid_tab_released(void * data, Evas_Object * obj, void * event_info);
    static void _gengrid_tab_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _close_clicked(void *data, Evas_Object *obj, void *event_info);
    void updateNoTabsText();

    static void _openedtabs_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _new_tab_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _newincognitotab_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _enable_secret_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _close_all_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _close_tab_clicked(void *data, Evas_Object*, void*);
    static void _cm_sync_clicked(void*, Evas_Object*, void*);
    static void _cm_secret_clicked(void*, Evas_Object*, void*);
    static void _cm_close_clicked(void*, Evas_Object*, void*);
    static Evas_Event_Flags _gesture_occured(void * data, void * event_info);

    SharedNaviframeWrapper m_naviframe;
    void createTabUILayout();
    void createTopContent();
    void createBottomContent();
    void createEmptyLayout();
    void createGengrid();
    void createTabItemClass();
    void closeAllTabs();
    void addTabItem(basic_webengine::TabContentPtr);

    Evas_Object *m_parent;
    Evas_Object *m_content;
    Evas_Object *m_gengrid;
    Evas_Object *m_empty_layout;

    Elm_Object_Item* m_itemToShow;
    Elm_Object_Item* m_last_pressed_gengrid_item;

    Elm_Gengrid_Item_Class * m_item_class;
    std::string m_edjFilePath;

    const unsigned int GENGRID_ITEM_WIDTH = 700;
    const unsigned int GENGRID_ITEM_HEIGHT = 312;
    const unsigned int GENGRID_ITEM_WIDTH_LANDSCAPE = 636;
    const unsigned int GENGRID_ITEM_HEIGHT_LANDSCAPE = 274;
    const unsigned int GESTURE_MOMENTUM_MIN = 2000;
};
}
}

#endif // BOOKMARKSUI_H
