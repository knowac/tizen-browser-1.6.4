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

#ifndef BOOKMARKSUI_H
#define BOOKMARKSUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "BookmarkItem.h"

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT BookmarksUI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
{
public:
    BookmarksUI();
    ~BookmarksUI();
    void init(Evas_Object *main_layout);
    Evas_Object *getContent();
    virtual std::string getName();
    void addBookmarkItem(std::shared_ptr<tizen_browser::services::BookmarkItem>);
    void addBookmarkItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> >);
    void removeBookmarkItem(const std::string& uri);
    void deleteAllItems();
    std::shared_ptr<tizen_browser::services::BookmarkItem> getSelectedBookmarkItem();


    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> bookmarkClicked;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> bookmarkDeleteClicked;
private:
    static Evas_Object * _grid_content_get(void *data, Evas_Object *obj, const char *part);
    static void _itemSelected(void * data, Evas_Object * obj, void * event_info);
    static void _item_deleted(void *data, Evas_Object *obj);
    static void _thumbSelected(void * data, Evas_Object * obj, void * event_info);
    static void _deleteBookmark(void *data, Evas_Object *obj, void *event_info);
    void setEmptyGengrid(bool setEmpty);

private:
    Evas_Object *m_gengrid;
    Evas_Object *m_parent;
    Elm_Gengrid_Item_Class * m_item_class;
    std::map<std::string,Elm_Object_Item*> m_map_bookmark_views;
    bool m_gengridSetup;
    Evas_Object *createNoBookmarksLabel();

    static void focusItem(void* data, Evas_Object* obj, void* event_info);
    static void unFocusItem(void* data, Evas_Object* obj, void* event_info);
};


}
}

#endif // BOOKMARKSUI_H