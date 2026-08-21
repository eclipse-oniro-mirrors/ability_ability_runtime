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
#include "ability_loader.h"
#include "ability_thread.h"
#include "mock_serviceability_manager_service.h"
#define private public
#include "local_call_record.h"
#define protected public
#include "system_ability_definition.h"
#include "sys_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace {
constexpr int32_t FOREGROUND = 2;
constexpr int32_t BACKGROUND = 4;
}

class LocalCallRecordTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    std::unique_ptr<LocalCallRecord> context_ = nullptr;
};

void LocalCallRecordTest::SetUpTestCase(void)
{
    OHOS::sptr<OHOS::IRemoteObject> abilityObject = new (std::nothrow) MockServiceAbilityManagerService();

    auto sysMgr = OHOS::DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == NULL) {
        GTEST_LOG_(ERROR) << "fail to get ISystemAbilityManager";
        return;
    }
    sysMgr->RegisterSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID, abilityObject);
}

void LocalCallRecordTest::TearDownTestCase(void)
{}

void LocalCallRecordTest::SetUp(void)
{}

void LocalCallRecordTest::TearDown(void)
{}

/**
 * @tc.number: Local_Call_Record_AddCaller_0100
 * @tc.name: AddCaller
 * @tc.desc: LocalCallRecord to process AddCaller success.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_AddCaller_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    EXPECT_TRUE(localCallRecord.callers_.size() == 0);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    localCallRecord.AddCaller(callback);
    EXPECT_TRUE(localCallRecord.callers_.size() != 0);
}

/**
 * @tc.number: Local_Call_Record_RemoveCaller_0100
 * @tc.name: RemoveCaller
 * @tc.desc: LocalCallRecord to process RemoveCaller success.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_RemoveCaller_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    localCallRecord.callers_.emplace_back(callback);
    bool ret = localCallRecord.RemoveCaller(callback);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.number: Local_Call_Record_RemoveCaller_0200
 * @tc.name: RemoveCaller
 * @tc.desc: LocalCallRecord to process RemoveCaller fail because callers_ is empty.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_RemoveCaller_0200, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    bool ret = localCallRecord.RemoveCaller(callback);
    EXPECT_EQ(ret, false);
}

/**
* @tc.number: Local_Call_Record_RemoveCaller_0300
* @tc.name: RemoveCaller
* @tc.desc: LocalCallRecord to process RemoveCaller fail because not find call back.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_RemoveCaller_0300, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    localCallRecord.callers_.emplace_back(callback);
    std::shared_ptr<CallerCallBack> callback1 = std::make_shared<CallerCallBack>();
    bool ret = localCallRecord.RemoveCaller(callback1);
    EXPECT_EQ(ret, false);
}

/**
* @tc.number: Local_Call_Record_OnCallStubDied_0100
* @tc.name: OnCallStubDied
* @tc.desc: LocalCallRecord to process OnCallStubDied success.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_OnCallStubDied_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetOnRelease([](const std::string& result) {
        GTEST_LOG_(ERROR) << "OnRelease-----------" << result;
        EXPECT_TRUE(result == "died");
        });
    localCallRecord.callers_.emplace_back(callback);
    localCallRecord.OnCallStubDied();
}

/**
* @tc.number: Local_Call_Record_OnCallStubDied_0200
* @tc.name: OnCallStubDied
* @tc.desc: LocalCallRecord to process OnCallStubDied fail because callers_ is empty.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_OnCallStubDied_0200, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetOnRelease([](const std::string& result) {
        GTEST_LOG_(ERROR) << "OnRelease-----------" << result;
        EXPECT_TRUE(result != "died");
        });
    localCallRecord.OnCallStubDied();
}

/**
* @tc.number: Local_Call_Record_OnCallStubDied_0300
* @tc.name: OnCallStubDied
* @tc.desc: LocalCallRecord to process OnCallStubDied fail because callback is null.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_OnCallStubDied_0300, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetOnRelease([](const std::string& result) {
        GTEST_LOG_(ERROR) << "OnRelease-----------" << result;
        EXPECT_TRUE(result != "died");
        });
    localCallRecord.callers_.emplace_back(nullptr);
    localCallRecord.OnCallStubDied();
}

/**
* @tc.number: Local_Call_Record_InvokeCallBack_0100
* @tc.name: InvokeCallBack
* @tc.desc: LocalCallRecord to process InvokeCallBack success.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_InvokeCallBack_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    localCallRecord.remoteObject_ = new (std::nothrow) MockServiceAbilityManagerService();
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    EXPECT_TRUE(callback->IsCallBack() == false);
    localCallRecord.callers_.emplace_back(callback);
    localCallRecord.InvokeCallBack();
    EXPECT_TRUE(callback->IsCallBack() == true);
}

/**
* @tc.number: Local_Call_Record_InvokeCallBack_0200
* @tc.name: InvokeCallBack
* @tc.desc: LocalCallRecord to process InvokeCallBack fail because remote is null.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_InvokeCallBack_0200, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    EXPECT_TRUE(callback->IsCallBack() == false);
    localCallRecord.InvokeCallBack();
    EXPECT_TRUE(callback->IsCallBack() == false);
}

/**
* @tc.number: Local_Call_Record_InvokeCallBack_0300
* @tc.name: InvokeCallBack
* @tc.desc: LocalCallRecord to process InvokeCallBack fail because callers_ is empty.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_InvokeCallBack_0300, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    localCallRecord.remoteObject_ = new (std::nothrow) MockServiceAbilityManagerService();
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    EXPECT_TRUE(callback->IsCallBack() == false);
    localCallRecord.InvokeCallBack();
    EXPECT_TRUE(callback->IsCallBack() == false);
}

/**
* @tc.number: Local_Call_Record_InvokeCallBack_0400
* @tc.name: InvokeCallBack
* @tc.desc: LocalCallRecord to process InvokeCallBack fail because call back is null.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_InvokeCallBack_0400, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    localCallRecord.remoteObject_ = new (std::nothrow) MockServiceAbilityManagerService();
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    EXPECT_TRUE(callback->IsCallBack() == false);
    localCallRecord.callers_.emplace_back(nullptr);
    localCallRecord.InvokeCallBack();
    EXPECT_TRUE(callback->IsCallBack() == false);
}

/**
* @tc.number: Local_Call_Record_NotifyRemoteStateChanged_0100
* @tc.name: NotifyRemoteStateChanged
* @tc.desc: LocalCallRecord to process NotifyRemoteStateChanged success.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_NotifyRemoteStateChanged_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    localCallRecord.remoteObject_ = new (std::nothrow) MockServiceAbilityManagerService();
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    EXPECT_TRUE(callback->IsCallBack() == false);
    localCallRecord.callers_.emplace_back(callback);
    localCallRecord.InvokeCallBack();
    int32_t abilityState = FOREGROUND;
    localCallRecord.NotifyRemoteStateChanged(abilityState);
    EXPECT_TRUE(callback->IsCallBack() == true);
}

/**
* @tc.number: Local_Call_Record_NotifyRemoteStateChanged_0200
* @tc.name: NotifyRemoteStateChanged
* @tc.desc: LocalCallRecord to process NotifyRemoteStateChanged fail because remote is null.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_NotifyRemoteStateChanged_0200, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    EXPECT_TRUE(callback->IsCallBack() == false);
    int32_t abilityState = FOREGROUND;
    localCallRecord.NotifyRemoteStateChanged(abilityState);
    EXPECT_TRUE(callback->IsCallBack() == false);
}

/**
* @tc.number: Local_Call_Record_NotifyRemoteStateChanged_0300
* @tc.name: NotifyRemoteStateChanged
* @tc.desc: LocalCallRecord to process NotifyRemoteStateChanged fail because callers_ is empty.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_NotifyRemoteStateChanged_0300, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    localCallRecord.remoteObject_ = new (std::nothrow) MockServiceAbilityManagerService();
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    EXPECT_TRUE(callback->IsCallBack() == false);
    int32_t abilityState = BACKGROUND;
    localCallRecord.NotifyRemoteStateChanged(abilityState);
    EXPECT_TRUE(callback->IsCallBack() == false);
}

/**
* @tc.number: Local_Call_Record_NotifyRemoteStateChanged_0400
* @tc.name: NotifyRemoteStateChanged
* @tc.desc: LocalCallRecord to process NotifyRemoteStateChanged fail because call back is null.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_NotifyRemoteStateChanged_0400, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    localCallRecord.remoteObject_ = new (std::nothrow) MockServiceAbilityManagerService();
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    EXPECT_TRUE(callback->IsCallBack() == false);
    localCallRecord.callers_.emplace_back(nullptr);
    int32_t abilityState = 0;
    localCallRecord.NotifyRemoteStateChanged(abilityState);
    EXPECT_TRUE(callback->IsCallBack() == false);
}

/**
* @tc.number: Local_Call_Record_GetRemoteObject_0100
* @tc.name: GetRemoteObject
* @tc.desc: LocalCallRecord to process GetRemoteObject success.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_GetRemoteObject_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    localCallRecord.remoteObject_ = new (std::nothrow) MockServiceAbilityManagerService();
    EXPECT_TRUE(localCallRecord.GetRemoteObject() != nullptr);
}

/**
* @tc.number: Local_Call_Record_GetElementName_0100
* @tc.name: GetElementName
* @tc.desc: LocalCallRecord to process GetElementName success.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_GetElementName_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    EXPECT_TRUE(localCallRecord.GetElementName().GetBundleName() == "DemoBundleName");
}

/**
* @tc.number: Local_Call_Record_IsExistCallBack_0100
* @tc.name: IsExistCallBack
* @tc.desc: LocalCallRecord to process  success.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_IsExistCallBack_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    localCallRecord.AddCaller(callback);
    EXPECT_TRUE(localCallRecord.IsExistCallBack() == true);
}

/**
* @tc.number: Local_Call_Record_GetRecordId_0100
* @tc.name: GetRecordId
* @tc.desc: Verify that function GetRecordId is 1.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_GetRecordId_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    localCallRecord.recordId_ = 1;
    EXPECT_TRUE(localCallRecord.GetRecordId() == 1);
}

/**
* @tc.number: Local_Call_Record_GetCallers_0100
* @tc.name: GetCallers
* @tc.desc: Verify that function GetCallers.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_GetCallers_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    auto result = localCallRecord.GetCallers();
    EXPECT_TRUE(result.empty());
}

/**
* @tc.number: Local_Call_Record_IsSameObject_0100
* @tc.name: IsSameObject
* @tc.desc: Verify that function IsSameObject.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_IsSameObject_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    sptr<IRemoteObject> remote = new (std::nothrow) MockServiceAbilityManagerService();
    localCallRecord.remoteObject_ = remote;
    auto result = localCallRecord.IsSameObject(remote);
    EXPECT_TRUE(result);
}

/**
* @tc.number: Local_Call_Record_IsSameObject_0200
* @tc.name: IsSameObject
* @tc.desc: Verify that function IsSameObject.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_IsSameObject_0200, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    sptr<IRemoteObject> remote = nullptr;
    auto result = localCallRecord.IsSameObject(remote);
    EXPECT_FALSE(result);
}

/**
* @tc.number: Local_Call_Record_IsSameObject_0300
* @tc.name: IsSameObject
* @tc.desc: Verify that function IsSameObject.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_IsSameObject_0300, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    sptr<IRemoteObject> remote = new (std::nothrow) MockServiceAbilityManagerService();
    auto result = localCallRecord.IsSameObject(remote);
    EXPECT_FALSE(result);
}

/**
* @tc.number: Local_Call_Record_SetIsSingleton_0100
* @tc.name: SetIsSingleton
* @tc.desc: SetIsSingleton input parameter false and IsSingletonRemote return false.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_SetIsSingleton_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    localCallRecord.SetIsSingleton(false);
    EXPECT_FALSE(localCallRecord.IsSingletonRemote());
}

/**
* @tc.number: Local_Call_Record_SetIsSingleton_0200
* @tc.name: SetIsSingleton
* @tc.desc: SetIsSingleton input parameter true and IsSingletonRemote return true.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_SetIsSingleton_0200, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    localCallRecord.SetIsSingleton(true);
    EXPECT_TRUE(localCallRecord.IsSingletonRemote());
}

/**
* @tc.number: Local_Call_Record_SetConnection_0100
* @tc.name: SetConnection
* @tc.desc: SetConnection input parameter connect and GetConnection return connect.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_SetConnection_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    sptr<IRemoteObject> connect = new (std::nothrow) MockServiceAbilityManagerService();
    localCallRecord.SetConnection(connect);
    EXPECT_EQ(localCallRecord.GetConnection(), connect);
}

/**
* @tc.number: Local_Call_Record_SetUserId_0100
* @tc.name: SetUserId
* @tc.desc: SetUserId input parameter connect and GetConnection return connect.
*/
HWTEST_F(LocalCallRecordTest, Local_Call_Record_SetUserId_0100, Function | MediumTest | Level1)
{
    constexpr int32_t USER_ID_U100 = 100;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    localCallRecord.SetUserId(USER_ID_U100);
    EXPECT_EQ(USER_ID_U100, localCallRecord.GetUserId());
}

