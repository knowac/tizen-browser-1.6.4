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

#if defined(USE_EWEBKIT)
#include <ewk_chromium.h>
#endif

#include <boost/any.hpp>
#include <memory>
#include <algorithm>
#include <Elementary.h>
#include <Ecore.h>
#include <Ecore_Wayland.h>
#include <Edje.h>
#include <Evas.h>
#include "Config.h"

#include "BrowserLogger.h"
#include "ServiceManager.h"
#include "AbstractWebEngine.h"
#include "TabId.h"
#include "Tools/EflTools.h"
#include "BrowserImage.h"
#include "SimpleUI.h"
#include "BookmarkItem.h"
#include "Tools/EflTools.h"
#include "BrowserImage.h"
#include "HistoryItem.h"
#include "BookmarkItem.h"
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "SqlStorage.h"
#include "DetailPopup.h"


namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(SimpleUI, "org.tizen.browser.simpleui")

const std::string HomePageURL = "about:home";
const int ROOT_FOLDER = 0;

SimpleUI::SimpleUI()
    : AbstractMainWindow()
    , m_popup(nullptr)
    , m_moreMenuUI()
    , m_tabUI()
    , m_bookmarkManagerUI()
    , m_mainUI()
    , m_initialised(false)
    , items_vector()
    , m_networkErrorPopup(0)
    , m_wvIMEStatus(false)
    , m_ewkContext(ewk_context_new())
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_init(0, nullptr);
    Evas_Object *main_window = elm_win_util_standard_add("browserApp", "browserApp");
    if (main_window == nullptr)
        BROWSER_LOGE("Failed to create main window");

    setMainWindow(main_window);
}

SimpleUI::~SimpleUI() {
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_sessionService->getStorage()->deleteSession(m_currentSession);
    /// \todo Auto-generated destructor stub
    evas_object_del(m_window.get());
    ewk_context_delete(m_ewkContext);
}

void SimpleUI::destroyUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_del(m_window.get());
}

std::string SimpleUI::edjePath(const std::string &file)
{
    return std::string(EDJE_DIR) + file;
}

std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > SimpleUI::getBookmarks(int folder_id)
{
    return m_favoriteService->getBookmarks(folder_id);
}

std::shared_ptr<services::HistoryItemVector> SimpleUI::getMostVisitedItems()
{
    return m_historyService->getMostVisitedHistoryItems();
}

std::shared_ptr<services::HistoryItemVector> SimpleUI::getHistory()
{
    return m_historyService->getHistoryToday();
}

int SimpleUI::exec(const std::string& _url)
{
    BROWSER_LOGD("[%s] _url=%s, initialised=%d", __func__, _url.c_str(), m_initialised);
    std::string url = _url;

    if(!m_initialised){
        if (m_window.get()) {
            config::DefaultConfig config;
            config.load("");
            m_tabLimit = boost::any_cast <int> (config.get("TAB_LIMIT"));
            m_favoritesLimit = boost::any_cast <int> (config.get("FAVORITES_LIMIT"));

            // Set up main window
            //TODO: These functions seems redundant. Check if these functions are neccessary.
            int width = 0;
            int height = 0;
            ecore_wl_screen_size_get(&width, &height);
            evas_object_move(m_window.get(), 0, 0);
            evas_object_resize(m_window.get(), width, height);

            elm_win_focus_highlight_style_set(m_window.get(), "invisible");

            //set global show tooltip timeout
            elm_config_tooltip_delay_set( boost::any_cast <double> (config.get("TOOLTIP_DELAY")));

            loadUIServices();
            loadModelServices();
            createActions();

            // initModelServices() needs to be called after initUIServices()
            initUIServices();
            initModelServices();

            // create view layouts
            elm_win_resize_object_add(m_window.get(), m_webPageUI->getContent());

            connectModelSignals();
            connectUISignals();
            connectActions();

            // show main layout and window
            evas_object_show(m_webPageUI->getContent());
            evas_object_show(m_window.get());
        }
        m_initialised = true;
    }

    m_currentSession = std::move(m_sessionService->getStorage()->createSession());

    if (url.empty())
    {
        BROWSER_LOGD("[%s]: changing to homeUrl", __func__);
        switchViewToQuickAccess();
        restoreLastSession();
    } else {
        openNewTab(url);
    }
    m_webPageUI->getURIEntry().setFocus();

    BROWSER_LOGD("[%s]:%d url=%s", __func__, __LINE__, url.c_str());
    return 0;
}

void SimpleUI::restoreLastSession()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_sessionService);
    Session::Session lastSession = std::move(m_sessionService->getStorage()->getLastSession());
    if(lastSession.items().size() >= 1)
    {
        for(auto iter=lastSession.items().begin(), end=lastSession.items().end(); iter != end; ++iter)
        {
            openNewTab(iter->second);
        }
        m_sessionService->getStorage()->deleteSession(lastSession);
    }
}


//TODO: Move all service creation here:
void SimpleUI::loadUIServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_webPageUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::WebPageUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webpageui"));

    m_mainUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::MainUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.mainui"));
}

