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

#ifndef __ABSTRACT_WEBENGINE_H__
#define __ABSTRACT_WEBENGINE_H__ 1

#include <boost/signals2/signal.hpp>
#include <memory>
#include <Evas.h>

#include "BrowserImage.h"
#include "../ServiceManager/Debug/Lifecycle.h"
#include "../ServiceManager/AbstractService.h"

#include "TabId.h"
#include "WebConfirmation.h"

namespace tizen_browser {
namespace basic_webengine {
/***
 * AbstractWebEngine defines interface for WebEngine object.
 * It is designed for multiple tabs operations.
 * It is designed to be only way for communication with WebEngine. It should NOT return WebEngine object.
 */
template<typename T>
#ifndef NDEBUG
class AbstractWebEngine : public tizen_browser::core::AbstractService, ShowLifeCycle<AbstractWebEngine<T>>
#else
class AbstractWebEngine: public tizen_browser::core::AbstractService
#endif
{
public:
    /**
     * \todo this function should return nonEFL object in future
     * Remember that there must be at least 1 tab created to return layout
     * @return pointer Evas_Object for current WebView.
     */
    virtual T * getLayout() = 0;

    /**
     * Initialize WebEngine.
     * @param guiParent GUI parent object (now should pass Evas_Object)
     * \todo make guiParent nonEFL object
     */
    virtual void init(void * guiParent) = 0;

    /**
     * Set URI address for current tab.
     * @param uri
     */
    virtual void setURI(const std::string & uri) = 0;

    /**
     * @return uri address for current tab.
     */
    virtual std::string getURI(void) const = 0;

    /**
     * @return title of page opened in current tab.
     */
    virtual std::string getTitle(void) const = 0;

    /**
     * Stop loading current page.
     */
    virtual void stopLoading(void) = 0;

    /**
     * Reload current page.
     */
    virtual void reload(void) = 0;

    /**
     * Go to previous page (if possible)
     */
    virtual void back(void) = 0;

    /**
     * Go to next page (if possible)
     */
    virtual void forward(void) = 0;

    /**
     * @return true if it is possible to go back, false otherwise
     */
    virtual bool isBackEnabled(void) const = 0;

    /**
     * @return true if it is possible to go forward, false otherwise
     */
    virtual bool isForwardEnabled(void) const = 0;

    /**
     * @return true if page is still loading, false otherwise
     */
    virtual bool isLoading() const = 0;

    /**
     * @return number of tabs
     */
    virtual int tabsCount() const = 0;

    /**
     * @return TabId of current tab
     */
    virtual TabId currentTabId() const = 0;

    /**
     * @return list (RVO) of tabs in most recent order
     */
    virtual std::list<TabId> listTabs() const = 0;


    /**
     *  @return vector of metadata for all opened tabs
     */
    virtual std::vector<std::shared_ptr<TabContent>> getTabContents() const = 0;

    /**
     * Adds new tab
     * @param uri if not empty opens specified uri
     * @return TabId of created tab
     */
    virtual TabId addTab(const std::string & uri = std::string(), const TabId * openerId = NULL) = 0;

    /**
     * @param tab id
     * @return returns underlaying UI component
     */
    virtual T* getTabView(TabId id) = 0;

    /**
     * Switch current tab for tab with tabId
     * @param tabId of tab
     * @return true if tab changed, false otherwise
     */
    virtual bool switchToTab(TabId tabId) = 0;

    /**
     * Close current tab. Switch tab to other (not defined which).
     * Should NOT close last tab (there should be only at least 1 tab)
     * @return true if tab closed successfully, false otherwise
     */
    virtual bool closeTab() = 0;

    /**
     * Close tab specified with id parameter. Switch tab to other (not defined which).
     * Should NOT close last tab (there should be only at least 1 tab)
     * @return true if tab closed successfully, false otherwise
     */
    virtual bool closeTab(TabId id) = 0;

    /**
     * Go to next tab.
     * @return true if tab changed successfully, false otherwise
     */
    virtual bool nextTab() = 0;

    /**
     * Go to prev tab.
     * @return true if tab changed successfully, false otherwise
     */
    virtual bool prevTab() = 0;

    /**
     * Process confirmation result
     * \param web confirmation with request and result value
     */
    virtual void confirmationResult(WebConfirmationPtr) = 0;

    /**
     * Gets snapshot data as void* for current tab
     */
    virtual std::shared_ptr<tizen_browser::tools::BrowserImage> getSnapshotData(int width = 324, int height = 177) = 0;

    /**
     * Gets snapshot data as void* for tab provided as parameter
     */
    virtual std::shared_ptr<tizen_browser::tools::BrowserImage> getSnapshotData(TabId id, int width, int height) = 0;

    /**
     * Set private mode on/off
     * \param private mode on when true, off otherwise
     */
    virtual void setPrivateMode(bool) = 0;

    virtual bool isPrivateMode() const = 0;

    virtual bool isLoadError() const = 0;

    /**
     * \brief Sets Focus to URI entry.
     */
    virtual void setFocus()=0;

    /**
     * @brief Remove focus form URI
     */
    virtual void clearFocus()=0;

    /**
     * @brief check if URI is focused
     */
    virtual bool hasFocus() const = 0;

    /**
     * @brief return current zoom factor in percentage.
     *
     * @return real zoom, also for "fit to screen" mode
     */
    virtual int getZoomFactor()const = 0;


    /**
     * @brief Sets zoom factor in percentage
     *
     * @param zoomFactor in percentage of default zoom, 0 means autofit
     */
    virtual void setZoomFactor(int zoomFactor) = 0;

    /**
     * @brief Clear private data of WebView
     */
    virtual void clearPrivateData() = 0;

    virtual void disconnectCurrentWebViewSignals() = 0;

    /**
     * @brief Search string on searchOnWebsite
     *
     * @param string to search on searchOnWebsite
     * @param flags for search options
     */
    virtual void searchOnWebsite(const std::string &, int flags) = 0;

    /**
     * @brief Get favicon of current page loaded
     */
    virtual std::shared_ptr<tizen_browser::tools::BrowserImage> getFavicon() = 0;

    /**
     * @brief back or exit when back key is pressed
     */
    virtual void backButtonClicked() const = 0;

    /**
     * FavIcon of current page changed
     */
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::tools::BrowserImage>)> favIconChanged;

