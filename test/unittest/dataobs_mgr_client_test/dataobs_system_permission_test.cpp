/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>
#include <functional>

#include "common_utils.h"
#include "dataobs_mgr_proxy.h"
#include "dataobs_mgr_client.h"
#include "dataobs_mgr_errors.h"
#include "datashare_errno.h"
#include "data_ability_observer_stub.h"
#include "hap_token_info.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "mock_token.h"
#include "token_setproc.h"

namespace OHOS {
namespace AAFwk {
using namespace testing::ext;
using namespace testing;
using namespace OHOS::Security::AccessToken;

class DataObsSystemPermissionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DataObsSystemPermissionTest::SetUpTestCase(void)
{}
void DataObsSystemPermissionTest::TearDownTestCase(void)
{}
void DataObsSystemPermissionTest::SetUp()
{}
void DataObsSystemPermissionTest::TearDown()
{}

class MyDataAbilityObserver : public OHOS::AAFwk::DataAbilityObserverStub {
public:
    MyDataAbilityObserver() = default;
    virtual ~MyDataAbilityObserver() = default;

    void OnChange() override
    {
        TAG_LOGI(AAFwkTag::DBOBSMGR, "MyDataAbilityObserver onChange");
    }

    void OnChangeExt(const OHOS::AAFwk::ChangeInfo &changeInfo) override
    {
        TAG_LOGI(AAFwkTag::DBOBSMGR, "MyDataAbilityObserver changeType %{public}d", changeInfo.changeType_);
        int size = changeInfo.uris_.size();
        TAG_LOGI(AAFwkTag::DBOBSMGR, "MyDataAbilityObserver Uri list size %{public}d", size);
        for (auto &uri : changeInfo.uris_) {
            TAG_LOGI(AAFwkTag::DBOBSMGR, "MyDataAbilityObserver Uri %{public}s", uri.ToString().c_str());
        }
    }
};

// system app policy
HapPolicyParams GetPolicy()
{
    HapPolicyParams policy = {
        .apl = APL_SYSTEM_CORE,
        .domain = "test.domain",
        .permList = {
            {
                .permissionName = "ohos.permission.test",
                .bundleName = "ohos.datashareclienttest.demo",
                .grantMode = 1,
                .availableLevel = APL_SYSTEM_CORE,
                .label = "label",
                .labelId = 1,
                .description = "ohos.datashareclienttest.demo",
                .descriptionId = 1
            }
        },
        .permStateList = {
            {
                .permissionName = "ohos.permission.GET_BUNDLE_INFO",
                .isGeneral = true,
                .resDeviceID = { "local" },
                .grantStatus = { PermissionState::PERMISSION_GRANTED },
                .grantFlags = { 1 }
            }
        }
    };
    return policy;
}

// nomral app policy
HapPolicyParams GetNormalPolicy()
{
    HapPolicyParams policy = {
        .apl = APL_NORMAL,
        .domain = "test.domain",
        .permList = {
            {
                .permissionName = "ohos.permission.test",
                .bundleName = "ohos.datashareclienttest.demo",
                .grantMode = 1,
                .availableLevel = APL_NORMAL,
                .label = "label",
                .labelId = 1,
                .description = "ohos.datashareclienttest.demo",
                .descriptionId = 1
            }
        },
        .permStateList = {
            {
                .permissionName = "ohos.permission.GET_BUNDLE_INFO",
                .isGeneral = true,
                .resDeviceID = { "local" },
                .grantStatus = { PermissionState::PERMISSION_GRANTED },
                .grantFlags = { 1 }
            }
        }
    };
    return policy;
}

/**
 * @tc.name: RegisterObserverSystemPermissionTest_0100
 * @tc.desc: Test RegisterObserver with system permission where uri is in allowlist
 * @tc.type: FUNC
 * @tc.require:
 * @tc.precon:
    1. process is equivalent to system ability
    2. uri is in allowlist
 * @tc.step:
    1. Define a test Uri and an observer
    2. Get a DataObsMgrClient instance
    3. Call RegisterObserver using DataObsMgrClient
 * @tc.expect:
    1. RegisterObserver return E_OK
 */
HWTEST_F(DataObsSystemPermissionTest, RegisterObserverSystemPermissionTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "RegisterObserverSystemPermissionTest_0100 start");

