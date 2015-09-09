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

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <vector>
#include <AbstractMainWindow.h>

#include "MoreMenuUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "../Tools/BrowserImage.h"

#define efl_scale       (elm_config_scale_get() / elm_app_base_scale_get())

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(MoreMenuUI, "org.tizen.browser.moremenuui")

struct ItemData{
        tizen_browser::base_ui::MoreMenuUI * m_moreMenu;
        std::shared_ptr<tizen_browser::services::HistoryItem> h_item;
        Elm_Object_Item * e_item;
    };

typedef struct _MoreItemData
{
    ItemType item;
    std::shared_ptr<tizen_browser::base_ui::MoreMenuUI> moreMenuUI;
} MoreMenuItemData;

MoreMenuUI::MoreMenuUI()
    : m_gengrid(NULL)
    , m_parent(NULL)
    , m_item_class(NULL)
    , m_desktopMode(true)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("MoreMenuUI/MoreMenu.edj");
}

MoreMenuUI::~MoreMenuUI()
{
}

void MoreMenuUI::show(Evas_Object* parent, bool desktopMode)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_parent = parent;
    m_desktopMode = desktopMode;
    elm_theme_extension_add(NULL, m_edjFilePath.c_str());
    m_mm_layout = elm_layout_add(parent);
    elm_layout_file_set(m_mm_layout, m_edjFilePath.c_str(), "moremenu-layout");
    evas_object_size_hint_weight_set(m_mm_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_mm_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_mm_layout);

    m_gengrid = elm_gengrid_add(m_mm_layout);
    elm_object_part_content_set(m_mm_layout, "elm.swallow.grid", m_gengrid);

      if (!m_item_class) {
            m_item_class = elm_gengrid_item_class_new();
            m_item_class->item_style = "menu_item";
            m_item_class->func.text_get = _grid_text_get;
            m_item_class->func.content_get =  _grid_content_get;
            m_item_class->func.state_get = NULL;
            m_item_class->func.del = NULL;
        }

    elm_gengrid_align_set(m_gengrid, 0, 0);
    elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
    elm_gengrid_horizontal_set(m_gengrid, EINA_FALSE);
    elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_size_set(m_gengrid, 0, 327);
    evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_gengrid_item_size_set(m_gengrid, 364 * efl_scale, 320 * efl_scale);

    addItems();
}

void MoreMenuUI::showCurrentTab()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_current_tab_bar = elm_layout_add(m_mm_layout);
    elm_layout_file_set(m_current_tab_bar, m_edjFilePath.c_str(), "current_tab_layout");
    evas_object_size_hint_weight_set(m_current_tab_bar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_content_set(m_mm_layout, "current_tab_bar", m_current_tab_bar);

    Evas_Object* button = elm_button_add(m_current_tab_bar);
    elm_object_style_set(button, "hidden_button");
    evas_object_smart_callback_add(button, "clicked", _star_clicked, this);
    elm_object_part_content_set(m_current_tab_bar, "star_click", button);

    button = elm_button_add(m_current_tab_bar);
    elm_object_style_set(button, "hidden_button");
    evas_object_smart_callback_add(button, "clicked", _close_clicked, this);
    elm_object_part_content_set(m_current_tab_bar, "close_click", button);
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void MoreMenuUI::setFavIcon(std::shared_ptr<tizen_browser::tools::BrowserImage> favicon)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(favicon && favicon->imageType != tools::BrowserImage::ImageTypeNoImage) {
        if(m_icon)
            evas_object_del(m_icon);

        m_icon = tizen_browser::tools::EflTools::getEvasImage(favicon, m_current_tab_bar);
        if(m_icon) {
            evas_object_size_hint_weight_set(m_icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            evas_object_size_hint_align_set(m_icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
            elm_object_part_content_set(m_current_tab_bar, "favicon", m_icon);
            evas_object_show(m_icon);
        }
    }
    else {
        setDocIcon();
    }
}

void MoreMenuUI::setDocIcon()
{
    if(m_icon)
        evas_object_del(m_icon);

    m_icon = elm_icon_add(m_mm_layout);
    elm_image_file_set(m_icon, m_edjFilePath.c_str(), "ico_url.png");
    evas_object_size_hint_weight_set(m_icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_content_set(m_current_tab_bar, "favicon", m_icon);
}

void MoreMenuUI::setWebTitle(const std::string& title)
{
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, title.c_str());
    if(!title.empty())
        elm_object_part_text_set(m_current_tab_bar, "webpage_title", title.c_str());
    else
        elm_object_part_text_set(m_current_tab_bar, "webpage_title", "New Tab");
}

void MoreMenuUI::setURL(const std::string& url)
{
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, url.c_str());
    if(!url.empty())
        elm_object_part_text_set(m_current_tab_bar, "webpage_url", url.c_str());
    else
        elm_object_part_text_set(m_current_tab_bar, "webpage_url", "");
}

void MoreMenuUI::_star_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        MoreMenuUI *moreMenuUI = static_cast<MoreMenuUI*>(data);
        moreMenuUI->addToBookmarkClicked();
        moreMenuUI->AddBookmarkPopupCalled();
    }
}

