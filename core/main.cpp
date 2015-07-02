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
#include "browser_config.h"

#include "BrowserLogger.h"
#include <Ecore.h>
#include <Edje.h>
#include <Elementary.h>
#include <stdexcept>

#if PLATFORM(TIZEN)
#include <app.h>
#endif

#include <ewk_chromium.h>

// for tests...
#include "Lifecycle.h"
#include "ServiceManager.h"
#include "BasicUI/AbstractMainWindow.h"

///\note Odroid platform modification
#if PLATFORM(TIZEN)
const std::string DEFAULT_URL = "";

static bool app_create(void * /*app_data*/)
{
//    elm_config_accel_preference_set("opengl");
	elm_config_accel_preference_set("3d");

    elm_config_focus_move_policy_set(ELM_FOCUS_MOVE_POLICY_CLICK);
    // Enabling focus
    elm_config_focus_highlight_enabled_set(EINA_TRUE);

    /// \todo: clean casts, depends on ServiceManager
    std::shared_ptr<tizen_browser::base_ui::AbstractMainWindow<Evas_Object>> mainUi =
    std::dynamic_pointer_cast
    <
        tizen_browser::base_ui::AbstractMainWindow<Evas_Object>,
        tizen_browser::core::AbstractService
    >
    (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.simpleui"));

    //mainUi->exec("");
    return true;
}

static void app_terminate(void */*app_data*/)
{
    BROWSER_LOGD("%s\n", __func__);
    std::shared_ptr<tizen_browser::base_ui::AbstractMainWindow<Evas_Object>> mainUi =
    std::dynamic_pointer_cast
    <
        tizen_browser::base_ui::AbstractMainWindow<Evas_Object>,
        tizen_browser::core::AbstractService
    >
    (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.simpleui"));
    mainUi->destroyUI();
}

static void app_service(service_h service, void */*app_data*/){
    /* to test this functionality please use aul_test command on target:
     *  $aul_test org.tizen.browser __APP_SVC_URI__ <http://full.url.com/>
     */
    BROWSER_LOGD("%s\n", __func__);

    char *operation = NULL;
    char *request_uri = NULL;
    char *request_mime_type = NULL;

    if (service_get_operation(service, &operation) != SERVICE_ERROR_NONE) {
        BROWSER_LOGD("get service operation failed");
        return;
    }

    if (service_get_uri(service, &request_uri) != SERVICE_ERROR_NONE)
        BROWSER_LOGD("get service uri failed");

    if (service_get_mime(service, &request_mime_type) != SERVICE_ERROR_NONE)
        BROWSER_LOGD("get service mime failed");


    BROWSER_LOGD("operation = [%s], request_uri = [%s], request_mime_type = [%s]"
            , operation, request_uri, request_mime_type);

    std::string uri = request_uri != NULL ? std::string(request_uri) : DEFAULT_URL;

    BROWSER_LOGD("[%s] uri=%s", __func__, uri.c_str());
    free(request_uri);
    free(request_mime_type);
    free(operation);

    std::shared_ptr<tizen_browser::base_ui::AbstractMainWindow<Evas_Object>> mainUi =
    std::dynamic_pointer_cast
    <
        tizen_browser::base_ui::AbstractMainWindow<Evas_Object>,
        tizen_browser::core::AbstractService
    >
    (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.simpleui"));

    mainUi->exec(uri);

    evas_object_show(mainUi->getMainWindow().get());
    elm_win_activate(mainUi->getMainWindow().get());
}

static void app_pause(void *){
    BROWSER_LOGD("%s", __PRETTY_FUNCTION__);
}

static void app_resume(void *){
    BROWSER_LOGD("%s", __PRETTY_FUNCTION__);
}

int main(int argc, char* argv[])try
{
    BEGIN()
    ewk_init();
    setenv("ELM_PROFILE", "tv", true);

//#if !defined(NDEBUG)
    //Initialization of logger module
    tizen_browser::logger::Logger::getInstance().init();
    tizen_browser::logger::Logger::getInstance().setLogTag("browser");
//#endif

	BROWSER_LOGD("BROWSER IS SAYING HELLO");
	BROWSER_LOGD("BROWSER TAG: %s",tizen_browser::logger::Logger::getInstance().getLogTag().c_str());
	BROWSER_LOGD("BROWSER REGISTERED LOGGERS COUNT: %d",tizen_browser::logger::Logger::getInstance().getLoggersCount());

    app_event_callback_s ops;
    memset(&ops, 0x00, sizeof(app_event_callback_s));

    ops.create = app_create;
    ops.terminate = app_terminate;
    ops.service = app_service;

    ops.pause = app_pause;
    ops.resume = app_resume;

    app_efl_main(&argc, &argv, &ops, NULL);

    ewk_shutdown();
    END()

} catch (std::exception & e)
{
    std::cerr << "UNHANDLED EXCEPTION " << e.what() << std::endl;
} catch (...)
{
    std::cerr << "UNHANDLED EXCEPTION" << std::endl;
}
#else
int main(int argc, char* argv[]) try
{
    BEGIN()
    elm_init(argc, argv);

#if PLATFORM(TIZEN)
    elm_config_focus_move_policy_set(ELM_FOCUS_MOVE_POLICY_CLICK);
    // Enabling focus
    elm_config_focus_highlight_enabled_set(EINA_TRUE);
#endif

    tizen_browser::logger::Logger::getInstance().init();
    tizen_browser::logger::Logger::getInstance().setLogTag("browser");

    std::shared_ptr<tizen_browser::base_ui::AbstractMainWindow<Evas_Object>> mainUi =
    std::dynamic_pointer_cast
    <
        tizen_browser::base_ui::AbstractMainWindow<Evas_Object>,
        tizen_browser::core::AbstractService
    >
    (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.simpleui"));

    if (mainUi) {
        evas_object_show(mainUi->getMainWindow().get());
        mainUi->exec("http://enlightenment.org");
    }

    elm_run();

    elm_shutdown();
    END()
} catch (std::exception & e)
{
    std::cerr << "UNHANDLED EXCEPTION " << e.what() << std::endl;
} catch (...)
{
    std::cerr << "UNHANDLED EXCEPTION" << std::endl;
}
#endif