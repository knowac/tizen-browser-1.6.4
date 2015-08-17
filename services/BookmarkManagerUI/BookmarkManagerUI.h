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

#ifndef BOOKMARKMANAGERUI_H
#define BOOKMARKMANAGERUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "services/HistoryService/HistoryItem.h"
#include "BookmarkItem.h"
#include "AddNewFolderPopup.h"

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT BookmarkManagerUI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
{
public:
    BookmarkManagerUI();
    ~BookmarkManagerUI();
    void show(Evas_Object *main_layout);
    Evas_Object *getContent();
    Evas_Object *getGenList();
    void showTopContent();
    virtual std::string getName();
    void addBookmarkFolderItem(std::shared_ptr<tizen_browser::services::BookmarkItem>);
    void addBookmarkFolderItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> >);
    void addBookmarkItem(std::shared_ptr<tizen_browser::services::BookmarkItem>);
    void addBookmarkItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> >);
    void hide();
    void clearItems();
    void set_folder(const std::string &name) {m_folder = name;};
    void updateGengrid();
    void newFolderPopup();
    Evas_Object* createNoHistoryLabel();
    void setEmptyGengrid(bool setEmpty);

    boost::signals2::signal<void (std::string)> closeBookmarkManagerClicked;
    boost::signals2::signal<void (std::string)> addNewFolderClicked;
    boost::signals2::signal<void (const char*, int, int)> saveFolderClicked;
    boost::signals2::signal<void (int)> bookmarkFolderClicked;

private:
    static char* _grid_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _grid_content_get(void *data, Evas_Object *obj, const char *part);
    static char* _grid_bookmark_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _grid_bookmark_content_get(void *data, Evas_Object *obj, const char *part);

    static void _itemSelected(void * data, Evas_Object * obj, void * event_info);
    static void _thumbSelected(void * data, Evas_Object * obj, void * event_info);
    static void _bookmark_thumbSelected(void * data, Evas_Object *, void *);

    void NewFolderCreate(Evas_Object * popup_content);
    void CancelClicked(Evas_Object * popup_content);

    static Evas_Object* listItemContentGet(void *data, Evas_Object *obj, const char *part);
    static char*        listItemTextGet(void *data, Evas_Object *, const char *part);

    static void item_clicked_cb(void *, Evas_Object *, void *);
    static void close_clicked_cb(void *data, Evas_Object *, void *);
    static void new_folder_clicked_cb(void *data, Evas_Object *, void *);

private:
    std::string m_folderName;
    Evas_Object *m_genList;
    Evas_Object *popup_content;
    std::shared_ptr<tizen_browser::base_ui::AddNewFolderPopup > m_popup;
    Evas_Object *b_mm_layout;
    Elm_Genlist_Item_Class *m_itemClass;
    Evas_Object *m_gengrid;
    Evas_Object *m_parent;
    Elm_Gengrid_Item_Class * m_item_class;
    Elm_Gengrid_Item_Class * m_detail_item_class;
    std::map<std::string,Elm_Object_Item*> m_map_bookmark_folder_views;
    std::string edjFilePath;
    std::string m_folder;
    bool m_gengridSetup;

    static void focusItem(void*, Evas_Object*, void* event_info);
    static void unFocusItem(void*, Evas_Object*, void* event_info);
};

}
}

#endif // BOOKMARKSUI_H