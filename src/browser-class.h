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

#ifndef BROWSER_CLASS_H
#define BROWSER_CLASS_H

#include "browser-config.h"

class Browser_Authetication_Manager;
class Browser_Certificate_Manager;
class Browser_Download_Manager;
class Browser_Network_Manager;
class Browser_Notification_Manager;
class Browser_User_Agent_DB;
class Browser_View;
class Browser_Window;

class Browser_Class {
public:
	Browser_Class(Evas_Object *win, Evas_Object *navi_bar, Evas_Object *bg, Evas_Object *layout);
	~Browser_Class(void);

	Eina_Bool init(void);
	Eina_Bool launch(const char *url, Eina_Bool new_window = EINA_FALSE);

	Browser_Window *create_new_window(Eina_Bool created_by_user = EINA_FALSE);
	void delete_window(Browser_Window *delete_window, Browser_Window *parent);
	void delete_window(Browser_Window *window);
	void change_order(std::vector<Browser_Window *> window_list);
	std::vector<Browser_Window *> get_window_list(void) { return m_window_list; }
	void set_focused_window(Browser_Window *window, Eina_Bool show_most_visited_sites = EINA_TRUE);
	void pause(void);
	void resume(void);
	void reset(void);
	void ewk_view_init(Evas_Object *ewk_view);
	void ewk_view_deinit(Evas_Object *ewk_view);
	std::string get_user_agent(void);
	void clean_up_windows(void);
	Browser_Window *create_deleted_window(int index);
	Browser_Certificate_Manager *get_certificate_manager(void) { return m_certificate_manager; }
private:
	Eina_Bool _set_ewk_view_options(Evas_Object *ewk_view);
	Eina_Bool _set_http_accepted_language_header(Evas_Object *ewk_view);
	Eina_Bool _set_ewk_view_options_listener(void);
	Eina_Bool _set_user_agent(Evas_Object *ewk_view);

	/* vconf changed callback functions */
	static void __vconf_changed_cb(keynode_t *keynode, void *data);

	/* Ecore timer callback functions */
	static Eina_Bool __clean_up_windows_timer_cb(void *data);

	Evas_Object *m_win;
	Evas_Object *m_navi_bar;
	Evas_Object *m_bg;
	Evas_Object *m_layout;

	Browser_View *m_browser_view;
	Browser_Window *m_focused_window;
	Browser_User_Agent_DB *m_user_agent_db;
	Browser_Download_Manager *m_download_manager;
	Browser_Authetication_Manager *m_authentication_manager;
	Browser_Certificate_Manager *m_certificate_manager;
	Browser_Notification_Manager *m_notification_manager;
	Browser_Network_Manager *m_network_manager;

	std::vector<Browser_Window *> m_window_list;
	Ecore_Timer *m_clean_up_windows_timer;
};
#endif /* BROWSER_CLASS_H */