void SimpleUI::connectUISignals()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    M_ASSERT(m_webPageUI.get());
    m_webPageUI->getURIEntry().uriChanged.connect(boost::bind(&SimpleUI::filterURL, this, _1));

    M_ASSERT(m_mainUI.get());
    m_mainUI->getDetailPopup().openURLInNewTab.connect(boost::bind(&SimpleUI::onOpenURLInNewTab, this, _1, _2));
    m_mainUI->openURLInNewTab.connect(boost::bind(&SimpleUI::onOpenURLInNewTab, this, _1, _2));
    m_mainUI->mostVisitedTileClicked.connect(boost::bind(&SimpleUI::onMostVisitedTileClicked, this, _1, _2));
    m_mainUI->mostVisitedClicked.connect(boost::bind(&SimpleUI::onMostVisitedClicked, this,_1));
    m_mainUI->bookmarkClicked.connect(boost::bind(&SimpleUI::onBookmarkButtonClicked, this,_1));
    m_mainUI->bookmarkManagerClicked.connect(boost::bind(&SimpleUI::onBookmarkManagerButtonClicked, this,_1));
}

void SimpleUI::loadModelServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_webEngine =
        std::dynamic_pointer_cast
        <basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));

    m_favoriteService =
        std::dynamic_pointer_cast
        <tizen_browser::interfaces::AbstractFavoriteService,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.favoriteservice"));

    m_historyService =
        std::dynamic_pointer_cast
        <tizen_browser::services::HistoryService,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.historyservice"));

    m_platformInputManager =
        std::dynamic_pointer_cast
        <tizen_browser::services::PlatformInputManager,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.platforminputmanager"));

    m_sessionService = std::dynamic_pointer_cast
        <tizen_browser::services::SessionStorage,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.sessionStorageService"));
}

void SimpleUI::initUIServices()
{
    m_webPageUI->init(m_window.get());
    m_mainUI->init(m_webPageUI->getContent());
}

void SimpleUI::initModelServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    M_ASSERT(m_webEngine);
    M_ASSERT(m_webPageUI->getContent());
    m_webEngine->init(m_webPageUI->getContent());

    M_ASSERT(m_favoriteService);
    m_favoriteService->synchronizeBookmarks();
    m_favoriteService->getBookmarks();

    M_ASSERT(m_platformInputManager);
    m_platformInputManager->init(m_window.get());
}

void SimpleUI::connectModelSignals()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_webEngine->uriChanged.connect(boost::bind(&SimpleUI::webEngineURLChanged, this, _1));
    m_webEngine->uriChanged.connect(boost::bind(&URIEntry::changeUri, &m_webPageUI->getURIEntry(), _1));
    m_webEngine->uriOnTabChanged.connect(boost::bind(&SimpleUI::checkTabId,this,_1));
    m_webEngine->webViewClicked.connect(boost::bind(&URIEntry::clearFocus, &m_webPageUI->getURIEntry()));
    m_webEngine->backwardEnableChanged.connect(boost::bind(&SimpleUI::backEnable, this, _1));
    m_webEngine->forwardEnableChanged.connect(boost::bind(&SimpleUI::forwardEnable, this, _1));
    m_webEngine->loadStarted.connect(boost::bind(&SimpleUI::loadStarted, this));
    m_webEngine->loadProgress.connect(boost::bind(&SimpleUI::progressChanged,this,_1));
    m_webEngine->loadFinished.connect(boost::bind(&SimpleUI::loadFinished, this));
    m_webEngine->loadStop.connect(boost::bind(&SimpleUI::loadFinished, this));
    m_webEngine->loadError.connect(boost::bind(&SimpleUI::loadError, this));
    m_webEngine->confirmationRequest.connect(boost::bind(&SimpleUI::handleConfirmationRequest, this, _1));
    m_webEngine->tabCreated.connect(boost::bind(&SimpleUI::tabCreated, this));
    m_webEngine->tabClosed.connect(boost::bind(&SimpleUI::tabClosed,this,_1));
    m_webEngine->IMEStateChanged.connect(boost::bind(&SimpleUI::setwvIMEStatus, this, _1));

    m_favoriteService->bookmarkAdded.connect(boost::bind(&SimpleUI::onBookmarkAdded, this,_1));
    m_favoriteService->bookmarkDeleted.connect(boost::bind(&SimpleUI::onBookmarkRemoved, this, _1));

    m_historyService->historyAdded.connect(boost::bind(&SimpleUI::onHistoryAdded, this,_1));
    m_historyService->historyDeleted.connect(boost::bind(&SimpleUI::onHistoryRemoved, this,_1));

    m_platformInputManager->returnPressed.connect(boost::bind(&elm_exit));
    m_platformInputManager->backPressed.connect(boost::bind(&SimpleUI::onBackPressed, this));

}

void SimpleUI::createActions()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ///\todo Add MulitStateAction. and convert m_stopLoading and m_reload actons to it?

    m_settingPrivateBrowsing = sharedAction(new Action("Private browsing"));
    m_settingPrivateBrowsing->setToolTip("On exit from private mode all cookies, history, and stored data will be deleted");
    m_settingPrivateBrowsing->setCheckable(true);
    m_settingPrivateBrowsing->setChecked(m_webEngine->isPrivateMode());
    m_settingPrivateBrowsing->setEnabled(true);

    m_settingDeleteHistory = sharedAction(new Action("Delete history"));
    m_settingDeleteHistory->setToolTip("Delete History");

    m_settingDeleteData = sharedAction(new Action("Delete data"));
    m_settingDeleteData->setToolTip("Delete Data");

    m_settingDeleteFavorite = sharedAction(new Action("Delete favorite site"));
    m_settingDeleteFavorite->setToolTip("Delete favorite site");

    m_bookmarks_manager_BookmarkBar = sharedAction(new Action("Bookmark Bar"));
    m_bookmarks_manager_BookmarkBar->setToolTip("show Bookmark bar");
}

