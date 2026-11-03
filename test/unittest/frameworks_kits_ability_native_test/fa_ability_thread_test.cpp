/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <functional>
#include <gtest/gtest.h>
#define private public
#define protected public
#include "ability_loader.h"
#include "fa_ability_thread.h"
#undef private
#undef protected
#include "ability.h"
#include "ability_impl.h"
#include "ability_impl_factory.h"
#include "context_deal.h"
#include "mock_ability_impl.h"
#include "mock_ability_thread.h"
#include "mock_ability_token.h"
#include "mock_data_ability.h"
#include "mock_data_obs_mgr_stub.h"
#include "mock_page_ability.h"
#include "mock_service_ability.h"
#include "ohos_application.h"
#include "page_ability_impl.h"
#include "uri.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

REGISTER_AA(MockDataAbility)
REGISTER_AA(MockPageAbility)
REGISTER_AA(MockServiceAbility)
static const int32_t STARTID = 0;
static const int32_t ASSERT_NUM = -1;
static const std::string DEVICE_ID = "deviceId";
static const std::string TEST = "test";

class FaAbilityThreadTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void FaAbilityThreadTest::SetUpTestCase(void)
{}

void FaAbilityThreadTest::TearDownTestCase(void)
{}

void FaAbilityThreadTest::SetUp(void)
{}

void FaAbilityThreadTest::TearDown(void)
{}