    auto obsMgrClient = OHOS::AAFwk::DataObsMgrClient::GetInstance();
    EXPECT_NE(obsMgrClient, nullptr);

    sptr dataObserver = sptr(new (std::nothrow) MyDataAbilityObserver());
    EXPECT_NE(dataObserver, nullptr);

    Uri uri = Uri("datashare:///com.ohos.contactsdataability");
    // obsOption(false, true) means not from datashare arkts kit, and calling path is from datashare
    ErrCode ret = obsMgrClient->RegisterObserver(Uri(uri), dataObserver,
        DataObsManagerProxy::DATAOBS_DEFAULT_CURRENT_USER, AAFwk::DataObsOption(false, true));
    EXPECT_EQ(ret, DataShare::E_OK);

    TAG_LOGI(AAFwkTag::DBOBSMGR, "RegisterObserverSystemPermissionTest_0100 end");
}

/**
 * @tc.name: RegisterObserverSystemPermissionTest_0200
 * @tc.desc: Test RegisterObserver with system permission and uri not in allowlist
 * @tc.type: FUNC
 * @tc.require:
 * @tc.precon:
    1. process is equivalent to a system ability
    2. uri is not in allowlist
 * @tc.step:
    1. Define a test Uri and an observer
    2. Get a DataObsMgrClient instance
    3. Call RegisterObserver using DataObsMgrClient
 * @tc.expect:
    1. RegisterObserver return E_NOT_SYSTEM_APP
 */
HWTEST_F(DataObsSystemPermissionTest, RegisterObserverSystemPermissionTest_0200, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "RegisterObserverSystemPermissionTest_0200 start");

    auto obsMgrClient = OHOS::AAFwk::DataObsMgrClient::GetInstance();
    EXPECT_NE(obsMgrClient, nullptr);

    sptr dataObserver = sptr(new (std::nothrow) MyDataAbilityObserver());
    EXPECT_NE(dataObserver, nullptr);

    Uri uri = Uri("datashare:///com.ohos.globalparamsability");
    // obsOption(false, true) means not from datashare arkts kit, and calling path is from datashare
    ErrCode ret = obsMgrClient->RegisterObserver(Uri(uri), dataObserver,
        DataObsManagerProxy::DATAOBS_DEFAULT_CURRENT_USER, AAFwk::DataObsOption(false, true));
    EXPECT_EQ(ret, DATAOBS_PERMISSION_DENY);

    TAG_LOGI(AAFwkTag::DBOBSMGR, "RegisterObserverSystemPermissionTest_0200 end");
}

/**
 * @tc.name: RegisterObserverExtSystemPermissionTest_0100
 * @tc.desc: Test RegisterObserverExt with system permission with uri in allowlist
 * @tc.type: FUNC
 * @tc.require:
 * @tc.precon:
    1. process is equivalent to a system app
    2. uri in allowlist
 * @tc.step:
    1. Define a test Uri and an observer
    2. Get a DataObsMgrClient instance
    3. Call RegisterObserverExt using DataObsMgrClient
 * @tc.expect:
    1. RegisterObserverExt return E_OK
 */