void SimpleUI::connectActions()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_settingPrivateBrowsing->toggled.connect(boost::bind(&SimpleUI::settingsPrivateModeSwitch, this, _1));

    //left bar
    m_webPageUI->backPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::back, m_webEngine.get()));
    m_webPageUI->backPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
    m_webPageUI->forwardPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::forward, m_webEngine.get()));
    m_webPageUI->stopLoadingPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::stopLoading, m_webEngine.get()));
    m_webPageUI->reloadPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::reload, m_webEngine.get()));
    m_webPageUI->reloadPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));

    //right bar
    m_webPageUI->showTabUI.connect(boost::bind(&SimpleUI::showTabUI, this));
    m_webPageUI->showMoreMenu.connect(boost::bind(&SimpleUI::showMoreMenu, this));
}

void SimpleUI::switchViewToWebPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_webPageUI);
    M_ASSERT(m_mainUI);
    if(m_webPageUI->isHomePageActive())
        m_mainUI->hide();
    m_webPageUI->switchViewToWebPage(m_webEngine->getLayout(), m_webEngine->getURI());
}

void SimpleUI::switchToTab(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_webEngine->currentTabId() != tabId) {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        m_webEngine->switchToTab(tabId);
    }
    if(m_webEngine->isLoadError()){
        BROWSER_LOGD("[%s:%d] LOAD ERROR!", __PRETTY_FUNCTION__, __LINE__);
        loadError();
	return;
    }
    BROWSER_LOGD("[%s:%d] swiching to web_view ", __PRETTY_FUNCTION__, __LINE__);
    switchViewToWebPage();
}

bool SimpleUI::isErrorPageActive()
{
    return m_webPageUI->isErrorPageActive();
}

void SimpleUI::switchViewToQuickAccess()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_webPageUI->isHomePageActive())
        return;

    m_webPageUI->switchViewToQuickAccess(m_mainUI->getContent());
    m_webEngine->disconnectCurrentWebViewSignals();
    m_mainUI->showMostVisited(getMostVisitedItems());
}

void SimpleUI::checkTabId(const tizen_browser::basic_webengine::TabId& id){
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if(m_webEngine->currentTabId() != id || isErrorPageActive()){
        BROWSER_LOGD("URL changed on non-visible tab, updating browser view");
        switchToTab(id);
    }
}

void SimpleUI::openNewTab(const std::string &uri, bool desktopMode)
{
    switchToTab(m_webEngine->addTab(uri, nullptr, desktopMode));
}

void SimpleUI::closeTab(){
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    auto tabId = m_webEngine->currentTabId();
    closeTab(tabId);
}

void SimpleUI::closeTab(const tizen_browser::basic_webengine::TabId& id)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    m_currentSession.removeItem(id.toString());
    m_webEngine->closeTab(id);
    updateView();
}

bool SimpleUI::checkBookmark()
{
    if (m_webPageUI->isHomePageActive())
        return false;

    if(m_favoriteService->bookmarkExists(m_webEngine->getURI())) {
        BROWSER_LOGD("[%s] There is bookmark for this site [%s], set indicator on!", __func__, m_webEngine->getURI().c_str());
        return true;
    }
    else {
        BROWSER_LOGD("[%s] There is no bookmark for this site [%s], set indicator off", __func__, m_webEngine->getURI().c_str());
        return false;
    }
}
// Consider removing these functions
void SimpleUI::onBookmarkAdded(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem)
{
    if (m_moreMenuUI) {
        m_moreMenuUI->changeBookmarkStatus(true);
        m_moreMenuUI->createToastPopup( (std::string(m_webEngine->getTitle()) + std::string(" added to bookmark")).c_str() );
    }
}

void SimpleUI::onBookmarkRemoved(const std::string& uri)
{
    BROWSER_LOGD("[%s] deleted %s", __func__, uri.c_str());
    if (m_moreMenuUI) {
        m_moreMenuUI->changeBookmarkStatus(false);
        m_moreMenuUI->createToastPopup( (std::string(m_webEngine->getTitle()) + std::string(" removed from bookmark")).c_str() );
    }
}

void SimpleUI::onHistoryAdded(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem)
{
}

void SimpleUI::onOpenURLInNewTab(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem, bool desktopMode)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    std::string historyAddress = historyItem->getUrl();
    if(m_historyUI) {                // TODO: remove this section when naviframes will be available
        m_historyUI->clearItems();
        closeHistoryUI(std::string());
    }

    if(m_moreMenuUI) {               // TODO: remove this section when naviframes will be available
        m_moreMenuUI->clearItems();
        closeMoreMenu(std::string());
    }
    openNewTab(historyAddress, desktopMode);
}

void SimpleUI::onMostVisitedTileClicked(std::shared_ptr< services::HistoryItem > historyItem, int itemsNumber)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    m_mainUI->openDetailPopup(historyItem, m_historyService->getHistoryItemsByURL(historyItem->getUrl(), itemsNumber));
}

