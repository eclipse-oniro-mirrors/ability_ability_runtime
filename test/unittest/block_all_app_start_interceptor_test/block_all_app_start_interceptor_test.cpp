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

#include <atomic>
#include <functional>
#include <gtest/gtest.h>
#include <memory>
#define private public
#define protected public
#include "interceptor/block_all_app_start_interceptor.h"
#undef private
#undef protected

#include "ability_record/ability_request.h"
#include "hilog_tag_wrapper.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class BlockAllAppStartInterceptorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void BlockAllAppStartInterceptorTest::SetUpTestCase()
{}

void BlockAllAppStartInterceptorTest::TearDownTestCase()
{}

void BlockAllAppStartInterceptorTest::SetUp()
{}

void BlockAllAppStartInterceptorTest::TearDown()
{}

namespace {
constexpr int32_t TEST_USER_ID = 100;
AbilityInterceptorParam BuildParam(AbilityRequest &request, int32_t userId,
    AppExecFwk::AbilityType type, std::function<bool(int32_t)> escapeCheck = {})
{
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->type = type;
    InterceptorParamBuilder builder(request.want, 0, userId);
    builder.CallerToken(nullptr).AbilityInfo(abilityInfo);
    if (escapeCheck) {
        builder.BlockAllEscapeCheck(escapeCheck);
    }
    return builder.Build();
}

AbilityInterceptorParam BuildRemoteParam()
{
    return InterceptorParamBuilder(Want(), 0, TEST_USER_ID)
        .CallerToken(nullptr)
        .Context<AbilityInterceptorParam::RemoteDispatchCtx>({}).Build();
}
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_001
 * @tc.desc: DoProcess with no shouldBlockFunc set (nullptr) returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_001, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    AbilityRequest request;
    auto param = BuildParam(request, TEST_USER_ID, AppExecFwk::AbilityType::PAGE);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_002
 * @tc.desc: DoProcess returns ERR_ALL_APP_START_BLOCKED when shouldBlockFunc returns true and no escape hatch applies
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_002, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    interceptor.SetShouldBlockFunc([]() -> bool { return true; });
    AbilityRequest request;
    auto param = BuildParam(request, TEST_USER_ID, AppExecFwk::AbilityType::SERVICE);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_ALL_APP_START_BLOCKED);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_003
 * @tc.desc: DoProcess returns ERR_OK when shouldBlockFunc returns false
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_003, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    interceptor.SetShouldBlockFunc([]() -> bool { return false; });
    AbilityRequest request;
    auto param = BuildParam(request, TEST_USER_ID, AppExecFwk::AbilityType::PAGE);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_004
 * @tc.desc: DoProcess no-ops (ERR_OK) on RemoteDispatchCtx even when block mode is active
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_004, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    interceptor.SetShouldBlockFunc([]() -> bool { return true; });
    auto param = BuildRemoteParam();
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_005
 * @tc.desc: DoProcess with block flag false never calls the escape-check lambda
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_005, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    auto startedCount = std::make_shared<std::atomic<int32_t>>(0);
    interceptor.SetShouldBlockFunc([]() { return false; });
    auto escapeCheck = [&startedCount](int32_t userId) {
        startedCount->fetch_add(1);
        return true;
    };
    AbilityRequest request;
    auto param = BuildParam(request, TEST_USER_ID, AppExecFwk::AbilityType::PAGE, escapeCheck);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(startedCount->load(), 0);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_006
 * @tc.desc: DoProcess blocks non-PAGE ability even if escapeCheck returns true
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_006, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    interceptor.SetShouldBlockFunc([]() { return true; });
    AbilityRequest request;
    for (auto type : { AppExecFwk::AbilityType::SERVICE, AppExecFwk::AbilityType::EXTENSION }) {
        auto param = BuildParam(request, TEST_USER_ID, type, [](int32_t) { return true; });
        auto ret = interceptor.DoProcess(param);
        EXPECT_EQ(ret, ERR_ALL_APP_START_BLOCKED);
    }
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_007
 * @tc.desc: DoProcess allows a started PAGE ability despite block mode (escape hatch)
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_007, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    interceptor.SetShouldBlockFunc([]() { return true; });
    AbilityRequest request;
    auto param = BuildParam(request, TEST_USER_ID, AppExecFwk::AbilityType::PAGE,
        [](int32_t) { return true; });
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_008
 * @tc.desc: DoProcess blocks a non-started PAGE ability in block mode
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_008, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    interceptor.SetShouldBlockFunc([]() { return true; });
    AbilityRequest request;
    auto param = BuildParam(request, TEST_USER_ID, AppExecFwk::AbilityType::PAGE,
        [](int32_t) { return false; });
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_ALL_APP_START_BLOCKED);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_009
 * @tc.desc: DoProcess fails-closed (blocks) for a PAGE ability when the escape-check lambda
 *           is absent, i.e. the escape hatch is unavailable at the chain site
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_009, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    interceptor.SetShouldBlockFunc([]() { return true; });
    AbilityRequest request;
    auto param = BuildParam(request, TEST_USER_ID, AppExecFwk::AbilityType::PAGE);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_ALL_APP_START_BLOCKED);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_010
 * @tc.desc: DoProcess fails-closed (blocks) for a PAGE ability when the escape-check lambda
 *           is unset (empty std::function)
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_010, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    interceptor.SetShouldBlockFunc([]() { return true; });
    AbilityRequest request;
    auto param = BuildParam(request, TEST_USER_ID, AppExecFwk::AbilityType::PAGE);
    auto ret = interceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_ALL_APP_START_BLOCKED);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_011
 * @tc.desc: DoProcess escape hatch is consulted exactly once per PAGE-under-block call; the
 *           escape-check lambda receives the param userId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_011, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    auto blockFlag = std::make_shared<std::atomic<bool>>(false);
    std::function<bool()> shouldBlock = [blockFlag]() { return blockFlag->load(); };
    interceptor.SetShouldBlockFunc(shouldBlock);
    auto startedFlag = std::make_shared<std::atomic<bool>>(false);
    auto startedCount = std::make_shared<std::atomic<int32_t>>(0);
    auto capturedUserId = std::make_shared<std::atomic<int32_t>>(0);
    auto escapeCheck = [startedFlag, startedCount, capturedUserId](int32_t userId) {
        startedCount->fetch_add(1);
        capturedUserId->store(userId);
        return startedFlag->load();
    };

    int32_t userId = TEST_USER_ID;
    AbilityRequest pageRequest;
    AbilityRequest serviceRequest;

    // scenario 1: block mode, PAGE not started -> blocked, escape check consulted once
    blockFlag->store(true);
    startedFlag->store(false);
    auto pageParam = BuildParam(pageRequest, userId, AppExecFwk::AbilityType::PAGE, escapeCheck);
    EXPECT_EQ(interceptor.DoProcess(pageParam), ERR_ALL_APP_START_BLOCKED);
    auto serviceParam = BuildParam(serviceRequest, userId, AppExecFwk::AbilityType::SERVICE, escapeCheck);
    EXPECT_EQ(interceptor.DoProcess(serviceParam), ERR_ALL_APP_START_BLOCKED);
    // escape check only consulted for the PAGE request
    EXPECT_EQ(startedCount->load(), 1);
    EXPECT_EQ(capturedUserId->load(), userId);

    // scenario 2: block mode, started PAGE is exempted
    startedFlag->store(true);
    auto pageParam2 = BuildParam(pageRequest, userId, AppExecFwk::AbilityType::PAGE, escapeCheck);
    EXPECT_EQ(interceptor.DoProcess(pageParam2), ERR_OK);
    auto serviceParam2 = BuildParam(serviceRequest, userId, AppExecFwk::AbilityType::SERVICE, escapeCheck);
    EXPECT_EQ(interceptor.DoProcess(serviceParam2), ERR_ALL_APP_START_BLOCKED);
    EXPECT_EQ(startedCount->load(), 2);

    // scenario 3: no block mode -> all pass, escape check not consulted
    auto countBefore = startedCount->load();
    blockFlag->store(false);
    auto pageParam3 = BuildParam(pageRequest, userId, AppExecFwk::AbilityType::PAGE, escapeCheck);
    EXPECT_EQ(interceptor.DoProcess(pageParam3), ERR_OK);
    auto serviceParam3 = BuildParam(serviceRequest, userId, AppExecFwk::AbilityType::SERVICE, escapeCheck);
    EXPECT_EQ(interceptor.DoProcess(serviceParam3), ERR_OK);
    EXPECT_EQ(startedCount->load(), countBefore);
}
} // namespace AAFwk
} // namespace OHOS
