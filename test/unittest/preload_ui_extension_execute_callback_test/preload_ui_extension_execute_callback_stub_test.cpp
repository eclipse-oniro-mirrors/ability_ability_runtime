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
#include <gmock/gmock.h>
#include "ability_manager_errors.h"
#include "preload_ui_extension_execute_callback_stub.h"
#include "preload_ui_extension_execute_callback_proxy.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "iremote_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t FOUNDATION_UID = 5523;
}

class MockPreloadUIExtensionExecuteCallback : public PreloadUIExtensionExecuteCallbackStub {
public:
    MockPreloadUIExtensionExecuteCallback() = default;
    virtual ~MockPreloadUIExtensionExecuteCallback() = default;

    void OnLoadedDone(int32_t extensionAbilityId) override
    {
        onLoadedDoneCalled_ = true;
        extensionAbilityId_ = extensionAbilityId;
    }

    void OnDestroyDone(int32_t extensionAbilityId) override
    {
        onDestroyDoneCalled_ = true;
        extensionAbilityId_ = extensionAbilityId;
    }

    void OnPreloadSuccess(int32_t requestCode, int32_t extensionAbilityId, int32_t innerErrCode) override
    {
        onPreloadSuccessCalled_ = true;
        requestCode_ = requestCode;
        extensionAbilityId_ = extensionAbilityId;
        innerErrCode_ = innerErrCode;
    }

    bool IsOnLoadedDoneCalled() const { return onLoadedDoneCalled_; }
    bool IsOnDestroyDoneCalled() const { return onDestroyDoneCalled_; }
    bool IsOnPreloadSuccessCalled() const { return onPreloadSuccessCalled_; }
    
    int32_t GetExtensionAbilityId() const { return extensionAbilityId_; }
    int32_t GetRequestCode() const { return requestCode_; }
    int32_t GetInnerErrCode() const { return innerErrCode_; }

    void Reset()
    {
        onLoadedDoneCalled_ = false;
        onDestroyDoneCalled_ = false;
        onPreloadSuccessCalled_ = false;
        extensionAbilityId_ = 0;
        requestCode_ = 0;
        innerErrCode_ = 0;
    }

private:
    bool onLoadedDoneCalled_ = false;
    bool onDestroyDoneCalled_ = false;
    bool onPreloadSuccessCalled_ = false;
    int32_t extensionAbilityId_ = 0;
    int32_t requestCode_ = 0;
    int32_t innerErrCode_ = 0;
};

class MockRemoteObject : public IRemoteStub<IPreloadUIExtensionExecuteCallback> {
public:
    MockRemoteObject() = default;
    virtual ~MockRemoteObject() = default;

    MOCK_METHOD4(SendRequest, int(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option));
    
    void OnLoadedDone(int32_t extensionAbilityId) override {}
    void OnDestroyDone(int32_t extensionAbilityId) override {}
    void OnPreloadSuccess(int32_t requestCode, int32_t extensionAbilityId, int32_t innerErrCode) override {}
    
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return ERR_OK;
    }
};

class TestPreloadUIExtensionExecuteCallbackProxy : public PreloadUIExtensionExecuteCallbackProxy {
public:
    explicit TestPreloadUIExtensionExecuteCallbackProxy(const sptr<IRemoteObject> &impl)
        : PreloadUIExtensionExecuteCallbackProxy(impl)
    {}

    sptr<IRemoteObject> GetRemoteObject()
    {
        return Remote();
    }
};

class PreloadUIExtensionExecuteCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        IPCSkeleton::SetCallingUid(FOUNDATION_UID);
        stub_ = new (std::nothrow) MockPreloadUIExtensionExecuteCallback();
    }
    void TearDown() override
    {
        IPCSkeleton::SetCallingUid(0);
        stub_ = nullptr;
    }

    sptr<MockPreloadUIExtensionExecuteCallback> stub_ = nullptr;
};