void MoreMenuUI::_close_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        MoreMenuUI *moreMenuUI = static_cast<MoreMenuUI*>(data);
        moreMenuUI->closeMoreMenuClicked(std::string());
        moreMenuUI->clearItems();
    }
}

void MoreMenuUI::AddBookmarkPopupCalled()
{

    m_add_bookmark_popup = std::make_shared<tizen_browser::base_ui::AddBookmarkPopup>(m_mm_layout);
    m_add_bookmark_popup->show();
    m_add_bookmark_popup->addBookmarkFolderItems(m_map_bookmark_folder_list);
    m_add_bookmark_popup->folderSelected.disconnect_all_slots();
    m_add_bookmark_popup->folderSelected.connect(boost::bind(&MoreMenuUI::addToBookmarks, this, _1));
    m_add_bookmark_popup->addNewFolderClicked.disconnect_all_slots();
    m_add_bookmark_popup->addNewFolderClicked.connect(boost::bind(&MoreMenuUI::newFolderPopup, this,_1));
}

void MoreMenuUI::newFolderPopup(std::string)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_new_folder_popup =
        std::make_shared<tizen_browser::base_ui::NewFolderPopup>(m_mm_layout
                                                               , nullptr
                                                               , "Add New Folder for adding to Bookmark"
                                                               , "New Folder"
                                                               , "Add to bookmark"
                                                               , "Cancel");
   m_new_folder_popup->on_ok.disconnect_all_slots();
   m_new_folder_popup->on_ok.connect(boost::bind(&MoreMenuUI::NewFolderCreate, this, _1));
   m_new_folder_popup->on_cancel.disconnect_all_slots();
   m_new_folder_popup->on_cancel.connect(boost::bind(&MoreMenuUI::CancelClicked, this, _1));
   m_add_bookmark_popup->hide();
   m_new_folder_popup->show();
}

void MoreMenuUI::NewFolderCreate(Evas_Object* popup_content)
{
   BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
   if (popup_content) {
       m_folderName = elm_entry_entry_get(popup_content);
       BookmarkFolderCreated(m_folderName.c_str(), 0);
       m_new_folder_popup->hide();
   }
}

void MoreMenuUI::CancelClicked(Evas_Object*)
{
   BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
   m_new_folder_popup->hide();
}

void MoreMenuUI::addToBookmarks(int folder_id)
{
     AddBookmarkInput(folder_id);
     BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
     m_add_bookmark_popup->hide();
     m_add_bookmark_popup.reset();
}

void MoreMenuUI::hide()
{
    evas_object_hide(elm_layout_content_get(m_mm_layout, "elm.swallow.grid"));
    evas_object_hide(elm_layout_content_get(m_mm_layout, "current_tab_bar"));
    evas_object_hide(m_mm_layout);
}

void MoreMenuUI::addItems()
{
     BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
     for (int i = 0; i <= EXIT_BROWSER; i++) {
         ItemType type = static_cast<ItemType>(i);
         // take proper image for desktop/mobile view
         if (type == ItemType::VIEW_DESKTOP_WEB && m_desktopMode)
             continue;
         if (type == ItemType::VIEW_MOBILE_WEB && !m_desktopMode)
             continue;

         MoreMenuItemData *itemData = new MoreMenuItemData();
         itemData->item = type;
         itemData->moreMenuUI = std::shared_ptr<tizen_browser::base_ui::MoreMenuUI>(this);
         Elm_Object_Item* bookmarkView = elm_gengrid_item_append(m_gengrid, m_item_class, itemData, NULL, this);
         m_map_menu_views.insert(std::pair<ItemType, Elm_Object_Item*>(itemData->item, bookmarkView));
         elm_gengrid_item_selected_set(bookmarkView, EINA_FALSE);
     }
     BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
}

