/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "accesstoken_kit.h"
#include "appexecfwk_errors.h"
#include "form_constants.h"
#include "form_mgr_errors.h"
#define private public
#include "form_runtime/form_extension_provider_client.h"
#include "form_runtime/js_form_extension.h"
#undef private
#include "form_supply_stub.h"
#include "mock_form_supply_callback.h"
#include "runtime.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Security;
using testing::_;
using testing::Return;

const std::string FORM_MANAGER_SERVICE_BUNDLE_NAME = "com.form.fms.app";
const std::string FORM_SUPPLY_INFO = "com.form.supply.info.test";

class FormSupplyCallbackMock : public FormSupplyStub {
public:
    FormSupplyCallbackMock() = default;
    virtual ~FormSupplyCallbackMock() = default;
    MOCK_METHOD2(OnAcquire, int(const FormProviderInfo& formInfo, const Want& want));
    MOCK_METHOD1(OnEventHandle, int(const Want& want));
    MOCK_METHOD4(OnAcquireStateResult, int(FormState state, const std::string& provider, const Want& wantArg,
        const Want& want));
    MOCK_METHOD5(OnShareAcquire, void(int64_t formId, const std::string& remoteDeviceId,
        const AAFwk::WantParams& wantParams, int64_t requestCode, const bool& result));
    MOCK_METHOD2(OnRenderTaskDone, int32_t(int64_t formId, const Want &want));
    MOCK_METHOD2(OnStopRenderingTaskDone, int32_t(int64_t formId, const Want &want));
    MOCK_METHOD2(OnAcquireDataResult, int32_t(const AAFwk::WantParams &wantParams, int64_t requestCode));
};

