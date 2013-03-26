/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CUSTOM_USER_AGENT_H
#define CUSTOM_USER_AGENT_H

#include "browser-object.h"
#include "common-view.h"

class custom_user_agent_view : public common_view {
public:
	custom_user_agent_view(void);
	~custom_user_agent_view(void);

	Eina_Bool show(void);
private:
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_genlist(Evas_Object *parent);
	Evas_Object *_create_edit_field(Evas_Object *parent);

/* callback functions */
	static Evas_Object *__content_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_Info);
	static void __save_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_changed_cb(void *data, Evas_Object *obj, void *event_info);

	Elm_Genlist_Item_Class *m_item_ic;
	Evas_Object *m_editfield_layout;
	Evas_Object *m_save_button;
};

#endif /* CUSTOM_USER_AGENT_H */