/**
 * @tc.name: HandleOnLoadedDone_0100
 * @tc.desc: Test HandleOnLoadedDone with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, HandleOnLoadedDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IPreloadUIExtensionExecuteCallback::GetDescriptor());
    
    int32_t extensionAbilityId = 100;
    data.WriteInt32(extensionAbilityId);
    
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE,
        data, reply, option);
    
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(stub_->IsOnLoadedDoneCalled());
    EXPECT_EQ(stub_->GetExtensionAbilityId(), extensionAbilityId);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: HandleOnLoadedDone_EmptyBundleName_0200
 * @tc.desc: Test HandleOnLoadedDone with another extensionAbilityId
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, HandleOnLoadedDone_EmptyBundleName_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IPreloadUIExtensionExecuteCallback::GetDescriptor());
    
    int32_t extensionAbilityId = 200;
    data.WriteInt32(extensionAbilityId);
    
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE,
        data, reply, option);
    
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(stub_->IsOnLoadedDoneCalled());
    EXPECT_EQ(stub_->GetExtensionAbilityId(), extensionAbilityId);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: HandleOnDestroyDone_0100
 * @tc.desc: Test HandleOnDestroyDone with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, HandleOnDestroyDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IPreloadUIExtensionExecuteCallback::GetDescriptor());
    
    int32_t extensionAbilityId = 300;
    data.WriteInt32(extensionAbilityId);
    
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_DESTROY_DONE, data, reply, option);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(stub_->IsOnDestroyDoneCalled());
    EXPECT_EQ(stub_->GetExtensionAbilityId(), extensionAbilityId);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: OnRemoteRequest_TokenMismatch_0100
 * @tc.desc: Test InterfaceToken mismatch.
 *           Expected: ERR_INVALID_STATE
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, OnRemoteRequest_TokenMismatch_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequest_TokenMismatch_0100 start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(u"Invalid.Token.Descriptor");
    
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE, data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequest_TokenMismatch_0100 end");
}

/**
 * @tc.name: OnRemoteRequest_UidRejected_0100
 * @tc.desc: Test OnRemoteRequest with non-foundation calling uid.
 *           Expected: CHECK_PERMISSION_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, OnRemoteRequest_UidRejected_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequest_UidRejected_0100 start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IPreloadUIExtensionExecuteCallback::GetDescriptor());
    data.WriteInt32(100);

    IPCSkeleton::SetCallingUid(0); // simulate a non-foundation caller
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE, data, reply, option);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    EXPECT_FALSE(stub_->IsOnLoadedDoneCalled());
    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequest_UidRejected_0100 end");
}

/**
 * @tc.name: HandleOnPreloadSuccess_0100
 * @tc.desc: Test ON_PRELOAD_UI_EXTENSION_ABILITY_SUCCESS branch and HandleOnPreloadSuccess impl.
 *           Expected: ERR_OK and callback invoked.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, HandleOnPreloadSuccess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleOnPreloadSuccess_0100 start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPreloadUIExtensionExecuteCallback::GetDescriptor());
    
    int32_t requestCode = 10;
    int32_t extensionAbilityId = 500;
    int32_t innerErrCode = 0;

    data.WriteInt32(requestCode);
    data.WriteInt32(extensionAbilityId);
    data.WriteInt32(innerErrCode);
    
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_SUCCESS,
        data, reply, option);
    
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "HandleOnPreloadSuccess_0100 end");
}

/**
 * @tc.number: PreloadUIExtProxy_OnLoadedDone_0100
 * @tc.name: PreloadUIExtProxy_OnLoadedDone
 * @tc.desc: Test OnLoadedDone with SendRequest success.
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, PreloadUIExtProxy_OnLoadedDone_0100,
    Function | MediumTest | Level1)
{
    sptr<MockRemoteObject> remote = new (std::nothrow) MockRemoteObject();
    ASSERT_NE(remote, nullptr);
    TestPreloadUIExtensionExecuteCallbackProxy proxy(remote);

    EXPECT_CALL(*remote, SendRequest(IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE,
        _, _, _))
        .WillOnce(Invoke([](uint32_t, MessageParcel &data, MessageParcel &, MessageOption &) {
            auto token = data.ReadInterfaceToken();
            EXPECT_EQ(token, IPreloadUIExtensionExecuteCallback::GetDescriptor());
            int32_t extensionAbilityId = -1;
            EXPECT_TRUE(data.ReadInt32(extensionAbilityId));
            EXPECT_EQ(extensionAbilityId, 101);
            return ERR_OK;
        }));

    proxy.OnLoadedDone(101);
}

/**
 * @tc.number: PreloadUIExtProxy_OnLoadedDone_0200
 * @tc.name: PreloadUIExtProxy_OnLoadedDone
 * @tc.desc: Test OnLoadedDone with SendRequest error.
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, PreloadUIExtProxy_OnLoadedDone_0200,
    Function | MediumTest | Level1)
{
    sptr<MockRemoteObject> remote = new (std::nothrow) MockRemoteObject();
    ASSERT_NE(remote, nullptr);
    TestPreloadUIExtensionExecuteCallbackProxy proxy(remote);

    EXPECT_CALL(*remote, SendRequest(IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE,
        _, _, _))
        .WillOnce(Invoke([](uint32_t, MessageParcel &data, MessageParcel &, MessageOption &) {
            auto token = data.ReadInterfaceToken();
            EXPECT_EQ(token, IPreloadUIExtensionExecuteCallback::GetDescriptor());
            int32_t extensionAbilityId = -1;
            EXPECT_TRUE(data.ReadInt32(extensionAbilityId));
            EXPECT_EQ(extensionAbilityId, 202);
            return ERR_INVALID_STATE;
        }));

    proxy.OnLoadedDone(202);
}

/**
 * @tc.number: PreloadUIExtProxy_OnDestroyDone_0300
 * @tc.name: PreloadUIExtProxy_OnDestroyDone
 * @tc.desc: Test OnDestroyDone with SendRequest success.
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, PreloadUIExtProxy_OnDestroyDone_0300,
    Function | MediumTest | Level1)
{
    sptr<MockRemoteObject> remote = new (std::nothrow) MockRemoteObject();
    ASSERT_NE(remote, nullptr);
    TestPreloadUIExtensionExecuteCallbackProxy proxy(remote);

    EXPECT_CALL(*remote, SendRequest(IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_DESTROY_DONE,
        _, _, _))
        .WillOnce(Invoke([](uint32_t, MessageParcel &data, MessageParcel &, MessageOption &) {
            auto token = data.ReadInterfaceToken();
            EXPECT_EQ(token, IPreloadUIExtensionExecuteCallback::GetDescriptor());
            int32_t extensionAbilityId = -1;
            EXPECT_TRUE(data.ReadInt32(extensionAbilityId));
            EXPECT_EQ(extensionAbilityId, 303);
            return ERR_OK;
        }));

    proxy.OnDestroyDone(303);
}

/**
 * @tc.number: PreloadUIExtProxy_OnDestroyDone_0400
 * @tc.name: PreloadUIExtProxy_OnDestroyDone
 * @tc.desc: Test OnDestroyDone with SendRequest error.
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, PreloadUIExtProxy_OnDestroyDone_0400,
    Function | MediumTest | Level1)
{
    sptr<MockRemoteObject> remote = new (std::nothrow) MockRemoteObject();
    ASSERT_NE(remote, nullptr);
    TestPreloadUIExtensionExecuteCallbackProxy proxy(remote);

    EXPECT_CALL(*remote, SendRequest(IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_DESTROY_DONE,
        _, _, _))
        .WillOnce(Invoke([](uint32_t, MessageParcel &data, MessageParcel &, MessageOption &) {
            auto token = data.ReadInterfaceToken();
            EXPECT_EQ(token, IPreloadUIExtensionExecuteCallback::GetDescriptor());
            int32_t extensionAbilityId = -1;
            EXPECT_TRUE(data.ReadInt32(extensionAbilityId));
            EXPECT_EQ(extensionAbilityId, 404);
            return ERR_INVALID_STATE;
        }));

    proxy.OnDestroyDone(404);
}

/**
 * @tc.number: PreloadUIExtProxy_OnPreloadSuccess_0500
 * @tc.name: PreloadUIExtProxy_OnPreloadSuccess
 * @tc.desc: Test OnPreloadSuccess with SendRequest success.
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, PreloadUIExtProxy_OnPreloadSuccess_0500,
    Function | MediumTest | Level1)
{
    sptr<MockRemoteObject> remote = new (std::nothrow) MockRemoteObject();
    ASSERT_NE(remote, nullptr);
    TestPreloadUIExtensionExecuteCallbackProxy proxy(remote);

    EXPECT_CALL(*remote, SendRequest(IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_SUCCESS,
        _, _, _))
        .WillOnce(Invoke([](uint32_t, MessageParcel &data, MessageParcel &, MessageOption &) {
            auto token = data.ReadInterfaceToken();
            EXPECT_EQ(token, IPreloadUIExtensionExecuteCallback::GetDescriptor());
            int32_t requestCode = -1;
            int32_t extensionAbilityId = -1;
            int32_t innerErrCode = -1;
            EXPECT_TRUE(data.ReadInt32(requestCode));
            EXPECT_TRUE(data.ReadInt32(extensionAbilityId));
            EXPECT_TRUE(data.ReadInt32(innerErrCode));
            EXPECT_EQ(requestCode, 1);
            EXPECT_EQ(extensionAbilityId, 2);
            EXPECT_EQ(innerErrCode, 3);
            return ERR_OK;
        }));

    proxy.OnPreloadSuccess(1, 2, 3);
}

/**
 * @tc.number: PreloadUIExtProxy_OnPreloadSuccess_0600
 * @tc.name: PreloadUIExtProxy_OnPreloadSuccess
 * @tc.desc: Test OnPreloadSuccess with SendRequest error.
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, PreloadUIExtProxy_OnPreloadSuccess_0600,
    Function | MediumTest | Level1)
{
    sptr<MockRemoteObject> remote = new (std::nothrow) MockRemoteObject();
    ASSERT_NE(remote, nullptr);
    TestPreloadUIExtensionExecuteCallbackProxy proxy(remote);

    EXPECT_CALL(*remote, SendRequest(IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_SUCCESS,
        _, _, _))
        .WillOnce(Invoke([](uint32_t, MessageParcel &data, MessageParcel &, MessageOption &) {
            auto token = data.ReadInterfaceToken();
            EXPECT_EQ(token, IPreloadUIExtensionExecuteCallback::GetDescriptor());
            int32_t requestCode = -1;
            int32_t extensionAbilityId = -1;
            int32_t innerErrCode = -1;
            EXPECT_TRUE(data.ReadInt32(requestCode));
            EXPECT_TRUE(data.ReadInt32(extensionAbilityId));
            EXPECT_TRUE(data.ReadInt32(innerErrCode));
            EXPECT_EQ(requestCode, 10);
            EXPECT_EQ(extensionAbilityId, 20);
            EXPECT_EQ(innerErrCode, 30);
            return ERR_INVALID_STATE;
        }));

    proxy.OnPreloadSuccess(10, 20, 30);
}

/**
 * @tc.number: PreloadUIExtProxy_NullRemote_0700
 * @tc.name: PreloadUIExtProxy_NullRemote
 * @tc.desc: Test OnLoadedDone with null remote.
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, PreloadUIExtProxy_NullRemote_0700,
    Function | MediumTest | Level1)
{
    TestPreloadUIExtensionExecuteCallbackProxy proxy(nullptr);
    EXPECT_EQ(proxy.GetRemoteObject(), nullptr);
    proxy.OnLoadedDone(1);
}

/**
 * @tc.number: PreloadUIExtProxy_NullRemote_0800
 * @tc.name: PreloadUIExtProxy_NullRemote
 * @tc.desc: Test OnDestroyDone with null remote.
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, PreloadUIExtProxy_NullRemote_0800,
    Function | MediumTest | Level1)
{
    TestPreloadUIExtensionExecuteCallbackProxy proxy(nullptr);
    EXPECT_EQ(proxy.GetRemoteObject(), nullptr);
    proxy.OnDestroyDone(1);
}

/**
 * @tc.number: PreloadUIExtProxy_NullRemote_0900
 * @tc.name: PreloadUIExtProxy_NullRemote
 * @tc.desc: Test OnPreloadSuccess with null remote.
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, PreloadUIExtProxy_NullRemote_0900,
    Function | MediumTest | Level1)
{
    TestPreloadUIExtensionExecuteCallbackProxy proxy(nullptr);
    EXPECT_EQ(proxy.GetRemoteObject(), nullptr);
    proxy.OnPreloadSuccess(1, 2, 3);
}
} // namespace AAFwk
} // namespace OHOS