class FormExtensionProviderClientTest : public testing::Test {
public:
    FormExtensionProviderClientTest()
    {}
    ~FormExtensionProviderClientTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FormExtensionProviderClientTest::SetUpTestCase(void)
{}

void FormExtensionProviderClientTest::TearDownTestCase(void)
{}

void FormExtensionProviderClientTest::SetUp(void)
{}

void FormExtensionProviderClientTest::TearDown(void)
{}

/**
 * @tc.number: formExtensionProviderClient_0100
 * @tc.name: NotifyFormExtensionDelete
 * @tc.desc: Test NotifyFormExtensionDelete function with hostToken is nullptr.
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0100 start";

    Want want;
    // don't set Constants::PARAM_FORM_HOST_TOKEN then hostToken is nullptr.
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 103)
        .SetParam(Constants::FORM_CONNECT_ID, 103L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 103L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    int64_t formId = 723L;
    const sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AbilityRuntime::FormExtensionProviderClient> formExtensionProviderClient =
        std::make_shared<AbilityRuntime::FormExtensionProviderClient>();
    ASSERT_NE(formExtensionProviderClient, nullptr);
    formExtensionProviderClient->NotifyFormExtensionDelete(formId, want, callerToken);

    GTEST_LOG_(INFO) << "formExtensionProviderClient_0100 end";
}

/**
 * @tc.number: formExtensionProviderClient_0200
 * @tc.name: NotifyFormExtensionDelete
 * @tc.desc: Test NotifyFormExtensionDelete function with hostToken is not nullptr.
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0200 start";

    // callerToken
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    Want want;
    // set Constants::PARAM_FORM_HOST_TOKEN hostToken pointer can be obtained.
    want.SetParam(Constants::PARAM_FORM_HOST_TOKEN, callerToken);

    int64_t formId = 723L;
    std::shared_ptr<AbilityRuntime::FormExtensionProviderClient> formExtensionProviderClient =
        std::make_shared<AbilityRuntime::FormExtensionProviderClient>();
    ASSERT_NE(formExtensionProviderClient, nullptr);
    formExtensionProviderClient->NotifyFormExtensionDelete(formId, want, callerToken);

    GTEST_LOG_(INFO) << "formExtensionProviderClient_0200 end";
}

/**
 * @tc.number: formExtensionProviderClient_1900
 * @tc.name: NotifyFormExtensionDelete
 * @tc.desc: Test NotifyFormExtensionDelete function with hostToken is not nullptr.
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_1900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1900 start";

    // callerToken
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    Want want;
    // set Constants::PARAM_FORM_HOST_TOKEN hostToken pointer can be obtained.
    want.SetParam(Constants::PARAM_FORM_HOST_TOKEN, callerToken);

    int64_t formId = 723L;
    std::shared_ptr<AbilityRuntime::FormExtensionProviderClient> formExtensionProviderClient =
        std::make_shared<AbilityRuntime::FormExtensionProviderClient>();
    ASSERT_NE(formExtensionProviderClient, nullptr);

    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension(AbilityRuntime::JsFormExtension::Create(runtime));
    formExtensionProviderClient->SetOwner(formExtension);

    auto extension = formExtensionProviderClient->GetOwner();
    EXPECT_TRUE(formExtension == extension);

    formExtensionProviderClient->NotifyFormExtensionDelete(formId, want, callerToken);

    GTEST_LOG_(INFO) << "formExtensionProviderClient_1900 end";
}

/**
 * @tc.number: formExtensionProviderClient_0300
 * @tc.name: NotifyFormExtensionUpdate
 * @tc.desc: Test NotifyFormExtensionUpdate function with HasParameter(Constants::FORM_CONNECT_ID) is true .
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0300 start";

    Want want;
    // set Constants::FORM_CONNECT_ID
    want.SetParam(Constants::FORM_CONNECT_ID, 103L);

    int64_t formId = 723L;
    const sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AbilityRuntime::FormExtensionProviderClient> formExtensionProviderClient =
        std::make_shared<AbilityRuntime::FormExtensionProviderClient>();
    ASSERT_NE(formExtensionProviderClient, nullptr);
    formExtensionProviderClient->NotifyFormExtensionUpdate(formId, want, callerToken);

    GTEST_LOG_(INFO) << "formExtensionProviderClient_0300 end";
}

/**
 * @tc.number: formExtensionProviderClient_0400
 * @tc.name: NotifyFormExtensionUpdate
 * @tc.desc: Test NotifyFormExtensionUpdate function with HasParameter(Constants::FORM_CONNECT_ID) is false .
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0400 start";

    Want want;
    // don't set Constants::FORM_CONNECT_ID
    want.SetParam(Constants::ACQUIRE_TYPE, 103L);

    int64_t formId = 723L;
    const sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AbilityRuntime::FormExtensionProviderClient> formExtensionProviderClient =
        std::make_shared<AbilityRuntime::FormExtensionProviderClient>();
    ASSERT_NE(formExtensionProviderClient, nullptr);
    formExtensionProviderClient->NotifyFormExtensionUpdate(formId, want, callerToken);

    GTEST_LOG_(INFO) << "formExtensionProviderClient_0400 end";
}

/**
 * @tc.number: formExtensionProviderClient_0500
 * @tc.name: FireFormExtensionEvent
 * @tc.desc: Test FireFormExtensionEvent function with HasParameter(Constants::FORM_CONNECT_ID) is false .
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0500 start";

    Want want;
    // don't set Constants::FORM_CONNECT_ID
    want.SetParam(Constants::ACQUIRE_TYPE, 103L);

    int64_t formId = 723L;
    std::string message = "event message";
    const sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AbilityRuntime::FormExtensionProviderClient> formExtensionProviderClient =
        std::make_shared<AbilityRuntime::FormExtensionProviderClient>();
    ASSERT_NE(formExtensionProviderClient, nullptr);
    formExtensionProviderClient->FireFormExtensionEvent(formId, message, want, callerToken);

    GTEST_LOG_(INFO) << "formExtensionProviderClient_0500 end";
}

/**
 * @tc.number: formExtensionProviderClient_0600
 * @tc.name: FireFormExtensionEvent
 * @tc.desc: Test FireFormExtensionEvent function with HasParameter(Constants::FORM_CONNECT_ID) is true .
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0600 start";

    Want want;
    // set Constants::FORM_CONNECT_ID
    want.SetParam(Constants::FORM_CONNECT_ID, 103L);

    int64_t formId = 723L;
    std::string message = "event message";
    const sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AbilityRuntime::FormExtensionProviderClient> formExtensionProviderClient =
        std::make_shared<AbilityRuntime::FormExtensionProviderClient>();
    ASSERT_NE(formExtensionProviderClient, nullptr);
    formExtensionProviderClient->FireFormExtensionEvent(formId, message, want, callerToken);

    GTEST_LOG_(INFO) << "formExtensionProviderClient_0600 end";
}

/**
 * @tc.number: formExtensionProviderClient_0700
 * @tc.name: AcquireProviderFormInfo
 * @tc.desc: callerToken is nullptr, failed to verify AcquireProviderFormInfo.
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0700 start";
    FormJsInfo formJsInfo;
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    auto result = formExtensionProviderClient.AcquireProviderFormInfo(formJsInfo, want, callerToken);
    EXPECT_EQ(result, ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED);
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0700 end";
}

/**
 * @tc.number: formExtensionProviderClient_0800
 * @tc.name: SetOwner and GetOwner
 * @tc.desc: FormExtension is nullptr Verify GetOwner is nullptr.
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_0800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0800 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension = nullptr;
    formExtensionProviderClient.SetOwner(formExtension);
    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(extension == nullptr);
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0800 end";
}

/**
 * @tc.number: formExtensionProviderClient_0900
 * @tc.name: SetOwner and GetOwner
 * @tc.desc: FormExtension is not nullptr Verify that GetOwner is not null.
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_0900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0900 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension(AbilityRuntime::JsFormExtension::Create(runtime));
    formExtensionProviderClient.SetOwner(formExtension);

    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(formExtension == extension);

    formExtensionProviderClient.ClearOwner(nullptr);
    extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(formExtension == extension);

    formExtensionProviderClient.ClearOwner(formExtension);
    extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(extension == nullptr);
    GTEST_LOG_(INFO) << "formExtensionProviderClient_0900 end";
}

/**
 * @tc.number: formExtensionProviderClient_1000
 * @tc.name: AcquireShareFormData
 * @tc.desc: formSupplyCallback is nullptr, failed to verify AcquireShareFormData.
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_1000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1000 start";
    int64_t formId = 0;
    std::string remoteDeviceId = "deviceId";
    sptr<IRemoteObject> formSupplyCallback = nullptr;
    int64_t requestCode = 0;
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    auto result = formExtensionProviderClient.AcquireShareFormData(
        formId, remoteDeviceId, formSupplyCallback, requestCode);
    EXPECT_EQ(result, ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY);
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1000 end";
}

/**
 * @tc.number: formExtensionProviderClient_1100
 * @tc.name: HandleResultCode
 * @tc.desc: callerToken is nullptr, failed to verify HandleResultCode.
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_1100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1100 start";
    int32_t errorCode = ERR_OK;
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    auto result = formExtensionProviderClient.HandleResultCode(errorCode, want, callerToken);
    EXPECT_EQ(result, ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED);
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1100 end";
}

/**
 * @tc.number: formExtensionProviderClient_1200
 * @tc.name: AcquireShareFormData
 * @tc.desc: callerToken is not nullptr, verify HandleResultCode success.
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_1200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1200 start";
    int32_t errorCode = ERR_OK;
    Want want;
    sptr<IRemoteObject> callerToken = new (std::nothrow) MockFormSupplyCallback;
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    auto result = formExtensionProviderClient.HandleResultCode(errorCode, want, callerToken);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1200 end";
}

/**
 * @tc.number: formExtensionProviderClient_1300
 * @tc.name: AcquireShareFormData
 * @tc.desc: errorCode is error, HandleResultCode return errorCode.
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_1300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1300 start";
    int32_t errorCode = 1;
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    auto result = formExtensionProviderClient.HandleResultCode(errorCode, want, callerToken);
    EXPECT_EQ(result, errorCode);
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1300 end";
}

/**
 * @tc.number: formExtensionProviderClient_1400
 * @tc.name: AcquireFormExtensionProviderInfo
 * @tc.desc: Successful case of verifying AcquireFormExtensionProviderInfo
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_1400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1400 start";
    sptr<FormSupplyCallbackMock> callerToken = new (std::nothrow) FormSupplyCallbackMock;
    EXPECT_CALL(*callerToken, OnAcquire(_, _)).Times(1).WillOnce(Return(0));
    FormJsInfo formJsInfo;
    Want want;
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    formExtensionProviderClient.AcquireFormExtensionProviderInfo(formJsInfo, want, callerToken->AsObject());
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1400 end";
}

/**
 * @tc.number: formExtensionProviderClient_1500
 * @tc.name: NotifyFormExtensionAcquireState
 * @tc.desc: Successful case of verifying NotifyFormExtensionAcquireState
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_1500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1500 start";
    Want wantArg;
    std::string provider;
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    formExtensionProviderClient.NotifyFormExtensionAcquireState(wantArg, provider, want, callerToken);

    sptr<FormSupplyCallbackMock> callback = new (std::nothrow) FormSupplyCallbackMock;
    EXPECT_CALL(*callback, OnAcquireStateResult(_, _, _, _)).Times(1).WillOnce(Return(0));
    formExtensionProviderClient.NotifyFormExtensionAcquireState(wantArg, provider, want, callback->AsObject());
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1500 end";
}

/**
 * @tc.number: formExtensionProviderClient_1600
 * @tc.name: AcquireFormData
 * @tc.desc: formSupplyCallback is nullptr, failed to verify AcquireFormData.
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_1600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1600 start";
    int64_t formId = 0;
    sptr<IRemoteObject> formSupplyCallback = nullptr;
    int64_t requestCode = 0;
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    auto result = formExtensionProviderClient.AcquireFormData(
        formId, formSupplyCallback, requestCode);
    EXPECT_EQ(result, ERR_APPEXECFWK_FORM_INVALID_PARAM);
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1600 end";
}

/**
 * @tc.number: formExtensionProviderClient_1700
 * @tc.name: SetOwner and GetOwner
 * @tc.desc: test AcquireFormExtensionProviderInfo when GetOwner not null.
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_1700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1700 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension(AbilityRuntime::JsFormExtension::Create(runtime));
    formExtensionProviderClient.SetOwner(formExtension);

    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(formExtension == extension);

    sptr<FormSupplyCallbackMock> callerToken = new (std::nothrow) FormSupplyCallbackMock;
    FormJsInfo formJsInfo;
    Want want;
    formExtensionProviderClient.AcquireFormExtensionProviderInfo(formJsInfo, want, callerToken->AsObject());
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1700 end";
}

/**
 * @tc.number: formExtensionProviderClient_1800
 * @tc.name: NotifyFormExtensionsDelete
 * @tc.desc: test NotifyFormExtensionsDelete.
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_1800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1800 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension(AbilityRuntime::JsFormExtension::Create(runtime));
    formExtensionProviderClient.SetOwner(formExtension);

    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(formExtension == extension);

    sptr<FormSupplyCallbackMock> callerToken = new (std::nothrow) FormSupplyCallbackMock;
    int64_t formId = 10;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);
    Want want;
    formExtensionProviderClient.NotifyFormExtensionsDelete(formIds, want, callerToken);
    GTEST_LOG_(INFO) << "formExtensionProviderClient_1800 end";
}

/**
 * @tc.number: formExtensionProviderClient_2000
 * @tc.name: NotifyFormExtensionsDelete
 * @tc.desc: test NotifyFormExtensionsDelete.
 */
HWTEST_F(FormExtensionProviderClientTest, formExtensionProviderClient_2000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "formExtensionProviderClient_2000 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension = nullptr;
    formExtensionProviderClient.SetOwner(formExtension);
    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(extension == nullptr);

    sptr<FormSupplyCallbackMock> callerToken = new (std::nothrow) FormSupplyCallbackMock;
    int64_t formId = 10;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);
    Want want;
    formExtensionProviderClient.NotifyFormExtensionsDelete(formIds, want, callerToken);
    GTEST_LOG_(INFO) << "formExtensionProviderClient_2000 end";
}

