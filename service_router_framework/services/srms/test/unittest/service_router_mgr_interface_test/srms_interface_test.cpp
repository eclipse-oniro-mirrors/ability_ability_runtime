/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define private public

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "appexecfwk_errors.h"
#include "service_info.h"
#include "service_router_data_mgr.h"
#include "service_router_mgr_proxy.h"
#include "want.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
const std::string WRONG_BUNDLE_NAME = "wrong";
const std::string MIME_TYPE = "html";
const std::string BUNDLE_NAME = "bundleName";
const std::string PURPOSE_NAME = "pay";
}  // namespace

class ServiceRouterMgrInterfaceTest : public testing::Test {
public:
    ServiceRouterMgrInterfaceTest();
    ~ServiceRouterMgrInterfaceTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

ServiceRouterMgrInterfaceTest::ServiceRouterMgrInterfaceTest()
{}

ServiceRouterMgrInterfaceTest::~ServiceRouterMgrInterfaceTest()
{}

void ServiceRouterMgrInterfaceTest::SetUpTestCase()
{}

void ServiceRouterMgrInterfaceTest::TearDownTestCase()
{}

void ServiceRouterMgrInterfaceTest::SetUp()
{}

void ServiceRouterMgrInterfaceTest::TearDown()
{}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest
 * @tc.name: test QueryBusinessAbilityInfos
 * @tc.require: issueI6HQLK
 * @tc.desc: 1. system running normally
 *           2. test serviceType is invalid
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0001, Function | SmallTest | Level0)
{
    auto serviceRouterMgr = std::make_shared<ServiceRouterDataMgr>();
    EXPECT_NE(serviceRouterMgr, nullptr);
    if (serviceRouterMgr != nullptr) {
        BusinessAbilityFilter filter;
        filter.businessType = BusinessType::UNSPECIFIED;
        std::vector<BusinessAbilityInfo> abilityInfos;
        auto ret = serviceRouterMgr->QueryBusinessAbilityInfos(filter, abilityInfos);
        EXPECT_EQ(ret, ERR_BUNDLE_MANAGER_PARAM_ERROR);
    }
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest
 * @tc.name: test QueryBusinessAbilityInfos
 * @tc.require: issueI6HQLK
 * @tc.desc: 1. system running normally
 *           2. test serviceType is valid
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0002, Function | SmallTest | Level0)
{
    auto serviceRouterMgr = std::make_shared<ServiceRouterDataMgr>();
    EXPECT_NE(serviceRouterMgr, nullptr);
    if (serviceRouterMgr != nullptr) {
        BusinessAbilityFilter filter;
        filter.businessType = BusinessType::SHARE;
        std::vector<BusinessAbilityInfo> abilityInfos;
        auto ret = serviceRouterMgr->QueryBusinessAbilityInfos(filter, abilityInfos);
        EXPECT_EQ(ret, ERR_OK);
    }
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest_0003
 * Function: BusinessAbilityFilter
 * @tc.name: test BusinessAbilityFilter
 * @tc.desc: BusinessAbilityFilter
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0003, Function | SmallTest | Level0)
{
    BusinessAbilityFilter filter;
    filter.mimeType = MIME_TYPE;
    Parcel parcel;
    auto result = BusinessAbilityFilter::Unmarshalling(parcel);
    EXPECT_NE(result->mimeType, MIME_TYPE);
    auto ret = filter.Marshalling(parcel);
    EXPECT_TRUE(ret);
    result = BusinessAbilityFilter::Unmarshalling(parcel);
    EXPECT_EQ(result->mimeType, MIME_TYPE);
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest
 * @tc.name: test QueryPurposeInfos
 * @tc.require: issueI6HQLK
 * @tc.desc: 1. system running normally
 *           2. test purposeName empty
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0004, Function | SmallTest | Level0)
{
    auto serviceRouterMgr = std::make_shared<ServiceRouterDataMgr>();
    EXPECT_NE(serviceRouterMgr, nullptr);
    if (serviceRouterMgr != nullptr) {
        Want want;
        std::vector<PurposeInfo> purposeInfos;
        auto ret = serviceRouterMgr->QueryPurposeInfos(want, "", purposeInfos);
        EXPECT_EQ(ret, ERR_BUNDLE_MANAGER_PARAM_ERROR);
    }
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest
 * @tc.name: test QueryPurposeInfos
 * @tc.require: issueI6HQLK
 * @tc.desc: 1. system running normally
 *           2. test purposeName is valid
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0005, Function | SmallTest | Level0)
{
    auto serviceRouterMgr = std::make_shared<ServiceRouterDataMgr>();
    EXPECT_NE(serviceRouterMgr, nullptr);
    if (serviceRouterMgr != nullptr) {
        Want want;
        std::vector<PurposeInfo> purposeInfos;
        auto ret = serviceRouterMgr->QueryPurposeInfos(want, PURPOSE_NAME, purposeInfos);
        EXPECT_EQ(ret, ERR_OK);
    }
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest
 * @tc.name: test QueryPurposeInfos
 * @tc.require: issueI6HQLK
 * @tc.desc: 1. system running normally
 *           2. test bundleName not found
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0006, Function | SmallTest | Level0)
{
    auto serviceRouterMgr = std::make_shared<ServiceRouterDataMgr>();
    EXPECT_NE(serviceRouterMgr, nullptr);
    if (serviceRouterMgr != nullptr) {
        Want want;
        want.SetElementName(WRONG_BUNDLE_NAME, "");
        std::vector<PurposeInfo> purposeInfos;
        auto ret = serviceRouterMgr->QueryPurposeInfos(want, PURPOSE_NAME, purposeInfos);
        EXPECT_EQ(ret, ERR_BUNDLE_MANAGER_BUNDLE_NOT_EXIST);
    }
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest_0007
 * Function: AppInfo
 * @tc.name: test AppInfo
 * @tc.desc: AppInfo
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0007, Function | SmallTest | Level0)
{
    AppInfo info;
    info.bundleName = BUNDLE_NAME;
    Parcel parcel;
    auto result = AppInfo::Unmarshalling(parcel);
    EXPECT_NE(result->bundleName, BUNDLE_NAME);
    auto ret = info.Marshalling(parcel);
    EXPECT_TRUE(ret);
    result = AppInfo::Unmarshalling(parcel);
    EXPECT_EQ(result->bundleName, BUNDLE_NAME);
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest_0008
 * Function: BusinessAbilityInfo
 * @tc.name: test BusinessAbilityInfo
 * @tc.desc: BusinessAbilityInfo
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0008, Function | SmallTest | Level0)
{
    BusinessAbilityInfo info;
    info.bundleName = BUNDLE_NAME;
    Parcel parcel;
    auto ret = info.Marshalling(parcel);
    EXPECT_TRUE(ret);
    auto result = BusinessAbilityInfo::Unmarshalling(parcel);
    EXPECT_EQ(result->bundleName, BUNDLE_NAME);
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest_0009
 * Function: PurposeInfo
 * @tc.name: test PurposeInfo
 * @tc.desc: PurposeInfo
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0009, Function | SmallTest | Level0)
{
    PurposeInfo info;
    info.bundleName = BUNDLE_NAME;
    Parcel parcel;
    auto ret = info.Marshalling(parcel);
    EXPECT_TRUE(ret);
    auto result = PurposeInfo::Unmarshalling(parcel);
    EXPECT_EQ(result->bundleName, BUNDLE_NAME);
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest_0010
 * Function: BusinessAbilityFilter
 * @tc.name: test BusinessAbilityFilter
 * @tc.desc: BusinessAbilityFilter
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0010, Function | SmallTest | Level0)
{
    BusinessAbilityFilter filter;
    filter.mimeType = MIME_TYPE;
    Parcel parcel;
    auto ret = filter.Marshalling(parcel);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest_0011
 * Function: BusinessAbilityFilter
 * @tc.name: test BusinessAbilityFilter
 * @tc.desc: BusinessAbilityFilter
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0011, Function | SmallTest | Level0)
{
    BusinessAbilityFilter filter;
    filter.mimeType = MIME_TYPE;
    Parcel parcel;
    auto ret = filter.ReadFromParcel(parcel);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest_0012
 * Function: AppInfo
 * @tc.name: test AppInfo
 * @tc.desc: AppInfo
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0012, Function | SmallTest | Level0)
{
    AppInfo info;
    info.bundleName = BUNDLE_NAME;
    Parcel parcel;
    auto ret = info.Marshalling(parcel);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest_0013
 * Function: AppInfo
 * @tc.name: test AppInfo
 * @tc.desc: AppInfo
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0013, Function | SmallTest | Level0)
{
    AppInfo info;
    info.bundleName = BUNDLE_NAME;
    Parcel parcel;
    auto ret = info.ReadFromParcel(parcel);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest_0014
 * Function: PurposeInfo
 * @tc.name: test PurposeInfo
 * @tc.desc: PurposeInfo
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0014, Function | SmallTest | Level0)
{
    PurposeInfo info;
    info.bundleName = BUNDLE_NAME;
    Parcel parcel;
    auto ret = info.Marshalling(parcel);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest_0015
 * Function: PurposeInfo
 * @tc.name: test PurposeInfo
 * @tc.desc: PurposeInfo
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0015, Function | SmallTest | Level0)
{
    PurposeInfo info;
    info.bundleName = BUNDLE_NAME;
    Parcel parcel;
    auto ret = info.ReadFromParcel(parcel);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest_0016
 * Function: PurposeInfo
 * @tc.name: test PurposeInfo
 * @tc.desc: PurposeInfo
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0016, Function | SmallTest | Level0)
{
    PurposeInfo info;
    info.bundleName = BUNDLE_NAME;
    Parcel parcel;
    parcel.WriteParcelable(&info);
    auto ret = info.ReadFromParcel(parcel);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest_0017
 * Function: BusinessAbilityInfo
 * @tc.name: test BusinessAbilityInfo
 * @tc.desc: BusinessAbilityInfo
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0017, Function | SmallTest | Level0)
{
    BusinessAbilityInfo info;
    info.bundleName = BUNDLE_NAME;
    Parcel parcel;
    auto ret = info.Marshalling(parcel);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest_0018
 * Function: BusinessAbilityInfo
 * @tc.name: test BusinessAbilityInfo
 * @tc.desc: BusinessAbilityInfo
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0018, Function | SmallTest | Level0)
{
    BusinessAbilityInfo info;
    info.bundleName = BUNDLE_NAME;
    Parcel parcel;
    auto ret = info.ReadFromParcel(parcel);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest_0019
 * Function: BusinessAbilityInfo
 * @tc.name: test BusinessAbilityInfo
 * @tc.desc: BusinessAbilityInfo
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0019, Function | SmallTest | Level0)
{
    BusinessAbilityInfo info;
    info.businessType = BusinessType::SHARE;
    Parcel parcel;
    parcel.WriteParcelable(&info);
    auto ret = info.ReadFromParcel(parcel);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest
 * @tc.name: test LoadAllBundleInfos
 * @tc.desc: test LoadAllBundleInfos function
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0020, Function | SmallTest | Level0)
{
    auto serviceRouterMgr = std::make_shared<ServiceRouterDataMgr>();
    EXPECT_NE(serviceRouterMgr, nullptr);
    if (serviceRouterMgr != nullptr) {
        auto ret = serviceRouterMgr->LoadAllBundleInfos();
        EXPECT_EQ(ret, true);
    }
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest
 * @tc.name: test LoadBundleInfo
 * @tc.desc: test LoadBundleInfo function
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0021, Function | SmallTest | Level0)
{
    auto serviceRouterMgr = std::make_shared<ServiceRouterDataMgr>();
    EXPECT_NE(serviceRouterMgr, nullptr);
    if (serviceRouterMgr != nullptr) {
        std::string bundleName = BUNDLE_NAME;
        auto ret = serviceRouterMgr->LoadBundleInfo(bundleName);
        EXPECT_EQ(ret, false);
    }
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest
 * @tc.name: test UpdateBundleInfoLocked
 * @tc.desc: test UpdateBundleInfoLocked function
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0022, Function | SmallTest | Level0)
{
    auto serviceRouterMgr = std::make_shared<ServiceRouterDataMgr>();
    EXPECT_NE(serviceRouterMgr, nullptr);
    if (serviceRouterMgr != nullptr) {
        BundleInfo bundleInfo;
        serviceRouterMgr->UpdateBundleInfoLocked(bundleInfo);
    }
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest
 * @tc.name: test DeleteBundleInfo
 * @tc.desc: test DeleteBundleInfo function
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0023, Function | SmallTest | Level0)
{
    auto serviceRouterMgr = std::make_shared<ServiceRouterDataMgr>();
    EXPECT_NE(serviceRouterMgr, nullptr);
    if (serviceRouterMgr != nullptr) {
        std::string bundleName = BUNDLE_NAME;
        serviceRouterMgr->DeleteBundleInfo(bundleName);
    }
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest
 * @tc.name: test GetBusinessType
 * @tc.desc: test GetBusinessType function 1
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0024, Function | SmallTest | Level0)
{
    auto serviceRouterMgr = std::make_shared<ServiceRouterDataMgr>();
    EXPECT_NE(serviceRouterMgr, nullptr);
    if (serviceRouterMgr != nullptr) {
        BusinessAbilityFilter filter;
        filter.businessType = BusinessType::SHARE;
        auto ret = serviceRouterMgr->GetBusinessType(filter);
        EXPECT_EQ(ret, BusinessType::SHARE);
    }
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest
 * @tc.name: test GetBusinessType
 * @tc.desc: test GetBusinessType function 2
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0025, Function | SmallTest | Level0)
{
    auto serviceRouterMgr = std::make_shared<ServiceRouterDataMgr>();
    EXPECT_NE(serviceRouterMgr, nullptr);
    if (serviceRouterMgr != nullptr) {
        BusinessAbilityFilter filter;
        filter.businessType = BusinessType::UNSPECIFIED;
        filter.uri = "";
        auto ret = serviceRouterMgr->GetBusinessType(filter);
        EXPECT_EQ(ret, BusinessType::UNSPECIFIED);
    }
}