void SimpleUI::onClearHistoryClicked(const std::string&)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    m_historyService->clearAllHistory();
    m_historyUI.reset();
}

void SimpleUI::onMostVisitedClicked(const std::string&)
{
   BROWSER_LOGD("[%s]", __func__);
   m_mainUI->showMostVisited(getMostVisitedItems());
}

void SimpleUI::onBookmarkButtonClicked(const std::string&)
{
   BROWSER_LOGD("[%s]", __func__);
   m_mainUI->showBookmarks(getBookmarks());
}

void SimpleUI::onBookmarkManagerButtonClicked(const std::string&)
{
    BROWSER_LOGD("[%s]", __func__);
    m_mainUI->hide();

    if(m_moreMenuUI) {               // TODO: remove this section when naviframes will be available
        m_moreMenuUI->clearItems();
    }

    showBookmarkManagerMenu();
}

void SimpleUI::onBookmarkClicked(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem)
{
    std::string bookmarkAddress = bookmarkItem->getAddress();
    if(m_bookmarkManagerUI) {                // TODO: remove this section when naviframes will be available
        m_bookmarkManagerUI->clearItems();
        closeBookmarkManagerMenu(std::string());
    }

    if(m_moreMenuUI) {               // TODO: remove this section when naviframes will be available
        m_moreMenuUI->clearItems();
        closeMoreMenu(std::string());
    }
    openNewTab(bookmarkAddress);
}

// Consider removing these functions

void SimpleUI::onHistoryRemoved(const std::string& uri)
{
    BROWSER_LOGD("[%s] deleted %s", __func__, uri.c_str());
}

void SimpleUI::onReturnPressed(MenuButton *m)
{
    BROWSER_LOGD("[%s]", __func__);
    m_platformInputManager->returnPressed.disconnect_all_slots();
    m_platformInputManager->returnPressed.connect(boost::bind(&elm_exit));
    hidePopup.disconnect_all_slots();
    m->hidePopup();
}

void SimpleUI::setwvIMEStatus(bool status)
{
    BROWSER_LOGD("[%s]", __func__);
    m_wvIMEStatus = status;
}

void SimpleUI::onBackPressed()
{
    BROWSER_LOGD("[%s]", __func__);
    if (!m_webPageUI->getURIEntry().hasFocus() && !m_wvIMEStatus && !m_webPageUI->isHomePageActive())
        m_webEngine->backButtonClicked();
}

void SimpleUI::backEnable(bool enable)
{
    m_webPageUI->setBackButtonEnabled(enable);
}

void SimpleUI::forwardEnable(bool enable)
{
    m_webPageUI->setForwardButtonEnabled(enable);
}

void SimpleUI::reloadEnable(bool enable)
{
    m_webPageUI->setReloadButtonEnabled(enable);
}

void SimpleUI::stopEnable(bool enable)
{
    m_webPageUI->setStopButtonEnabled(enable);
}

void SimpleUI::addBookmarkEnable(bool enable)
{
}

void SimpleUI::removeBookmarkEnable(bool enable)
{
}
void SimpleUI::zoomEnable(bool enable)
{
    m_zoom_in->setEnabled(enable);
}

void SimpleUI::settingsButtonEnable(bool enable)
{
    m_webPageUI->setMoreMenuButtonEnabled(enable);
}

void SimpleUI::loadStarted()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    addBookmarkEnable(false);
    if(!m_webEngine->isPrivateMode()){
        m_currentSession.updateItem(m_webEngine->currentTabId().toString(), m_webEngine->getURI());
    }
    m_webPageUI->loadStarted();
}

void SimpleUI::progressChanged(double progress)
{
    m_webPageUI->progressChanged(progress);
}

void SimpleUI::loadFinished()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    addBookmarkEnable(m_favoriteService->countBookmarks() < m_favoritesLimit);

    if(!m_webEngine->isPrivateMode()){
        m_historyService->addHistoryItem(std::make_shared<tizen_browser::services::HistoryItem> (m_webEngine->getURI(),
                                                                                                m_webEngine->getTitle(),
                                                                                                m_webEngine->getFavicon()), m_webEngine->getSnapshotData(MainUI::MAX_THUMBNAIL_WIDTH, MainUI::MAX_THUMBNAIL_HEIGHT));
    }
    m_webPageUI->loadFinished();
}

void SimpleUI::loadError()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webPageUI->switchViewToErrorPage();
}

void SimpleUI::filterURL(const std::string& url)
{
    BROWSER_LOGD("[%s:%d] url=%s", __PRETTY_FUNCTION__, __LINE__, url.c_str());
    //check for special urls (like:  'about:home')
    //if there will be more addresses may be we should
    //create some kind of std::man<std::string url, bool *(doSomethingWithUrl)()>
    //and then just map[url]() ? m_webEngine->setURI(url) : /*do nothing*/;;
    if(/*url.empty() ||*/ url == HomePageURL){
        m_webPageUI->getURIEntry().changeUri("");
    } else if (!url.empty()){

    //check if url is in favorites

    //check if url is in blocked

    //no filtering
        if (m_webPageUI->isHomePageActive())
            openNewTab(url);
        else
            m_webEngine->setURI(url);
    }
    m_webPageUI->getURIEntry().clearFocus();
    //addBookmarkEnable(false);
}

void SimpleUI::webEngineURLChanged(const std::string url)
{
    BROWSER_LOGD("webEngineURLChanged:%s", url.c_str());
    m_webPageUI->getURIEntry().clearFocus();
}