char* MoreMenuUI::_grid_text_get(void* data, Evas_Object*, const char* part)
{
    BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
    if (data && part) {
        MoreMenuItemData *itemData = static_cast<MoreMenuItemData*>(data);
        const char *part_name = "menu_label";
        static const int part_name_len = strlen(part_name);

        if (!strncmp(part_name, part, part_name_len)) {
            const char* item_name = NULL;
            switch (itemData->item) {
#ifdef READER_MODE_ENABLED
            case READER_MODE:
                item_name = "Reader mode";
                break;
#endif
            case BOOKMARK_MANAGER:
                item_name = "Bookmark manager";
                break;
            case HISTORY:
                item_name = "History";
                break;
            case SCREEN_ZOOM:
                item_name = "Screen zoom";
                break;
#ifdef START_MINIBROWSER_ENABLED
            case START_MINIBROWSER:
                item_name = "Start minibrowser";
                break;
#endif
            case FOCUS_MODE:
                item_name = "Focus mode";
                break;
            case VIEW_MOBILE_WEB:
                item_name = "View mobile web";
                break;
            case VIEW_DESKTOP_WEB:
                item_name = "View desktop web";
                break;
            case SHARE:
                item_name = "Share";
                break;
            case SETTINGS:
                item_name = "Settings";
                break;
            case EXIT_BROWSER:
                item_name = "Exit browser";
                break;
            default:
                item_name = "";
            }
            return strdup(item_name);
        }
    }
    return NULL;
}

static const char* getImageFileNameForType(ItemType type, bool focused)
{
    const char* file_name = NULL;
    switch (type) {
#ifdef READER_MODE_ENABLED
    case READER_MODE:
        file_name = focused ? "ic_more_readermode_foc.png" : "ic_more_readermode_nor.png";
        break;
#endif
    case BOOKMARK_MANAGER:
        file_name = focused ? "ic_more_bookmark_foc.png" : "ic_more_bookmark_nor.png";
        break;
    case HISTORY:
        file_name = focused ? "ic_more_history_foc.png" : "ic_more_history_nor.png";
        break;
    case SCREEN_ZOOM:
        file_name = focused ? "ic_more_zoom_foc.png" : "ic_more_zoom_nor.png";
        break;
#ifdef START_MINIBROWSER_ENABLED
    case START_MINIBROWSER:
        file_name = focused ? "ic_more_minibrowser_foc.png" : "ic_more_minibrowser_nor.png";
        break;
#endif
    case FOCUS_MODE:
        file_name = focused ? "ic_more_focusmode_foc.png" : "ic_more_focusmode_nor.png";
        break;
    case VIEW_MOBILE_WEB:
        file_name = focused ? "ic_more_mobileview_foc.png" : "ic_more_mobileview_nor.png";
        break;
    case VIEW_DESKTOP_WEB:
        file_name = focused ? "ic_more_desktopview_foc.png" : "ic_more_desktopview_nor.png";
        break;
    case SHARE:
        file_name = focused ? "ic_more_share_foc.png" : "ic_more_share_nor.png";
        break;
    case SETTINGS:
        file_name = focused ? "ic_more_setting_foc.png" : "ic_more_setting_nor.png";
        break;
    case EXIT_BROWSER:
        file_name = focused ? "ic_more_exit_foc.png" : "ic_more_exit_nor.png";
        break;
    default:
        file_name = "";
    }
    return file_name;
}

