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
#include <boost/any.hpp>

#include "Field.h"
#include "BrowserLogger.h"
#include "DriverManager.h"
#include "Blob.h"
#include "StorageException.h"
#include "StorageExceptionInitialization.h"
#include "EflTools.h"
#include "Config.h"
#include "BookmarksStorage.h"
#include "BookmarksSQLite.h"
#include "StorageService.h"

namespace
{
// ------ Database SETTINGS ------
const std::string TABLE_SETTINGS = "SETTINGS";
const std::string COL_SETTINGS_KEY = "KEY";
const std::string COL_SETTINGS_VALUE_INT = "VALUE_INT";
const std::string COL_SETTINGS_VALUE_DOUBLE = "VALUE_DOUBLE";
const std::string COL_SETTINGS_VALUE_TEXT = "VALUE_TEXT";

const std::string DDL_CREATE_TABLE_SETTINGS = "CREATE TABLE " + TABLE_SETTINGS
                                              + " (" + COL_SETTINGS_KEY + " TEXT PRIMARY KEY, "
                                              + COL_SETTINGS_VALUE_INT + " INTEGER, "
                                              + COL_SETTINGS_VALUE_DOUBLE + " DOUBLE, "
                                              + COL_SETTINGS_VALUE_TEXT + " TEXT)";

const std::string INSERT_TABLE_SETTINGS_INT_VALUE = "insert or replace into " + TABLE_SETTINGS + " ("
                                                    + COL_SETTINGS_KEY + ", "
                                                    + COL_SETTINGS_VALUE_INT
                                                    + ") values (?,?)";

const std::string INSERT_TABLE_SETTINGS_DOUBLE_VALUE = "insert or replace into " + TABLE_SETTINGS + " ("
                                                       + COL_SETTINGS_KEY + ", "
                                                       + COL_SETTINGS_VALUE_DOUBLE
                                                       + ") values (?,?)";

const std::string INSERT_TABLE_SETTINGS_TEXT_VALUE = "insert or replace into " + TABLE_SETTINGS + " ("
                                                     + COL_SETTINGS_KEY + ", "
                                                     + COL_SETTINGS_VALUE_TEXT
                                                     + ") values (?,?)";

const std::string DELETE_SETTING = "delete from " + TABLE_SETTINGS + " where "
                                   + COL_SETTINGS_KEY + "=?";

const std::string DELETE_ALL_SETTINGS = "delete from " + TABLE_SETTINGS;

const std::string SQL_FIND_VALUE_INT_SETTINGS = "select " + COL_SETTINGS_VALUE_INT + " from " + TABLE_SETTINGS
                                                + " where " + COL_SETTINGS_KEY + "=?";

const std::string SQL_FIND_VALUE_DOUBLE_SETTINGS = "select " + COL_SETTINGS_VALUE_DOUBLE + " from " + TABLE_SETTINGS
                                                   + " where " + COL_SETTINGS_KEY + "=?";

const std::string SQL_FIND_VALUE_TEXT_SETTINGS = "select " + COL_SETTINGS_VALUE_TEXT + " from " + TABLE_SETTINGS
                                                 + " where " + COL_SETTINGS_KEY + "=?";

// ------ (end) Database SETTINGS ------

}

namespace tizen_browser
{
namespace services
{

EXPORT_SERVICE(StorageService, DOMAIN_STORAGE_SERVICE)

void StorageService::init(bool testmode)
{
    if (m_isInitialized) {
        return;
    }

    config.load("whatever");

    std::string resourceDbDir;
    std::string dbSettings;

    if (!testmode) {
        resourceDbDir = boost::any_cast < std::string > (config.get("resourcedb/dir"));
        dbSettings = boost::any_cast < std::string > (config.get("DB_SETTINGS"));
    } else {
        resourceDbDir = boost::any_cast < std::string > (config.get("resourcedb/dir"));
        dbSettings = "settings_test.db";
    }

    DB_SETTINGS = resourceDbDir + dbSettings;

    BROWSER_LOGD("DB_SETTINGS=%s", DB_SETTINGS.c_str());

    try {
        initDatabaseSettings(DB_SETTINGS);
    } catch (storage::StorageExceptionInitialization & e) {
        BROWSER_LOGE("Cannot initialize database %s!", DB_SETTINGS.c_str());
    }

    initHistoryService("SQLITE", testmode);

    m_isInitialized = true;
}

StorageService::StorageService()
    : m_dbSettingsInitialised(false)
    , m_isInitialized(false)
{
    BROWSER_LOGD("StorageService");
}

StorageService::~StorageService()
{
}

void StorageService::checkAndCreateTable(const std::string & db_str, const std::string & tablename, const std::string & ddl)
{
    auto conCheckExistance = storage::DriverManager::getDatabase(db_str);

    bool isTabPresent = conCheckExistance->tableExists(tablename);
    if (!isTabPresent) {
        BROWSER_LOGI("A table %s can't be found. It will be recreated.", tablename.c_str());

        conCheckExistance->exec(ddl);
    }
}

void StorageService::checkAndCreateTable(storage::SQLTransactionScope * transactionScope, const std::string & tablename, const std::string & ddl)
{
    auto conCheckExistance = transactionScope->database();

    bool isTabPresent = conCheckExistance->tableExists(tablename);

    if (!isTabPresent) {
        BROWSER_LOGI("A table %s can't be found. It will be recreated.", tablename.c_str());

        conCheckExistance->exec(ddl);
    }
}

/**
 * @throws StorageExceptionInitialization on error
 */
void StorageService::initDatabaseSettings(const std::string & db_str)
{
    BROWSER_LOGI("StorageService::initDatabaseSettings begin");

    if (!m_dbSettingsInitialised) {
        try {
            checkAndCreateTable(db_str, TABLE_SETTINGS, DDL_CREATE_TABLE_SETTINGS);
        } catch (storage::StorageException & e) {
            throw storage::StorageExceptionInitialization(e.getMessage(),
                                                          e.getErrorCode());
        }

        m_dbSettingsInitialised = true;
    }

    M_ASSERT(m_dbSettingsInitialised);

    BROWSER_LOGI("StorageService::initDatabaseSettings end");
}

/**
 * @throws StorageException on error
 */
int StorageService::getSettingsInt(const std::string & key, const int defaultValue) const
{
    auto con = storage::DriverManager::getDatabase(DB_SETTINGS);

    storage::SQLQuery select(con->prepare(SQL_FIND_VALUE_INT_SETTINGS));
    select.bindText(1, key);
    select.exec();

    if (select.hasNext()) {
        return select.getInt(0);
    }

    return defaultValue;
}

/**
 * @throws StorageException on error
 */
double StorageService::getSettingsDouble(const std::string & key, const double defaultValue) const
{
    auto con = storage::DriverManager::getDatabase(DB_SETTINGS);

    storage::SQLQuery select(con->prepare(SQL_FIND_VALUE_DOUBLE_SETTINGS));
    select.bindText(1, key);
    select.exec();

    if (select.hasNext()) {
        return select.getDouble(0);
    }

    return defaultValue;
}

/**
 * @throws StorageException on error
 */
const std::string StorageService::getSettingsText(const std::string & key, const std::string & defaultValue) const
{
    auto con = storage::DriverManager::getDatabase(DB_SETTINGS);

    storage::SQLQuery select(con->prepare(SQL_FIND_VALUE_TEXT_SETTINGS));
    select.bindText(1, key);
    select.exec();

    if (select.hasNext()) {
        return select.getString(0);
    }

    return defaultValue;
}

/**
 * @throws StorageException on error
 */
void StorageService::setSettingsValue(const std::string & key, storage::FieldPtr field) const
{
    auto con = storage::DriverManager::getDatabase(DB_SETTINGS);

    storage::SQLQuery insert;

    switch (field->getType()) {
        case SQLITE_INTEGER:
            insert = con->prepare(INSERT_TABLE_SETTINGS_INT_VALUE);
            insert.bindInt(2, field->getInt());
            break;
        case SQLITE_FLOAT:
            insert = con->prepare(INSERT_TABLE_SETTINGS_DOUBLE_VALUE);
            insert.bindDouble(2, field->getDouble());
            break;
        case SQLITE3_TEXT:
            insert = con->prepare(INSERT_TABLE_SETTINGS_TEXT_VALUE);
            insert.bindText(2, field->getString());
            break;
        default:
            M_ASSERT(0);
            break;
    }

    insert.bindText(1, key);
    insert.exec();
}

/**
 * @throws StorageException on error
 */
void StorageService::setSettingsInt(const std::string & key, int value) const
{
    storage::FieldPtr field = std::make_shared<storage::Field>(value);
    setSettingsValue(key, field);
}

/**
 * @throws StorageException on error
 */
void StorageService::setSettingsDouble(const std::string & key, double value) const
{
    storage::FieldPtr field = std::make_shared<storage::Field>(value);
    setSettingsValue(key, field);
}

/**
 * @throws StorageException on error
 */
void StorageService::setSettingsString(const std::string & key, std::string value) const
{
    storage::FieldPtr field = std::make_shared<storage::Field>(value);
    setSettingsValue(key, field);
}

/**
 * @throws HistoryException on error
 */
void StorageService::addHistoryItem(std::shared_ptr<HistoryItem> hi)
{
    M_ASSERT(m_history);
    m_history->addHistoryItem(hi);
}

/**
 * If hi->getUrl() exists on a table HISTORY update visit_counter and visit_date, unless insert hi to database.
 *
 * @throws HistoryException on error
 */
void StorageService::insertOrRefresh(std::shared_ptr<HistoryItem> hi)
{
    M_ASSERT(m_history);
    m_history->insertOrRefresh(hi);
}

/**
 * @throws HistoryException on error
 */
std::shared_ptr<tizen_browser::tools::BrowserImage> StorageService::getFavicon(const std::string & url)
{
    M_ASSERT(m_history);
    return m_history->getFavicon(url);
}

/**
 * @throws HistoryException on error
 */
void StorageService::deleteHistory()
{
    M_ASSERT(m_history);
    m_history->deleteHistory();
}

/**
 * @throws HistoryException on error
 */
void StorageService::deleteHistory(const std::string & url)
{
    M_ASSERT(m_history);
    m_history->deleteHistory(url);
}

/**
 * @throws HistoryException on error
 */
std::shared_ptr<HistoryItem> StorageService::getHistoryItem(const std::string & url)
{
    M_ASSERT(m_history);
    return m_history->getHistoryItem(url);
}

/**
 * @throws HistoryException on error
 */
HistoryItemVector & StorageService::getHistoryItems(int historyDepthInDays, int maxItems)
{
    M_ASSERT(m_history);
    return m_history->getHistoryItems(historyDepthInDays, maxItems);
}

/**
 * @throws HistoryException on error
 */
int StorageService::getHistoryItemsCount()
{
    M_ASSERT(m_history);
    return m_history->getHistoryItemsCount();
}

/**
 * @throws HistoryException on error
 */
int StorageService::getHistoryVisitCounter(const std::string & url)
{
    M_ASSERT(m_history);
    return m_history->getHistoryVisitCounter(url);
}

void StorageService::initHistoryService(const std::string & storage, bool testmode)
{
    // Depend on "storage" variable use an appropriate class as a storage. By default it's a HistorySQLite class.
    if (m_history.get() == NULL) {
        if (storage.compare("SQLITE") == 0) {
            m_history = std::make_shared<HistorySQLite>();
            m_history->init(testmode);
        }
    }
}




}
}