void SimpleUI::showTabUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_tabUI = std::dynamic_pointer_cast<tizen_browser::base_ui::TabUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.tabui"));
    M_ASSERT(m_tabUI);
    m_tabUI->closeTabUIClicked.connect(boost::bind(&SimpleUI::closeTabUI, this,_1));
    m_tabUI->newTabClicked.connect(boost::bind(&SimpleUI::newTabClicked, this,_1));
    m_tabUI->tabClicked.connect(boost::bind(&SimpleUI::tabClicked, this,_1));
    m_tabUI->closeTabsClicked.connect(boost::bind(&SimpleUI::closeTabsClicked, this,_1));
    m_tabUI->newIncognitoTabClicked.connect(boost::bind(&SimpleUI::newTabClicked, this,_1));
    m_tabUI->tabsCount.connect(boost::bind(&SimpleUI::tabsCount, this));
    m_tabUI->show(m_window.get());
    m_tabUI->addTabItems(m_webEngine->getTabContents());
}

void SimpleUI::closeTabUI(const std::string& str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_tabUI = nullptr;
}

void SimpleUI::newTabClicked(const std::string& str)
{
    BROWSER_LOGD("%s", __func__);
    switchViewToQuickAccess();
}

void SimpleUI::tabClicked(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("%s", __func__);
    switchToTab(tabId);
}

void SimpleUI::closeTabsClicked(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("%s", __func__);
    m_webEngine->closeTab(tabId);
}

int SimpleUI::tabsCount()
{
    return m_webEngine->tabsCount();
}

void SimpleUI::handleConfirmationRequest(basic_webengine::WebConfirmationPtr webConfirmation)
{
    BROWSER_LOGD("%s", __func__);
    switch(webConfirmation->getConfirmationType())
    {
        case basic_webengine::WebConfirmation::ConfirmationType::Authentication:
        {
        basic_webengine::AuthenticationConfirmationPtr auth = std::dynamic_pointer_cast<basic_webengine::AuthenticationConfirmation, basic_webengine::WebConfirmation>(webConfirmation);

        Evas_Object *popup_content = elm_layout_add(m_webPageUI->getContent());
        std::string edjFilePath = EDJE_DIR;
        edjFilePath.append("SimpleUI/AuthenticationPopup.edj");
        Eina_Bool layoutSetResult = elm_layout_file_set(popup_content, edjFilePath.c_str(), "authentication_popup");
        if(!layoutSetResult)
            throw std::runtime_error("Layout file not found: " + edjFilePath);

    #if PLATFORM(TIZEN)
        elm_object_translatable_part_text_set(popup_content, "login_label", "Login");
        elm_object_translatable_part_text_set(popup_content, "password_label", "Password");
    #else
        elm_object_part_text_set(popup_content, "login_label", "Login");
        elm_object_part_text_set(popup_content, "password_label", "Password");
    #endif

        Evas_Object *loginEntry = elm_entry_add(popup_content);
        elm_object_part_content_set(popup_content, "login", loginEntry);

        Evas_Object *passwordEntry = elm_entry_add(popup_content);
        elm_object_part_content_set(popup_content, "password", passwordEntry);

        SimplePopup *popup = SimplePopup::createPopup();
        popup->setTitle("Authentication request");
        popup->addButton(OK);
        popup->addButton(CANCEL);
        popup->setContent(popup_content);
        std::shared_ptr<AuthenticationPopupData> popupData = std::make_shared<AuthenticationPopupData>();
        popupData->loginEntry = loginEntry;
        popupData->passwordEntry = passwordEntry;
        popupData->auth = auth;
        popup->setData(popupData);
        popup->buttonClicked.connect(boost::bind(&SimpleUI::authPopupButtonClicked, this, _1, _2));
        popup->show();
        break;
        }
        case basic_webengine::WebConfirmation::ConfirmationType::CertificateConfirmation:
        case basic_webengine::WebConfirmation::ConfirmationType::Geolocation:
        case basic_webengine::WebConfirmation::ConfirmationType::UserMedia:
        case basic_webengine::WebConfirmation::ConfirmationType::Notification:
        {
        // Implicitly accepted
        BROWSER_LOGE("NOT IMPLEMENTED: popups to confirm Ceritificate, Geolocation, UserMedia, Notification");
        webConfirmation->setResult(tizen_browser::basic_webengine::WebConfirmation::ConfirmationResult::Confirmed);
        m_webEngine->confirmationResult(webConfirmation);
        break;
        }

    default:
        break;
    }
}

void SimpleUI::authPopupButtonClicked(PopupButtons button, std::shared_ptr<PopupData> popupData)
{
    std::shared_ptr<AuthenticationPopupData> authPopupData = std::dynamic_pointer_cast<AuthenticationPopupData, PopupData>(popupData);
    switch(button){
        case OK:
            authPopupData->auth->setLogin(elm_entry_entry_get(authPopupData->loginEntry) ? elm_entry_entry_get(authPopupData->loginEntry) : "");
            authPopupData->auth->setPassword(elm_entry_entry_get(authPopupData->passwordEntry) ? elm_entry_entry_get(authPopupData->passwordEntry) : "");
            authPopupData->auth->setResult(basic_webengine::WebConfirmation::ConfirmationResult::Confirmed);
            m_webEngine->confirmationResult(authPopupData->auth);
            break;
        case CANCEL:
            authPopupData->auth->setResult(basic_webengine::WebConfirmation::ConfirmationResult::Rejected);
            m_webEngine->confirmationResult(authPopupData->auth);
            break;
    case YES:
    case NO:
    case CLOSE:
    case CONNECT:
        break;
    default:
        break;
    }
}