/**
 * @tc.number: Local_Call_Record_RemoveCaller_0400
 * @tc.name: RemoveCaller
 * @tc.desc: RemoveCaller with onRelease callback set, verify InvokeOnRelease(ON_RELEASE) fires.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_RemoveCaller_0400, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    bool isOnReleaseCalled = false;
    std::string receivedKey;
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetOnRelease([&isOnReleaseCalled, &receivedKey](const std::string& key) {
        isOnReleaseCalled = true;
        receivedKey = key;
    });
    localCallRecord.callers_.emplace_back(callback);
    bool ret = localCallRecord.RemoveCaller(callback);
    EXPECT_EQ(ret, true);
    EXPECT_TRUE(isOnReleaseCalled);
    EXPECT_EQ(receivedKey, ON_RELEASE);
    EXPECT_TRUE(localCallRecord.callers_.empty());
}

/**
 * @tc.number: Local_Call_Record_ClearData_0100
 * @tc.name: ClearData
 * @tc.desc: ClearData removes death recipient and clears callers.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_ClearData_0100, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    sptr<IRemoteObject> remote = new (std::nothrow) MockServiceAbilityManagerService();
    localCallRecord.remoteObject_ = remote;
    localCallRecord.callRecipient_ = sptr<CallRecipient>::MakeSptr([](const wptr<IRemoteObject>&) {});
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    localCallRecord.callers_.emplace_back(callback);
    localCallRecord.ClearData();
    EXPECT_EQ(localCallRecord.remoteObject_, nullptr);
    EXPECT_EQ(localCallRecord.callRecipient_, nullptr);
    EXPECT_TRUE(localCallRecord.callers_.empty());
}

/**
 * @tc.number: Local_Call_Record_ClearData_0200
 * @tc.name: ClearData
 * @tc.desc: ClearData when remoteObject_ is already null, does nothing.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_ClearData_0200, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    localCallRecord.ClearData();
    EXPECT_EQ(localCallRecord.remoteObject_, nullptr);
}

/**
 * @tc.number: Local_Call_Record_SetRemoteObject_0300
 * @tc.name: SetRemoteObject
 * @tc.desc: SetRemoteObject two-arg version with custom death recipient.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_SetRemoteObject_0300, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    sptr<IRemoteObject> call = new (std::nothrow) MockServiceAbilityManagerService();
    bool diedCalled = false;
    auto recipient = sptr<CallRecipient>::MakeSptr([&diedCalled](const wptr<IRemoteObject>&) {
        diedCalled = true;
    });
    localCallRecord.SetRemoteObject(call, recipient);
    EXPECT_EQ(localCallRecord.remoteObject_, call);
    EXPECT_TRUE(localCallRecord.callRecipient_ != nullptr);
}

/**
 * @tc.number: Local_Call_Record_SetRemoteObject_0400
 * @tc.name: SetRemoteObject
 * @tc.desc: SetRemoteObject two-arg version with null call, does nothing.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_SetRemoteObject_0400, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    auto recipient = sptr<CallRecipient>::MakeSptr([](const wptr<IRemoteObject>&) {});
    localCallRecord.SetRemoteObject(nullptr, recipient);
    EXPECT_EQ(localCallRecord.remoteObject_, nullptr);
}

/**
 * @tc.number: Local_Call_Record_SetRemoteObject_0500
 * @tc.name: SetRemoteObject
 * @tc.desc: SetRemoteObject does not overwrite when remoteObject_ already set (re-entrancy guard).
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_SetRemoteObject_0500, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    sptr<IRemoteObject> callA = new (std::nothrow) MockServiceAbilityManagerService();
    sptr<IRemoteObject> callB = new (std::nothrow) MockServiceAbilityManagerService();
    sptr<IRemoteObject::DeathRecipient> recipientA = sptr<CallRecipient>::MakeSptr([](const wptr<IRemoteObject>&) {});
    sptr<IRemoteObject::DeathRecipient> recipientB = sptr<CallRecipient>::MakeSptr([](const wptr<IRemoteObject>&) {});

    localCallRecord.SetRemoteObject(callA, recipientA);
    EXPECT_EQ(localCallRecord.remoteObject_, callA);
    EXPECT_EQ(localCallRecord.callRecipient_, recipientA);

    // Second call must not overwrite the already-set remote/recipient.
    localCallRecord.SetRemoteObject(callB, recipientB);
    EXPECT_EQ(localCallRecord.remoteObject_, callA);
    EXPECT_EQ(localCallRecord.callRecipient_, recipientA);
    EXPECT_NE(localCallRecord.remoteObject_, callB);
    EXPECT_NE(localCallRecord.callRecipient_, recipientB);
}

/**
 * @tc.number: Local_Call_Record_SetRemoteObject_0600
 * @tc.name: SetRemoteObject
 * @tc.desc: After ClearData resets remoteObject_, SetRemoteObject can set again.
 */