/**
 * @tc.number: NotifyFormExtensionUpdate_2100
 * @tc.name: NotifyFormExtensionUpdate
 * @tc.desc: test NotifyFormExtensionUpdate.
 */
HWTEST_F(FormExtensionProviderClientTest, NotifyFormExtensionUpdate_2100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "NotifyFormExtensionUpdate_2100 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;

    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension(AbilityRuntime::JsFormExtension::Create(runtime));
    formExtensionProviderClient.SetOwner(formExtension);

    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(formExtension == extension);

    sptr<FormSupplyCallbackMock> callerToken = new (std::nothrow) FormSupplyCallbackMock;
    int64_t formId = 10;
    Want want;
    formExtensionProviderClient.NotifyFormExtensionUpdate(formId, want, callerToken);
    GTEST_LOG_(INFO) << "NotifyFormExtensionUpdate_2100 end";
}

/**
 * @tc.number: EventNotifyExtension_2200
 * @tc.name: EventNotifyExtension
 * @tc.desc: test EventNotifyExtension.
 */
HWTEST_F(FormExtensionProviderClientTest, EventNotifyExtension_2200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "EventNotifyExtension_2200 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension(AbilityRuntime::JsFormExtension::Create(runtime));
    formExtensionProviderClient.SetOwner(formExtension);

    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(formExtension == extension);

    sptr<FormSupplyCallbackMock> callerToken = new (std::nothrow) FormSupplyCallbackMock;
    int64_t formId = 10;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);
    int32_t formVisibleType = 10;
    Want want;
    formExtensionProviderClient.EventNotifyExtension(formIds, formVisibleType, want, callerToken);
    GTEST_LOG_(INFO) << "EventNotifyExtension_2200 end";
}

