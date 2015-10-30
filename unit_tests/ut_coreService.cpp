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

#include <memory>
#include <boost/test/unit_test.hpp>
#include <boost/any.hpp>

#include "ServiceManager.h"
#include "Services/ServiceInterface.h"

BOOST_AUTO_TEST_SUITE(core_ServiceManager)

namespace tbc=tizen_browser::core;

BOOST_AUTO_TEST_CASE(singleton)
{
//     check if service manager is singleton
    tbc::ServiceManager *serviceManager_instance_01 = &tbc::ServiceManager::getInstance();
    tbc::ServiceManager *serviceManager_instance_02 = &tbc::ServiceManager::getInstance();

    BOOST_CHECK_EQUAL(serviceManager_instance_01, serviceManager_instance_02);
}

BOOST_AUTO_TEST_CASE(getService)
{
    std::string serviceName("org.tizen.browser.TestServiceOne");
    tbc::ServiceManager *servManager = &tbc::ServiceManager::getInstance();
    std::shared_ptr<ServiceInterface> service1 = std::dynamic_pointer_cast
    <ServiceInterface,tbc::AbstractService>
    (servManager->getService(serviceName));
    BOOST_REQUIRE(service1);
    std::string nameFromService = service1->getName();
    BOOST_CHECK_EQUAL_COLLECTIONS(nameFromService.begin(),
                                 nameFromService.end(),
                                 serviceName.begin(),
                                 serviceName.end());
}

BOOST_AUTO_TEST_SUITE_END()