HWTEST_F(LocalCallRecordTest, Local_Call_Record_SetRemoteObject_0600, Function | MediumTest | Level1)
{
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);
    sptr<IRemoteObject> callA = new (std::nothrow) MockServiceAbilityManagerService();
    sptr<IRemoteObject::DeathRecipient> recipientA = sptr<CallRecipient>::MakeSptr([](const wptr<IRemoteObject>&) {});
    localCallRecord.SetRemoteObject(callA, recipientA);
    EXPECT_EQ(localCallRecord.remoteObject_, callA);

    localCallRecord.ClearData();
    EXPECT_EQ(localCallRecord.remoteObject_, nullptr);
    EXPECT_EQ(localCallRecord.callRecipient_, nullptr);

    // After ClearData, remoteObject_ is null; SetRemoteObject should succeed again.
    sptr<IRemoteObject> callB = new (std::nothrow) MockServiceAbilityManagerService();
    sptr<IRemoteObject::DeathRecipient> recipientB = sptr<CallRecipient>::MakeSptr([](const wptr<IRemoteObject>&) {});
    localCallRecord.SetRemoteObject(callB, recipientB);
    EXPECT_EQ(localCallRecord.remoteObject_, callB);
    EXPECT_EQ(localCallRecord.callRecipient_, recipientB);
}
} // namespace AppExecFwk
} // namespace OHOS