HWTEST_F(DataObsSystemPermissionTest, RegisterObserverExtSystemPermissionTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "RegisterObserverExtSystemPermissionTest_0100 start");
    // mock system app
    MockToken::SetTestEnvironment();

    HapInfoParams info = {
        .userID = 100,
        .bundleName = "ohos.datashareclienttest.demo",
        .instIndex = 0,
        .isSystemApp = true,
        .appIDDesc = "ohos.datashareclienttest.demo"
    };
    auto policy = GetPolicy();
    AccessTokenIDEx tokenIdEx = MockToken::AllocTestHapToken(info, policy);
    uint64_t token = tokenIdEx.tokenIdExStruct.tokenID;
    EXPECT_NE(token, INVALID_TOKENID);
    auto originalToken = GetSelfTokenID();
    int setRet = SetSelfTokenID(token);
    EXPECT_EQ(setRet, DataShare::E_OK);

    auto obsMgrClient = OHOS::AAFwk::DataObsMgrClient::GetInstance();
    EXPECT_NE(obsMgrClient, nullptr);

    sptr dataObserver = sptr(new (std::nothrow) MyDataAbilityObserver());
    EXPECT_NE(dataObserver, nullptr);

    // obsOption(false, true) means not from datashare arkts kit, and calling path is from datashare
    bool isDescendants = true;
    Uri uri = Uri("datashare:///com.ohos.contactsdataability");
    ErrCode ret = obsMgrClient->RegisterObserverExt(uri, dataObserver, isDescendants,
        AAFwk::DataObsOption(false, true));
    EXPECT_EQ(ret, DataShare::E_OK);
    SetSelfTokenID(originalToken);
    MockToken::ResetTestEnvironment();

    TAG_LOGI(AAFwkTag::DBOBSMGR, "RegisterObserverExtSystemPermissionTest_0100 end");
}

/**
 * @tc.name: RegisterObserverExtSystemPermissionTest_0200
 * @tc.desc: Test RegisterObserverExt with normal permission with uri in allowlist
 * @tc.type: FUNC
 * @tc.require:
 * @tc.precon:
    1. process is equivalent to a normal app
    2. uri is in allowlist
 * @tc.step:
    1. Define a test Uri and an observer
    2. Get a DataObsMgrClient instance
    3. Call RegisterObserverExt using DataObsMgrClient
 * @tc.expect:
    1. RegisterObserverExt return E_OK
 */
HWTEST_F(DataObsSystemPermissionTest, RegisterObserverExtSystemPermissionTest_0200, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "RegisterObserverExtSystemPermissionTest_0200 start");
    // mock normal app
    MockToken::SetTestEnvironment();

    HapInfoParams info = {
        .userID = 100,
        .bundleName = "ohos.datashareclienttest.demo",
        .instIndex = 0,
        .isSystemApp = false,
        .appIDDesc = "ohos.datashareclienttest.demo"
    };
    auto policy = GetNormalPolicy();
    AccessTokenIDEx tokenIdEx = MockToken::AllocTestHapToken(info, policy);
    uint64_t token = tokenIdEx.tokenIdExStruct.tokenID;
    EXPECT_NE(token, INVALID_TOKENID);
    auto originalToken = GetSelfTokenID();
    int setRet = SetSelfTokenID(token);
    EXPECT_EQ(setRet, DataShare::E_OK);

    auto obsMgrClient = OHOS::AAFwk::DataObsMgrClient::GetInstance();
    EXPECT_NE(obsMgrClient, nullptr);

    sptr dataObserver = sptr(new (std::nothrow) MyDataAbilityObserver());
    EXPECT_NE(dataObserver, nullptr);

    // obsOption(false, true) means not from datashare arkts kit, and calling path is from datashare
    bool isDescendants = true;
    Uri uri = Uri("datashare:///com.ohos.contactsdataability");
    ErrCode ret = obsMgrClient->RegisterObserverExt(uri, dataObserver, isDescendants,
        AAFwk::DataObsOption(false, true));
    EXPECT_EQ(ret, DataShare::E_OK);
    SetSelfTokenID(originalToken);
    MockToken::ResetTestEnvironment();

    TAG_LOGI(AAFwkTag::DBOBSMGR, "RegisterObserverExtSystemPermissionTest_0200 end");
}

