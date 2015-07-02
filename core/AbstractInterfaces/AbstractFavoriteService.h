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

/*
 *
 * Created on: Apr, 2014
 *     Author: m.kielak
 */

#ifndef ABSTRACTFAVORITESERVICE_H
#define ABSTRACTFAVORITESERVICE_H

#include "browser_config.h"

#include <vector>
#include <boost/signals2/signal.hpp>
#include <Evas.h>

#include "AbstractService.h"
#include "service_macros.h"
#include "BookmarkItem.h"
#include "BrowserImage.h"
#include "StorageService.h"
#include "Config/Config.h"

namespace tizen_browser{
namespace interfaces{

class AbstractFavoriteService
        : public tizen_browser::core::AbstractService
{
public:

    /**
     * @brief Add page to bookmarks
     *
     * @param address Webpage url.
     * @param tittle Title of bookmark.
     * @param note Bookmark note, default is empty .
     * @param dirId Directory numeric ID, default is 0.
     * @param thumbnail Page thumbnail, default is empty image.
     * @param favicon Page favicon image, default is empty image.
     *
     * @return BookmarkItem class
     */
    virtual std::shared_ptr<tizen_browser::services::BookmarkItem> addToBookmarks(const std::string & address,
                                                 const std::string & tittle,
                                                 const std::string & note = std::string(),
                                                 std::shared_ptr<tizen_browser::tools::BrowserImage> thumbnail=std::shared_ptr<tizen_browser::tools::BrowserImage>(),
                                                 std::shared_ptr<tizen_browser::tools::BrowserImage> favicon = std::shared_ptr<tizen_browser::tools::BrowserImage>(),
                                                 unsigned int dirId = 0) = 0;
    /**
     * @brief Count bookmarks and subfolders
     * @return Number of bookmarks and subfolders
     */
    virtual int countBookmarksAndSubFolders() = 0;

    /** \todo Need to change this callback function for finding stored bookmark, check getBookmarkId function
     * @brief Check if bookmark exists
     *
     * @param url url to find
     * @return true if exists, false if not
     */
     virtual bool bookmarkExists(const std::string & url)= 0;

    /**
     * @brief Get bookmarks from platform service and store it in private m_bookmarksList
     *
     * @return list of bookmark items
     */
    virtual std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > getBookmarks()= 0;

    /**
     * @brief Delete all bookmarks
     *
     * @return true if success, false on error
     */
    virtual bool deleteAllBookmarks() = 0;

    /**
     * @brief Delete bookmark by given url
     *
     * @param url of bookmark to delete
     * @return true if success, false on error of not found bookmark
     */
    virtual bool deleteBookmark(const std::string & url) = 0;


    virtual void synchronizeBookmarks() = 0;

    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> bookmarkAdded;
    boost::signals2::signal<void (const std::string& uri)> bookmarkDeleted;
    boost::signals2::signal<void ()> bookmarksDeleted;

};

}
}

#endif // FAVORITESERVICE_H