/**
 * @tc.number: EventNotifyExtension_2300
 * @tc.name: EventNotifyExtension
 * @tc.desc: test EventNotifyExtension.
 */
HWTEST_F(FormExtensionProviderClientTest, EventNotifyExtension_2300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "EventNotifyExtension_2300 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension = nullptr;
    formExtensionProviderClient.SetOwner(formExtension);
    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(extension == nullptr);
    
    sptr<FormSupplyCallbackMock> callerToken = new (std::nothrow) FormSupplyCallbackMock;
    int64_t formId = 10;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);
    int32_t formVisibleType = 10;
    Want want;
    formExtensionProviderClient.EventNotifyExtension(formIds, formVisibleType, want, callerToken);
    GTEST_LOG_(INFO) << "EventNotifyExtension_2300 end";
}

/**
 * @tc.number: NotifyFormExtensionCastTempForm_2400
 * @tc.name: NotifyFormExtensionCastTempForm
 * @tc.desc: test NotifyFormExtensionCastTempForm.
 */
HWTEST_F(FormExtensionProviderClientTest, NotifyFormExtensionCastTempForm_2400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "NotifyFormExtensionCastTempForm_2400 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension(AbilityRuntime::JsFormExtension::Create(runtime));
    formExtensionProviderClient.SetOwner(formExtension);

    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(formExtension == extension);

    sptr<FormSupplyCallbackMock> callerToken = new (std::nothrow) FormSupplyCallbackMock;
    int64_t formId = 10;
    Want want;
    formExtensionProviderClient.NotifyFormExtensionCastTempForm(formId, want, callerToken);
    GTEST_LOG_(INFO) << "NotifyFormExtensionCastTempForm_2400 end";
}