/**
 * @tc.number: ServiceRouterMgrInterfaceTest
 * @tc.name: test ClearAllBundleInfos
 * @tc.desc: test ClearAllBundleInfos function
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, ServiceRouterMgrInterfaceTest_0026, Function | SmallTest | Level0)
{
    auto serviceRouterMgr = std::make_shared<ServiceRouterDataMgr>();
    EXPECT_NE(serviceRouterMgr, nullptr);
    if (serviceRouterMgr != nullptr) {
        serviceRouterMgr->ClearAllBundleInfos();
    }
}

/**
 * @tc.number: serviceRouterMgrProxy
 * @tc.name: test QueryBusinessAbilityInfos
 * @tc.require: I9KS48
 * @tc.desc: QueryBusinessAbilityInfos
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, serviceRouterMgrProxy_0001, Function | SmallTest | Level0)
{
    auto serviceRouterMgrProxy = std::make_shared<ServiceRouterMgrProxy>(nullptr);
    EXPECT_NE(serviceRouterMgrProxy, nullptr);
    BusinessAbilityFilter filter;
    int32_t funcResult = -1;
    filter.businessType = BusinessType::UNSPECIFIED;
    std::vector<BusinessAbilityInfo> abilityInfos;
    auto ret = serviceRouterMgrProxy->QueryBusinessAbilityInfos(filter, abilityInfos, funcResult);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
}

/**
 * @tc.number: serviceRouterMgrProxy
 * @tc.name: test QueryPurposeInfos
 * @tc.require: I9KS48
 * @tc.desc: QueryPurposeInfos
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, serviceRouterMgrProxy_0002, Function | SmallTest | Level0)
{
    auto serviceRouterMgrProxy = std::make_shared<ServiceRouterMgrProxy>(nullptr);
    EXPECT_NE(serviceRouterMgrProxy, nullptr);
    Want want;
    int32_t funcResult = -1;
    std::vector<PurposeInfo> purposeInfos;
    auto ret = serviceRouterMgrProxy->QueryPurposeInfos(want, "", purposeInfos, funcResult);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
}

/**
 * @tc.number: serviceRouterMgrProxy
 * @tc.name: test StartUIExtensionAbility
 * @tc.require: I9KS48
 * @tc.desc: StartUIExtensionAbility
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, serviceRouterMgrProxy_0003, Function | SmallTest | Level0)
{
    auto serviceRouterMgrProxy = std::make_shared<ServiceRouterMgrProxy>(nullptr);
    EXPECT_NE(serviceRouterMgrProxy, nullptr);
    SessionInfo sessionInfo;
    int32_t userId = 1;
    int32_t funcResult = -1;
    auto ret = serviceRouterMgrProxy->StartUIExtensionAbility(sessionInfo, userId, funcResult);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
}

/**
 * @tc.number: serviceRouterMgrProxy
 * @tc.name: test ConnectUIExtensionAbility
 * @tc.require: I9KS48
 * @tc.desc: ConnectUIExtensionAbility
 */
HWTEST_F(ServiceRouterMgrInterfaceTest, serviceRouterMgrProxy_0004, Function | SmallTest | Level0)
{
    auto serviceRouterMgrProxy = std::make_shared<ServiceRouterMgrProxy>(nullptr);
    EXPECT_NE(serviceRouterMgrProxy, nullptr);
    Want want;
    sptr<IAbilityConnection> connect = nullptr;
    SessionInfo sessionInfo;
    int32_t userId = 1;
    int32_t funcResult = -1;
    auto ret = serviceRouterMgrProxy->ConnectUIExtensionAbility(want, connect, sessionInfo, userId, funcResult);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
}
} // OHOS