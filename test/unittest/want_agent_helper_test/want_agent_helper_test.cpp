/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "ability_context.h"
#include "ability_runtime_error_util.h"
#include <algorithm>
#include "completed_callback.h"
#include "context_container.h"
#include "context_impl.h"
#include "context/application_context.h"
#include "element_name.h"
#include "event_handler.h"
#include "base_types.h"
#define private public
#define protected public
#include "pending_want.h"
#include "remote_native_token.h"
#undef private
#undef protected
#include "want.h"
#include "want_agent.h"
#include "want_agent_constant.h"
#define private public
#define protected public
#include "want_agent_helper.h"
#include "want_agent_client.h"
#include "want_agent_info.h"
#undef private
#undef protected
#include "want_params.h"
#include "want_receiver_stub.h"
#include "want_sender_stub.h"
#include "bool_wrapper.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS;
using OHOS::AppExecFwk::ElementName;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;
using vector_str = std::vector<std::string>;

namespace OHOS::AbilityRuntime::WantAgent {
class WantAgentHelperTest : public testing::Test {
public:
    WantAgentHelperTest()
    {}
    ~WantAgentHelperTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    static int callBackCancelListenerConnt;
    void SetUp();
    void TearDown();
    class CancelListenerSon : public CancelListener {
    public:
        void OnCancelled(int resultCode) override;
    };
};

int WantAgentHelperTest::callBackCancelListenerConnt = 0;

void WantAgentHelperTest::CancelListenerSon::OnCancelled(int resultCode)
{
    callBackCancelListenerConnt++;
}

void WantAgentHelperTest::SetUpTestCase(void)
{
    RemoteNativeToken::SetNativeToken();
}

void WantAgentHelperTest::TearDownTestCase(void)
{}

void WantAgentHelperTest::SetUp(void)
{}

void WantAgentHelperTest::TearDown(void)
{}

/*
 * @tc.number    : WantAgentHelper_0100
 * @tc.name      : WantAgentHelper Constructors
 * @tc.desc      : 1.WantAgentConstant::Flags::ONE_TIME_FLAG
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::vector<WantAgentConstant::Flags> flags;
    flags.emplace_back(WantAgentConstant::Flags::ONE_TIME_FLAG);
    EXPECT_EQ(wantAgentHelper->FlagsTransformer(flags), static_cast<unsigned int>(0x40000000));
}

/*
 * @tc.number    : WantAgentHelper_0200
 * @tc.name      : WantAgentHelper Constructors
 * @tc.desc      : 1.WantAgentConstant::Flags::NO_BUILD_FLAG
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::vector<WantAgentConstant::Flags> flags;
    flags.emplace_back(WantAgentConstant::Flags::NO_BUILD_FLAG);
    EXPECT_EQ(wantAgentHelper->FlagsTransformer(flags), static_cast<unsigned int>(0x20000000));
}

/*
 * @tc.number    : WantAgentHelper_0300
 * @tc.name      : WantAgentHelper Constructors
 * @tc.desc      : 1.WantAgentConstant::Flags::CANCEL_PRESENT_FLAG
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0300, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::vector<WantAgentConstant::Flags> flags;
    flags.emplace_back(WantAgentConstant::Flags::CANCEL_PRESENT_FLAG);
    EXPECT_EQ(wantAgentHelper->FlagsTransformer(flags), static_cast<unsigned int>(0x10000000));
}

/*
 * @tc.number    : WantAgentHelper_0400
 * @tc.name      : WantAgentHelper Constructors
 * @tc.desc      : 1.WantAgentConstant::Flags::UPDATE_PRESENT_FLAG
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0400, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::vector<WantAgentConstant::Flags> flags;
    flags.emplace_back(WantAgentConstant::Flags::UPDATE_PRESENT_FLAG);
    EXPECT_EQ(wantAgentHelper->FlagsTransformer(flags), static_cast<unsigned int>(0x8000000));
}

/*
 * @tc.number    : WantAgentHelper_0500
 * @tc.name      : WantAgentHelper Constructors
 * @tc.desc      : 1.WantAgentConstant::Flags::CONSTANT_FLAG
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0500, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::vector<WantAgentConstant::Flags> flags;
    flags.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    EXPECT_EQ(wantAgentHelper->FlagsTransformer(flags), static_cast<unsigned int>(0x4000000));
}

/*
 * @tc.number    : WantAgentHelper_0600
 * @tc.name      : WantAgentHelper Constructors
 * @tc.desc      : 1.WantAgentConstant::Flags::REPLACE_BUNDLE
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0600, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::vector<WantAgentConstant::Flags> flags;
    flags.emplace_back(WantAgentConstant::Flags::REPLACE_BUNDLE);
    EXPECT_EQ(
        wantAgentHelper->FlagsTransformer(flags), static_cast<unsigned int>(FLAG_UPDATE_CURRENT));
}

/*
 * @tc.number    : WantAgentHelper_0700
 * @tc.name      : WantAgentHelper Constructors
 * @tc.desc      : 1.WantAgentConstant::Flags::CONSTANT_FLAG | UPDATE_PRESENT_FLAG | REPLACE_BUNDLE
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0700, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::vector<WantAgentConstant::Flags> flags;
    flags.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    flags.emplace_back(WantAgentConstant::Flags::UPDATE_PRESENT_FLAG);
    flags.emplace_back(WantAgentConstant::Flags::REPLACE_BUNDLE);
    EXPECT_EQ(wantAgentHelper->FlagsTransformer(flags), static_cast<unsigned int>(0xc000000));
}

/*
 * @tc.number    : WantAgentHelper_0800
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0800, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    WantAgentInfo wantAgentInfo;
    std::shared_ptr<WantAgent> wantAgent = nullptr;
    wantAgentHelper->GetWantAgent(nullptr, wantAgentInfo, wantAgent);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_0900
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *          `      2.wantAgentInfo.wants_ empty
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0900, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.clear();
    std::shared_ptr<WantAgent> wantAgent = nullptr;
    wantAgentHelper->GetWantAgent(nullptr, wantAgentInfo, wantAgent);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1000
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *                 2.wantAgentInfo.wants_.size() != wantAgentInfo.flags_.size()
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1000, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        OHOS::AbilityRuntime::Context::GetApplicationContext();
    WantAgentInfo wantAgentInfo;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.clear();
    std::shared_ptr<WantAgent> wantAgent = nullptr;
    wantAgentHelper->GetWantAgent(nullptr, wantAgentInfo, wantAgent);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1100
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *                 2.wantAgentInfo.wants_.size() == wantAgentInfo.flags_.size()
 *                 3.FlagsTransformer return 0
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1100, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        OHOS::AbilityRuntime::Context::GetApplicationContext();
    WantAgentInfo wantAgentInfo;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::REPLACE_BUNDLE);
    std::shared_ptr<WantAgent> wantAgent = nullptr;
    wantAgentHelper->GetWantAgent(nullptr, wantAgentInfo, wantAgent);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1200
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *                 2.wantAgentInfo.wants_.size() == wantAgentInfo.flags_.size()
 *                 3.FlagsTransformer return ok
 *                 4.Type is WantAgentConstant::OperationType::START_ABILITY
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1200, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        AbilityRuntime::ApplicationContext::GetInstance();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::START_ABILITY;
    wantAgentInfo.requestCode_ = 10;
    std::shared_ptr<WantAgent> wantAgent = nullptr;
    wantAgentHelper->GetWantAgent(nullptr, wantAgentInfo, wantAgent);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1300
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *                 2.wantAgentInfo.wants_.size() == wantAgentInfo.flags_.size()
 *                 3.FlagsTransformer return ok
 *                 4.Type is WantAgentConstant::OperationType::START_ABILITIES
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1300, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        AbilityRuntime::ApplicationContext::GetInstance();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::START_ABILITIES;
    wantAgentInfo.requestCode_ = 10;
    std::shared_ptr<WantAgent> wantAgent = nullptr;
    wantAgentHelper->GetWantAgent(nullptr, wantAgentInfo, wantAgent);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1400
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *                 2.wantAgentInfo.wants_.size() == wantAgentInfo.flags_.size()
 *                 3.FlagsTransformer return ok
 *                 4.Type is WantAgentConstant::OperationType::START_SERVICE
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1400, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        AbilityRuntime::ApplicationContext::GetInstance();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::START_SERVICE;
    wantAgentInfo.requestCode_ = 10;
    std::shared_ptr<WantAgent> wantAgent = nullptr;
    wantAgentHelper->GetWantAgent(nullptr, wantAgentInfo, wantAgent);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1500
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *                 2.wantAgentInfo.wants_.size() == wantAgentInfo.flags_.size()
 *                 3.FlagsTransformer return ok
 *                 4.Type is WantAgentConstant::OperationType::START_FOREGROUND_SERVICE
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1500, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        AbilityRuntime::ApplicationContext::GetInstance();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::START_FOREGROUND_SERVICE;
    wantAgentInfo.requestCode_ = 10;
    std::shared_ptr<WantAgent> wantAgent = nullptr;
    wantAgentHelper->GetWantAgent(nullptr, wantAgentInfo, wantAgent);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1600
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *                 2.wantAgentInfo.wants_.size() == wantAgentInfo.flags_.size()
 *                 3.FlagsTransformer return ok
 *                 4.Type is WantAgentConstant::OperationType::START_FOREGROUND_SERVICE
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1600, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        AbilityRuntime::ApplicationContext::GetInstance();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::SEND_COMMON_EVENT;
    wantAgentInfo.requestCode_ = 10;
    std::shared_ptr<WantAgent> wantAgent = nullptr;
    wantAgentHelper->GetWantAgent(nullptr, wantAgentInfo, wantAgent);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1700
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *                 2.wantAgentInfo.wants_.size() == wantAgentInfo.flags_.size()
 *                 3.FlagsTransformer return ok
 *                 4.Type is 100
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1700, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        AbilityRuntime::ApplicationContext::GetInstance();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = static_cast<WantAgentConstant::OperationType>(100);
    wantAgentInfo.requestCode_ = 10;
    std::shared_ptr<WantAgent> wantAgent = nullptr;
    wantAgentHelper->GetWantAgent(nullptr, wantAgentInfo, wantAgent);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1800
 * @tc.name      : WantAgentHelper GetWantAgent(const WantAgentInfo &paramsInfo)
 * @tc.desc      : 1.wantAgentInfo.wants_.size() == 0
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1800, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.clear();
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = static_cast<WantAgentConstant::OperationType>(100);
    wantAgentInfo.requestCode_ = 10;
    auto wantAgent = wantAgentHelper->GetWantAgent(wantAgentInfo);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1900
 * @tc.name      : WantAgentHelper GetWantAgent(const WantAgentInfo &paramsInfo)
 * @tc.desc      : 1.wantAgentInfo.wants_ not empty
 *                 2.wantAgentInfo.wants_.size() != wantAgentInfo.flags_.size()
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1900, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.clear();
    wantAgentInfo.operationType_ = static_cast<WantAgentConstant::OperationType>(100);
    wantAgentInfo.requestCode_ = 10;
    auto wantAgent = wantAgentHelper->GetWantAgent(wantAgentInfo);
    EXPECT_NE(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_2000
 * @tc.name      : WantAgentHelper GetWantAgent(const WantAgentInfo &paramsInfo)
 * @tc.desc      : 1.wantAgentInfo.wants_ not empty
 *                 2.wantAgentInfo.wants_.size() == wantAgentInfo.flags_.size()
 *                 3.wantAgentInfo.wants_[0] is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2000, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(nullptr);
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = static_cast<WantAgentConstant::OperationType>(100);
    wantAgentInfo.requestCode_ = 10;
    auto wantAgent = wantAgentHelper->GetWantAgent(wantAgentInfo);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_2100
 * @tc.name      : WantAgentHelper GetWantAgent(const WantAgentInfo &paramsInfo)
 * @tc.desc      : 1.wantAgentInfo.wants_ not empty
 *                 2.wantAgentInfo.wants_.size() == wantAgentInfo.flags_.size()
 *                 3.wantAgentInfo.wants_[0] is not nullptr
 *                 4.wantAgentInfo.extraInfo_ is not nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2100, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = static_cast<WantAgentConstant::OperationType>(100);
    wantAgentInfo.requestCode_ = 10;
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    wantAgentInfo.extraInfo_ = wParams;
    auto wantAgent = wantAgentHelper->GetWantAgent(wantAgentInfo);
    EXPECT_NE(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_2200
 * @tc.name      : WantAgentHelper GetWantAgent(const WantAgentInfo &paramsInfo)
 * @tc.desc      : 1.wantAgentInfo.wants_ not empty
 *                 2.wantAgentInfo.wants_.size() == wantAgentInfo.flags_.size()
 *                 3.wantAgentInfo.wants_[0] is not nullptr
 *                 4.wantAgentInfo.extraInfo_ is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2200, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = static_cast<WantAgentConstant::OperationType>(100);
    wantAgentInfo.requestCode_ = 10;
    wantAgentInfo.extraInfo_ = nullptr;
    auto wantAgent = wantAgentHelper->GetWantAgent(wantAgentInfo);
    EXPECT_NE(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_2300
 * @tc.name      : WantAgentHelper GetType
 * @tc.desc      : 1.agent is not nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2300, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    WantAgentInfo wantAgentInfo;
    std::shared_ptr<WantAgent> wantAgent = nullptr;
    wantAgentHelper->GetWantAgent(nullptr, wantAgentInfo, wantAgent);
    auto type = wantAgentHelper->GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::UNKNOWN_TYPE);
}

/*
 * @tc.number    : WantAgentHelper_2400
 * @tc.name      : WantAgentHelper GetType
 * @tc.desc      : 1.agent is not nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2400, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<WantAgent> wantAgent(nullptr);
    auto type = wantAgentHelper->GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::UNKNOWN_TYPE);
}

/*
 * @tc.number    : WantAgentHelper_2500
 * @tc.name      : WantAgentHelper GetType
 * @tc.desc      : 1.agent is not nullptr,PendingWant is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2500, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<PendingWant> pendingWant(nullptr);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    auto type = wantAgentHelper->GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::UNKNOWN_TYPE);
}

/*
 * @tc.number    : WantAgentHelper_2600
 * @tc.name      : WantAgentHelper JudgeEquality
 * @tc.desc      : 1.JudgeEquality
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2600, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<WantAgent> wantAgent(nullptr);
    std::shared_ptr<WantAgent> wantAgent2(nullptr);
    ErrCode isEqual = wantAgentHelper->IsEquals(wantAgent, wantAgent2);
    EXPECT_EQ(isEqual, ERR_OK);
}

/*
 * @tc.number    : WantAgentHelper_2700
 * @tc.name      : WantAgentHelper JudgeEquality
 * @tc.desc      : 1.JudgeEquality,Judge whether it is equal or not
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2700, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();

    // pendingwant
    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::START_FOREGROUND_SERVICE;
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        AbilityRuntime::ApplicationContext::GetInstance();
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::BuildServicePendingWant(context, requestCode, want, flags, type, pendingWant);

    // pendingwant2
    int requestCode2 = 11;
    std::shared_ptr<Want> want2 = std::make_shared<Want>();
    ElementName element2("device", "bundle", "ability");
    want2->SetElement(element2);
    std::shared_ptr<PendingWant> pendingWant2 = nullptr;
    PendingWant::BuildServicePendingWant(context, requestCode2, want2, flags, type, pendingWant2);

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    std::shared_ptr<WantAgent> wantAgent2 = std::make_shared<WantAgent>(pendingWant2);
    ErrCode isEqual = wantAgentHelper->IsEquals(wantAgent, wantAgent2);
    EXPECT_NE(isEqual, ERR_OK);
}

/*
 * @tc.number    : WantAgentHelper_2800
 * @tc.name      : WantAgentHelper JudgeEquality
 * @tc.desc      : 1.JudgeEquality,Judge whether it is equal or not
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2800, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();

    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::START_FOREGROUND_SERVICE;
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        AbilityRuntime::ApplicationContext::GetInstance();
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::BuildServicePendingWant(context, requestCode, want, flags, type, pendingWant);

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    std::shared_ptr<WantAgent> wantAgent2 = std::make_shared<WantAgent>(pendingWant);
    ErrCode isEqual = wantAgentHelper->IsEquals(wantAgent, wantAgent2);
    EXPECT_EQ(isEqual, ERR_OK);
}

/*
 * @tc.number    : WantAgentHelper_2900
 * @tc.name      : WantAgentHelper JudgeEquality
 * @tc.desc      : 1.JudgeEquality,Judge whether it is equal or not
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2900, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();

    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::START_FOREGROUND_SERVICE;
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        AbilityRuntime::ApplicationContext::GetInstance();
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::BuildServicePendingWant(context, requestCode, want, flags, type, pendingWant);

    std::shared_ptr<PendingWant> pendingWant2(nullptr);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    std::shared_ptr<WantAgent> wantAgent2 = std::make_shared<WantAgent>(pendingWant2);
    ErrCode isEqual = wantAgentHelper->IsEquals(wantAgent, wantAgent2);
    EXPECT_NE(isEqual, ERR_OK);
}

/*
 * @tc.number    : WantAgentHelper_3000
 * @tc.name      : WantAgentHelper GetBundleName
 * @tc.desc      : 1.GetBundleName WantAgent is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3000, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<WantAgent> wantAgent(nullptr);
    std::string bundleName = "";
    wantAgentHelper->GetBundleName(wantAgent, bundleName);
    EXPECT_EQ(bundleName, "");
}

/*
 * @tc.number    : WantAgentHelper_3100
 * @tc.name      : WantAgentHelper GetBundleName
 * @tc.desc      : 1.GetBundleName WantAgent.PendingWant.target is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3100, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<PendingWant> pendingWant(nullptr);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    std::string bundleName = "";
    wantAgentHelper->GetBundleName(wantAgent, bundleName);
    EXPECT_EQ(bundleName, "");
}

/*
 * @tc.number    : WantAgentHelper_3200
 * @tc.name      : WantAgentHelper GetUid
 * @tc.desc      : 1.GetUid WantAgent is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3200, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    EXPECT_NE(wantAgentHelper, nullptr);
    std::shared_ptr<WantAgent> wantAgent(nullptr);
    int32_t uid = -1;
    wantAgentHelper->GetUid(wantAgent, uid);
}

/*
 * @tc.number    : WantAgentHelper_3300
 * @tc.name      : WantAgentHelper GetUid
 * @tc.desc      : 1.GetUid WantAgent.PendingWant.target is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3300, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    EXPECT_NE(wantAgentHelper, nullptr);
    std::shared_ptr<PendingWant> pendingWant(nullptr);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    int32_t uid = -1;
    wantAgentHelper->GetUid(wantAgent, uid);
}

/*
 * @tc.number    : WantAgentHelper_3400
 * @tc.name      : WantAgentHelper GetWant
 * @tc.desc      : 1.GetWant WantAgent is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3400, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<WantAgent> wantAgent(nullptr);
    auto want = wantAgentHelper->GetWant(wantAgent);
    EXPECT_EQ(want, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_3500
 * @tc.name      : WantAgentHelper GetWant
 * @tc.desc      : 1.GetWant WantAgent.PendingWant.target is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3500, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<PendingWant> pendingWant(nullptr);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    auto want = wantAgentHelper->GetWant(wantAgent);
    EXPECT_EQ(want, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_3600
 * @tc.name      : WantAgentHelper ParseFlags
 * @tc.desc      : 1.ParseFlags Check ParseFlags
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3600, Function | MediumTest | Level1)
{
    std::vector<WantAgentConstant::Flags> flagsVec;
    nlohmann::json jsonObject;

    jsonObject["flags"] = -1;
    flagsVec = WantAgentHelper::ParseFlags(jsonObject);
    EXPECT_EQ(flagsVec.size(), 0);

    jsonObject.clear();
    jsonObject["flags"] = 1111000000;
    flagsVec = WantAgentHelper::ParseFlags(jsonObject);
    std::vector<WantAgentConstant::Flags>::iterator oneTimeIt = std::find(flagsVec.begin(), flagsVec.end(),
        WantAgentConstant::Flags::ONE_TIME_FLAG);
    EXPECT_EQ(oneTimeIt != flagsVec.end(), true);

    jsonObject.clear();
    jsonObject["flags"] = 111100000000000;
    flagsVec = WantAgentHelper::ParseFlags(jsonObject);
    std::vector<WantAgentConstant::Flags>::iterator cancelPresentIt = std::find(flagsVec.begin(), flagsVec.end(),
        WantAgentConstant::Flags::CANCEL_PRESENT_FLAG);
    EXPECT_EQ(cancelPresentIt != flagsVec.end(), true);

    jsonObject.clear();
    jsonObject["flags"] = 111100000000000;
    flagsVec = WantAgentHelper::ParseFlags(jsonObject);
    std::vector<WantAgentConstant::Flags>::iterator updateResentIt = std::find(flagsVec.begin(), flagsVec.end(),
        WantAgentConstant::Flags::UPDATE_PRESENT_FLAG);
    EXPECT_EQ(updateResentIt != flagsVec.end(), true);

    jsonObject.clear();
    jsonObject["flags"] = 111100000000000;
    flagsVec = WantAgentHelper::ParseFlags(jsonObject);
    std::vector<WantAgentConstant::Flags>::iterator constantIt = std::find(flagsVec.begin(), flagsVec.end(),
        WantAgentConstant::Flags::CONSTANT_FLAG);
    EXPECT_EQ(constantIt != flagsVec.end(), true);

    jsonObject.clear();
    jsonObject["flags"] = 1000000000;
    flagsVec = WantAgentHelper::ParseFlags(jsonObject);
    std::vector<WantAgentConstant::Flags>::iterator noBuildIt = std::find(flagsVec.begin(), flagsVec.end(),
        WantAgentConstant::Flags::NO_BUILD_FLAG);
    EXPECT_EQ(noBuildIt != flagsVec.end(), true);
}

/*
 * @tc.number    : WantAgentHelper_3700
 * @tc.name      : WantAgentHelper GetType
 * @tc.desc      : 1.agent is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3700, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<WantAgent> wantAgent(nullptr);
    int32_t type = static_cast<int32_t>(WantAgentConstant::OperationType::UNKNOWN_TYPE);
    EXPECT_EQ(wantAgentHelper->GetType(wantAgent, type), ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    EXPECT_EQ(type, static_cast<int32_t>(WantAgentConstant::OperationType::UNKNOWN_TYPE));
}

/*
 * @tc.number    : WantAgentHelper_3800
 * @tc.name      : WantAgentHelper GetType
 * @tc.desc      : 1.agent is not nullptr,PendingWant is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3800, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<PendingWant> pendingWant(nullptr);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    auto type = static_cast<int32_t>(WantAgentConstant::OperationType::UNKNOWN_TYPE);
    EXPECT_EQ(wantAgentHelper->GetType(wantAgent, type), ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    EXPECT_EQ(type, static_cast<int32_t>(WantAgentConstant::OperationType::UNKNOWN_TYPE));
}

/*
 * @tc.number    : WantAgentHelper_3900
 * @tc.name      : WantAgentHelper GetType
 * @tc.desc      : 1.agent is not nullptr, PendingWant is not nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3900, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    sptr<AAFwk::IWantSender> target(nullptr);
    std::shared_ptr<PendingWant> pendingWant = std::make_shared<PendingWant>(target);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    auto type = static_cast<int32_t>(WantAgentConstant::OperationType::UNKNOWN_TYPE);
    EXPECT_EQ(wantAgentHelper->GetType(wantAgent, type), ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    EXPECT_EQ(type, static_cast<int32_t>(WantAgentConstant::OperationType::UNKNOWN_TYPE));
}

/*
 * @tc.number    : WantAgentHelper_4000
 * @tc.name      : WantAgentHelper GetWant
 * @tc.desc      : 1.GetWant WantAgent is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_4000, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<WantAgent> wantAgent(nullptr);
    std::shared_ptr<AAFwk::Want> want(nullptr);
    EXPECT_EQ(wantAgentHelper->GetWant(wantAgent, want), ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    EXPECT_EQ(want, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_4100
 * @tc.name      : WantAgentHelper GetWant
 * @tc.desc      : 1.GetWant WantAgent.PendingWant is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_4100, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<PendingWant> pendingWant(nullptr);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    std::shared_ptr<AAFwk::Want> want(nullptr);
    EXPECT_EQ(wantAgentHelper->GetWant(wantAgent, want), ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    EXPECT_EQ(want, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_4200
 * @tc.name      : WantAgentHelper GetWant
 * @tc.desc      : 1.GetWant WantAgent and WantAgent.PendingWant is not nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_4200, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    sptr<AAFwk::IWantSender> target(nullptr);
    std::shared_ptr<PendingWant> pendingWant = std::make_shared<PendingWant>(target);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    std::shared_ptr<AAFwk::Want> want(nullptr);
    EXPECT_EQ(wantAgentHelper->GetWant(wantAgent, want), ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    EXPECT_EQ(want, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_4500
 * @tc.name      : WantAgentHelper RegisterCancelListener
 * @tc.desc      : 1.RegisterCancelListener when agent is null.
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_4500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "WantAgentHelper::RegisterCancelListener start";

    std::shared_ptr<CancelListener> cancelListener1 = std::make_shared<CancelListenerSon>();
    std::shared_ptr<CancelListener> cancelListener2 = std::make_shared<CancelListenerSon>();

    WantAgentHelper::RegisterCancelListener(cancelListener1, nullptr);
    WantAgentHelper::RegisterCancelListener(cancelListener2, nullptr);

    GTEST_LOG_(INFO) << "WantAgentHelper::RegisterCancelListener end";
}

/*
 * @tc.number    : WantAgentHelper_4600
 * @tc.name      : WantAgentHelper RegisterCancelListener
 * @tc.desc      : 2.RegisterCancelListener normal test
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_4600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "WantAgentHelper::RegisterCancelListener start";

    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::START_FOREGROUND_SERVICE;
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        AbilityRuntime::ApplicationContext::GetInstance();
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::BuildServicePendingWant(context, requestCode, want, flags, type, pendingWant);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);

    std::shared_ptr<CancelListener> cancelListener = std::make_shared<CancelListenerSon>();

    WantAgentHelper::RegisterCancelListener(cancelListener, wantAgent);

    EXPECT_EQ(static_cast<int>(wantAgent->GetPendingWant()->cancelListeners_.size()), 1);

    GTEST_LOG_(INFO) << "WantAgentHelper::RegisterCancelListener end";
}

/*
 * @tc.number    : WantAgentHelper_4700
 * @tc.name      : WantAgentHelper UnregisterCancelListener
 * @tc.desc      : 1.UnregisterCancelListener when agent is nullptr.
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_4700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "WantAgentHelper::UnregisterCancelListener start";

    std::shared_ptr<CancelListener> cancelListener1 = std::make_shared<CancelListenerSon>();
    std::shared_ptr<CancelListener> cancelListener2 = std::make_shared<CancelListenerSon>();
    WantAgentHelper::UnregisterCancelListener(cancelListener1, nullptr);
    WantAgentHelper::UnregisterCancelListener(cancelListener2, nullptr);

    GTEST_LOG_(INFO) << "WantAgentHelper::UnregisterCancelListener end";
}

/*
 * @tc.number    : WantAgentHelper_4800
 * @tc.name      : WantAgentHelper UnregisterCancelListener
 * @tc.desc      : 2.UnregisterCancelListener normal test.
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_4800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "WantAgentHelper::UnregisterCancelListener start";

    int requestCode = 10;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    unsigned int flags = 1;
    flags |= FLAG_ONE_SHOT;
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::START_FOREGROUND_SERVICE;
    std::shared_ptr<AbilityRuntime::ApplicationContext> context =
        AbilityRuntime::ApplicationContext::GetInstance();
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    PendingWant::BuildServicePendingWant(context, requestCode, want, flags, type, pendingWant);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);

    std::shared_ptr<CancelListener> cancelListener = std::make_shared<CancelListenerSon>();

    WantAgentHelper::RegisterCancelListener(cancelListener, wantAgent);
    EXPECT_EQ(static_cast<int>(wantAgent->GetPendingWant()->cancelListeners_.size()), 1);

    WantAgentHelper::UnregisterCancelListener(cancelListener, wantAgent);
    EXPECT_EQ(static_cast<int>(wantAgent->GetPendingWant()->cancelListeners_.size()), 0);

    GTEST_LOG_(INFO) << "WantAgentHelper::UnregisterCancelListener end";
}

/*
 * @tc.number    : WantAgentHelper_4900
 * @tc.name      : WantAgentHelper TriggerWantAgent
 * @tc.desc      : Test TriggerWantAgent when agent is nullptr.
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_4900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "WantAgentHelper::TriggerWantAgent start";

    std::shared_ptr<CompletedCallback> callback;
    TriggerInfo paramsInfo;
    sptr<CompletedDispatcher> completedData;
    WantAgentHelper::TriggerWantAgent(nullptr, callback, paramsInfo, completedData, nullptr);

    GTEST_LOG_(INFO) << "WantAgentHelper::TriggerWantAgent end";
}

/*
 * @tc.number    : WantAgentHelper_5000
 * @tc.name      : WantAgentHelper TriggerWantAgent
 * @tc.desc      : Test TriggerWantAgent when callback is nullptr.
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_5000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "WantAgentHelper::TriggerWantAgent start";

    std::shared_ptr<WantAgent> wantAgent(nullptr);
    TriggerInfo paramsInfo;
    sptr<CompletedDispatcher> completedData;
    auto result = WantAgentHelper::TriggerWantAgent(wantAgent, nullptr, paramsInfo, completedData, nullptr);
    EXPECT_EQ(result, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    GTEST_LOG_(INFO) << "WantAgentHelper::TriggerWantAgent end";
}

/*
 * @tc.number    : WantAgentHelper_5100
 * @tc.name      : WantAgentHelper TriggerWantAgent
 * @tc.desc      : Test TriggerWantAgent.
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_5100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "WantAgentHelper::TriggerWantAgent start";

    std::shared_ptr<WantAgent> wantAgent(nullptr);
    std::shared_ptr<CompletedCallback> callback;
    TriggerInfo paramsInfo;
    sptr<CompletedDispatcher> completedData;
    WantAgentHelper::TriggerWantAgent(wantAgent, callback, paramsInfo, completedData, nullptr);

    GTEST_LOG_(INFO) << "WantAgentHelper::TriggerWantAgent end";
}

/*
 * @tc.number    : WantAgentHelper_5200
 * @tc.name      : WantAgentHelper ToString
 * @tc.desc      : Test ToString.
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_5200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "WantAgentHelper::ToString start";
    std::shared_ptr<WantAgent> agent(nullptr);
    auto result = WantAgentHelper::ToString(agent);
    EXPECT_TRUE(result == "");
    GTEST_LOG_(INFO) << "WantAgentHelper::ToString end";
}

/*
 * @tc.number    : WantAgentHelper_5300
 * @tc.name      : WantAgentHelper FromString
 * @tc.desc      : Test FromString.
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_5300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "WantAgentHelper::FromString start";
    std::string jsonString;
    auto result = WantAgentHelper::FromString(jsonString);
    EXPECT_TRUE(result == nullptr);
    GTEST_LOG_(INFO) << "WantAgentHelper::FromString end";
}

/*
 * @tc.number    : WantAgentHelper_5400
 * @tc.name      : WantAgentHelper Cancel by flags FLAG_ONE_SHOT
 * @tc.desc      : 1.wantAgentInfo.wants_ not empty
 *                 2.wantAgentInfo.wants_.size() == wantAgentInfo.flags_.size()
 *                 3.wantAgentInfo.wants_[0] is not nullptr
 *                 4.wantAgentInfo.extraInfo_ is not nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_5400, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::ONE_TIME_FLAG);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::START_ABILITY;
    wantAgentInfo.requestCode_ = 10;
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    wantAgentInfo.extraInfo_ = wParams;
    auto wantAgent = wantAgentHelper->GetWantAgent(wantAgentInfo);
    EXPECT_NE(wantAgent, nullptr);
    auto type = wantAgentHelper->GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::START_ABILITY);

    WantAgentHelper::Cancel(wantAgent, FLAG_UPDATE_CURRENT);
    type = WantAgentHelper::GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::START_ABILITY);

    WantAgentHelper::Cancel(wantAgent, FLAG_ONE_SHOT);
    type = WantAgentHelper::GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::UNKNOWN_TYPE);
}

/*
 * @tc.number    : WantAgentHelper_5500
 * @tc.name      : WantAgentHelper Cancel again
 * @tc.desc      : 1.wantAgentInfo.wants_ not empty
 *                 2.wantAgentInfo.wants_.size() == wantAgentInfo.flags_.size()
 *                 3.wantAgentInfo.wants_[0] is not nullptr
 *                 4.wantAgentInfo.extraInfo_ is not nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_5500, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::ONE_TIME_FLAG);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::START_ABILITY;
    wantAgentInfo.requestCode_ = 10;
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    wantAgentInfo.extraInfo_ = wParams;
    auto wantAgent = wantAgentHelper->GetWantAgent(wantAgentInfo);
    EXPECT_NE(wantAgent, nullptr);
    auto type = wantAgentHelper->GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::START_ABILITY);

    WantAgentHelper::Cancel(wantAgent);
    type = WantAgentHelper::GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::UNKNOWN_TYPE);

    WantAgentHelper::Cancel(wantAgent, FLAG_ONE_SHOT);
    type = WantAgentHelper::GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::UNKNOWN_TYPE);

    WantAgentHelper::Cancel(wantAgent);
    type = WantAgentHelper::GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::UNKNOWN_TYPE);
}

/*
 * @tc.number    : WantAgentHelper_5600
 * @tc.name      : WantAgentHelper Cancel by flags FLAG_UPDATE_CURRENT
 * @tc.desc      : 1.wantAgentInfo.wants_ not empty
 *                 2.wantAgentInfo.wants_.size() == wantAgentInfo.flags_.size()
 *                 3.wantAgentInfo.wants_[0] is not nullptr
 *                 4.wantAgentInfo.extraInfo_ is not nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_5600, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::START_ABILITY;
    wantAgentInfo.requestCode_ = 10;
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    wantAgentInfo.extraInfo_ = wParams;
    auto wantAgent = wantAgentHelper->GetWantAgent(wantAgentInfo);
    EXPECT_NE(wantAgent, nullptr);
    auto type = wantAgentHelper->GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::START_ABILITY);

    WantAgentHelper::Cancel(wantAgent, FLAG_ONE_SHOT);
    type = WantAgentHelper::GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::START_ABILITY);

    WantAgentHelper::Cancel(wantAgent, FLAG_UPDATE_CURRENT);
    type = WantAgentHelper::GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::UNKNOWN_TYPE);
}

/*
 * @tc.number    : WantAgentHelper_5700
 * @tc.name      : WantAgentHelper Cancel by flags FLAG_ONE_SHOT|FLAG_ALLOW_CANCEL
 * @tc.desc      : 1.wantAgentInfo.wants_ not empty
 *                 2.wantAgentInfo.wants_.size() == wantAgentInfo.flags_.size()
 *                 3.wantAgentInfo.wants_[0] is not nullptr
 *                 4.wantAgentInfo.extraInfo_ is not nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_5700, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::ALLOW_CANCEL_FLAG);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::START_ABILITY;
    wantAgentInfo.requestCode_ = 20;
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    wantAgentInfo.extraInfo_ = wParams;
    auto wantAgent = wantAgentHelper->GetWantAgent(wantAgentInfo);
    EXPECT_NE(wantAgent, nullptr);
    auto type = wantAgentHelper->GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::START_ABILITY);

    WantAgentHelper::Cancel(wantAgent, FLAG_UPDATE_CURRENT);
    type = WantAgentHelper::GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::START_ABILITY);

    WantAgentHelper::Cancel(wantAgent, FLAG_ONE_SHOT);
    type = WantAgentHelper::GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::START_ABILITY);

    WantAgentHelper::Cancel(wantAgent, FLAG_ONE_SHOT|FLAG_ALLOW_CANCEL);
    type = WantAgentHelper::GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::UNKNOWN_TYPE);
}

/*
 * @tc.number    : WantAgentHelper_5800
 * @tc.name      : WantAgentHelper Cancel success after remove death recipient
 * @tc.desc      : RemoveDeathRecipient normal test
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_5800, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::START_ABILITY;
    wantAgentInfo.requestCode_ = 10;
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    wantAgentInfo.extraInfo_ = wParams;
    auto wantAgent = wantAgentHelper->GetWantAgent(wantAgentInfo);
    EXPECT_NE(wantAgent, nullptr);
    auto type = wantAgentHelper->GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::START_ABILITY);

    // remove death recipient
    WantAgentClient::GetInstance().RemoveDeathRecipient();

    WantAgentHelper::Cancel(wantAgent);
    type = WantAgentHelper::GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::UNKNOWN_TYPE);
}

/*
 * @tc.number    : WantAgentHelper_5900
 * @tc.name      : WantAgentHelper GetWant for local wantagent
 * @tc.desc      : wantAgent is local and not null
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_5900, Function | MediumTest | Level1)
{
    std::shared_ptr<AAFwk::Want> expectedWant = std::make_shared<AAFwk::Want>();
    expectedWant->SetElement(ElementName("deviceId", "bundleName", "abilityName"));
    std::shared_ptr<LocalPendingWant> localPendingWant = std::make_shared<LocalPendingWant>(
        "TestBundleName", expectedWant, 0);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(localPendingWant);
    std::shared_ptr<AAFwk::Want> actualWant;

    WantAgentHelper::GetWant(wantAgent, actualWant);
    ASSERT_EQ(actualWant->GetElement().GetBundleName(), expectedWant->GetElement().GetBundleName());
    ASSERT_EQ(WantAgentHelper::GetWant(wantAgent)->GetElement().GetBundleName(),
        expectedWant->GetElement().GetBundleName());
}

/*
 * @tc.number    : WantAgentHelper_6000
 * @tc.name      : WantAgentHelper GetWant for local wantagent
 * @tc.desc      : wantAgent is local and null
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_6000, Function | MediumTest | Level1)
{
    std::shared_ptr<LocalPendingWant> localPendingWant = nullptr;
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(localPendingWant);
    std::shared_ptr<AAFwk::Want> actualWant;
    const auto retCode = WantAgentHelper::GetWant(wantAgent, actualWant);
    ASSERT_TRUE(actualWant == nullptr);
    ASSERT_TRUE(retCode == ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    ASSERT_TRUE(WantAgentHelper::GetWant(wantAgent) == nullptr);
}

/*
 * @tc.number    : WantAgentHelper_6100
 * @tc.name      : WantAgentHelper GetType for local wantagent
 * @tc.desc      : wantAgent is local and not null
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_6100, Function | MediumTest | Level1)
{
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    want->SetElement(ElementName("deviceId", "bundleName", "abilityName"));
    const int32_t expectedType = 100;
    std::shared_ptr<LocalPendingWant> localPendingWant = std::make_shared<LocalPendingWant>(
        "TestBundleName", want, expectedType);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(localPendingWant);
    int32_t actualType = 0;
    WantAgentHelper::GetType(wantAgent, actualType);
    ASSERT_EQ(actualType, expectedType);
    ASSERT_EQ(static_cast<int32_t>(WantAgentHelper::GetType(wantAgent)), expectedType);
}

/*
 * @tc.number    : WantAgentHelper_6200
 * @tc.name      : WantAgentHelper GetType for local wantagent
 * @tc.desc      : wantAgent is local and null
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_6200, Function | MediumTest | Level1)
{
    std::shared_ptr<LocalPendingWant> localPendingWant = nullptr;
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(localPendingWant);
    int32_t actualType = 0;
    const auto retCode = WantAgentHelper::GetType(wantAgent, actualType);
    ASSERT_TRUE(retCode == ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    ASSERT_TRUE(WantAgentHelper::GetType(wantAgent) == WantAgentConstant::OperationType::UNKNOWN_TYPE);
}

/*
 * @tc.number    : WantAgentHelper_6300
 * @tc.name      : WantAgentHelper GetUid for local wantagent
 * @tc.desc      : wantAgent is local and not null
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_6300, Function | MediumTest | Level1)
{
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    want->SetElement(ElementName("deviceId", "bundleName", "abilityName"));
    const int32_t expectedUid = 6200;
    std::shared_ptr<LocalPendingWant> localPendingWant = std::make_shared<LocalPendingWant>(
        "TestBundleName", want, 0);
    localPendingWant->SetUid(expectedUid);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(localPendingWant);
    int32_t actualUid = 0;
    WantAgentHelper::GetUid(wantAgent, actualUid);
    ASSERT_EQ(actualUid, expectedUid);
}

/*
 * @tc.number    : WantAgentHelper_6400
 * @tc.name      : WantAgentHelper GetUid for local wantagent
 * @tc.desc      : wantAgent is local and null
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_6400, Function | MediumTest | Level1)
{
    std::shared_ptr<LocalPendingWant> localPendingWant = nullptr;
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(localPendingWant);
    int32_t actualUid = 0;
    const auto retCode = WantAgentHelper::GetType(wantAgent, actualUid);
    ASSERT_TRUE(retCode == ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/*
 * @tc.number    : WantAgentHelper_6500
 * @tc.name      : WantAgentHelper GetBundleName for local wantagent
 * @tc.desc      : wantAgent is local and not null
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_6500, Function | MediumTest | Level1)
{
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    want->SetElement(ElementName("deviceId", "bundleName", "abilityName"));
    const std::string expectedBundleName = "TestBundleName";
    std::shared_ptr<LocalPendingWant> localPendingWant = std::make_shared<LocalPendingWant>(
        expectedBundleName, want, 0);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(localPendingWant);
    std::string actualBundleName;
    WantAgentHelper::GetBundleName(wantAgent, actualBundleName);
    ASSERT_EQ(actualBundleName, expectedBundleName);
}

/*
 * @tc.number    : WantAgentHelper_6600
 * @tc.name      : WantAgentHelper GetBundleName for local wantagent
 * @tc.desc      : wantAgent is local and null
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_6600, Function | MediumTest | Level1)
{
    std::shared_ptr<LocalPendingWant> localPendingWant = nullptr;
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(localPendingWant);
    std::string actualBundleName;
    const auto retCode = WantAgentHelper::GetBundleName(wantAgent, actualBundleName);
    ASSERT_TRUE(retCode == ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: CreateLocalWantAgent_0100
 * @tc.desc: Test CreateLocalWantAgent with null context
 * @tc.type: FUNC
 */
HWTEST_F(WantAgentHelperTest, CreateLocalWantAgent_0100, TestSize.Level1)
{
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::vector<std::shared_ptr<Want>> wants = {want};
    LocalWantAgentInfo paramsInfo(10, WantAgentConstant::OperationType::START_ABILITY, wants);
    
    auto requestCode = paramsInfo.GetRequestCode();
    EXPECT_EQ(requestCode, 10);
    auto operationType = paramsInfo.GetOperationType();
    EXPECT_EQ(operationType, WantAgentConstant::OperationType::START_ABILITY);
    auto wantGet = paramsInfo.GetWants();
    EXPECT_EQ(wantGet.size(), 1);

    std::shared_ptr<WantAgent> wantAgent;
    ErrCode result = WantAgentHelper::CreateLocalWantAgent(nullptr, paramsInfo, wantAgent);
    
    EXPECT_EQ(result, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: CreateLocalWantAgent_0200
 * @tc.desc: Test CreateLocalWantAgent with empty wants
 * @tc.type: FUNC
 */
HWTEST_F(WantAgentHelperTest, CreateLocalWantAgent_0200, TestSize.Level1)
{
    auto context = std::make_shared<ApplicationContext>();
    
    std::vector<std::shared_ptr<Want>> wants;
    LocalWantAgentInfo paramsInfo(10, WantAgentConstant::OperationType::START_ABILITY, wants);

    std::shared_ptr<WantAgent> wantAgent;
    ErrCode result = WantAgentHelper::CreateLocalWantAgent(context, paramsInfo, wantAgent);
    
    EXPECT_EQ(result, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: CreateLocalWantAgent_0300
 * @tc.desc: Test CreateLocalWantAgent with null want in wants
 * @tc.type: FUNC
 */
HWTEST_F(WantAgentHelperTest, CreateLocalWantAgent_0300, TestSize.Level1)
{
    auto context = std::make_shared<ApplicationContext>();

    std::vector<std::shared_ptr<Want>> wants = {nullptr};
    LocalWantAgentInfo paramsInfo(10, WantAgentConstant::OperationType::START_ABILITY, wants);

    std::shared_ptr<WantAgent> wantAgent;
    ErrCode result = WantAgentHelper::CreateLocalWantAgent(context, paramsInfo, wantAgent);
    
    EXPECT_EQ(result, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: CreateLocalWantAgent_0400
 * @tc.desc: Test CreateLocalWantAgent with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(WantAgentHelperTest, CreateLocalWantAgent_0400, TestSize.Level1)
{
    auto context = std::make_shared<ApplicationContext>();
    
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::vector<std::shared_ptr<Want>> wants = {want};
    LocalWantAgentInfo paramsInfo(10, WantAgentConstant::OperationType::START_ABILITY, wants);

    std::shared_ptr<WantAgent> wantAgent;
    ErrCode result = WantAgentHelper::CreateLocalWantAgent(context, paramsInfo, wantAgent);
    
    EXPECT_EQ(result, ERR_OK);
    
    // Verify the created WantAgent
    std::shared_ptr<Want> retrievedWant;
    ErrCode getWantResult = WantAgentHelper::GetWant(wantAgent, retrievedWant);
    EXPECT_EQ(getWantResult, ERR_OK);
    
    int32_t operationType;
    ErrCode getTypeResult = WantAgentHelper::GetType(wantAgent, operationType);
    EXPECT_EQ(getTypeResult, ERR_OK);
    EXPECT_EQ(operationType, static_cast<int32_t>(WantAgentConstant::OperationType::START_ABILITY));
}

/**
 * @tc.name: CreateLocalWantAgent_0500
 * @tc.desc: Test CreateLocalWantAgent with different operation types
 * @tc.type: FUNC
 */
HWTEST_F(WantAgentHelperTest, CreateLocalWantAgent_0500, TestSize.Level1)
{
    auto context = std::make_shared<ApplicationContext>();
    
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::vector<std::shared_ptr<Want>> wants = {want};
    LocalWantAgentInfo paramsInfo(10, WantAgentConstant::OperationType::SEND_COMMON_EVENT, wants);
    
    std::shared_ptr<WantAgent> wantAgent;
    ErrCode result = WantAgentHelper::CreateLocalWantAgent(context, paramsInfo, wantAgent);
    
    EXPECT_EQ(result, ERR_OK);
    
    int32_t operationType;
    ErrCode getTypeResult = WantAgentHelper::GetType(wantAgent, operationType);
    EXPECT_EQ(getTypeResult, ERR_OK);
    EXPECT_EQ(operationType, static_cast<int32_t>(WantAgentConstant::OperationType::SEND_COMMON_EVENT));
}

/**
 * @tc.name: GetWantFromProxy_0100
 * @tc.desc: Test GetWantFromProxy with null WantAgent
 * @tc.type: FUNC
 */
HWTEST_F(WantAgentHelperTest, GetWantFromProxy_0100, TestSize.Level1)
{
    std::shared_ptr<WantAgent> wantAgent(nullptr);
    auto want = WantAgentHelper::GetWantFromProxy(wantAgent);
    EXPECT_EQ(want, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_6700
 * @tc.name      : WantAgentHelper ToStringWithEnvelope
 * @tc.desc      : Test ToStringWithEnvelope uses WantParamWrapperJson without changing ToString format.
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_6700, Function | MediumTest | Level1)
{
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityNameToJson");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::START_ABILITY;
    wantAgentInfo.requestCode_ = 10;
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam("key", Boolean::Box(true));
    wantAgentInfo.extraInfo_ = wParams;
    auto wantAgent = WantAgentHelper::GetWantAgent(wantAgentInfo);
    ASSERT_NE(wantAgent, nullptr);

    auto legacyString = WantAgentHelper::ToString(wantAgent);
    auto jsonString = WantAgentHelper::ToStringWithEnvelope(wantAgent);
    ASSERT_FALSE(legacyString.empty());
    ASSERT_FALSE(jsonString.empty());

    auto legacyObject = nlohmann::json::parse(legacyString, nullptr, false);
    auto jsonObject = nlohmann::json::parse(jsonString, nullptr, false);
    ASSERT_FALSE(legacyObject.is_discarded());
    ASSERT_FALSE(jsonObject.is_discarded());
    auto legacyExtraInfo = legacyObject.at("extraInfo").at("extraInfoValue").get<std::string>();
    auto jsonExtraInfo = jsonObject.at("extraInfo").at("extraInfoValue").get<std::string>();
    EXPECT_EQ(legacyExtraInfo.find("ohos.want.paramsStringEnvelope"), std::string::npos);
    EXPECT_NE(jsonExtraInfo.find("ohos.want.paramsStringEnvelope"), std::string::npos);
    EXPECT_FALSE(WantAgentHelper::HasWantParamsEnvelope(legacyString));
    EXPECT_TRUE(WantAgentHelper::HasWantParamsEnvelope(jsonString));
}

/*
 * @tc.number    : WantAgentHelper_6710
 * @tc.name      : WantAgentHelper FromStringWithEnvelope
 * @tc.desc      : Test FromStringWithEnvelope restores WantAgent from WantParamWrapperJson extraInfo.
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_6710, Function | MediumTest | Level1)
{
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityNameFromJson");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::START_ABILITY;
    wantAgentInfo.requestCode_ = 10;
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam("key", Boolean::Box(true));
    wantAgentInfo.extraInfo_ = wParams;
    auto wantAgent = WantAgentHelper::GetWantAgent(wantAgentInfo);
    ASSERT_NE(wantAgent, nullptr);

    auto jsonString = WantAgentHelper::ToStringWithEnvelope(wantAgent);
    auto restoredWantAgent = WantAgentHelper::FromStringWithEnvelope(jsonString);
    ASSERT_NE(restoredWantAgent, nullptr);

    auto restoredWant = WantAgentHelper::GetWant(restoredWantAgent);
    ASSERT_NE(restoredWant, nullptr);
    auto value = restoredWant->GetParams().GetParam("key");
    auto boolValue = IBoolean::Query(value);
    ASSERT_NE(boolValue, nullptr);
    EXPECT_TRUE(Boolean::Unbox(boolValue));
}

/*
 * @tc.number    : WantAgentHelper_6720
 * @tc.name      : WantAgentHelper ToStringWithEnvelope invalid input
 * @tc.desc      : Test ToStringWithEnvelope invalid agent branches.
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_6720, Function | MediumTest | Level1)
{
    EXPECT_TRUE(WantAgentHelper::ToStringWithEnvelope(nullptr).empty());

    auto wantAgentWithoutPendingWant = std::make_shared<WantAgent>(std::shared_ptr<PendingWant>(nullptr));
    EXPECT_TRUE(WantAgentHelper::ToStringWithEnvelope(wantAgentWithoutPendingWant).empty());

    auto pendingWantWithoutTarget = std::make_shared<PendingWant>(sptr<IWantSender>(nullptr));
    auto wantAgentWithoutInfo = std::make_shared<WantAgent>(pendingWantWithoutTarget);
    EXPECT_TRUE(WantAgentHelper::ToStringWithEnvelope(wantAgentWithoutInfo).empty());
}

/*
 * @tc.number    : WantAgentHelper_6730
 * @tc.name      : WantAgentHelper FromStringWithEnvelope invalid input
 * @tc.desc      : Test FromStringWithEnvelope invalid string and malformed field branches.
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_6730, Function | MediumTest | Level1)
{
    EXPECT_EQ(WantAgentHelper::FromStringWithEnvelope(""), nullptr);
    EXPECT_EQ(WantAgentHelper::FromStringWithEnvelope("{"), nullptr);
    EXPECT_EQ(WantAgentHelper::FromStringWithEnvelope(R"({"requestCode":"bad","wants":[]})"),
        nullptr);
    EXPECT_EQ(WantAgentHelper::FromStringWithEnvelope(R"({"wants":[100]})"), nullptr);

    Want want;
    ElementName element("device", "bundleName", "abilityNameFromJsonInvalid");
    want.SetElement(element);
    const auto originParamSize = want.GetParams().Size();
    nlohmann::json jsonObject;
    jsonObject["requestCode"] = 6730;
    jsonObject["operationType"] = static_cast<int32_t>(WantAgentConstant::OperationType::START_ABILITY);
    jsonObject["flags"] = static_cast<int32_t>(FLAG_UPDATE_CURRENT);
    jsonObject["userId"] = -1;
    jsonObject["appIndex"] = "bad";
    jsonObject["wants"] = nlohmann::json::array({ want.ToString() });
    jsonObject["extraInfo"] = { { "extraInfoValue", "invalid want params json" } };

    auto wantAgent = WantAgentHelper::FromStringWithEnvelope(jsonObject.dump());
    ASSERT_NE(wantAgent, nullptr);
    auto restoredWant = WantAgentHelper::GetWant(wantAgent);
    ASSERT_NE(restoredWant, nullptr);
    EXPECT_EQ(restoredWant->GetParams().Size(), originParamSize);

    jsonObject["requestCode"] = 6731;
    jsonObject["extraInfo"] = "bad";
    EXPECT_NE(WantAgentHelper::FromStringWithEnvelope(jsonObject.dump()), nullptr);

    jsonObject["requestCode"] = 6732;
    jsonObject["extraInfo"] = { { "extraInfoValue", 100 } };
    EXPECT_NE(WantAgentHelper::FromStringWithEnvelope(jsonObject.dump()), nullptr);
}

/*
 * @tc.number    : WantAgentHelper_6740
 * @tc.name      : WantAgentHelper HasWantParamsEnvelope
 * @tc.desc      : Test HasWantParamsEnvelope dispatches only when extraInfoValue uses WantParamWrapperJson envelope.
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_6740, Function | MediumTest | Level1)
{
    nlohmann::json jsonObject;
    jsonObject["extraInfo"] = { { "extraInfoValue", R"( {"ohos.want.paramsStringEnvelope":{}} )" } };
    EXPECT_FALSE(WantAgentHelper::HasWantParamsEnvelope(jsonObject.dump()));

    jsonObject["extraInfo"] = { { "extraInfoValue", "legacy want params" } };
    EXPECT_FALSE(WantAgentHelper::HasWantParamsEnvelope(jsonObject.dump()));

    EXPECT_FALSE(WantAgentHelper::HasWantParamsEnvelope(""));
    EXPECT_FALSE(WantAgentHelper::HasWantParamsEnvelope("{"));
    EXPECT_FALSE(WantAgentHelper::HasWantParamsEnvelope(R"({"extraInfo":"bad"})"));
    EXPECT_FALSE(WantAgentHelper::HasWantParamsEnvelope(R"({"extraInfo":{"extraInfoValue":100}})"));
    EXPECT_FALSE(WantAgentHelper::HasWantParamsEnvelope(R"({"wants":[]})"));
}
}  // namespace OHOS::AbilityRuntime::WantAgent