/**
 * @tc.name: RegisterObserverExtSystemPermissionTest_0300
 * @tc.desc: Test RegisterObserverExt with normal permission while uri not in allowlist
 * @tc.type: FUNC
 * @tc.require:
 * @tc.precon:
    1. process is equivalent to a normal app
    2. uri not in allowlist
 * @tc.step:
    1. Define a test Uri and an observer
    2. Get a DataObsMgrClient instance
    3. Call RegisterObserverExt using DataObsMgrClient
 * @tc.expect:
    1. RegisterObserverExt return E_OK
 */
HWTEST_F(DataObsSystemPermissionTest, RegisterObserverExtSystemPermissionTest_0300, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "RegisterObserverExtSystemPermissionTest_0300 start");
    // mock normal app
    MockToken::SetTestEnvironment();

    HapInfoParams info = {
        .userID = 100,
        .bundleName = "ohos.datashareclienttest.demo",
        .instIndex = 0,
        .isSystemApp = false,
        .appIDDesc = "ohos.datashareclienttest.demo"
    };
    auto policy = GetNormalPolicy();
    AccessTokenIDEx tokenIdEx = MockToken::AllocTestHapToken(info, policy);
    uint64_t token = tokenIdEx.tokenIdExStruct.tokenID;
    EXPECT_NE(token, INVALID_TOKENID);
    auto originalToken = GetSelfTokenID();
    int setRet = SetSelfTokenID(token);
    EXPECT_EQ(setRet, DataShare::E_OK);

    auto obsMgrClient = OHOS::AAFwk::DataObsMgrClient::GetInstance();
    EXPECT_NE(obsMgrClient, nullptr);

    sptr dataObserver = sptr(new (std::nothrow) MyDataAbilityObserver());
    EXPECT_NE(dataObserver, nullptr);

    // obsOption(false, true) means not from datashare arkts kit, and calling path is from datashare
    bool isDescendants = true;
    // use telephony uri as test sample
    Uri uri = Uri("datashare:///com.ohos.globalparamsability");
    ErrCode ret = obsMgrClient->RegisterObserverExt(uri, dataObserver, isDescendants,
        AAFwk::DataObsOption(false, true));
    EXPECT_EQ(ret, DATAOBS_NOT_SYSTEM_APP);
    SetSelfTokenID(originalToken);
    MockToken::ResetTestEnvironment();

    TAG_LOGI(AAFwkTag::DBOBSMGR, "RegisterObserverExtSystemPermissionTest_0300 end");
}

/**
 * @tc.name: NotifyChangeSystemPermissionTest_0100
 * @tc.desc: Test NotifyChange with system permission
 * @tc.type: FUNC
 * @tc.require:
 * @tc.precon:
    1. process is equivalent to a system app
 * @tc.step:
    1. Define a test Uri and an observer
    2. Get a DataObsMgrClient instance
    3. Call NotifyChange using DataObsMgrClient
 * @tc.expect:
    1. NotifyChange return E_OK
 */
HWTEST_F(DataObsSystemPermissionTest, NotifyChangeSystemPermissionTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "NotifyChangeSystemPermissionTest_0100 start");
    // mock system app
    MockToken::SetTestEnvironment();

    HapInfoParams info = {
        .userID = 100,
        .bundleName = "ohos.datashareclienttest.demo",
        .instIndex = 0,
        .isSystemApp = true,
        .appIDDesc = "ohos.datashareclienttest.demo"
    };
    auto policy = GetPolicy();
    AccessTokenIDEx tokenIdEx = MockToken::AllocTestHapToken(info, policy);
    uint64_t token = tokenIdEx.tokenIdExStruct.tokenID;
    EXPECT_NE(token, INVALID_TOKENID);
    auto originalToken = GetSelfTokenID();
    int setRet = SetSelfTokenID(token);
    EXPECT_EQ(setRet, DataShare::E_OK);

    auto obsMgrClient = OHOS::AAFwk::DataObsMgrClient::GetInstance();
    EXPECT_NE(obsMgrClient, nullptr);

    sptr dataObserver = sptr(new (std::nothrow) MyDataAbilityObserver());
    EXPECT_NE(dataObserver, nullptr);

    // obsOption(false, true) means not from datashare arkts kit, and calling path is from datashare
    Uri uri = Uri("datashare:///com.ohos.contactsdataability");
    ErrCode ret = obsMgrClient->NotifyChange(uri, DataObsManagerProxy::DATAOBS_DEFAULT_CURRENT_USER,
        AAFwk::DataObsOption(false, true));
    EXPECT_EQ(ret, DataShare::E_OK);
    SetSelfTokenID(originalToken);
    MockToken::ResetTestEnvironment();

    TAG_LOGI(AAFwkTag::DBOBSMGR, "NotifyChangeSystemPermissionTest_0100 end");
}

