/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "native_child_process.h"
#include "app_utils.h"
#include "child_process_args_manager.h"
#include "child_process_configs.h"
#include "native_child_callback.h"

extern void SetGlobalNativeChildCallbackStub(OHOS::sptr<OHOS::AbilityRuntime::NativeChildCallback> local);

namespace OHOS {
namespace AbilityRuntime {

using namespace testing;
using namespace testing::ext;

class ChildProcessCapiTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);

    static void OnNativeChildProcessStarted(int errCode, OHIPCRemoteProxy *remoteProxy);
    static void OnNativeChildProcessExit(int32_t pid, int32_t signal);
    static void OnNativeChildProcessExit1(int32_t pid, int32_t signal);

    void SetUp();
    void TearDown();
};

void ChildProcessCapiTest::SetUpTestCase(void)
{}

void ChildProcessCapiTest::TearDownTestCase(void)
{}

void ChildProcessCapiTest::SetUp(void)
{}

void ChildProcessCapiTest::TearDown(void)
{}

void ChildProcessCapiTest::OnNativeChildProcessStarted(int errCode, OHIPCRemoteProxy *remoteProxy)
{
}

void ChildProcessCapiTest::OnNativeChildProcessExit(int32_t pid, int32_t signal)
{
    GTEST_LOG_(INFO) << "OnNativeChildProcessExit call";
}

void ChildProcessCapiTest::OnNativeChildProcessExit1(int32_t pid, int32_t signal)
{
    GTEST_LOG_(INFO) << "OnNativeChildProcessExit1 call";
}