void SimpleUI::showHistoryUI(const std::string& str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(!m_historyUI)
    {
        m_historyUI =
        std::dynamic_pointer_cast<tizen_browser::base_ui::HistoryUI,tizen_browser::core::AbstractService>
            (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.historyui"));
        M_ASSERT(m_historyUI);
        m_historyUI->clearHistoryClicked.connect(boost::bind(&SimpleUI::onClearHistoryClicked, this,_1));
        m_historyUI->closeHistoryUIClicked.connect(boost::bind(&SimpleUI::closeHistoryUI, this,_1));
        m_historyUI->historyItemClicked.connect(boost::bind(&SimpleUI::onOpenURLInNewTab, this, _1, true));     // desktop mode as default
        m_historyUI->addHistoryItems(getHistory());
        m_historyUI->show(m_window.get());
    }
}

void SimpleUI::closeHistoryUI(const std::string& str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_historyUI->clearHistoryClicked.disconnect(boost::bind(&SimpleUI::onClearHistoryClicked, this,_1));
    m_historyUI->closeHistoryUIClicked.disconnect(boost::bind(&SimpleUI::closeHistoryUI, this,_1));
    m_historyUI->historyItemClicked.disconnect(boost::bind(&SimpleUI::onOpenURLInNewTab, this, _1, true));     // desktop mode as default
    m_historyUI.reset();
}

void SimpleUI::showSettingsUI(const std::string& str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(!m_settingsUI){
        m_settingsUI =
                std::dynamic_pointer_cast
                <tizen_browser::base_ui::SettingsUI,tizen_browser::core::AbstractService>
                (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.settingsui"));
        M_ASSERT(m_settingsUI);
        m_settingsUI->closeSettingsUIClicked.disconnect_all_slots();
        m_settingsUI->closeSettingsUIClicked.connect(boost::bind(&SimpleUI::closeSettingsUI, this,_1));
        m_settingsUI->deleteSelectedDataClicked.disconnect_all_slots();
        m_settingsUI->deleteSelectedDataClicked.connect(boost::bind(&SimpleUI::settingsDeleteSelectedData, this,_1));
        m_settingsUI->resetMostVisitedClicked.disconnect_all_slots();
        m_settingsUI->resetMostVisitedClicked.connect(boost::bind(&SimpleUI::settingsResetMostVisited, this,_1));
        m_settingsUI->resetBrowserClicked.disconnect_all_slots();
        m_settingsUI->resetBrowserClicked.connect(boost::bind(&SimpleUI::settingsResetBrowser, this,_1));
        m_settingsUI->show(m_window.get());
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    }
}

void SimpleUI::closeSettingsUI(const std::string& str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_settingsUI.reset();
}

void SimpleUI::showMoreMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    bool desktopMode = m_webPageUI->isHomePageActive() ? m_mainUI->isDesktopMode() : m_webEngine->isDesktopMode();
    if(!m_moreMenuUI){
        m_moreMenuUI =
                std::dynamic_pointer_cast
                <tizen_browser::base_ui::MoreMenuUI,tizen_browser::core::AbstractService>
                (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.moremenuui"));
        M_ASSERT(m_moreMenuUI);
        m_webEngine->favIconChanged.connect(boost::bind(&MoreMenuUI::setFavIcon, m_moreMenuUI.get(), _1));
        m_webEngine->titleChanged.connect(boost::bind(&MoreMenuUI::setWebTitle, m_moreMenuUI.get(), _1));
        m_webEngine->uriChanged.connect(boost::bind(&MoreMenuUI::setURL, m_moreMenuUI.get(), _1));
        m_moreMenuUI->bookmarkManagerClicked.connect(boost::bind(&SimpleUI::onBookmarkManagerButtonClicked, this, _1));
        m_moreMenuUI->historyUIClicked.connect(boost::bind(&SimpleUI::showHistoryUI, this,_1));
        m_moreMenuUI->settingsClicked.connect(boost::bind(&SimpleUI::showSettingsUI, this,_1));
        m_moreMenuUI->closeMoreMenuClicked.connect(boost::bind(&SimpleUI::closeMoreMenu, this,_1));
        m_moreMenuUI->switchToMobileMode.connect(boost::bind(&SimpleUI::switchToMobileMode, this));
        m_moreMenuUI->switchToDesktopMode.connect(boost::bind(&SimpleUI::switchToDesktopMode, this));
        m_moreMenuUI->addToBookmarkClicked.connect(boost::bind(&SimpleUI::addToBookmarks, this, _1));
        m_moreMenuUI->isBookmark.connect(boost::bind(&SimpleUI::checkBookmark, this));
        m_moreMenuUI->deleteBookmark.connect(boost::bind(&SimpleUI::deleteBookmark, this));

        m_moreMenuUI->show(m_window.get(), desktopMode);
        m_moreMenuUI->showCurrentTab();
        m_moreMenuUI->setFavIcon(m_webEngine->getFavicon());
        m_moreMenuUI->setWebTitle(m_webEngine->getTitle());
        m_moreMenuUI->setURL(m_webEngine->getURI());
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    }
}

void SimpleUI::closeMoreMenu(const std::string& str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webEngine->favIconChanged.disconnect(boost::bind(&MoreMenuUI::setFavIcon, m_moreMenuUI.get(), _1));
    m_webEngine->titleChanged.disconnect(boost::bind(&MoreMenuUI::setWebTitle, m_moreMenuUI.get(), _1));
    m_webEngine->uriChanged.disconnect(boost::bind(&MoreMenuUI::setURL, m_moreMenuUI.get(), _1));
    m_moreMenuUI->bookmarkManagerClicked.disconnect(boost::bind(&SimpleUI::onBookmarkManagerButtonClicked, this, _1));
    m_moreMenuUI->historyUIClicked.disconnect(boost::bind(&SimpleUI::showHistoryUI, this,_1));
    m_moreMenuUI->settingsClicked.disconnect(boost::bind(&SimpleUI::showSettingsUI, this,_1));
    m_moreMenuUI->closeMoreMenuClicked.disconnect(boost::bind(&SimpleUI::closeMoreMenu, this,_1));
    m_moreMenuUI->switchToMobileMode.disconnect(boost::bind(&SimpleUI::switchToMobileMode, this));
    m_moreMenuUI->switchToDesktopMode.disconnect(boost::bind(&SimpleUI::switchToDesktopMode, this));
    m_moreMenuUI->addToBookmarkClicked.disconnect(boost::bind(&SimpleUI::addToBookmarks, this, _1));
    m_moreMenuUI->isBookmark.disconnect(boost::bind(&SimpleUI::checkBookmark, this));
    m_moreMenuUI->deleteBookmark.disconnect(boost::bind(&SimpleUI::deleteBookmark, this));
    m_moreMenuUI.reset();
}

void SimpleUI::switchToMobileMode()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_webPageUI->isHomePageActive()) {
        m_webEngine->switchToMobileMode();
        m_webEngine->reload();
    } else {
        m_mainUI->setDesktopMode(false);
    }
}