    /**
     * Title of current page changed
     * \param new title
     */
    boost::signals2::signal<void (const std::string &)> titleChanged;

    /**
     * URI of current page changed
     * \param new URI
     */
    boost::signals2::signal<void (const std::string &)> uriChanged;

    /**
     * URI on current/other page changed
     */
    boost::signals2::signal<void (TabId)> uriOnTabChanged;

    /**
     * Possibility of go forward changed
     * \param bool true if it is possible to go forward, false otherwise
     */
    boost::signals2::signal<void (bool)> forwardEnableChanged;

    /**
     * Possibility of go back changed
     * \param bool true if it is possible to go back, false otherwise
     */
    boost::signals2::signal<void (bool)> backwardEnableChanged;

    /**
     * Page load finished
     */
    boost::signals2::signal<void ()> loadFinished;

    /**
     * Page load started
     */
    boost::signals2::signal<void ()> loadStarted;

    /**
     * Load progress changed
     * \param double 0..1 of progress
     */
    boost::signals2::signal<void (double)> loadProgress;

    /**
     * Page load stopped.
     */
    boost::signals2::signal<void ()> loadStop;

    /**
     * Page load error.
     */
    boost::signals2::signal<void ()> loadError;

    /**
     * Current tab changed
     * \param TabId of new tab
     */
    boost::signals2::signal<void (TabId)> currentTabChanged;

    /**
    * New tab was creted. It could be explicit call (by user) or tab could be opened from JavaScript.
    */
    boost::signals2::signal<void ()> tabCreated;

    /**
     * Tab closed
     * \param TabId of closed tab
     */
    boost::signals2::signal<void (TabId)> tabClosed;

    /**
     * Confirmation Reuest
     */
    boost::signals2::signal<void (basic_webengine::WebConfirmationPtr)> confirmationRequest;

    /**
     * Web Engine area clicked
     */
    boost::signals2::signal<void ()> webViewClicked;

    /**
     * All links to RSS/Atom channels were gathered from webpage.
     */
    boost::signals2::signal<void (std::vector<std::string>)> gotFeedLinks;

    /**
     * Status of IME
     * \param bool true if IME is opened, false otherwise
     */
    boost::signals2::signal<void (bool)> IMEStateChanged;
};

} /* end of basic_webengine */
} /* end of tizen_browser */

#endif /* __ABSTRACT_WEBENGINE_H__ */