/**
 * @tc.number: OH_Ability_CreateNativeChildProcess_001
 * @tc.desc: Test API OH_Ability_CreateNativeChildProcess works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_CreateNativeChildProcess_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "OH_Ability_CreateNativeChildProcess_001 begin";
    int ret = OH_Ability_CreateNativeChildProcess(nullptr, ChildProcessCapiTest::OnNativeChildProcessStarted);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);

    ret = OH_Ability_CreateNativeChildProcess("test.so", nullptr);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);

    ret = OH_Ability_CreateNativeChildProcess("test.so", ChildProcessCapiTest::OnNativeChildProcessStarted);
    if (!AAFwk::AppUtils::GetInstance().IsMultiProcessModel()) {
        EXPECT_EQ(ret, NCP_ERR_SERVICE_ERROR);
        return;
    } else if (!AAFwk::AppUtils::GetInstance().IsSupportNativeChildProcess()) {
        EXPECT_EQ(ret, NCP_ERR_MULTI_PROCESS_DISABLED);
        return;
    }

    GTEST_LOG_(INFO) << "OH_Ability_CreateNativeChildProcess return " << ret;
    EXPECT_NE(ret, NCP_ERR_NOT_SUPPORTED);
}

/**
 * @tc.number: OH_Ability_StartNativeChildProcess_001
 * @tc.desc: Test API OH_Ability_StartNativeChildProcess_001 works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_StartNativeChildProcess_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "OH_Ability_StartNativeChildProcess_001 begin";
    NativeChildProcess_Args args;
    NativeChildProcess_Options options;
    int32_t pid = 0;
    auto ret = OH_Ability_StartNativeChildProcess(nullptr, args, options, &pid);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);
    GTEST_LOG_(INFO) << "OH_Ability_StartNativeChildProcess_001 begin";
}

/**
 * @tc.number: OH_Ability_CreateNativeChildProcessWithConfigs_001
 * @tc.desc: Test API OH_Ability_CreateNativeChildProcessWithConfigs works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_CreateNativeChildProcessWithConfigs, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "OH_Ability_CreateNativeChildProcess_001 begin";
    auto configs = OH_Ability_CreateChildProcessConfigs();
    int ret = OH_Ability_CreateNativeChildProcessWithConfigs(nullptr, configs,
        ChildProcessCapiTest::OnNativeChildProcessStarted);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);

    ret = OH_Ability_CreateNativeChildProcessWithConfigs("test.so", configs, nullptr);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);

    ret = OH_Ability_CreateNativeChildProcessWithConfigs("test.so", nullptr,
        ChildProcessCapiTest::OnNativeChildProcessStarted);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);

    ret = OH_Ability_CreateNativeChildProcessWithConfigs("test.so", configs,
        ChildProcessCapiTest::OnNativeChildProcessStarted);
    if (!AAFwk::AppUtils::GetInstance().IsMultiProcessModel()) {
        EXPECT_EQ(ret, NCP_ERR_SERVICE_ERROR);
        return;
    } else if (!AAFwk::AppUtils::GetInstance().IsSupportNativeChildProcess()) {
        EXPECT_EQ(ret, NCP_ERR_MULTI_PROCESS_DISABLED);
        return;
    }

    GTEST_LOG_(INFO) << "OH_Ability_CreateNativeChildProcess return " << ret;
    EXPECT_NE(ret, NCP_ERR_NOT_SUPPORTED);
}

/**
 * @tc.number: OH_Ability_StartNativeChildProcessWithConfigs_001
 * @tc.desc: Test API OH_Ability_StartNativeChildProcessWithConfigs_001 works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_StartNativeChildProcessWithConfigs_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "OH_Ability_StartNativeChildProcessWithConfigs_001 begin";
    NativeChildProcess_Args args;
    auto configs = OH_Ability_CreateChildProcessConfigs();
    int32_t pid = 0;
    auto ret = OH_Ability_StartNativeChildProcessWithConfigs(nullptr, args, configs, &pid);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);
    ret = OH_Ability_StartNativeChildProcessWithConfigs("a.so:Main", args, nullptr, &pid);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);
    ret = OH_Ability_DestroyChildProcessConfigs(configs);
    EXPECT_EQ(ret, NCP_NO_ERROR);
    ret = OH_Ability_DestroyChildProcessConfigs(nullptr);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);
    GTEST_LOG_(INFO) << "OH_Ability_StartNativeChildProcess_001 begin";
}

/**
 * @tc.number: OH_Ability_Ability_ChildProcessConfigs_001
 * @tc.desc: Test API OH_Ability_Ability_ChildProcessConfigs_001 works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_ChildProcessConfigs_SetIsolationMode_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "OH_Ability_Ability_ChildProcessConfigs_001 begin";
    auto configs = OH_Ability_CreateChildProcessConfigs();
    auto ret = OH_Ability_ChildProcessConfigs_SetIsolationMode(configs, NCP_ISOLATION_MODE_ISOLATED);
    EXPECT_EQ(ret, NCP_NO_ERROR);
    ret = OH_Ability_ChildProcessConfigs_SetIsolationMode(nullptr, NCP_ISOLATION_MODE_ISOLATED);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);
    ret = OH_Ability_DestroyChildProcessConfigs(configs);
    EXPECT_EQ(ret, NCP_NO_ERROR);
    GTEST_LOG_(INFO) << "OH_Ability_Ability_ChildProcessConfigs_001 begin";
}

/**
 * @tc.number: OH_Ability_Ability_ChildProcessConfigs_002
 * @tc.desc: Test API OH_Ability_Ability_ChildProcessConfigs_002 works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_ChildProcessConfigs_SetIsolationUid_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "OH_Ability_Ability_ChildProcessConfigs_002 begin";
    auto configs = OH_Ability_CreateChildProcessConfigs();
    auto ret = OH_Ability_ChildProcessConfigs_SetIsolationUid(configs, true);
    EXPECT_EQ(ret, NCP_NO_ERROR);
    ret = OH_Ability_ChildProcessConfigs_SetIsolationUid(nullptr, false);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);
    ret = OH_Ability_DestroyChildProcessConfigs(configs);
    EXPECT_EQ(ret, NCP_NO_ERROR);
    GTEST_LOG_(INFO) << "OH_Ability_Ability_ChildProcessConfigs_002 begin";
}

/**
 * @tc.number: OH_Ability_ChildProcessConfigs_SetProcessName_001
 * @tc.desc: Test API OH_Ability_ChildProcessConfigs_SetProcessName_001 works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_ChildProcessConfigs_SetProcessName_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "OH_Ability_Ability_ChildProcessConfigs_001 begin";
    auto configs = OH_Ability_CreateChildProcessConfigs();
    auto ret = OH_Ability_ChildProcessConfigs_SetProcessName(configs, nullptr);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);
    ret = OH_Ability_ChildProcessConfigs_SetProcessName(nullptr, "");
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);
    ret = OH_Ability_ChildProcessConfigs_SetProcessName(configs, "");
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);
    ret = OH_Ability_ChildProcessConfigs_SetProcessName(configs, "abc_123");
    EXPECT_EQ(ret, NCP_NO_ERROR);
    ret = OH_Ability_ChildProcessConfigs_SetProcessName(configs, "abc_123[");
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);
    ret = OH_Ability_DestroyChildProcessConfigs(configs);
    EXPECT_EQ(ret, NCP_NO_ERROR);
    GTEST_LOG_(INFO) << "OH_Ability_Ability_ChildProcessConfigs_001 begin";
}


/**
 * @tc.number: OH_Ability_RegisterNativeChildProcessExitCallback_001
 * @tc.desc: Test API OH_Ability_RegisterNativeChildProcessExitCallback_001 works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_RegisterNativeChildProcessExitCallback_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "OH_Ability_RegisterNativeChildProcessExitCallback_001 begin";
    auto ret = OH_Ability_RegisterNativeChildProcessExitCallback(nullptr);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);
    ret = OH_Ability_RegisterNativeChildProcessExitCallback(ChildProcessCapiTest::OnNativeChildProcessExit);
    EXPECT_EQ(ret, NCP_ERR_INTERNAL);
    ret = OH_Ability_UnregisterNativeChildProcessExitCallback(ChildProcessCapiTest::OnNativeChildProcessExit);
    EXPECT_EQ(ret, NCP_ERR_CALLBACK_NOT_EXIST);
    GTEST_LOG_(INFO) << "OH_Ability_RegisterNativeChildProcessExitCallback_001 end";
}

/**
 * @tc.number: OH_Ability_RegisterNativeChildProcessExitCallback_002
 * @tc.desc: Test API OH_Ability_RegisterNativeChildProcessExitCallback_002 works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_RegisterNativeChildProcessExitCallback_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "OH_Ability_RegisterNativeChildProcessExitCallback_002 begin";
    auto localCallbackStub = sptr<NativeChildCallback>::MakeSptr(nullptr);
    ::SetGlobalNativeChildCallbackStub(localCallbackStub);
    auto ret = OH_Ability_RegisterNativeChildProcessExitCallback(ChildProcessCapiTest::OnNativeChildProcessExit);
    EXPECT_EQ(ret, NCP_NO_ERROR);
    ret = OH_Ability_RegisterNativeChildProcessExitCallback(ChildProcessCapiTest::OnNativeChildProcessExit);
    EXPECT_EQ(ret, NCP_NO_ERROR);
    int32_t pid = 111;
    int32_t signal = 9;
    localCallbackStub->OnNativeChildExit(pid, signal);
    EXPECT_EQ(ret, NCP_NO_ERROR);
    ret = OH_Ability_UnregisterNativeChildProcessExitCallback(ChildProcessCapiTest::OnNativeChildProcessExit);
    EXPECT_EQ(ret, NCP_ERR_INTERNAL);
    ::SetGlobalNativeChildCallbackStub(nullptr);
    GTEST_LOG_(INFO) << "OH_Ability_RegisterNativeChildProcessExitCallback_002 end";
}

/**
 * @tc.number: OH_Ability_UnregisterNativeChildProcessExitCallback_001
 * @tc.desc: Test API OH_Ability_UnregisterNativeChildProcessExitCallback_001 works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_UnregisterNativeChildProcessExitCallback_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "OH_Ability_UnregisterNativeChildProcessExitCallback_001 begin";
    auto ret = OH_Ability_UnregisterNativeChildProcessExitCallback(nullptr);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);
    ret = OH_Ability_UnregisterNativeChildProcessExitCallback(ChildProcessCapiTest::OnNativeChildProcessExit);
    EXPECT_EQ(ret, NCP_ERR_CALLBACK_NOT_EXIST);
    GTEST_LOG_(INFO) << "OH_Ability_UnregisterNativeChildProcessExitCallback_001 end";
}

/**
 * @tc.number: OH_Ability_UnregisterNativeChildProcessExitCallback_002
 * @tc.desc: Test API OH_Ability_UnregisterNativeChildProcessExitCallback_002 works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_UnregisterNativeChildProcessExitCallback_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "OH_Ability_UnregisterNativeChildProcessExitCallback_002 begin";
    auto localCallbackStub = sptr<NativeChildCallback>::MakeSptr(nullptr);
    ::SetGlobalNativeChildCallbackStub(localCallbackStub);
    auto ret = OH_Ability_RegisterNativeChildProcessExitCallback(ChildProcessCapiTest::OnNativeChildProcessExit);
    EXPECT_EQ(ret, NCP_NO_ERROR);
    ret = OH_Ability_UnregisterNativeChildProcessExitCallback(ChildProcessCapiTest::OnNativeChildProcessExit1);
    EXPECT_EQ(ret, NCP_ERR_CALLBACK_NOT_EXIST);
    ::SetGlobalNativeChildCallbackStub(nullptr);
    GTEST_LOG_(INFO) << "OH_Ability_UnregisterNativeChildProcessExitCallback_002 end";
}

/**
 * @tc.number: OH_Ability_GetCurrentChildProcessArgs_001
 * @tc.desc: Test API OH_Ability_GetCurrentChildProcessArgs_001 works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_GetCurrentChildProcessArgs_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "OH_Ability_GetCurrentChildProcessArgs_001 begin";
    EXPECT_EQ(OH_Ability_GetCurrentChildProcessArgs(), nullptr);
    NativeChildProcess_Args args = { 0 };
    ChildProcessArgsManager::GetInstance().SetChildProcessArgs(args);
    EXPECT_NE(OH_Ability_GetCurrentChildProcessArgs(), nullptr);
    GTEST_LOG_(INFO) << "OH_Ability_GetCurrentChildProcessArgs_001 end";
}

/**
 * @tc.number: OH_Ability_KillChildProcess_001
 * @tc.desc: Test API OH_Ability_KillChildProcess_001 works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_KillChildProcess_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "OH_Ability_KillChildProcess_001 begin";
    int32_t pid = -1;
    auto ret = OH_Ability_KillChildProcess(pid);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PID);
    GTEST_LOG_(INFO) << "OH_Ability_KillChildProcess_001 end";
}

/**
 * @tc.number: OH_Ability_AcquireChildProcessInfos_001
 * @tc.desc: Test API OH_Ability_AcquireChildProcessInfos with invalid params
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_AcquireChildProcessInfos_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "OH_Ability_AcquireChildProcessInfos_001 begin";
    uint32_t count = 0;
    auto ret = OH_Ability_AcquireChildProcessInfos(nullptr, &count);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);

    OH_AbilityRuntime_ChildProcessInfosHandle infos = nullptr;
    ret = OH_Ability_AcquireChildProcessInfos(&infos, nullptr);
    EXPECT_EQ(ret, NCP_ERR_INVALID_PARAM);
    GTEST_LOG_(INFO) << "OH_Ability_AcquireChildProcessInfos_001 end";
}

/**
 * @tc.number: OH_Ability_AcquireChildProcessInfos_002
 * @tc.desc: Test API OH_Ability_AcquireChildProcessInfos normal call and release
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_AcquireChildProcessInfos_002, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "OH_Ability_AcquireChildProcessInfos_002 begin";
    OH_AbilityRuntime_ChildProcessInfosHandle infos = nullptr;
    uint32_t count = 0;
    auto ret = OH_Ability_AcquireChildProcessInfos(&infos, &count);
    if (ret == NCP_NO_ERROR) {
        EXPECT_EQ(count, 0);
        EXPECT_EQ(infos, nullptr);
    } else {
        EXPECT_EQ(ret, NCP_ERR_INTERNAL);
    }
    OH_AbilityRuntime_ReleaseChildProcessInfos(&infos);
    EXPECT_EQ(infos, nullptr);
    GTEST_LOG_(INFO) << "OH_Ability_AcquireChildProcessInfos_002 end";
}

/**
 * @tc.number: OH_Ability_AcquireChildProcessInfos_003
 * @tc.desc: Test child process info accessor functions with nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessCapiTest, OH_Ability_AcquireChildProcessInfos_003, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "OH_Ability_AcquireChildProcessInfos_003 begin";
    int32_t pid = 0;
    auto ret = OH_AbilityRuntime_ChildProcessInfo_GetPid(nullptr, &pid);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    int32_t parentPid = 0;
    ret = OH_AbilityRuntime_ChildProcessInfo_GetParentPid(nullptr, &parentPid);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    char processName[32] = {0};
    uint32_t requiredSize = 0;
    ret = OH_AbilityRuntime_ChildProcessInfo_GetProcessName(nullptr, processName, 32, &requiredSize);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    OH_AbilityRuntime_ChildProcessInfoHandle outInfo = nullptr;
    ret = OH_AbilityRuntime_GetChildProcessInfoByIndex(nullptr, 0, &outInfo);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    OH_AbilityRuntime_ChildProcessInfosHandle infos = nullptr;
    ret = OH_AbilityRuntime_GetChildProcessInfoByIndex(infos, 0, &outInfo);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ret = OH_AbilityRuntime_GetChildProcessInfoByIndex(infos, 0, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    OH_AbilityRuntime_ReleaseChildProcessInfos(nullptr);
    OH_AbilityRuntime_ReleaseChildProcessInfos(&infos);
    EXPECT_EQ(infos, nullptr);
    GTEST_LOG_(INFO) << "OH_Ability_AcquireChildProcessInfos_003 end";
}
}  // namespace AbilityRuntime
}  // namespace OHOS
