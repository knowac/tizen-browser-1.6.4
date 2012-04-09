/*
Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved

This file is part of browser
Written by Hyerim Bae hyerim.bae@samsung.com.

PROPRIETARY/CONFIDENTIAL

This software is the confidential and proprietary information of
SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered
into with SAMSUNG ELECTRONICS.

SAMSUNG make no representations or warranties about the suitability
of the software, either express or implied, including but not limited
to the implied warranties of merchantability, fitness for a particular
purpose, or non-infringement. SAMSUNG shall not be liable for any
damages suffered by licensee as a result of using, modifying or
distributing this software or its derivatives.
*/

#ifndef BROWSER_COMMON_VIEW_H
#define BROWSER_COMMON_VIEW_H

#include "browser-config.h"
#include "browser-data-manager.h"
#include "browser-utility.h"

class Browser_Data_Manager;
class Browser_Class;
class Browser_Window;

class Browser_Common_View {
public:
	Browser_Common_View(void);
	~Browser_Common_View(void);

	virtual Eina_Bool init(void) = 0;

	Eina_Bool is_landscape(void);
	Browser_Data_Manager *get_data_manager(void) { return m_data_manager; }
	Browser_Class *get_browser(void) { return m_browser; }
	void show_msg_popup(const char *msg, int timeout = 3);
	void show_msg_popup(const char *title, const char *msg, int timeout);
	void show_notify_popup(const char *msg, int timeout = 0, Eina_Bool has_control_bar = EINA_FALSE);
	void hide_notify_popup(void);
	std::string get_domain_name(const char *url);
	Eina_Bool show_modal_popup(const char *msg);
protected:
	Evas_Object *_capture_snapshot(Browser_Window *window, float scale);
	void _set_navigationbar_title(const char *title);
	Eina_Bool _has_url_sheme(const char *url);
	Eina_Bool _launch_streaming_player(const char *url, const char *cookie = NULL);


	/* set focus to edit field idler callback to show ime. */
	static Eina_Bool __set_focus_editfield_idler_cb(void *edit_field);

	char *_trim(char *str);
	/* For ug effect. */
	static void __ug_layout_cb(struct ui_gadget *ug, enum ug_mode mode, void *priv);
	static void __ug_result_cb(struct ui_gadget *ug, bundle *result, void *priv);
	static void __ug_destroy_cb(struct ui_gadget *ug, void *priv);

	static Browser_Data_Manager *m_data_manager;
	static Evas_Object *m_win;
	static Evas_Object *m_navi_bar;
	static Evas_Object *m_bg;
	static Evas_Object *m_layout;
	static Browser_Class *m_browser;

	std::vector<std::string> m_sns_name_list;
	std::vector<std::string> m_sns_path_list;
	std::vector<Evas_Object *> m_sns_icon_list;
private:
	Evas_Object *m_popup;
	Evas_Object *m_selection_info;
	Evas_Object *m_selection_info_layout;
	Evas_Object *m_share_popup;
	Evas_Object *m_share_list;
	std::string m_share_url;

	struct ui_gadget *m_ug;
};
#endif /* BROWSER_COMMON_VIEW_H */