/**
 * @tc.number: AaFwk_AbilityThread_DumpAbilityInfoInner_0200
 * @tc.name: DumpAbilityInfoInner
 * @tc.desc: Test DumpAbilityInfoInner function when currentAbility_ is nullptr
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_DumpAbilityInfoInner_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpAbilityInfoInner_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> params;
    std::vector<std::string> info;

    EXPECT_EQ(abilitythread->currentAbility_, nullptr);
    abilitythread->currentExtension_ = std::make_shared<AbilityRuntime::Extension>();
    EXPECT_NE(abilitythread->currentExtension_, nullptr);
    abilitythread->DumpAbilityInfoInner(params, info);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpAbilityInfoInner_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_DumpAbilityInfoInner_0300
 * @tc.name: DumpAbilityInfoInner
 * @tc.desc: Test DumpAbilityInfoInner function when currentExtension_ is nullptr
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_DumpAbilityInfoInner_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpAbilityInfoInner_0300 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> params;
    std::vector<std::string> info;

    abilitythread->currentAbility_ = std::make_shared<Ability>();
    EXPECT_NE(abilitythread->currentAbility_, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    EXPECT_EQ(abilitythread->currentExtension_, nullptr);
    abilitythread->DumpAbilityInfoInner(params, info);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpAbilityInfoInner_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_DumpOtherInfo_0100
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when abilityHandler_ and currentAbility_ is not nullptr
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_DumpOtherInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpOtherInfo_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockPageAbility";
    abilityInfo->type = AbilityType::PAGE;
    auto setRunner = EventRunner::Create(abilityInfo->name);
    abilitythread->abilityHandler_->SetEventRunner(setRunner);
    auto getRunner = abilitythread->abilityHandler_->GetEventRunner();
    EXPECT_NE(getRunner, nullptr);

    std::vector<std::string> info;
    abilitythread->currentAbility_ = std::make_shared<Ability>();
    EXPECT_NE(abilitythread->currentAbility_, nullptr);
    abilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpOtherInfo_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_DumpOtherInfo_0200
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when abilityHandler_ is nullptr
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_DumpOtherInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpOtherInfo_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> info;
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    abilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpOtherInfo_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_DumpOtherInfo_0300
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when currentAbility_ is nullptr
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_DumpOtherInfo_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpOtherInfo_0300 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> info;
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    EXPECT_EQ(abilitythread->currentAbility_, nullptr);
    abilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpOtherInfo_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_CallRequest_0100
 * @tc.name: CallRequest
 * @tc.desc: Test CallRequest function when abilityHandler_ and currentAbility_ is not nullptr
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_CallRequest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CallRequest_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> info;
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    abilitythread->currentAbility_ = std::make_shared<Ability>();
    EXPECT_NE(abilitythread->currentAbility_, nullptr);
    abilitythread->CallRequest();
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CallRequest_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_CallRequest_0200
 * @tc.name: CallRequest
 * @tc.desc: Test CallRequest function when abilityHandler_ and currentAbility_ is not nullptr
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_CallRequest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CallRequest_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> info;
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    EXPECT_EQ(abilitythread->currentAbility_, nullptr);
    abilitythread->CallRequest();
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CallRequest_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_CreateAbilityName_0100
 * @tc.name: CreateAbilityName
 * @tc.desc: Test CreateAbilityName function when parameters are application and abilityRecord
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_CreateAbilityName_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CreateAbilityName_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockPageAbility";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);

    std::string abilityName = abilitythread->CreateAbilityName(abilityRecord, nullptr);
    EXPECT_EQ(abilityName, "");
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CreateAbilityName_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_CreateAbilityName_0200
 * @tc.name: CreateAbilityName
 * @tc.desc: Test CreateAbilityName function when parameters are application and abilityRecord
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_CreateAbilityName_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CreateAbilityName_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::string abilityName = abilitythread->CreateAbilityName(nullptr, application);
    EXPECT_EQ(abilityName, "");
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CreateAbilityName_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_CreateExtensionAbilityName_0101
 * @tc.name: CreateExtensionAbilityName
 * @tc.desc: Test CreateExtensionAbilityName function when parameters are application and abilityRecord
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_CreateExtensionAbilityName_0101, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CreateExtensionAbilityName_0101 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::string abilityName = "MockPageAbility";
    abilitythread->CreateExtensionAbilityName(application, abilityInfo, abilityName);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::string ret = abilitythread->CreateAbilityName(abilityRecord, application);
    EXPECT_EQ(abilityName, "MockPageAbility");
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CreateExtensionAbilityName_0101 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_CreateExtensionAbilityName_0200
 * @tc.name: CreateExtensionAbilityName
 * @tc.desc: Test CreateExtensionAbilityName function when parameters are application and abilityRecord
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_CreateExtensionAbilityName_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CreateExtensionAbilityName_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::STATICSUBSCRIBER;
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::string abilityName = "";
    abilitythread->CreateExtensionAbilityName(application, abilityInfo, abilityName);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::string ret = abilitythread->CreateAbilityName(abilityRecord, application);
    EXPECT_EQ(abilityName, "ServiceExtension");
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CreateExtensionAbilityName_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_AttachExtension_0300
 * @tc.name: AttachExtension
 * @tc.desc: Test AttachExtension function when application is null
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_AttachExtension_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_AttachExtension_0300 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockPageAbility";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);

    std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);

    std::string abilityName = abilitythread->CreateAbilityName(abilityRecord, nullptr);
    auto extension = AbilityLoader::GetInstance().GetExtensionByName(abilityName,
        OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0);
    EXPECT_EQ(extension, nullptr);

    abilitythread->AttachExtension(nullptr, abilityRecord, mainRunner);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_AttachExtension_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_AttachExtension_0400
 * @tc.name: AttachExtension
 * @tc.desc: Test AttachExtension function when abilityRecord is null
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_AttachExtension_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_AttachExtension_0400 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockPageAbility";
    std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);

    std::string abilityName = abilitythread->CreateAbilityName(nullptr, application);
    auto extension = AbilityLoader::GetInstance().GetExtensionByName(abilityName,
        OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0);
    EXPECT_EQ(extension, nullptr);

    abilitythread->AttachExtension(application, nullptr, mainRunner);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_AttachExtension_0400 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_AttachExtension_0500
 * @tc.name: AttachExtension
 * @tc.desc: Test AttachExtension function when mainRunner is null
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_AttachExtension_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_AttachExtension_0500 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockPageAbility";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);

    std::string abilityName = abilitythread->CreateAbilityName(abilityRecord, application);
    auto extension = AbilityLoader::GetInstance().GetExtensionByName(abilityName,
        OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0);
    EXPECT_EQ(extension, nullptr);

    abilitythread->AttachExtension(application, abilityRecord, nullptr);
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_AttachExtension_0500 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_CreateAndInitContextDeal_0500
 * @tc.name: CreateAndInitContextDeal
 * @tc.desc: Test CreateAndInitContextDeal function when application is null
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_CreateAndInitContextDeal_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CreateAndInitContextDeal_0500 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockPageAbility";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);

    std::string abilityName = abilitythread->CreateAbilityName(abilityRecord, application);
    auto extension = AbilityLoader::GetInstance().GetExtensionByName(abilityName,
        OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0);
    EXPECT_EQ(extension, nullptr);

    auto ret = abilitythread->CreateAndInitContextDeal(application, abilityRecord, nullptr);
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CreateAndInitContextDeal_0500 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_CreateAndInitContextDeal_0600
 * @tc.name: CreateAndInitContextDeal
 * @tc.desc: Test CreateAndInitContextDeal function when application is null
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_CreateAndInitContextDeal_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CreateAndInitContextDeal_0600 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockPageAbility";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);

    std::shared_ptr<AppExecFwk::AbilityContext> abilityObject = std::make_shared<AppExecFwk::AbilityContext>();
    std::string abilityName = abilitythread->CreateAbilityName(abilityRecord, nullptr);
    auto extension = AbilityLoader::GetInstance().GetExtensionByName(abilityName,
        OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0);
    EXPECT_EQ(extension, nullptr);

    auto ret = abilitythread->CreateAndInitContextDeal(nullptr, abilityRecord, abilityObject);
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CreateAndInitContextDeal_0600 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_InitExtensionFlag_0200
 * @tc.name: InitExtensionFlag
 * @tc.desc: Test InitExtensionFlag function when isUIAbility_ is true
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_InitExtensionFlag_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_InitExtensionFlag_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockPageAbility";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);

    std::string abilityName = abilitythread->CreateAbilityName(abilityRecord, application);
    auto extension = AbilityLoader::GetInstance().GetExtensionByName(abilityName,
        OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0);
    EXPECT_EQ(extension, nullptr);
    abilitythread->InitExtensionFlag(abilityRecord);

    uint32_t state = AAFwk::ABILITY_STATE_FOREGROUND_NEW;
    std::string methodName = "methodName";
    abilitythread->AddLifecycleEvent(state, methodName);
    EXPECT_EQ(abilitythread->isUIAbility_, true);

    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_InitExtensionFlag_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_InitExtensionFlag_0300
 * @tc.name: InitExtensionFlag
 * @tc.desc: Test InitExtensionFlag function when isUIAbility_ is true
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_InitExtensionFlag_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_InitExtensionFlag_0300 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockPageAbility";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);

    std::string abilityName = abilitythread->CreateAbilityName(abilityRecord, application);
    auto extension = AbilityLoader::GetInstance().GetExtensionByName(abilityName,
        OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0);
    EXPECT_EQ(extension, nullptr);
    abilitythread->InitExtensionFlag(abilityRecord);

    uint32_t state = AAFwk::ABILITY_STATE_BACKGROUND_NEW;
    std::string methodName = "methodName";
    abilitythread->AddLifecycleEvent(state, methodName);
    EXPECT_EQ(abilitythread->isUIAbility_, true);

    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_InitExtensionFlag_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_HandleShareData_0100
 * @tc.name: HandleShareData
 * @tc.desc: Test HandleShareData function when abilityImpl_ is null
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_HandleShareData_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleShareData_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    int32_t uniqueId = 1;
    abilitythread->HandleShareData(uniqueId);
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);

    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleShareData_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_HandleDisconnectExtension_0100
 * @tc.name: HandleDisconnectExtension
 * @tc.desc: Test HandleDisconnectExtension function when extensionImpl_ is null
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_HandleDisconnectExtension_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleDisconnectExtension_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Want want;
    abilitythread->extensionImpl_ = nullptr;
    abilitythread->HandleDisconnectExtension(want);

    EXPECT_EQ(abilitythread->token_, nullptr);
    int32_t uniqueId = 1;
    abilitythread->ScheduleShareData(uniqueId);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleDisconnectExtension_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_HandleDisconnectExtension_0200
 * @tc.name: HandleDisconnectExtension
 * @tc.desc: Test HandleDisconnectExtension function when extensionImpl_ is null
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_HandleDisconnectExtension_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleDisconnectExtension_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Want want;
    abilitythread->extensionImpl_ = nullptr;
    abilitythread->HandleDisconnectExtension(want);

    abilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilitythread->token_, nullptr);
    int32_t uniqueId = 1;
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    abilitythread->ScheduleShareData(uniqueId);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleDisconnectExtension_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_HandleDisconnectExtension_0300
 * @tc.name: HandleDisconnectExtension
 * @tc.desc: Test HandleDisconnectExtension function when extensionImpl_ is null
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_HandleDisconnectExtension_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleDisconnectExtension_0300 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Want want;
    abilitythread->extensionImpl_ = nullptr;
    abilitythread->HandleDisconnectExtension(want);

    abilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilitythread->token_, nullptr);
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    int32_t uniqueId = 1;
    abilitythread->ScheduleShareData(uniqueId);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleDisconnectExtension_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_SchedulePrepareTerminateAbility_0100
 * @tc.name: SchedulePrepareTerminateAbility
 * @tc.desc: Test SchedulePrepareTerminateAbility function when extensionImpl_ is null
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_SchedulePrepareTerminateAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_SchedulePrepareTerminateAbility_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    abilitythread->abilityImpl_ = nullptr;
    bool ret = abilitythread->SchedulePrepareTerminateAbility();
    abilitythread->HandlePrepareTerminateAbility();
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_SchedulePrepareTerminateAbility_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_SchedulePrepareTerminateAbility_0200
 * @tc.name: SchedulePrepareTerminateAbility
 * @tc.desc: Test SchedulePrepareTerminateAbility function when extensionImpl_ is null
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_SchedulePrepareTerminateAbility_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_SchedulePrepareTerminateAbility_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    bool ret = abilitythread->SchedulePrepareTerminateAbility();
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_SchedulePrepareTerminateAbility_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_SchedulePrepareTerminateAbility_0300
 * @tc.name: SchedulePrepareTerminateAbility
 * @tc.desc: Test SchedulePrepareTerminateAbility function when extensionImpl_ is null
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_SchedulePrepareTerminateAbility_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_SchedulePrepareTerminateAbility_0300 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->abilityHandler_ = nullptr;
    bool ret = abilitythread->SchedulePrepareTerminateAbility();
    abilitythread->HandlePrepareTerminateAbility();
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_SchedulePrepareTerminateAbility_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_DumpAbilityInfoInner_NullImpl_0100
 * @tc.name: DumpAbilityInfoInner with null abilityImpl_ skips ability dump
 * @tc.desc: Test DumpAbilityInfoInner does not crash when abilityImpl_ is null.
 *           abilityImpl_==null means JsAbility::Init was never called.
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_DumpAbilityInfoInner_NullImpl_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpAbilityInfoInner_NullImpl_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    ASSERT_NE(abilitythread, nullptr);

    abilitythread->currentAbility_ = std::make_shared<Ability>();
    ASSERT_NE(abilitythread->currentAbility_, nullptr);
    abilitythread->abilityImpl_ = nullptr;
    abilitythread->abilityHandler_ = nullptr;

    std::vector<std::string> params;
    std::vector<std::string> info;
    // Should not crash; should skip ability dump and proceed to DumpOtherInfo
    abilitythread->DumpAbilityInfoInner(params, info);
    SUCCEED();

    delete abilitythread;
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpAbilityInfoInner_NullImpl_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_PrepareTerminate_NullImpl_0100
 * @tc.name: HandlePrepareTerminateAbility with null abilityImpl_ triggers notify
 * @tc.desc: Test HandlePrepareTerminateAbility notifies waiting thread when abilityImpl_ is null
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_PrepareTerminate_NullImpl_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_PrepareTerminate_NullImpl_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    ASSERT_NE(abilitythread, nullptr);

    abilitythread->abilityImpl_ = nullptr;
    abilitythread->isPrepareTerminateAbilityDone_.store(false);

    abilitythread->HandlePrepareTerminateAbility();

    EXPECT_TRUE(abilitythread->isPrepareTerminateAbilityDone_.load());

    delete abilitythread;
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_PrepareTerminate_NullImpl_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_CreateContextDeal_NullInfo_0100
 * @tc.name: CreateAndInitContextDeal with null abilityInfo
 * @tc.desc: Test CreateAndInitContextDeal handles null GetAbilityInfo without crash
 */
HWTEST_F(FaAbilityThreadTest, AaFwk_AbilityThread_CreateContextDeal_NullInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CreateContextDeal_NullInfo_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    ASSERT_NE(abilitythread, nullptr);

    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    auto abilityRecord = std::make_shared<AppExecFwk::AbilityLocalRecord>(nullptr, nullptr, nullptr, -1);
    auto abilityObject = std::make_shared<AppExecFwk::AbilityContext>();

    auto contextDeal = abilitythread->CreateAndInitContextDeal(application, abilityRecord, abilityObject);
    EXPECT_NE(contextDeal, nullptr);

    delete abilitythread;
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CreateContextDeal_NullInfo_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS