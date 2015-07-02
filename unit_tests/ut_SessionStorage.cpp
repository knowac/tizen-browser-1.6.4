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

#include <boost/any.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <iterator>
#include "core/ServiceManager/ServiceManager.h"
#include "core/ServiceManager/AbstractService.h"
#include "services/SessionStorage/SessionStorage.h"
#include "services/SessionStorage/SqlStorage.h"
#include "services/SessionStorage/Session.h"
#include "AbstractWebEngine/TabId.h"
#include <core/Config/Config.h>

BOOST_AUTO_TEST_SUITE(SessionStorage)

BOOST_AUTO_TEST_CASE(InitSession)
{
    tizen_browser::config::DefaultConfig config;
    config.load("not used text");
    std::string resourceDbDir(boost::any_cast < std::string > (config.get("resourcedb/dir")));
    std::string sessionDb(boost::any_cast < std::string > (config.get("DB_SESSION")));

    boost::filesystem::path dbFile(resourceDbDir + sessionDb);
    boost::filesystem::remove(dbFile);

    std::shared_ptr<tizen_browser::services::SessionStorage> sessionService =
    std::dynamic_pointer_cast
    <
        tizen_browser::services::SessionStorage,
        tizen_browser::core::AbstractService
    >(tizen_browser::core::ServiceManager::getInstance().getService(DOMAIN_SESSION_STORAE_SERVICE));

    BOOST_REQUIRE(sessionService);
    BOOST_CHECK_EQUAL(sessionService->getName(), DOMAIN_SESSION_STORAE_SERVICE);

    tizen_browser::Session::SqlStorage* storage=0;
    storage = sessionService->getStorage();

    BOOST_REQUIRE(storage);
}

BOOST_AUTO_TEST_CASE(CreateAndPopulateSession)
{
    std::shared_ptr<tizen_browser::services::SessionStorage> sessionService =
    std::dynamic_pointer_cast
    <
        tizen_browser::services::SessionStorage,
        tizen_browser::core::AbstractService
    >(tizen_browser::core::ServiceManager::getInstance().getService(DOMAIN_SESSION_STORAE_SERVICE));

    BOOST_REQUIRE(sessionService);

    tizen_browser::Session::SqlStorage* storage=0;
    storage = sessionService->getStorage();

    BOOST_REQUIRE(storage);

    tizen_browser::Session::Session session(storage->createSession());

    BOOST_CHECK(session.isValid());

    std::map<std::string, std::string> urls;

    urls["54379ff6-f9ff-4ef3-99b0-a0de00edd473"] = "http://www.calligra.org";
    urls["7b5719d4-c2f5-4d87-89ff-9cd70da1710f"] = "http://www.kde.org";
    urls["ce18e8e2-8d33-4ba7-9fc4-d602cdf3fa36"] = "http://www.krita.org";

    for(auto iter = urls.begin(), end = urls.end(); iter != end; iter++){
        session.updateItem(iter->first, iter->second);
    }


    BOOST_CHECK_EQUAL(session.items().size(), 3);

    std::string replaceUrl("https://marble.kde.org/");


    session.updateItem(std::next(urls.begin(),2)->first,replaceUrl);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        replaceUrl.begin(),
        replaceUrl.end(),
        session.items().at(std::next(urls.begin(),2)->first).begin(),
        session.items().at(std::next(urls.begin(),2)->first).end()
    );

    session.removeItem(std::next(urls.begin(),1)->first);

    BOOST_CHECK_EQUAL(session.items().size(), 2);

}

BOOST_AUTO_TEST_CASE(getLastSession)
{
    std::shared_ptr<tizen_browser::services::SessionStorage> sessionService =
    std::dynamic_pointer_cast
    <
        tizen_browser::services::SessionStorage,
        tizen_browser::core::AbstractService
    >(tizen_browser::core::ServiceManager::getInstance().getService(DOMAIN_SESSION_STORAE_SERVICE));

    BOOST_REQUIRE(sessionService);

    tizen_browser::Session::SqlStorage* storage=0;
    storage = sessionService->getStorage();

    BOOST_REQUIRE(storage);

    sleep(2);
    tizen_browser::Session::Session session(storage->createSession());
    const std::string newSessionName("theLastOfUs");

    session.setSessionName("theLastOfUs");

    BOOST_CHECK(session.isValid());

    std::map<std::string, std::string> urls;

    urls["54379ff6-f9ff-4ef3-99b0-a0de00edd473"] = "http://www.calligra.org";
    urls["7b5719d4-c2f5-4d87-89ff-9cd70da1710f"] = "http://www.kde.org";
    urls["ce18e8e2-8d33-4ba7-9fc4-d602cdf3fa36"] = "http://www.krita.org";

    for(auto iter = urls.begin(), end = urls.end(); iter != end; iter++){
        session.updateItem(iter->first, iter->second);
    }

    BOOST_CHECK_EQUAL(session.items().size(), 3);

    tizen_browser::Session::Session lastSession(storage->getLastSession());

    BOOST_REQUIRE(lastSession.isValid());

    BOOST_CHECK_EQUAL_COLLECTIONS(
        newSessionName.begin(),
        newSessionName.end(),
        lastSession.sessionName().begin(),
        lastSession.sessionName().end()
    );

    BOOST_CHECK_EQUAL(lastSession.items().size(), 3);
}

BOOST_AUTO_TEST_CASE(getAllSessions)
{
    std::shared_ptr<tizen_browser::services::SessionStorage> sessionService =
    std::dynamic_pointer_cast
    <
        tizen_browser::services::SessionStorage,
        tizen_browser::core::AbstractService
    >(tizen_browser::core::ServiceManager::getInstance().getService(DOMAIN_SESSION_STORAE_SERVICE));

    BOOST_REQUIRE(sessionService);

    tizen_browser::Session::SqlStorage* storage=0;
    storage = sessionService->getStorage();

    BOOST_REQUIRE(storage);

    //new session should be newer then previous one.
    tizen_browser::Session::SessionsVector sessions(storage->getAllSessions());

    BOOST_CHECK_EQUAL(sessions.size(), 2);


}


BOOST_AUTO_TEST_CASE(deleteSession)
{
    std::shared_ptr<tizen_browser::services::SessionStorage> sessionService =
    std::dynamic_pointer_cast
    <
        tizen_browser::services::SessionStorage,
        tizen_browser::core::AbstractService
    >(tizen_browser::core::ServiceManager::getInstance().getService(DOMAIN_SESSION_STORAE_SERVICE));

    BOOST_REQUIRE(sessionService);

    tizen_browser::Session::SqlStorage* storage=0;
    storage = sessionService->getStorage();

    BOOST_REQUIRE(storage);

    //new session should be newer then previous one.
    tizen_browser::Session::SessionsVector sessions(storage->getAllSessions());

    storage->deleteSession(sessions.at(0));

    tizen_browser::Session::SessionsVector sessionsBucket(storage->getAllSessions());

    BOOST_CHECK_EQUAL(sessionsBucket.size(),1);
}


BOOST_AUTO_TEST_CASE(deleteAllSessions)
{
    std::shared_ptr<tizen_browser::services::SessionStorage> sessionService =
    std::dynamic_pointer_cast
    <
        tizen_browser::services::SessionStorage,
        tizen_browser::core::AbstractService
    >(tizen_browser::core::ServiceManager::getInstance().getService(DOMAIN_SESSION_STORAE_SERVICE));

    BOOST_REQUIRE(sessionService);

    tizen_browser::Session::SqlStorage* storage=0;
    storage = sessionService->getStorage();

    BOOST_REQUIRE(storage);

    //new session should be newer then previous one.
    sleep(1);
    tizen_browser::Session::Session session2(storage->createSession());
    sleep(1);
    tizen_browser::Session::Session session3(storage->createSession());
    sleep(1);
    tizen_browser::Session::Session session4(storage->createSession());


    std::map<std::string, std::string> urls;

    urls["54379ff6-f9ff-4ef3-99b0-a0de00edd473"] = "http://www.calligra.org";
    urls["7b5719d4-c2f5-4d87-89ff-9cd70da1710f"] = "http://www.kde.org";
    urls["ce18e8e2-8d33-4ba7-9fc4-d602cdf3fa36"] = "http://www.krita.org";

    for(auto iter = urls.begin(), end = urls.end(); iter != end; iter++){
        session2.updateItem(iter->first, iter->second);
        sleep(1);
        session3.updateItem(iter->first, iter->second);
        sleep(1);
        session4.updateItem(iter->first, iter->second);
    }


    tizen_browser::Session::SessionsVector sessionsBucket(storage->getAllSessions());

    BOOST_CHECK_EQUAL(sessionsBucket.size(), 4);

    storage->deleteAllSessions();

    sessionsBucket = storage->getAllSessions();

    BOOST_CHECK_EQUAL(sessionsBucket.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()