/**
 * @tc.number: NotifyFormExtensionCastTempForm_2500
 * @tc.name: NotifyFormExtensionCastTempForm
 * @tc.desc: test NotifyFormExtensionCastTempForm.
 */
HWTEST_F(FormExtensionProviderClientTest, NotifyFormExtensionCastTempForm_2500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "NotifyFormExtensionCastTempForm_2500 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension = nullptr;
    formExtensionProviderClient.SetOwner(formExtension);
    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(extension == nullptr);
    
    sptr<FormSupplyCallbackMock> callerToken = new (std::nothrow) FormSupplyCallbackMock;
    int64_t formId = 10;
    Want want;
    formExtensionProviderClient.NotifyFormExtensionCastTempForm(formId, want, callerToken);
    GTEST_LOG_(INFO) << "NotifyFormExtensionCastTempForm_2500 end";
}

/**
 * @tc.number: FireFormExtensionEvent_2600
 * @tc.name: FireFormExtensionEvent
 * @tc.desc: test FireFormExtensionEvent.
 */
HWTEST_F(FormExtensionProviderClientTest, FireFormExtensionEvent_2600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "FireFormExtensionEvent_2600 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension(AbilityRuntime::JsFormExtension::Create(runtime));
    formExtensionProviderClient.SetOwner(formExtension);

    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(formExtension == extension);

    sptr<FormSupplyCallbackMock> callerToken = new (std::nothrow) FormSupplyCallbackMock;
    int64_t formId = 723L;
    std::string message = "event message";
    Want want;
    formExtensionProviderClient.FireFormExtensionEvent(formId, message, want, callerToken);
    GTEST_LOG_(INFO) << "FireFormExtensionEvent_2600 end";
}

/**
 * @tc.number: AcquireFormExtensionProviderShareFormInfo_2700
 * @tc.name: AcquireFormExtensionProviderShareFormInfo
 * @tc.desc: test AcquireFormExtensionProviderShareFormInfo.
 */
HWTEST_F(FormExtensionProviderClientTest, AcquireFormExtensionProviderShareFormInfo_2700,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AcquireFormExtensionProviderShareFormInfo_2700 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension(AbilityRuntime::JsFormExtension::Create(runtime));
    formExtensionProviderClient.SetOwner(formExtension);

    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(formExtension == extension);

    sptr<FormSupplyCallbackMock> callerToken = new (std::nothrow) FormSupplyCallbackMock;
    int64_t formId = 10;
    AAFwk::WantParams wantParam;
    bool result = formExtensionProviderClient.AcquireFormExtensionProviderShareFormInfo(formId, wantParam);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "AcquireFormExtensionProviderShareFormInfo_2700 end";
}