/**
 * @tc.name: NotifyChangeSystemPermissionTest_0200
 * @tc.desc: Test NotifyChange with normal permission
 * @tc.type: FUNC
 * @tc.require:
 * @tc.precon:
    1. process is equivalent to a normal app
 * @tc.step:
    1. Define a test Uri and an observer
    2. Get a DataObsMgrClient instance
    3. Call NotifyChange using DataObsMgrClient
 * @tc.expect:
    1. NotifyChange return E_OK
 */
HWTEST_F(DataObsSystemPermissionTest, NotifyChangeSystemPermissionTest_0200, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "NotifyChangeSystemPermissionTest_0200 start");
    // mock normal app
    MockToken::SetTestEnvironment();

    HapInfoParams info = {
        .userID = 100,
        .bundleName = "ohos.datashareclienttest.demo",
        .instIndex = 0,
        .isSystemApp = false,
        .appIDDesc = "ohos.datashareclienttest.demo"
    };
    auto policy = GetNormalPolicy();
    AccessTokenIDEx tokenIdEx = MockToken::AllocTestHapToken(info, policy);
    uint64_t token = tokenIdEx.tokenIdExStruct.tokenID;
    EXPECT_NE(token, INVALID_TOKENID);
    auto originalToken = GetSelfTokenID();
    int setRet = SetSelfTokenID(token);
    EXPECT_EQ(setRet, DataShare::E_OK);

    auto obsMgrClient = OHOS::AAFwk::DataObsMgrClient::GetInstance();
    EXPECT_NE(obsMgrClient, nullptr);

    sptr dataObserver = sptr(new (std::nothrow) MyDataAbilityObserver());
    EXPECT_NE(dataObserver, nullptr);

    // obsOption(false, true) means not from datashare arkts kit, and calling path is from datashare
    Uri uri = Uri("datashare:///com.ohos.contactsdataability");
    ErrCode ret = obsMgrClient->NotifyChange(uri, DataObsManagerProxy::DATAOBS_DEFAULT_CURRENT_USER,
        AAFwk::DataObsOption(false, true));
    EXPECT_EQ(ret, DataShare::E_OK);
    SetSelfTokenID(originalToken);
    MockToken::ResetTestEnvironment();

    TAG_LOGI(AAFwkTag::DBOBSMGR, "NotifyChangeSystemPermissionTest_0200 end");
}

/**
 * @tc.name: NotifyChangeExtSystemPermissionTest_0100
 * @tc.desc: Test NotifyChangeExt with system permission
 * @tc.type: FUNC
 * @tc.require:
 * @tc.precon:
    1. process is equivalent to a system app
 * @tc.step:
    1. Define a test Uri and an observer
    2. Get a DataObsMgrClient instance
    3. Call NotifyChangeExt using DataObsMgrClient
 * @tc.expect:
    1. NotifyChangeExt return E_OK
 */
