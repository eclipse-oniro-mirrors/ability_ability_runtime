/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#define private public
#include "local_call_container.h"
#define protected public
#include "ability_loader.h"
#include "ability_thread.h"
#include "mock_serviceability_manager_service.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

class LocalCallContainerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void LocalCallContainerTest::SetUpTestCase(void)
{
    OHOS::sptr<OHOS::IRemoteObject> abilityObject = new (std::nothrow) MockServiceAbilityManagerService();

    auto sysMgr = OHOS::DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == NULL) {
        GTEST_LOG_(ERROR) << "fail to get ISystemAbilityManager";
        return;
    }

    sysMgr->RegisterSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID, abilityObject);
}

void LocalCallContainerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "LocalCallContainerTest::TearDownTestCase end";
}

void LocalCallContainerTest::SetUp(void)
{}

void LocalCallContainerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "LocalCallContainerTest::TearDown end";
}

/**
 * @tc.number: Local_Call_Container_StartAbilityInner_0100
 * @tc.name: StartAbility
 * @tc.desc: Local Call Container to process StartAbilityInner, and the result is success.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_StartAbilityInner_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Local_Call_Container_StartAbilityInner_0100::" <<
        "Local_Call_Container_StartAbilityInner_0100 begain";

    auto localCallContainer = std::make_shared<LocalCallContainer>();

    Want want;
    want.SetElementName("DemoDeviceIdB", "DemoBundleNameB", "DemoAbilityNameB");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    GTEST_LOG_(INFO) << "Local_Call_Container_StartAbilityInner_0100::" <<
        "Local_Call_Container_StartAbilityInner_0100 begain1";
    ErrCode ret = localCallContainer->StartAbilityByCallInner(want, callback, nullptr);
    GTEST_LOG_(INFO) << "Local_Call_Container_StartAbilityInner_0100::" <<
        "Local_Call_Container_StartAbilityInner_0100 begain2";
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Local_Call_Container_StartAbilityInner_0100::" <<
        "Local_Call_Container_StartAbilityInner_0100 end";
}

/**
 * @tc.number: Local_Call_Container_StartAbilityInner_0200
 * @tc.name: StartAbility
 * @tc.desc: Local Call Container to process StartAbilityInner, and the result is fail because callback is null.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_StartAbilityInner_0200, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");

    sptr<IRemoteObject> callRemoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);

    ErrCode ret = localCallContainer.StartAbilityByCallInner(want, nullptr, callRemoteObject);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_StartAbilityInner_0300
 * @tc.name: StartAbility
 * @tc.desc: Local Call Container to process StartAbilityInner, and the result is fail because bundle name is null.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_StartAbilityInner_0300, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    Want want;
    want.SetElementName("DemoDeviceId", "", "DemoAbilityName");

    sptr<IRemoteObject> callRemoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);

    ErrCode ret = localCallContainer.StartAbilityByCallInner(want, nullptr, callRemoteObject);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_StartAbilityInner_0400
 * @tc.name: StartAbility
 * @tc.desc: Local Call Container to process StartAbilityInner, and the result is fail because ability name is null.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_StartAbilityInner_0400, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "");

    sptr<IRemoteObject> callRemoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);

    ErrCode ret = localCallContainer.StartAbilityByCallInner(want, nullptr, callRemoteObject);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_StartAbilityInner_0500
 * @tc.name: StartAbility
 * @tc.desc: Local Call Container to process StartAbilityInner, and the result is fail because ability name is null.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_StartAbilityInner_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Local_Call_Container_StartAbilityInner_0500 begain";
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    Want want;
    want.SetElementName("", "DemoBundleNameB", "DemoAbilityNameB");
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    auto localCallRecord = std::make_shared<LocalCallRecord>(want.GetElement());
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    localCallRecord->remoteObject_ = connect;
    localCallContainer->SetCallLocalRecord(localCallRecord);
    ErrCode ret = localCallContainer->StartAbilityByCallInner(want, callback, nullptr);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Local_Call_Container_StartAbilityInner_0500 end";
}

/**
 * @tc.number: Local_Call_Container_Release_0100
 * @tc.name: Release
 * @tc.desc: Local Call Container to process Release, and the result is success.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_0100, Function | MediumTest | Level1)
{
    auto localCallContainer = std::make_shared<LocalCallContainer>();

    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    std::shared_ptr<CallerCallBack> callbackSec = std::make_shared<CallerCallBack>();

    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    localCallRecord->AddCaller(callbackSec);

    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    callbackSec->SetCallBack([](const sptr<IRemoteObject>&) {});
    localCallContainer->SetCallLocalRecord(localCallRecord);

    ErrCode ret = localCallContainer->ReleaseCall(callback);
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.number: Local_Call_Container_Release_0200
 * @tc.name: Release
 * @tc.desc: Local Call Container to process Release, and the result is fail because callProxyRecords_ is null.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_0200, Function | MediumTest | Level1)
{
    auto localCallContainer = std::make_shared<LocalCallContainer>();

    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();

    callback->SetCallBack([](const sptr<IRemoteObject>&) {});

    ErrCode ret = localCallContainer->ReleaseCall(callback);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_Release_0300
 * @tc.name: Release
 * @tc.desc: Parameter 'callback' is nullptr, and Error 'ERR_INVALID_VALUE' returned.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_0300, Function | MediumTest | Level1)
{
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    ErrCode ret = localCallContainer->ReleaseCall(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_Release_0400
 * @tc.name: Release
 * @tc.desc: Parameter 'localCallRecord' is nullptr, and Error 'ERR_INVALID_VALUE' returned.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_0400, Function | MediumTest | Level1)
{
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    ErrCode ret = localCallContainer->ReleaseCall(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_Release_0700
 * @tc.name: Release
 * @tc.desc: Parameter 'connect' is nullptr, and Error 'ERR_INVALID_VALUE' returned.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_0700, Function | MediumTest | Level1)
{
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> LocalCallRecordFirst = std::make_shared<LocalCallRecord>(elementName);
    std::weak_ptr<LocalCallRecord> LocalCallRecordSec = LocalCallRecordFirst;
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetRecord(LocalCallRecordSec);
    ErrCode ret = localCallContainer->ReleaseCall(callback);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_Release_0800
 * @tc.name: Release
 * @tc.desc: When 'localCallRecord' is an 'IsSingletonRemote',
 * remove 'localCallRecord' by 'RemoveSingletonCallLocalRecord', and return 'ERR_OK'.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_0800, Function | MediumTest | Level1)
{
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> LocalCallRecordFirst = std::make_shared<LocalCallRecord>(elementName);
    std::weak_ptr<LocalCallRecord> LocalCallRecordSec = LocalCallRecordFirst;
    LocalCallRecordFirst->SetIsSingleton(true);
    LocalCallRecordFirst->SetConnection(connect);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetRecord(LocalCallRecordSec);
    localCallContainer->SetCallLocalRecord(LocalCallRecordFirst);
    ErrCode ret = localCallContainer->ReleaseCall(callback);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Local_Call_Container_Release_0900
 * @tc.name: Release
 * @tc.desc: Release record in multiple not found.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_0900, Function | MediumTest | Level1)
{
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> LocalCallRecordFirst = std::make_shared<LocalCallRecord>(elementName);
    std::weak_ptr<LocalCallRecord> LocalCallRecordSec = LocalCallRecordFirst;
    LocalCallRecordFirst->SetConnection(connect);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetRecord(LocalCallRecordSec);
    ErrCode ret = localCallContainer->ReleaseCall(callback);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_Release_1000
 * @tc.name: Release
 * @tc.desc: When 'element' doesn't have record, delete itself from 'multipleCallProxyRecords_'.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_1000, Function | MediumTest | Level1)
{
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> LocalCallRecordFirst = std::make_shared<LocalCallRecord>(elementName);
    std::weak_ptr<LocalCallRecord> LocalCallRecordSec = LocalCallRecordFirst;
    LocalCallRecordFirst->SetConnection(connect);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetRecord(LocalCallRecordSec);
    localCallContainer->SetMultipleCallLocalRecord(LocalCallRecordFirst);
    ErrCode ret = localCallContainer->ReleaseCall(callback);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Local_Call_Container_SetCallLocalRecord_0100
 * @tc.name: SetCallLocalRecord
 * @tc.desc: Set 'callProxyRecords_'.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_SetCallLocalRecord_0100, Function | MediumTest | Level1)
{
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallContainer->SetCallLocalRecord(localCallRecord);
    std::shared_ptr<LocalCallRecord> resultPtr = nullptr;
    EXPECT_TRUE(localCallContainer->GetCallLocalRecord(elementName, resultPtr, localCallRecord->GetUserId()));
    EXPECT_EQ(resultPtr, localCallRecord);
}

/**
 * @tc.number: Local_Call_Container_SetCallLocalRecord_0200
 * @tc.name: SetCallLocalRecord
 * @tc.desc: Set 'callProxyRecords_'.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_SetCallLocalRecord_0200, Function | MediumTest | Level1)
{
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    localCallContainer->SetCallLocalRecord(nullptr);
    EXPECT_TRUE(localCallContainer->callProxyRecords_.empty());
}

/**
 * @tc.number: Local_Call_Container_SetMultipleCallLocalRecord_0100
 * @tc.name: SetMultipleCallLocalRecord
 * @tc.desc: Set 'multipleCallProxyRecords_'.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_SetMultipleCallLocalRecord_0100, Function | MediumTest | Level1)
{
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecordFirst = std::make_shared<LocalCallRecord>(elementName);
    constexpr uint32_t COUNT_NUM = 1;
    localCallContainer->SetMultipleCallLocalRecord(localCallRecordFirst);
    EXPECT_EQ(
        COUNT_NUM, localCallContainer->multipleCallProxyRecords_[elementName.GetURI()].count(localCallRecordFirst));

    std::shared_ptr<LocalCallRecord> localCallRecordSecond = std::make_shared<LocalCallRecord>(elementName);
    localCallContainer->SetMultipleCallLocalRecord(localCallRecordSecond);

    EXPECT_EQ(
        COUNT_NUM, localCallContainer->multipleCallProxyRecords_[elementName.GetURI()].count(localCallRecordSecond));
}

/**
 * @tc.number: Local_Call_Container_SetMultipleCallLocalRecord_0200
 * @tc.name: SetMultipleCallLocalRecord
 * @tc.desc: Set 'multipleCallProxyRecords_'.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_SetMultipleCallLocalRecord_0200, Function | MediumTest | Level1)
{
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    localCallContainer->SetMultipleCallLocalRecord(nullptr);
    EXPECT_TRUE(localCallContainer->multipleCallProxyRecords_.empty());
}

/**
 * @tc.number: Local_Call_Container_SetRecordAndContainer_0100
 * @tc.name: SetRecordAndContainer
 * @tc.desc: Set 'container_' and 'localCallRecord_'.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_SetRecordAndContainer_0100, Function | MediumTest | Level1)
{
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    std::shared_ptr<LocalCallContainer> localCallContainer = std::make_shared<LocalCallContainer>();
    connect->SetRecordAndContainer(localCallRecord, localCallContainer);
    EXPECT_EQ(connect->localCallRecord_, localCallRecord);
    EXPECT_EQ(connect->container_.lock(), localCallContainer);
}

/**
 * @tc.number: Local_Call_Container_OnAbilityConnectDone_0400
 * @tc.name: OnAbilityConnectDone
 * @tc.desc: OnAbilityConnectDone container is nullptr.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityConnectDone_0400, Function | MediumTest | Level1)
{
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    sptr<IRemoteObject> callRemoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    connect->OnAbilityConnectDone(elementName, callRemoteObject,
        static_cast<int32_t>(AppExecFwk::LaunchMode::SINGLETON));
    EXPECT_EQ(nullptr, connect->container_.lock());
}

/**
 * @tc.number: Local_Call_Container_OnAbilityConnectDone_0500
 * @tc.name: OnAbilityConnectDone
 * @tc.desc: 'Code' is a Singleton.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityConnectDone_0500, Function | MediumTest | Level1)
{
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    connect->container_ = localCallContainer;
    constexpr int32_t code = 0;
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    connect->localCallRecord_ = localCallRecord;
    connect->OnAbilityConnectDone(elementName, connect, code);
    std::shared_ptr<LocalCallRecord> resultPtr = nullptr;
    EXPECT_TRUE(localCallContainer->GetCallLocalRecord(elementName, resultPtr, localCallRecord->GetUserId()));
    EXPECT_EQ(resultPtr, localCallRecord);
}

/**
 * @tc.number: Local_Call_Container_OnAbilityConnectDone_0600
 * @tc.name: OnAbilityConnectDone
 * @tc.desc: 'Code' is not a Singleton.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityConnectDone_0600, Function | MediumTest | Level1)
{
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    connect->container_ = localCallContainer;
    constexpr int32_t code = 1;
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    connect->localCallRecord_ = localCallRecord;
    connect->OnAbilityConnectDone(elementName, connect, code);
    constexpr uint32_t COUNT_NUM = 1;
    EXPECT_EQ(connect->container_.lock()->multipleCallProxyRecords_[elementName.GetURI()].
        count(connect->localCallRecord_), COUNT_NUM);
    EXPECT_TRUE(callback->isCallBack_);
}

/**
 * @tc.number: Local_Call_Container_OnAbilityDisconnectDone_0100
 * @tc.name: OnAbilityDisconnectDone
 * @tc.desc: OnAbilityDisconnectDone container is nullptr.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityDisconnectDone_0100, Function | MediumTest | Level1)
{
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    constexpr int32_t code = 0;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    connect->OnAbilityDisconnectDone(elementName, code);
    EXPECT_EQ(connect->container_.lock(), nullptr);
}

/**
 * @tc.number: Local_Call_Container_OnAbilityDisconnectDone_0200
 * @tc.name: OnAbilityDisconnectDone
 * @tc.desc: OnAbilityDisconnectDone container is not nullptr.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityDisconnectDone_0200, Function | MediumTest | Level1)
{
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    constexpr int32_t code = 0;
    connect->container_ = localCallContainer;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    connect->OnAbilityDisconnectDone(elementName, code);
    EXPECT_NE(connect->container_.lock(), nullptr);
}

/**
 * @tc.number: Local_Call_Container_DumpCalls_0100
 * @tc.name: DumpCalls
 * @tc.desc: Local Call Container to process DumpCalls, and the result is success(call back is called).
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_DumpCalls_0100, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->isCallBack_ = true;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    localCallContainer.SetCallLocalRecord(localCallRecord);

    std::vector<std::string> info;
    std::string resultReceiver = "";
    EXPECT_TRUE(resultReceiver == "");
    localCallContainer.DumpCalls(info);

    for (auto it : info) {
        GTEST_LOG_(ERROR) << it;
        resultReceiver += it + "\n";
    }
    string::size_type idx;
    idx = resultReceiver.find("REQUESTEND");
    EXPECT_TRUE(idx != string::npos);
}

/**
 * @tc.number: Local_Call_Container_DumpCalls_0200
 * @tc.name: DumpCalls
 * @tc.desc: Local Call Container to process DumpCalls, and the result is success(call back is not called).
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_DumpCalls_0200, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    localCallContainer.SetCallLocalRecord(localCallRecord);

    std::vector<std::string> info;
    std::string resultReceiver = "";
    std::string::size_type idx;
    idx = resultReceiver.find("REQUESTING");
    EXPECT_FALSE(idx != string::npos);
    localCallContainer.DumpCalls(info);

    for (auto it : info) {
        resultReceiver += it + "\n";
    }
    idx = resultReceiver.find("REQUESTING");
    EXPECT_TRUE(idx != string::npos);
}

/**
 * @tc.number: Local_Call_Container_DumpCalls_0300
 * @tc.name: DumpCalls
 * @tc.desc: Local Call Container to process DumpCalls, and the result is success(no caller).
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_DumpCalls_0300, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallContainer.SetCallLocalRecord(localCallRecord);

    std::vector<std::string> info;
    std::string resultReceiver = "";
    std::string::size_type idx;
    idx = resultReceiver.find("REQUESTEND");
    EXPECT_FALSE(idx != string::npos);
    localCallContainer.DumpCalls(info);

    for (auto it : info) {
        resultReceiver += it + "\n";
    }

    idx = resultReceiver.find("REQUESTEND");
    EXPECT_TRUE(idx != string::npos);
}

/**
 * @tc.number: Local_Call_Container_DumpCalls_0400
 * @tc.name: DumpCalls
 * @tc.desc: Local Call Container to process DumpCalls, and the result is success(callProxyRecords_ is empty).
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_DumpCalls_0400, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::vector<std::string> info;
    std::string resultReceiver = "";
    EXPECT_TRUE(resultReceiver == "");
    localCallContainer.DumpCalls(info);
    EXPECT_TRUE(resultReceiver == "");
}

/**
 * @tc.number: Local_Call_Container_OnRemoteStateChanged_0100
 * @tc.name: OnRemoteStateChanged
 * @tc.desc: Local Call Container to process OnRemoteStateChanged, and the result is success resultCode == ERR_OK.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnRemoteStateChanged_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    connect->container_ = localCallContainer;
    constexpr int32_t code = 1;
    connect->localCallRecord_ = localCallRecord;
    connect->OnAbilityConnectDone(elementName, connect, code);

    connect->OnRemoteStateChanged(elementName, 0);
    EXPECT_EQ(callback->isCallBack_, true);
}

/**
 * @tc.number: Local_Call_Container_OnRemoteStateChanged_0200
 * @tc.name: OnRemoteStateChanged
 * @tc.desc: Local Call Container to process OnRemoteStateChanged success when resultCode != ERR_OK.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnRemoteStateChanged_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    connect->container_ = localCallContainer;
    constexpr int32_t code = 1;
    connect->localCallRecord_ = localCallRecord;
    connect->OnAbilityConnectDone(elementName, connect, code);

    connect->OnRemoteStateChanged(elementName, -1);
    EXPECT_EQ(callback->isCallBack_, true);
}

/**
 * @tc.number: Local_Call_Container_OnRemoteStateChanged_0300
 * @tc.name: OnRemoteStateChanged
 * @tc.desc: Local Call Container to process OnRemoteStateChanged fail because localCallRecord is null.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnRemoteStateChanged_0300, Function | MediumTest | Level1)
{
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    EXPECT_EQ(callback->isCallBack_, false);
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    connect->SetRecordAndContainer(localCallRecord, localCallContainer);

    connect->OnRemoteStateChanged(elementName, 0);
    EXPECT_EQ(callback->isCallBack_, false);
}

/**
 * @tc.number: Local_Call_Container_GetCallLocalRecord_0100
 * @tc.name: GetCallLocalRecord
 * @tc.desc: Local Call Container to process GetCallLocalRecord, and the result is success.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_GetCallLocalRecord_0100, Function | MediumTest | Level1)
{
    constexpr int32_t USER_ID_U100 = 100;
    LocalCallContainer localCallContainer;

    AppExecFwk::ElementName elementNameFirst("DemoDeviceIdFirst", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecordFirst = std::make_shared<LocalCallRecord>(elementNameFirst);
    localCallContainer.SetCallLocalRecord(localCallRecordFirst);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    AppExecFwk::ElementName elementNameSecond("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecordSecond = std::make_shared<LocalCallRecord>(elementNameSecond);
    localCallRecordSecond->SetUserId(USER_ID_U100);
    localCallContainer.SetCallLocalRecord(localCallRecordSecond);
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    localCallContainer.SetCallLocalRecord(localCallRecord);

    bool ret = localCallContainer.GetCallLocalRecord(
        elementName, localCallRecord, AbilityRuntime::DEFAULT_INVAL_VALUE);
    EXPECT_TRUE(ret == true);
}

/**
 * @tc.number: Local_Call_Container_GetCallLocalRecord_0200
 * @tc.name: GetCallLocalRecord
 * @tc.desc: Local Call Container to process GetCallLocalRecord, and the result is fail
 *           because call proxy records is empty.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_GetCallLocalRecord_0200, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = nullptr;
    bool ret = localCallContainer.GetCallLocalRecord(
        elementName, localCallRecord, AbilityRuntime::DEFAULT_INVAL_VALUE);
    EXPECT_TRUE(ret == false);
    EXPECT_EQ(localCallRecord, nullptr);
}

/**
 * @tc.number: Local_Call_Container_OnCallStubDied_0100
 * @tc.name: OnCallStubDied
 * @tc.desc: Verify that function IsSameObject.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnCallStubDied_0100, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);

    sptr<IRemoteObject> remoteObject = new (std::nothrow) MockServiceAbilityManagerService();
    wptr<IRemoteObject> remote(remoteObject);

    localCallContainer.OnCallStubDied(remote);
    EXPECT_TRUE(localCallContainer.callProxyRecords_.empty());
}

/**
 * @tc.number: Local_Call_Container_OnCallStubDied_0200
 * @tc.name: OnCallStubDied
 * @tc.desc: Delete records with empty remote in single instance.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnCallStubDied_0200, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    sptr<IRemoteObject> remoteObject = new (std::nothrow) MockServiceAbilityManagerService();
    wptr<IRemoteObject> remote(remoteObject);
    const std::string strKey = elementName.GetURI();
    localCallRecord->remoteObject_ = remoteObject;
    localCallContainer.SetCallLocalRecord(localCallRecord);
    localCallContainer.OnCallStubDied(remote);
    EXPECT_TRUE(localCallContainer.callProxyRecords_.empty());
}

/**
 * @tc.number: Local_Call_Container_OnCallStubDied_0300
 * @tc.name: OnCallStubDied
 * @tc.desc: Delete the record of the specified remote in multiple instances.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnCallStubDied_0300, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    AppExecFwk::ElementName elementNameSec("DemoDeviceIdSec", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecordSec = std::make_shared<LocalCallRecord>(elementNameSec);
    localCallContainer.SetMultipleCallLocalRecord(localCallRecord);
    localCallContainer.SetMultipleCallLocalRecord(localCallRecordSec);
    sptr<IRemoteObject> remoteObject = new (std::nothrow) MockServiceAbilityManagerService();
    wptr<IRemoteObject> remote(remoteObject);
    localCallRecordSec->remoteObject_ = remoteObject;
    localCallContainer.OnCallStubDied(remote);
    EXPECT_FALSE(localCallContainer.multipleCallProxyRecords_.empty());
}

/**
 * @tc.number: Local_Call_Container_OnCallStubDied_0400
 * @tc.name: OnCallStubDied
 * @tc.desc: Delete the specified remote in multiple instances
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnCallStubDied_0400, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    std::shared_ptr<LocalCallRecord> localCallRecordSec = std::make_shared<LocalCallRecord>(elementName);
    localCallContainer.SetMultipleCallLocalRecord(localCallRecord);
    localCallContainer.SetMultipleCallLocalRecord(localCallRecordSec);
    sptr<IRemoteObject> remoteObject = new (std::nothrow) MockServiceAbilityManagerService();
    wptr<IRemoteObject> remote(remoteObject);
    localCallRecordSec->remoteObject_ = remoteObject;
    localCallContainer.OnCallStubDied(remote);
    EXPECT_FALSE(localCallContainer.multipleCallProxyRecords_.empty());
}

/**
 * @tc.number: Local_Call_Container_ClearFailedCallConnection_0400
 * @tc.name: ClearFailedCallConnection
 * @tc.desc: clear failed call connection execute normally
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_ClearFailedCallConnection_0400, Function | MediumTest | Level1)
{
    constexpr uint32_t COUNT_ZERO = 0;
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    localCallContainer->ClearFailedCallConnection(nullptr);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    localCallContainer->ClearFailedCallConnection(callback);
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    localCallContainer->ClearFailedCallConnection(callback);
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    connect->SetRecordAndContainer(localCallRecord, localCallContainer);
    localCallContainer->connections_.emplace(connect);
    localCallContainer->ClearFailedCallConnection(callback);
    EXPECT_EQ(COUNT_ZERO, localCallContainer->connections_.size());
}

/**
 * @tc.number: Local_Call_Container_ClearFailedCallConnection_0500
 * @tc.name: ClearFailedCallConnection empty deviceId
 * @tc.desc: clear failed call connection execute normally
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_ClearFailedCallConnection_0500, Function | MediumTest | Level1)
{
    constexpr uint32_t COUNT_ZERO = 0;
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    AppExecFwk::ElementName elementName("", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    auto connect = sptr<CallerConnection>::MakeSptr();
    connect->SetRecordAndContainer(localCallRecord, localCallContainer);
    localCallContainer->connections_.emplace(connect);
    localCallContainer->ClearFailedCallConnection(callback);
    EXPECT_EQ(COUNT_ZERO, localCallContainer->connections_.size());
}

/**
 * @tc.number: Local_Call_Container_RemoveSingletonCallLocalRecord_0100
 * @tc.name: RemoveSingletonCallLocalRecord
 * @tc.desc: remove singleton call local record execute normally
 */
