/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <mutex>

#define private public
#include "ability_connection.h"
#undef private

#include "refbase.h"
#include "hilog_tag_wrapper.h"
#include "oh_mock_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AbilityConnectionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityConnectionTest::SetUpTestCase()
{}

void AbilityConnectionTest::TearDownTestCase()
{}

void AbilityConnectionTest::SetUp()
{}

void AbilityConnectionTest::TearDown()
{}

/**
 * @tc.name: OnAbilityConnectDone_001
 * @tc.desc: Verify OnAbilityConnectDone call.
 *           Branch abilityConnectCallbackList_ = nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, OnAbilityConnectDone_001, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    instance->abilityConnectCallbackList_.clear();
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObject>::MakeSptr();
    int resultCode = 0;
    instance->OnAbilityConnectDone(element, remoteObject, resultCode);
    EXPECT_EQ(instance->remoteObject_, nullptr);
}

/**
 * @tc.name: OnAbilityConnectDone_002
 * @tc.desc: Verify OnAbilityConnectDone call.
 *           Branch abilityConnectCallbackList_ != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, OnAbilityConnectDone_002, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    sptr<AbilityRuntime::AbilityConnectCallback> callback = sptr<AbilityRuntime::AbilityConnectCallback>::MakeSptr();
    instance->abilityConnectCallbackList_.emplace_back(callback);
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObject>::MakeSptr();
    int resultCode = 0;
    instance->OnAbilityConnectDone(element, remoteObject, resultCode);
    EXPECT_NE(instance->remoteObject_, nullptr);
}

/**
 * @tc.name: OnAbilityConnectDone_003
 * @tc.desc: Verify OnAbilityConnectDone call.
 *           Branch DisconnectNonexistentService() == true
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, OnAbilityConnectDone_003, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    sptr<AbilityRuntime::AbilityConnectCallback> callback = sptr<AbilityRuntime::AbilityConnectCallback>::MakeSptr();
    instance->abilityConnectCallbackList_.emplace_back(callback);
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObject>::MakeSptr();
    int resultCode = 0;
    OH_EXPECT_RET({true}, ConnectionManager, DisconnectNonexistentService, const AppExecFwk::ElementName&,
        const sptr<AbilityConnection>);
    instance->OnAbilityConnectDone(element, remoteObject, resultCode);
}

/**
 * @tc.name: OnAbilityConnectDone_004
 * @tc.desc: Verify OnAbilityConnectDone call.
 *           Branch DisconnectNonexistentService() == false
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, OnAbilityConnectDone_004, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    sptr<AbilityRuntime::AbilityConnectCallback> callback = sptr<AbilityRuntime::AbilityConnectCallback>::MakeSptr();
    instance->abilityConnectCallbackList_.emplace_back(callback);
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObject>::MakeSptr();
    int resultCode = 0;
    OH_EXPECT_RET({false}, ConnectionManager, DisconnectNonexistentService, const AppExecFwk::ElementName&,
        const sptr<AbilityConnection>);
    instance->OnAbilityConnectDone(element, remoteObject, resultCode);
    EXPECT_EQ(instance->connectionState_, AbilityRuntime::CONNECTION_STATE_CONNECTED);
    EXPECT_LOG_EQ(LOG_INFO, AAFwkTag::CONNECTION, "ability_connection.cpp", 34);
}

/**
 * @tc.name: OnAbilityDisconnectDone_001
 * @tc.desc: Verify OnAbilityDisconnectDone call.
 *           Branch abilityConnectCallbackList_ = nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, OnAbilityDisconnectDone_001, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    instance->remoteObject_ = sptr<IRemoteObject>::MakeSptr();
    instance->abilityConnectCallbackList_.clear();
    AppExecFwk::ElementName element;
    int resultCode = 0;
    instance->OnAbilityDisconnectDone(element, resultCode);
    EXPECT_EQ(instance->connectionState_, AbilityRuntime::CONNECTION_STATE_DISCONNECTED);
    EXPECT_NE(instance->remoteObject_, nullptr);
}

/**
 * @tc.name: OnAbilityDisconnectDone_002
 * @tc.desc: Verify OnAbilityDisconnectDone call.
 *           Branch abilityConnectCallbackList_ != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, OnAbilityDisconnectDone_002, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    instance->remoteObject_ = sptr<IRemoteObject>::MakeSptr();
    sptr<AbilityRuntime::AbilityConnectCallback> callback = sptr<AbilityRuntime::AbilityConnectCallback>::MakeSptr();
    instance->abilityConnectCallbackList_.emplace_back(callback);
    AppExecFwk::ElementName element;
    int resultCode = 0;
    instance->OnAbilityDisconnectDone(element, resultCode);
    EXPECT_EQ(instance->connectionState_, AbilityRuntime::CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(instance->remoteObject_, nullptr);
}

/**
 * @tc.name: OnAbilityDisconnectDone_003
 * @tc.desc: Verify OnAbilityDisconnectDone call.
 *           Branch resultCode == DIED
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, OnAbilityDisconnectDone_003, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    instance->remoteObject_ = sptr<IRemoteObject>::MakeSptr();
    sptr<AbilityRuntime::AbilityConnectCallback> callback = sptr<AbilityRuntime::AbilityConnectCallback>::MakeSptr();
    instance->abilityConnectCallbackList_.emplace_back(callback);
    AppExecFwk::ElementName element;
    int resultCode = -1;
    OH_EXPECT_RET({true}, ConnectionManager, RemoveConnection, const sptr<AbilityConnection>);
    instance->OnAbilityDisconnectDone(element, resultCode);
    EXPECT_EQ(instance->connectionState_, AbilityRuntime::CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(instance->remoteObject_, nullptr);
}

/**
 * @tc.name: OnAbilityDisconnectDone_004
 * @tc.desc: Verify OnAbilityDisconnectDone call.
 *           Branch resultCode == DIED
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, OnAbilityDisconnectDone_004, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    instance->remoteObject_ = sptr<IRemoteObject>::MakeSptr();
    sptr<AbilityRuntime::AbilityConnectCallback> callback = sptr<AbilityRuntime::AbilityConnectCallback>::MakeSptr();
    instance->abilityConnectCallbackList_.emplace_back(callback);
    AppExecFwk::ElementName element;
    int resultCode = -1;
    OH_EXPECT_RET({false}, ConnectionManager, RemoveConnection, const sptr<AbilityConnection>);
    instance->OnAbilityDisconnectDone(element, resultCode);
    EXPECT_EQ(instance->connectionState_, AbilityRuntime::CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(instance->remoteObject_, nullptr);
}

/**
 * @tc.name: AddConnectCallback_001
 * @tc.desc: Verify AddConnectCallback call.
 *           Branch abilityConnectCallbackList_ is empty
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, AddConnectCallback_001, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    sptr<AbilityRuntime::AbilityConnectCallback> callback = sptr<AbilityRuntime::AbilityConnectCallback>::MakeSptr();
    instance->abilityConnectCallbackList_.clear();
    instance->AddConnectCallback(callback);
    EXPECT_EQ(instance->abilityConnectCallbackList_.size(), 1);
    EXPECT_EQ(instance->abilityConnectCallbackList_[0], callback);
}

/**
 * @tc.name: AddConnectCallback_002
 * @tc.desc: Verify AddConnectCallback call.
 *           Branch callback not in abilityConnectCallbackList_
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, AddConnectCallback_002, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    sptr<AbilityRuntime::AbilityConnectCallback> callback = sptr<AbilityRuntime::AbilityConnectCallback>::MakeSptr();
    sptr<AbilityRuntime::AbilityConnectCallback> callback1 = sptr<AbilityRuntime::AbilityConnectCallback>::MakeSptr();
    instance->abilityConnectCallbackList_.emplace_back(callback1);
    instance->AddConnectCallback(callback);
    EXPECT_EQ(instance->abilityConnectCallbackList_.size(), 2);
    EXPECT_EQ(instance->abilityConnectCallbackList_[0], callback1);
    EXPECT_EQ(instance->abilityConnectCallbackList_[1], callback);
}

/**
 * @tc.name: AddConnectCallback_003
 * @tc.desc: Verify AddConnectCallback call.
 *           Branch callback in abilityConnectCallbackList_
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, AddConnectCallback_003, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    sptr<AbilityRuntime::AbilityConnectCallback> callback = sptr<AbilityRuntime::AbilityConnectCallback>::MakeSptr();
    instance->abilityConnectCallbackList_.emplace_back(callback);
    instance->AddConnectCallback(callback);
    EXPECT_EQ(instance->abilityConnectCallbackList_.size(), 1);
    EXPECT_EQ(instance->abilityConnectCallbackList_[0], callback);
}

/**
 * @tc.name: RemoveConnectCallback_001
 * @tc.desc: Verify RemoveConnectCallback call.
 *           Branch abilityConnectCallbackList_ is empty
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, RemoveConnectCallback_001, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    sptr<AbilityRuntime::AbilityConnectCallback> callback = sptr<AbilityRuntime::AbilityConnectCallback>::MakeSptr();
    instance->abilityConnectCallbackList_.clear();
    instance->RemoveConnectCallback(callback);
    EXPECT_EQ(instance->abilityConnectCallbackList_.size(), 0);
}

/**
 * @tc.name: RemoveConnectCallback_002
 * @tc.desc: Verify RemoveConnectCallback call.
 *           Branch callback not in abilityConnectCallbackList_
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, RemoveConnectCallback_002, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    sptr<AbilityRuntime::AbilityConnectCallback> callback = sptr<AbilityRuntime::AbilityConnectCallback>::MakeSptr();
    sptr<AbilityRuntime::AbilityConnectCallback> callback1 = sptr<AbilityRuntime::AbilityConnectCallback>::MakeSptr();
    instance->abilityConnectCallbackList_.emplace_back(callback1);
    EXPECT_EQ(instance->abilityConnectCallbackList_.size(), 1);
    instance->RemoveConnectCallback(callback);
    EXPECT_EQ(instance->abilityConnectCallbackList_.size(), 1);
}

/**
 * @tc.name: RemoveConnectCallback_003
 * @tc.desc: Verify RemoveConnectCallback call.
 *           Branch callback not in abilityConnectCallbackList_
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, RemoveConnectCallback_003, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    sptr<AbilityRuntime::AbilityConnectCallback> callback = sptr<AbilityRuntime::AbilityConnectCallback>::MakeSptr();
    instance->abilityConnectCallbackList_.emplace_back(callback);
    EXPECT_EQ(instance->abilityConnectCallbackList_.size(), 1);
    instance->RemoveConnectCallback(callback);
    EXPECT_EQ(instance->abilityConnectCallbackList_.size(), 0);
}

/**
 * @tc.name: SetRemoteObject_001
 * @tc.desc: Verify SetRemoteObject call.
 *           Branch NA
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, SetRemoteObject_001, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    EXPECT_EQ(instance->remoteObject_, nullptr);
    sptr<IRemoteObject> object = sptr<IRemoteObject>::MakeSptr();
    instance->SetRemoteObject(object);
    EXPECT_EQ(instance->remoteObject_, object);
}

/**
 * @tc.name: SetResultCode_001
 * @tc.desc: Verify SetResultCode call.
 *           Branch NA
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, SetResultCode_001, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    EXPECT_EQ(instance->resultCode_, -1);
    int resultCode = 0;
    instance->SetResultCode(resultCode);
    EXPECT_EQ(instance->resultCode_, 0);
}

/**
 * @tc.name: SetConnectionState_001
 * @tc.desc: Verify SetConnectionState call.
 *           Branch NA
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, SetConnectionState_001, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    EXPECT_EQ(instance->connectionState_, AbilityRuntime::CONNECTION_STATE_DISCONNECTED);
    int connectionState = AbilityRuntime::CONNECTION_STATE_CONNECTED;
    instance->SetConnectionState(connectionState);
    EXPECT_EQ(instance->connectionState_, AbilityRuntime::CONNECTION_STATE_CONNECTED);
}

/**
 * @tc.name: GetRemoteObject_001
 * @tc.desc: Verify GetRemoteObject call.
 *           Branch NA
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, GetRemoteObject_001, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    EXPECT_EQ(instance->GetRemoteObject(), nullptr);
    sptr<IRemoteObject> object = sptr<IRemoteObject>::MakeSptr();
    instance->remoteObject_ = object;
    EXPECT_EQ(instance->GetRemoteObject(), object);
}

/**
 * @tc.name: GetResultCode_001
 * @tc.desc: Verify GetResultCode call.
 *           Branch NA
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, GetResultCode_001, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    EXPECT_EQ(instance->GetResultCode(), -1);
    int resultCode = 0;
    instance->resultCode_ = resultCode;
    EXPECT_EQ(instance->GetResultCode(), 0);
}

/**
 * @tc.name: GetConnectionState_001
 * @tc.desc: Verify GetConnectionState call.
 *           Branch NA
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, GetConnectionState_001, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    EXPECT_EQ(instance->GetConnectionState(), AbilityRuntime::CONNECTION_STATE_DISCONNECTED);
    int connectionState = AbilityRuntime::CONNECTION_STATE_CONNECTED;
    instance->connectionState_ = connectionState;
    EXPECT_EQ(instance->GetConnectionState(), AbilityRuntime::CONNECTION_STATE_CONNECTED);
}

/**
 * @tc.name: GetCallbackList_001
 * @tc.desc: Verify GetCallbackList call.
 *           Branch NA
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectionTest, GetCallbackList_001, TestSize.Level1)
{
    sptr<AbilityRuntime::AbilityConnection> instance =
        sptr<AbilityRuntime::AbilityConnection>::MakeSptr();
    EXPECT_NE(instance, nullptr);
    EXPECT_EQ(instance->GetCallbackList(), instance->abilityConnectCallbackList_);
}
} // namespace AppExecFwk
} // namespace OHOS