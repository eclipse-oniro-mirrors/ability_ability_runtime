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

public:
};

void BlockAllAppStartInterceptorTest::SetUpTestCase()
{}

void BlockAllAppStartInterceptorTest::TearDownTestCase()
{}

void BlockAllAppStartInterceptorTest::SetUp()
{}

void BlockAllAppStartInterceptorTest::TearDown()
{}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_001
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_001, TestSize.Level1)
{
    BlockAllAppStartInterceptor blockAllAppStartInterceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldBlockAllAppStartFunc = nullptr;
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldBlockAllAppStartFunc);
    auto ret = blockAllAppStartInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_002
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_002, TestSize.Level1)
{
    BlockAllAppStartInterceptor blockAllAppStartInterceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldBlockAllAppStartFunc = []() -> bool {
        return true;
    };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldBlockAllAppStartFunc);
    auto ret = blockAllAppStartInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_ALL_APP_START_BLOCKED);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_003
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_003, TestSize.Level1)
{
    BlockAllAppStartInterceptor blockAllAppStartInterceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldBlockAllAppStartFunc = []() -> bool {
        return false;
    };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldBlockAllAppStartFunc);
    auto ret = blockAllAppStartInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_Execute_001
 * @tc.desc: Execute without shouldBlockFunc
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, Execute_001, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    auto ret = interceptor.Execute();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_Execute_002
 * @tc.desc: Execute with shouldBlockFunc returning false
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, Execute_002, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    interceptor.SetShouldBlockFunc([]() { return false; });
    auto ret = interceptor.Execute();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_Execute_003
 * @tc.desc: Execute with shouldBlockFunc returning true
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, Execute_003, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    interceptor.SetShouldBlockFunc([]() { return true; });
    auto ret = interceptor.Execute();
    EXPECT_EQ(ret, ERR_ALL_APP_START_BLOCKED);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_ExecuteWithRequest_001
 * @tc.desc: Execute(AbilityRequest) with null shouldBlockFunc
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, ExecuteWithRequest_001, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    AbilityRequest request;
    int32_t userId = 100;
    // null shouldBlockFunc is rejected before the null isAbilityStartedFunc check
    auto ret = interceptor.Execute(request, userId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    // null isAbilityStartedFunc is rejected even when the block flag is false
    interceptor.SetShouldBlockFunc([]() { return false; });
    ret = interceptor.Execute(request, userId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_ExecuteWithRequest_002
 * @tc.desc: Execute(AbilityRequest) with block flag false, isAbilityStartedFunc not consulted
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, ExecuteWithRequest_002, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    auto startedCount = std::make_shared<std::atomic<int32_t>>(0);
    interceptor.SetShouldBlockFunc([]() { return false; });
    interceptor.SetIsAbilityStartedFunc(
        [&startedCount](AbilityRequest &request, int32_t userId) {
            startedCount->fetch_add(1);
            return true;
        });
    AbilityRequest request;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    int32_t userId = 100;
    auto ret = interceptor.Execute(request, userId);
    EXPECT_EQ(ret, ERR_OK);
    // no block mode: the started func is never consulted
    EXPECT_EQ(startedCount->load(), 0);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_ExecuteWithRequest_003
 * @tc.desc: Execute(AbilityRequest) blocks non-PAGE ability even if isAbilityStartedFunc returns true
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, ExecuteWithRequest_003, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    interceptor.SetShouldBlockFunc([]() { return true; });
    interceptor.SetIsAbilityStartedFunc([](AbilityRequest &request, int32_t userId) { return true; });
    int32_t userId = 100;
    // the started-exemption only applies to PAGE abilities
    for (auto type : { AppExecFwk::AbilityType::SERVICE, AppExecFwk::AbilityType::EXTENSION }) {
        AbilityRequest request;
        request.abilityInfo.type = type;
        auto ret = interceptor.Execute(request, userId);
        EXPECT_EQ(ret, ERR_ALL_APP_START_BLOCKED);
    }
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_ExecuteWithRequest_004
 * @tc.desc: Execute(AbilityRequest) allows a started PAGE ability despite block mode
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, ExecuteWithRequest_004, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    interceptor.SetShouldBlockFunc([]() { return true; });
    interceptor.SetIsAbilityStartedFunc([](AbilityRequest &request, int32_t userId) { return true; });
    AbilityRequest request;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    int32_t userId = 100;
    auto ret = interceptor.Execute(request, userId);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_ExecuteWithRequest_005
 * @tc.desc: Execute(AbilityRequest) blocks a non-started PAGE ability in block mode
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, ExecuteWithRequest_005, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    interceptor.SetShouldBlockFunc([]() { return true; });
    interceptor.SetIsAbilityStartedFunc([](AbilityRequest &request, int32_t userId) { return false; });
    AbilityRequest request;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    int32_t userId = 100;
    auto ret = interceptor.Execute(request, userId);
    EXPECT_EQ(ret, ERR_ALL_APP_START_BLOCKED);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_TwoPathConsistency_001
 * @tc.desc: Execute() and DoProcess() paths return identical verdicts under the same predicate
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, TwoPathConsistency_001, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    auto blockFlag = std::make_shared<std::atomic<bool>>(false);
    auto evalCount = std::make_shared<std::atomic<int32_t>>(0);
    std::function<bool()> shouldBlock = [blockFlag, evalCount]() {
        evalCount->fetch_add(1);
        return blockFlag->load();
    };
    interceptor.SetShouldBlockFunc(shouldBlock);

    Want want;
    int requestCode = 123;
    int32_t userId = 100;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldBlock);

    // scenario 1: no block, both paths allow
    blockFlag->store(false);
    EXPECT_EQ(interceptor.Execute(), ERR_OK);
    EXPECT_EQ(interceptor.DoProcess(param), ERR_OK);

    // scenario 2: block, both paths reject
    blockFlag->store(true);
    EXPECT_EQ(interceptor.Execute(), ERR_ALL_APP_START_BLOCKED);
    EXPECT_EQ(interceptor.DoProcess(param), ERR_ALL_APP_START_BLOCKED);

    // scenario 3: flip back to no block, both paths recover
    blockFlag->store(false);
    EXPECT_EQ(interceptor.Execute(), ERR_OK);
    EXPECT_EQ(interceptor.DoProcess(param), ERR_OK);

    // the shared predicate is really consulted on every evaluation
    EXPECT_GE(evalCount->load(), 6);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_TwoPathConsistency_002
 * @tc.desc: Execute() and Execute(AbilityRequest) paths are consistent for non-exempt requests,
 *           while a started PAGE ability is exempted only on the request path
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, TwoPathConsistency_002, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    auto blockFlag = std::make_shared<std::atomic<bool>>(false);
    std::function<bool()> shouldBlock = [blockFlag]() { return blockFlag->load(); };
    interceptor.SetShouldBlockFunc(shouldBlock);
    auto startedFlag = std::make_shared<std::atomic<bool>>(false);
    auto startedCount = std::make_shared<std::atomic<int32_t>>(0);
    interceptor.SetIsAbilityStartedFunc([startedFlag, startedCount](AbilityRequest &request, int32_t userId) {
        startedCount->fetch_add(1);
        return startedFlag->load();
    });

    int32_t userId = 100;
    AbilityRequest pageRequest;
    pageRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    AbilityRequest serviceRequest;
    serviceRequest.abilityInfo.type = AppExecFwk::AbilityType::SERVICE;

    // scenario 1: block mode, PAGE not started, both paths reject consistently
    blockFlag->store(true);
    startedFlag->store(false);
    EXPECT_EQ(interceptor.Execute(), ERR_ALL_APP_START_BLOCKED);
    EXPECT_EQ(interceptor.Execute(pageRequest, userId), ERR_ALL_APP_START_BLOCKED);
    EXPECT_EQ(interceptor.Execute(serviceRequest, userId), ERR_ALL_APP_START_BLOCKED);

    // scenario 2: block mode, started PAGE is exempted only on the request path
    startedFlag->store(true);
    EXPECT_EQ(interceptor.Execute(), ERR_ALL_APP_START_BLOCKED);
    EXPECT_EQ(interceptor.Execute(pageRequest, userId), ERR_OK);
    // non PAGE keeps being rejected
    EXPECT_EQ(interceptor.Execute(serviceRequest, userId), ERR_ALL_APP_START_BLOCKED);
    // the started func is only consulted for PAGE requests under block mode,
    // exactly twice: scenario 1 and scenario 2 pageRequest calls
    EXPECT_EQ(startedCount->load(), 2);

    // scenario 3: no block mode, both paths allow and the started func is not consulted
    auto countBefore = startedCount->load();
    blockFlag->store(false);
    EXPECT_EQ(interceptor.Execute(), ERR_OK);
    EXPECT_EQ(interceptor.Execute(pageRequest, userId), ERR_OK);
    EXPECT_EQ(interceptor.Execute(serviceRequest, userId), ERR_OK);
    EXPECT_EQ(startedCount->load(), countBefore);
}
} // namespace AAFwk
} // namespace OHOS