void SimpleUI::switchToDesktopMode()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_webPageUI->isHomePageActive()) {
        m_webEngine->switchToDesktopMode();
        m_webEngine->reload();
    } else {
        m_mainUI->setDesktopMode(true);
    }
}

void SimpleUI::showBookmarkManagerMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(!m_bookmarkManagerUI){
        m_bookmarkManagerUI =
                std::dynamic_pointer_cast
                <tizen_browser::base_ui::BookmarkManagerUI,tizen_browser::core::AbstractService>
                (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.bookmarkmanagerui"));
        M_ASSERT(m_bookmarkManagerUI);
        m_bookmarkManagerUI->closeBookmarkManagerClicked.connect(boost::bind(&SimpleUI::closeBookmarkManagerMenu, this,_1));
        m_bookmarkManagerUI->bookmarkItemClicked.connect(boost::bind(&SimpleUI::onBookmarkClicked, this, _1));
        m_bookmarkManagerUI->show(m_window.get());
        m_bookmarkManagerUI->addBookmarkItems(getBookmarks(ROOT_FOLDER));
        m_bookmarkManagerUI->showTopContent();
    }
}

void SimpleUI::closeBookmarkManagerMenu(const std::string& str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_bookmarkManagerUI->closeBookmarkManagerClicked.disconnect(boost::bind(&SimpleUI::closeBookmarkManagerMenu, this,_1));
    m_bookmarkManagerUI->bookmarkItemClicked.disconnect(boost::bind(&SimpleUI::onBookmarkClicked, this, _1));
    m_bookmarkManagerUI.reset();

    if(m_moreMenuUI) {
        closeMoreMenu(std::string());
        showMoreMenu();
    }

    if(m_mainUI) {
        m_mainUI->showBookmarks(getBookmarks());
    }
}

void SimpleUI::hideHistory()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    hidePopup();
}

void SimpleUI::settingsPointerModeSwitch(bool newState)
{
    BROWSER_LOGD("%s: Setting Pointer mode to:%s", __func__, (newState ? "true" : "false"));
}

void SimpleUI::settingsPrivateModeSwitch(bool newState)
{
    BROWSER_LOGD("%s: Setting Private mode to: %s", __func__, (newState ? "true" : "false"));
    m_webEngine->setPrivateMode(newState);
    BROWSER_LOGD("[%s:%d] webEngine private mode: %s", __PRETTY_FUNCTION__, __LINE__, (m_webEngine->isPrivateMode() ? "true" : "false"));
}

void SimpleUI::settingsDeleteSelectedData(const std::string& str)
{
    BROWSER_LOGD("[%s]: Deleting Hisory", __func__);
    SimplePopup *popup = SimplePopup::createPopup();
    popup->setTitle("Delete selected data");
    popup->addButton(OK);
    popup->addButton(CANCEL);
    popup->setMessage("Are you sure you want to delete all selected data?");
    std::shared_ptr<EntryPopupData> popupData = std::make_shared<EntryPopupData>();
    popupData->text = str;
    popup->setData(popupData);
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onDeleteSelectedDataButton, this, _1, _2));
    popup->show();
}

