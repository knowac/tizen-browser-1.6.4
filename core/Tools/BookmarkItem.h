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
 *     Author: k.dobkowski
 */

#ifndef BOOKMARKITEM_H
#define BOOKMARKITEM_H

#include <memory>
#include <list>
#include <vector>

#include "BrowserImage.h"

namespace tizen_browser{
namespace services{

class BookmarkItem
{
public:
    BookmarkItem();
    BookmarkItem(
        const std::string& url,
        const std::string& tittle,
        const std::string& note,
        unsigned int dir = 0,
        unsigned int id = 0
        );
    virtual ~BookmarkItem();

    void setAddress(const std::string & url) { m_url = url; };
    std::string getAddress() const { return m_url; };

    void setTittle(const std::string & tittle) { m_tittle = tittle; };
    std::string getTittle() const { return m_tittle; };

    void setNote(const std::string& note){m_note = note;};
    std::string getNote() const { return m_note;};

    void setId(int id) { m_saved_id = id; };
    int getId() const { return m_saved_id; };

    void setThumbnail(std::shared_ptr<tizen_browser::tools::BrowserImage> thumbnail);
    std::shared_ptr<tizen_browser::tools::BrowserImage> getThumbnail() const ;

    void setFavicon(std::shared_ptr<tizen_browser::tools::BrowserImage> favicon);
    std::shared_ptr<tizen_browser::tools::BrowserImage> getFavicon() const;

    void setDir(unsigned int dir){m_directory = dir;};
    unsigned int getDir() const {return m_directory;};

    void setTags(const std::vector<unsigned int>& tags) { m_tags = tags; };
    std::vector<unsigned int> getTags() const { return m_tags; };

private:
    unsigned int m_saved_id;
    std::string m_url;
    std::string m_tittle;
    std::string m_note;
    std::shared_ptr<tizen_browser::tools::BrowserImage> m_thumbnail;
    std::shared_ptr<tizen_browser::tools::BrowserImage> m_favicon;
    unsigned int m_directory;
    std::vector<unsigned int> m_tags;
};

typedef std::shared_ptr<BookmarkItem> SharedBookmarkItem;
typedef std::vector<SharedBookmarkItem> SharedBookmarkItemList;

}
}

#endif // BOOKMARKITEM_H