Evas_Object * MoreMenuUI::_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
    if (data && obj && part) {
        MoreMenuItemData *itemData = static_cast<MoreMenuItemData*>(data);
        const char *part_name1 = "thumbnail_item";
        static const int part_name1_len = strlen(part_name1);
        const char *part_name2 = "thumbbutton_item";
        static const int part_name2_len = strlen(part_name2);

        if (!strncmp(part_name1, part, part_name1_len)) {
            Evas_Object* thumb_nail = elm_icon_add(obj);
            const char* file_name = getImageFileNameForType(itemData->item, false);
            elm_image_file_set(thumb_nail, itemData->moreMenuUI->m_edjFilePath.c_str(), file_name);
            return thumb_nail;
        }

        if (!strncmp(part_name2, part, part_name2_len)) {
            Evas_Object *thumbButton = elm_button_add(obj);
            elm_object_style_set(thumbButton, "clickButton");
            evas_object_smart_callback_add(thumbButton, "clicked", tizen_browser::base_ui::MoreMenuUI::_thumbSelected, data);
            evas_object_event_callback_add(thumbButton, EVAS_CALLBACK_MOUSE_IN, __cb_mouse_in, data);
            evas_object_event_callback_add(thumbButton, EVAS_CALLBACK_MOUSE_OUT, __cb_mouse_out, data);
            return thumbButton;
        }
    }
    return NULL;
}

void MoreMenuUI::__cb_mouse_in(void * data, Evas *, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj) {
        elm_object_focus_set(obj, EINA_TRUE);

        MoreMenuItemData *itemData = static_cast<MoreMenuItemData*>(data);
        const char *file_name = getImageFileNameForType(itemData->item, true);
        Elm_Object_Item *selected = itemData->moreMenuUI->m_map_menu_views[itemData->item];
        Evas_Object *thumb_nail = elm_object_item_part_content_get(selected, "thumbnail_item");
        elm_image_file_set(thumb_nail, itemData->moreMenuUI->m_edjFilePath.c_str(), file_name);
    }
}

void MoreMenuUI::__cb_mouse_out(void * data, Evas *, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj) {
        elm_object_focus_set(obj, EINA_FALSE);

        MoreMenuItemData *itemData = static_cast<MoreMenuItemData*>(data);
        const char *file_name = getImageFileNameForType(itemData->item, false);
        Elm_Object_Item *selected = itemData->moreMenuUI->m_map_menu_views[itemData->item];
        Evas_Object *thumb_nail = elm_object_item_part_content_get(selected, "thumbnail_item");
        elm_image_file_set(thumb_nail, itemData->moreMenuUI->m_edjFilePath.c_str(), file_name);
    }
}

void MoreMenuUI::_thumbSelected(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        MoreMenuItemData *itemData = static_cast<MoreMenuItemData*>(data);
    BROWSER_LOGD("type: %d", itemData->item);
        switch (itemData->item) {
        case HISTORY:
            itemData->moreMenuUI->historyUIClicked(std::string());
            break;
        case SETTINGS:
            itemData->moreMenuUI->settingsClicked(std::string());
            break;
        case BOOKMARK_MANAGER:
            itemData->moreMenuUI->bookmarkManagerClicked(std::string());
            break;
#ifdef READER_MODE_ENABLED
        case READER_MODE:
            //TODO: Implement reader mode
            break;
#endif
        case SCREEN_ZOOM:
            break;
#ifdef START_MINIBROWSER_ENABLED
        case START_MINIBROWSER:
            //TODO: Implement minibrowser launching
            break;
#endif
        case FOCUS_MODE:
            break;
        case VIEW_MOBILE_WEB:
            itemData->moreMenuUI->switchToMobileMode();
            itemData->moreMenuUI->m_desktopMode = false;
            itemData->moreMenuUI->closeMoreMenuClicked(std::string());
            itemData->moreMenuUI->clearItems();
            break;
        case VIEW_DESKTOP_WEB:
            itemData->moreMenuUI->switchToDesktopMode();
            itemData->moreMenuUI->m_desktopMode = true;
            itemData->moreMenuUI->closeMoreMenuClicked(std::string());
            itemData->moreMenuUI->clearItems();
            break;
        case SHARE:
            break;
        case EXIT_BROWSER:
            _exitClicked();
            break;
        }
    }
}

void MoreMenuUI::getBookmarkFolderList(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > folderList)
{
    m_map_bookmark_folder_list = folderList;
}

void MoreMenuUI::clearItems()
{
    hide();
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_gengrid_clear(m_gengrid);
    m_map_menu_views.clear();
    m_map_bookmark_folder_list.clear();
    evas_object_del(m_current_tab_bar);
    elm_theme_extension_del(NULL, m_edjFilePath.c_str());
    elm_theme_full_flush();
    elm_cache_all_flush();
}

void MoreMenuUI::_exitClicked()
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    elm_exit();
}

}
}