void SimpleUI::onDeleteSelectedDataButton(PopupButtons button, std::shared_ptr< PopupData > popupData)
{
    if(button == OK){
        BROWSER_LOGD("[%s]: OK", __func__);
	std::string dataText = std::static_pointer_cast<EntryPopupData>(popupData)->text;
	BROWSER_LOGD("[%s]: TYPE : %s", __func__, dataText.c_str());
	if (dataText.find("CACHE") != std::string::npos)
		m_webEngine->clearPrivateData();
	if (dataText.find("COOKIES") != std::string::npos)
		m_webEngine->clearPrivateData();
	if (dataText.find("HISTORY") != std::string::npos)
		m_historyService->clearAllHistory();
    }
}

void SimpleUI::settingsResetMostVisited(const std::string& str)
{
    BROWSER_LOGD("[%s]: Deleting Hisory", __func__);
    SimplePopup *popup = SimplePopup::createPopup();
    popup->setTitle("Delete most visited");
    popup->addButton(OK);
    popup->addButton(CANCEL);
    popup->setMessage("Are you sure you want to delete most visited sites?");
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onDeleteMostVisitedButton, this, _1, _2));
    popup->show();
}

void SimpleUI::onDeleteMostVisitedButton(PopupButtons button, std::shared_ptr< PopupData > /*popupData*/)
{
    if(button == OK){
        BROWSER_LOGD("[%s]: OK", __func__);
        BROWSER_LOGD("[%s]: Deleting most visited", __func__);
    }
}

void SimpleUI::settingsResetBrowser(const std::string& str)
{
    BROWSER_LOGD("[%s]: Deleting Hisory", __func__);
    SimplePopup *popup = SimplePopup::createPopup();
    popup->setTitle("Reset browser");
    popup->addButton(OK);
    popup->addButton(CANCEL);
    popup->setMessage("Are you sure you want to reset browser?");
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onResetBrowserButton, this, _1, _2));
    popup->show();
}

void SimpleUI::onResetBrowserButton(PopupButtons button, std::shared_ptr< PopupData > /*popupData*/)
{
    if(button == OK){
        BROWSER_LOGD("[%s]: OK", __func__);
        BROWSER_LOGD("[%s]: Resetting browser", __func__);
    }
}

void SimpleUI::tabLimitPopupButtonClicked(PopupButtons button, std::shared_ptr< PopupData > /*popupData*/)
{
    if(button == CLOSE_TAB){
        BROWSER_LOGD("[%s]: CLOSE TAB", __func__);
        closeTab();
    }
}

void SimpleUI::tabCreated()
{
    int tabs = m_webEngine->tabsCount();

    if(tabs > m_tabLimit)
    {
        SimplePopup *popup = SimplePopup::createPopup();
        popup->setTitle("Too many tabs open");
        popup->addButton(CONTINUE);
        popup->addButton(CLOSE_TAB);
        popup->setMessage("Browser might slow down. Are you sure you want to continue?");
        popup->buttonClicked.connect(boost::bind(&SimpleUI::tabLimitPopupButtonClicked, this, _1, _2));
        popup->show();
    }
    m_webPageUI->setTabsNumber(tabs);
}

void SimpleUI::updateView() {
    int tabs = m_webEngine->tabsCount();
    BROWSER_LOGD("[%s] Opened tabs: %d", __func__, tabs);
    if (tabs == 0) {
        switchViewToQuickAccess();
    } else if (!m_webPageUI->isHomePageActive()) {
        switchViewToWebPage();
    }
    m_webPageUI->setTabsNumber(tabs);
}

void SimpleUI::tabClosed(const tizen_browser::basic_webengine::TabId& id) {
    updateView();
}

void SimpleUI::onNetworkError()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(!m_networkErrorPopup){
        m_networkErrorPopup = SimplePopup::createPopup();
        m_networkErrorPopup->setTitle("Network Error");
        m_networkErrorPopup->addButton(CONNECT);
        m_networkErrorPopup->addButton(CANCEL);
        m_networkErrorPopup->setMessage("Network is disconnected. Please check the connection.");
        m_networkErrorPopup->buttonClicked.connect(boost::bind(&SimpleUI::onNetErrorButtonPressed, this, _1, _2));
        m_networkErrorPopup->show();
    }
}

void SimpleUI::onNetErrorButtonPressed(PopupButtons, std::shared_ptr< PopupData >)
{
    m_networkErrorPopup = 0;
}

void SimpleUI::onNetworkConnected()
{
    if(m_networkErrorPopup){
        delete m_networkErrorPopup;
        m_networkErrorPopup = 0;
    }
}

void SimpleUI::searchWebPage(std::string &text, int flags)
{
    m_webEngine->searchOnWebsite(text, flags);
}

void SimpleUI::addToBookmarks(int folder_id)
{
    BROWSER_LOGD("[%s,%d],", __func__, __LINE__);
    if (m_favoriteService)
    {
        if( m_webEngine && !m_webEngine->getURI().empty())
         {
               m_favoriteService->addToBookmarks(m_webEngine->getURI(), m_webEngine->getTitle(), std::string(),
                                                 m_webEngine->getSnapshotData(373, 240),
                                                 m_webEngine->getFavicon(),(unsigned int)folder_id);
         }
    }
}

//TODO: Replace by direct call.
void SimpleUI::deleteBookmark()
{
	if (m_favoriteService)
		m_favoriteService->deleteBookmark(m_webEngine->getURI());
}
}
}