HWTEST_F(
    LocalCallContainerTest, Local_Call_Container_RemoveSingletonCallLocalRecord_0100, Function | MediumTest | Level1)
{
    constexpr int32_t USER_ID_U202 = 202;
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecordNull = nullptr;
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    std::shared_ptr<LocalCallRecord> localCallRecordSecond = std::make_shared<LocalCallRecord>(elementName);
    localCallRecordSecond->SetUserId(USER_ID_U202);
    EXPECT_EQ(ERR_INVALID_VALUE, localCallContainer->RemoveSingletonCallLocalRecord(localCallRecordNull));
    EXPECT_EQ(ERR_INVALID_VALUE, localCallContainer->RemoveSingletonCallLocalRecord(localCallRecord));
    localCallContainer->SetCallLocalRecord(localCallRecord);
    EXPECT_EQ(ERR_OK, localCallContainer->RemoveSingletonCallLocalRecord(localCallRecord));
    localCallContainer->SetCallLocalRecord(localCallRecord);
    localCallContainer->SetCallLocalRecord(localCallRecordSecond);
    EXPECT_EQ(ERR_OK, localCallContainer->RemoveSingletonCallLocalRecord(localCallRecord));
}

/**
 * @tc.number: Local_Call_Container_RemoveMultipleCallLocalRecord_0100
 * @tc.name: RemoveMultipleCallLocalRecord
 * @tc.desc: remove singleton call local record execute normally
 */
