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

#ifndef __DETAIL_POPUP_H__
#define __DETAIL_POPUP_H__

#include <Evas.h>
#include <string>
#include <memory>
#include <boost/signals2/signal.hpp>
#include "services/HistoryService/HistoryItem.h"

namespace tizen_browser{
namespace base_ui{

    class MainUI;

    class DetailPopup {
    public:
        DetailPopup(MainUI *mainUI);
        ~DetailPopup();

        /**
        * @brief Show popup window
        */
        void show(Evas_Object *parent, std::shared_ptr<services::HistoryItem> currItem, std::shared_ptr<services::HistoryItemVector> prevItems);

        boost::signals2::signal<void (std::shared_ptr<services::HistoryItem>)> openURLInNewTab;

        static const int HISTORY_ITEMS_NO;
    private:
        /**
        * @brief Mouse background click callback
        */
        static void _bg_click(void *data, Evas_Object *obj, const char *emission, const char *source);

        /**
        * @brief URL rectangle click callback
        */
        static void _url_click(void *data, Evas_Object *obj, const char *emission, const char *source);

        /**
        * @brief Hide popup
        */
        void hide();

        void createLayout(Evas_Object *parent);

        /**
        * @brief Provide texts for history genlist
        */
        static char* _get_history_link_text(void *data, Evas_Object *obj, const char *part);

        Evas_Object *m_layout;
        Evas_Object *m_historyList;
        Elm_Gengrid_Item_Class * m_history_item_class;
        MainUI *m_mainUI;
        std::string edjFilePath;
        std::shared_ptr<services::HistoryItem> m_item;
        std::shared_ptr<services::HistoryItemVector> m_prevItems;
    };

}
}

#endif // __DETAIL_POPUP_H__