/**
 * @tc.number: AcquireFormExtensionProviderShareFormInfo_2800
 * @tc.name: AcquireFormExtensionProviderShareFormInfo
 * @tc.desc: test AcquireFormExtensionProviderShareFormInfo.
 */
HWTEST_F(FormExtensionProviderClientTest, AcquireFormExtensionProviderShareFormInfo_2800,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AcquireFormExtensionProviderShareFormInfo_2800 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension = nullptr;
    formExtensionProviderClient.SetOwner(formExtension);
    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(extension == nullptr);
    
    sptr<FormSupplyCallbackMock> callerToken = new (std::nothrow) FormSupplyCallbackMock;
    int64_t formId = 10;
    AAFwk::WantParams wantParam;
    bool result = formExtensionProviderClient.AcquireFormExtensionProviderShareFormInfo(formId, wantParam);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "AcquireFormExtensionProviderShareFormInfo_2800 end";
}

/**
 * @tc.number: FormExtensionProviderAcquireFormData_2900
 * @tc.name: FormExtensionProviderAcquireFormData
 * @tc.desc: test FormExtensionProviderAcquireFormData.
 */
HWTEST_F(FormExtensionProviderClientTest, FormExtensionProviderAcquireFormData_2900,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "FormExtensionProviderAcquireFormData_2900 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension = nullptr;
    formExtensionProviderClient.SetOwner(formExtension);
    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(extension == nullptr);

    int64_t formId = 10;
    AAFwk::WantParams wantParam;
    int32_t result = formExtensionProviderClient.FormExtensionProviderAcquireFormData(formId, wantParam);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "FormExtensionProviderAcquireFormData_2900 end";
}

/**
 * @tc.number: FormExtensionProviderAcquireFormData_3000
 * @tc.name: FormExtensionProviderAcquireFormData
 * @tc.desc: test FormExtensionProviderAcquireFormData.
 */
HWTEST_F(FormExtensionProviderClientTest, FormExtensionProviderAcquireFormData_3000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "FormExtensionProviderAcquireFormData_3000 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension(AbilityRuntime::JsFormExtension::Create(runtime));
    formExtensionProviderClient.SetOwner(formExtension);

    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(formExtension == extension);

    int64_t formId = 10;
    AAFwk::WantParams wantParam;
    int32_t result = formExtensionProviderClient.FormExtensionProviderAcquireFormData(formId, wantParam);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "FormExtensionProviderAcquireFormData_3000 end";
}

/**
 * @tc.number: NotifyExtensionSizeChanged_3100
 * @tc.name: NotifyExtensionSizeChanged
 * @tc.desc: test NotifyExtensionSizeChanged.
 */
HWTEST_F(FormExtensionProviderClientTest, NotifyExtensionSizeChanged_3100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "NotifyExtensionSizeChanged_3100 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension = nullptr;
    formExtensionProviderClient.SetOwner(formExtension);
    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(extension == nullptr);

    int64_t formId = 10;
    std::string newDimension = "2*2";
    AppExecFwk::Rect newRect;
    Want want;
    sptr<IRemoteObject> formSupplyCallback = nullptr;
    formExtensionProviderClient.NotifyExtensionSizeChanged(
        formId, newDimension, newRect, want, formSupplyCallback);
    GTEST_LOG_(INFO) << "NotifyExtensionSizeChanged_3100 end";
}

/**
 * @tc.number: NotifyExtensionSizeChanged_3200
 * @tc.name: NotifyExtensionSizeChanged
 * @tc.desc: test NotifyExtensionSizeChanged.
 */
HWTEST_F(FormExtensionProviderClientTest, NotifyExtensionSizeChanged_3200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "NotifyExtensionSizeChanged_3200 start";
    AbilityRuntime::FormExtensionProviderClient formExtensionProviderClient;
    
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    std::shared_ptr<AbilityRuntime::FormExtension> formExtension(AbilityRuntime::JsFormExtension::Create(runtime));
    formExtensionProviderClient.SetOwner(formExtension);

    auto extension = formExtensionProviderClient.GetOwner();
    EXPECT_TRUE(formExtension == extension);

    int64_t formId = 10;
    std::string newDimension = "2*2";
    AppExecFwk::Rect newRect;
    Want want;
    sptr<IRemoteObject> formSupplyCallback = nullptr;
    formExtensionProviderClient.NotifyExtensionSizeChanged(
        formId, newDimension, newRect, want, formSupplyCallback);
    GTEST_LOG_(INFO) << "NotifyExtensionSizeChanged_3200 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