HWTEST_F(
    LocalCallContainerTest, Local_Call_Container_RemoveMultipleCallLocalRecord_0100, Function | MediumTest | Level1)
{
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecordNull = nullptr;
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    std::shared_ptr<LocalCallRecord> localCallRecordSecond = std::make_shared<LocalCallRecord>(elementName);
    EXPECT_EQ(ERR_INVALID_VALUE, localCallContainer->RemoveMultipleCallLocalRecord(localCallRecordNull));
    EXPECT_EQ(ERR_INVALID_VALUE, localCallContainer->RemoveMultipleCallLocalRecord(localCallRecord));
    localCallContainer->SetMultipleCallLocalRecord(localCallRecord);
    EXPECT_EQ(ERR_OK, localCallContainer->RemoveMultipleCallLocalRecord(localCallRecord));
    localCallContainer->SetMultipleCallLocalRecord(localCallRecord);
    localCallContainer->SetMultipleCallLocalRecord(localCallRecordSecond);
    EXPECT_EQ(ERR_OK, localCallContainer->RemoveMultipleCallLocalRecord(localCallRecord));
}

/**
 * @tc.number: Local_Call_Container_OnAbilityDisconnectDone_0300
 * @tc.name: OnAbilityDisconnectDone
 * @tc.desc: OnAbilityDisconnectDone with REASON_CALLEE_TERMINATE when localCallRecord_ is nullptr.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityDisconnectDone_0300, Function | MediumTest | Level1)
{
    auto connect = sptr<CallerConnection>::MakeSptr();
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    constexpr int32_t code = 1; // REASON_CALLEE_TERMINATE
    connect->container_ = localCallContainer;
    connect->localCallRecord_ = nullptr;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    connect->OnAbilityDisconnectDone(elementName, code);
    EXPECT_EQ(connect->localCallRecord_, nullptr);
}

/**
 * @tc.number: Local_Call_Container_OnAbilityDisconnectDone_0400
 * @tc.name: OnAbilityDisconnectDone
 * @tc.desc: OnAbilityDisconnectDone with REASON_CALLEE_TERMINATE when container is nullptr.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityDisconnectDone_0400, Function | MediumTest | Level1)
{
    auto connect = sptr<CallerConnection>::MakeSptr();
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    constexpr int32_t code = 1; // REASON_CALLEE_TERMINATE
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    connect->localCallRecord_ = localCallRecord;

    connect->OnAbilityDisconnectDone(elementName, code);
    EXPECT_NE(connect->localCallRecord_, nullptr);
}

/**
 * @tc.number: Local_Call_Container_OnAbilityDisconnectDone_0500
 * @tc.name: OnAbilityDisconnectDone
 * @tc.desc: OnAbilityDisconnectDone with REASON_CALLEE_TERMINATE notifies callers with ON_RELEASE.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityDisconnectDone_0500, Function | MediumTest | Level1)
{
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);

    bool isOnReleaseCalled = false;
    std::string receivedReason;
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetOnRelease([&isOnReleaseCalled, &receivedReason](const std::string& reason) {
        isOnReleaseCalled = true;
        receivedReason = reason;
    });

    localCallRecord->AddCaller(callback);
    auto connect = sptr<CallerConnection>::MakeSptr();
    connect->SetRecordAndContainer(localCallRecord, localCallContainer);
    localCallContainer->connections_.emplace(connect);

    constexpr int32_t code = 1; // REASON_CALLEE_TERMINATE
    connect->OnAbilityDisconnectDone(elementName, code);

    EXPECT_TRUE(isOnReleaseCalled);
}

/**
 * @tc.number: Local_Call_Container_OnAbilityDisconnectDone_0600
 * @tc.name: OnAbilityDisconnectDone
 * @tc.desc: OnAbilityDisconnectDone with REASON_CALLEE_TERMINATE cleans up singleton record.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityDisconnectDone_0600, Function | MediumTest | Level1)
{
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetOnRelease([](const std::string&) {});
    localCallRecord->AddCaller(callback);

    auto connect = sptr<CallerConnection>::MakeSptr();
    connect->SetRecordAndContainer(localCallRecord, localCallContainer);
    localCallContainer->connections_.emplace(connect);
    localCallContainer->SetCallLocalRecord(localCallRecord);

    constexpr int32_t code = 1; // REASON_CALLEE_TERMINATE
    connect->OnAbilityDisconnectDone(elementName, code);

    EXPECT_EQ(connect->localCallRecord_, nullptr);
    EXPECT_EQ(localCallContainer->connections_.size(), 0u);
}

/**
 * @tc.number: Local_Call_Container_OnAbilityDisconnectDone_0700
 * @tc.name: OnAbilityDisconnectDone
 * @tc.desc: OnAbilityDisconnectDone with REASON_CALLEE_TERMINATE cleans up multiple record.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityDisconnectDone_0700, Function | MediumTest | Level1)
{
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->SetIsSingleton(false);

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetOnRelease([](const std::string&) {});
    localCallRecord->AddCaller(callback);

    auto connect = sptr<CallerConnection>::MakeSptr();
    connect->SetRecordAndContainer(localCallRecord, localCallContainer);
    localCallContainer->connections_.emplace(connect);
    localCallContainer->SetMultipleCallLocalRecord(localCallRecord);

    constexpr int32_t code = 1; // REASON_CALLEE_TERMINATE
    connect->OnAbilityDisconnectDone(elementName, code);

    EXPECT_EQ(connect->localCallRecord_, nullptr);
    EXPECT_EQ(localCallContainer->connections_.size(), 0u);
}

/**
 * @tc.number: Local_Call_Container_OnAbilityDisconnectDone_0800
 * @tc.name: OnAbilityDisconnectDone
 * @tc.desc: OnAbilityDisconnectDone with code != REASON_CALLEE_TERMINATE does nothing.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityDisconnectDone_0800, Function | MediumTest | Level1)
{
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);

    bool isOnReleaseCalled = false;
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetOnRelease([&isOnReleaseCalled](const std::string&) {
        isOnReleaseCalled = true;
    });
    localCallRecord->AddCaller(callback);

    auto connect = sptr<CallerConnection>::MakeSptr();
    connect->SetRecordAndContainer(localCallRecord, localCallContainer);
    localCallContainer->connections_.emplace(connect);
    localCallContainer->SetCallLocalRecord(localCallRecord);

    // Test with code != REASON_CALLEE_TERMINATE
    constexpr int32_t code = 0;
    connect->OnAbilityDisconnectDone(elementName, code);

    EXPECT_FALSE(isOnReleaseCalled);
    EXPECT_NE(connect->localCallRecord_, nullptr);
    EXPECT_EQ(localCallContainer->connections_.size(), 1u);
}

/**
 * @tc.number: Local_Call_Container_OnCallStubDied_0500
 * @tc.name: OnCallStubDied
 * @tc.desc: OnCallStubDied notifies matching singleton record with ON_DIED callback.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnCallStubDied_0500, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);

    bool isOnReleaseCalled = false;
    std::string receivedKey;
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetOnRelease([&isOnReleaseCalled, &receivedKey](const std::string& key) {
        isOnReleaseCalled = true;
        receivedKey = key;
    });
    localCallRecord->AddCaller(callback);

    sptr<IRemoteObject> remoteObject = new (std::nothrow) MockServiceAbilityManagerService();
    localCallRecord->remoteObject_ = remoteObject;
    localCallContainer.SetCallLocalRecord(localCallRecord);

    wptr<IRemoteObject> remote(remoteObject);
    localCallContainer.OnCallStubDied(remote);

    EXPECT_TRUE(isOnReleaseCalled);
    EXPECT_EQ(receivedKey, "died");
    EXPECT_TRUE(localCallContainer.callProxyRecords_.empty());
}

/**
 * @tc.number: Local_Call_Container_OnCallStubDied_0600
 * @tc.name: OnCallStubDied
 * @tc.desc: OnCallStubDied notifies matching multiple record with ON_DIED callback.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnCallStubDied_0600, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);

    bool isOnReleaseCalled = false;
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetOnRelease([&isOnReleaseCalled](const std::string& key) {
        isOnReleaseCalled = true;
        EXPECT_EQ(key, "died");
    });
    localCallRecord->AddCaller(callback);

    sptr<IRemoteObject> remoteObject = new (std::nothrow) MockServiceAbilityManagerService();
    localCallRecord->remoteObject_ = remoteObject;
    localCallContainer.SetMultipleCallLocalRecord(localCallRecord);

    wptr<IRemoteObject> remote(remoteObject);
    localCallContainer.OnCallStubDied(remote);

    EXPECT_TRUE(isOnReleaseCalled);
}

/**
 * @tc.number: Local_Call_Container_OnAbilityDisconnectDone_0900
 * @tc.name: OnAbilityDisconnectDone
 * @tc.desc: OnAbilityDisconnectDone with re-entrant release in onRelease callback.
 *           The onRelease callback calls release(), which resets localCallRecord_.
 *           Verify the null re-check prevents crash and returns early.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityDisconnectDone_0900, Function | MediumTest | Level1)
{
    auto localCallContainer = std::make_shared<LocalCallContainer>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->SetIsSingleton(true);

    sptr<CallerConnection> connect = sptr<CallerConnection>::MakeSptr();
    connect->SetRecordAndContainer(localCallRecord, localCallContainer);
    localCallContainer->connections_.emplace(connect);
    localCallContainer->SetCallLocalRecord(localCallRecord);

    sptr<IRemoteObject> remoteObject = new (std::nothrow) MockServiceAbilityManagerService();
    localCallRecord->remoteObject_ = remoteObject;

    // Set up onRelease callback that simulates user calling release() inside the callback.
    // This triggers the re-entrant path: ClearCallRecord() resets localCallRecord_ under lock.
    // After OnCallStubDied() returns, the local shared_ptr copy keeps the record alive
    // so subsequent operations (RemoveSingletonCallLocalRecord, ClearData, etc.) are safe.
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetOnRelease([&connect](const std::string& key) {
        if (key == "died") {
            // Simulate user calling release() in onRelease("died") callback.
            // Use the proper locked method instead of directly resetting the member.
            connect->ClearCallRecord();
        }
    });
    localCallRecord->AddCaller(callback);

    constexpr int32_t code = 1; // REASON_CALLEE_TERMINATE
    connect->OnAbilityDisconnectDone(elementName, code);

    // Should not crash; localCallRecord_ should be nullptr (reset by re-entrant callback)
    EXPECT_EQ(connect->localCallRecord_, nullptr);
}
} // namespace AppExecFwk
} // namespace OHOS
