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

#include <string>
#include <BrowserAssert.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/date.hpp>
#include <boost/date_time/date_defs.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "ServiceManager.h"
#include "HistoryService.h"
#include "HistoryItem.h"
#include "AbstractWebEngine.h"
#include "EflTools.h"

namespace tizen_browser
{
namespace services
{

EXPORT_SERVICE(HistoryService, DOMAIN_HISTORY_SERVICE)

HistoryService::HistoryService() : m_testDbMod(false)
{
    BROWSER_LOGD("HistoryService");
}

HistoryService::~HistoryService()
{
}

std::shared_ptr<tizen_browser::services::StorageService> HistoryService::getStorageManager()
{
    if (!m_storageManager) {
        m_storageManager = std::dynamic_pointer_cast <
                           tizen_browser::services::StorageService,
                           tizen_browser::core::AbstractService > (
                               tizen_browser::core::ServiceManager::getInstance().getService(
                                   DOMAIN_STORAGE_SERVICE));
    }

    M_ASSERT(m_storageManager);
    m_storageManager->init(m_testDbMod);

    return m_storageManager;
}

void HistoryService::setStorageServiceTestMode(bool testmode) {
	m_testDbMod = testmode;
}

int HistoryService::getHistoryItemsCount(){
    return history_list.size();
}

static int __get_duplicated_ids_p(int **ids, int *count, const int limit, const int offset,
		const bp_history_offset order_column_offset, const int ordering,
		const bp_history_offset check_column_offset,
		const char *keyword, const int is_like)
{
    bp_history_rows_cond_fmt conds;
    memset(&conds, 0x00, sizeof(bp_history_rows_cond_fmt));

    conds.limit = limit;
    conds.offset = offset;
    conds.ordering = ordering;
    conds.order_offset = order_column_offset;
    conds.period_offset = BP_HISTORY_O_DATE_CREATED;
    conds.period_type = BP_HISTORY_DATE_ALL;

    return bp_history_adaptor_get_cond_ids_p
	(ids, count,
	 &conds,
	 check_column_offset,
	 keyword,
	 is_like);
}

void HistoryService::addHistoryItem(std::shared_ptr<HistoryItem> his){

    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    his->setFavIcon(his->getFavIcon());
    std::shared_ptr<tizen_browser::tools::BrowserImage> favicon = his->getFavIcon();

    int id = -1;
    int ret = bp_history_adaptor_create(&id);
    if (ret<0){
        BROWSER_LOGE("Error! Could not create new bookmark!");
    }

    int *ids=NULL;
    int count=-1;
    int **id1=&ids;
    int *count1=&count;

    bp_history_rows_cond_fmt conds;
    conds.limit = 20;  //no of rows to get negative means no limitation

    conds.offset = -1;   //the first row's index
    conds.order_offset =BP_HISTORY_O_DATE_CREATED; // property to sort

    conds.ordering = 1; //way of ordering 0 asc 1 desc

    conds.period_offset = BP_HISTORY_O_DATE_CREATED;
    conds.period_type = BP_HISTORY_DATE_TODAY;

    ret = bp_history_adaptor_get_cond_ids_p(id1 ,count1, &conds, 0, NULL, 0);
    if (ret<0){
        BROWSER_LOGE("Error! Could not get ids!");
    }

    bp_history_adaptor_set_url(id, (his->getUrl()).c_str());
    bp_history_adaptor_set_title(id, (his->getTitle()).c_str());
    bp_history_adaptor_set_date_visited(id,-1);

    std::unique_ptr<tizen_browser::tools::Blob> favicon_blob = tizen_browser::tools::EflTools::getBlobPNG(favicon);
    unsigned char * fav = std::move((unsigned char*)favicon_blob->getData());
    bp_history_adaptor_set_icon(id, favicon->width, favicon->height, fav, favicon_blob->getLength());
    history_list.push_back(his);

    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}


void HistoryService::insertOrRefresh(std::shared_ptr<HistoryItem> hi) {
	getStorageManager()->insertOrRefresh(hi);
}

/**
 * @throws HistoryException on error
 */
void HistoryService::clearAllHistory()
{
    bp_history_adaptor_reset();
    history_list.clear();
}

int HistoryService::getHistoryId(const std::string & url)
{
    bp_history_rows_cond_fmt conds;
    conds.limit = -1;
    conds.offset = 0;
    conds.order_offset = BP_HISTORY_O_DATE_CREATED;
    conds.ordering = 0;
    conds.period_offset = BP_HISTORY_O_DATE_CREATED;
    conds.period_type = BP_HISTORY_DATE_ALL;
    int *ids = 0;
    int ids_count = 0;
    int ret = bp_history_adaptor_get_cond_ids_p(&ids ,&ids_count, &conds, BP_HISTORY_O_URL, url.c_str(), 0);
    if (ids_count!=0){
        int i = *ids;
        free(ids);
        return i;
    }
    return 0;
}

/**
 * @throws HistoryException on error
 */
void HistoryService::clearURLHistory(const std::string & url)
{
    int id = getHistoryId(url);
    if (id != 0)
        bp_history_adaptor_delete(id);
    if (0 == (getHistoryItemsCount() - 1)) {
        historyEmpty(true);
        history_list.clear();
    }
}


HistoryItemVector& HistoryService::getHistoryItems(int historyDepthInDays, int maxItems)
{
    history_list.clear();

    int *ids=NULL;
    int count=-1;
    int **id1=&ids;
    int *count1=&count;

    bp_history_rows_cond_fmt conds;
    conds.limit = 20;  //no of rows to get negative means no limitation

    conds.offset = -1;   //the first row's index
    conds.order_offset =BP_HISTORY_O_DATE_CREATED; // property to sort

    conds.ordering = 1; //way of ordering 0 asc 1 desc

    conds.period_offset = BP_HISTORY_O_DATE_CREATED;

    conds.period_type = BP_HISTORY_DATE_TODAY;

    int ret = bp_history_adaptor_get_cond_ids_p(id1 ,count1, &conds, 0, NULL, 0);
    if (ret<0){
        BROWSER_LOGD("Error! Could not get ids!");
    }

    bp_history_offset offset = (BP_HISTORY_O_URL | BP_HISTORY_O_TITLE | BP_HISTORY_O_FAVICON | BP_HISTORY_O_DATE_CREATED);

    for(int i = 0; i< (*count1); i++){
        bp_history_info_fmt history_info;
        bp_history_adaptor_get_info(ids[i],offset,&history_info);

        int date;
        bp_history_adaptor_get_date_created(ids[i], &date);

        struct tm *item_time_info;
        time_t item_time = (time_t)date;
        item_time_info = localtime(&item_time);

        int m_year = item_time_info->tm_year;
        int m_month = item_time_info->tm_mon + 1;
        int m_day = item_time_info->tm_yday;
        int m_month_day = item_time_info->tm_mday;
        int m_date = date;
        int min = item_time_info->tm_min;
        int hour= item_time_info->tm_hour;
        int sec = item_time_info->tm_sec;
        m_year = 2000 + m_year % 100;

        std::shared_ptr<HistoryItem> history = std::make_shared<HistoryItem>(std::string(history_info.url));
        boost::gregorian::date d(m_year,m_month,m_month_day);
        boost::posix_time::ptime t(d,boost::posix_time::time_duration(hour,min,sec));
        history->setLastVisit(t);
        history->setUrl(std::string(history_info.url ? history_info.url : ""));
        history->setTitle(std::string(history_info.title ? history_info.title : ""));
        history_list.push_back(history);
    }
    ids = NULL;
    free(ids);

    return history_list;
}

int HistoryService::getHistoryVisitCounter(const std::string & url)
{
    return getStorageManager()->getHistoryVisitCounter(url);
}

}
}