HWTEST_F(DataObsSystemPermissionTest, NotifyChangeExtSystemPermissionTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "NotifyChangeExtSystemPermissionTest_0100 start");
    // mock normal app
    MockToken::SetTestEnvironment();

    HapInfoParams info = {
        .userID = 100,
        .bundleName = "ohos.datashareclienttest.demo",
        .instIndex = 0,
        .isSystemApp = true,
        .appIDDesc = "ohos.datashareclienttest.demo"
    };
    auto policy = GetPolicy();
    AccessTokenIDEx tokenIdEx = MockToken::AllocTestHapToken(info, policy);
    uint64_t token = tokenIdEx.tokenIdExStruct.tokenID;
    EXPECT_NE(token, INVALID_TOKENID);
    auto originalToken = GetSelfTokenID();
    int setRet = SetSelfTokenID(token);
    EXPECT_EQ(setRet, DataShare::E_OK);

    auto obsMgrClient = OHOS::AAFwk::DataObsMgrClient::GetInstance();
    EXPECT_NE(obsMgrClient, nullptr);

    sptr dataObserver = sptr(new (std::nothrow) MyDataAbilityObserver());
    EXPECT_NE(dataObserver, nullptr);

    Uri uri = Uri("datashare:///com.ohos.contactsdataability");
    ErrCode ret = obsMgrClient->NotifyChangeExt({ ChangeInfo::ChangeType::INSERT, { uri } },
        AAFwk::DataObsOption(false, true));
    EXPECT_EQ(ret, DataShare::E_OK);
    SetSelfTokenID(originalToken);
    MockToken::ResetTestEnvironment();

    TAG_LOGI(AAFwkTag::DBOBSMGR, "NotifyChangeExtSystemPermissionTest_0100 end");
}

/**
 * @tc.name: NotifyChangeExtSystemPermissionTest_0200
 * @tc.desc: Test NotifyChangeExt with normal permission
 * @tc.type: FUNC
 * @tc.require:
 * @tc.precon:
    1. process is equivalent to a normal app
 * @tc.step:
    1. Define a test Uri and an observer
    2. Get a DataObsMgrClient instance
    3. Call NotifyChangeExt using DataObsMgrClient
 * @tc.expect:
    1. NotifyChangeExt return E_OK
 */
HWTEST_F(DataObsSystemPermissionTest, NotifyChangeExtSystemPermissionTest_0200, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "NotifyChangeExtSystemPermissionTest_0200 start");
    // mock system app
    MockToken::SetTestEnvironment();

    HapInfoParams info = {
        .userID = 100,
        .bundleName = "ohos.datashareclienttest.demo",
        .instIndex = 0,
        .isSystemApp = true,
        .appIDDesc = "ohos.datashareclienttest.demo"
    };
    auto policy = GetNormalPolicy();
    AccessTokenIDEx tokenIdEx = MockToken::AllocTestHapToken(info, policy);
    uint64_t token = tokenIdEx.tokenIdExStruct.tokenID;
    EXPECT_NE(token, INVALID_TOKENID);
    auto originalToken = GetSelfTokenID();
    int setRet = SetSelfTokenID(token);
    EXPECT_EQ(setRet, DataShare::E_OK);

    auto obsMgrClient = OHOS::AAFwk::DataObsMgrClient::GetInstance();
    EXPECT_NE(obsMgrClient, nullptr);

    sptr dataObserver = sptr(new (std::nothrow) MyDataAbilityObserver());
    EXPECT_NE(dataObserver, nullptr);

    Uri uri = Uri("datashare:///com.ohos.contactsdataability");
    ErrCode ret = obsMgrClient->NotifyChangeExt({ ChangeInfo::ChangeType::INSERT, { uri } },
        AAFwk::DataObsOption(false, true));
    EXPECT_EQ(ret, DataShare::E_OK);
    SetSelfTokenID(originalToken);
    MockToken::ResetTestEnvironment();

    TAG_LOGI(AAFwkTag::DBOBSMGR, "NotifyChangeExtSystemPermissionTest_0200 end");
}
}
}