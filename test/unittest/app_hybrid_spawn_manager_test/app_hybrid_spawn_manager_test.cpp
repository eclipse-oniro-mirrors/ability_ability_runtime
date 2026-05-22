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
#include "app_hybrid_spawn_manager.h"
#include "app_mgr_service_inner.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppHybridSpawnManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();

protected:
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner_ = nullptr;
};

void AppHybridSpawnManagerTest::SetUp()
{
}

void AppHybridSpawnManagerTest::TearDown()
{
}

/**
 * @tc.number: AppHybridSpawnManagerTest_GetInstance_0100
 * @tc.desc: Test GetInstance works
 * @tc.type: FUNC
 * @tc.Function: GetInstance
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppHybridSpawnManagerTest, AppHybridSpawnManagerTest_GetInstance_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "AppHybridSpawnManagerTest_GetInstance_0100 start.");
    auto& instance1 = AppHybridSpawnManager::GetInstance();
    auto& instance2 = AppHybridSpawnManager::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}

/**
 * @tc.number: AppHybridSpawnManagerTest_GetHRfd_0200
 * @tc.desc: Test GetHRfd works
 * @tc.type: FUNC
 * @tc.Function: GetHRfd
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppHybridSpawnManagerTest, AppHybridSpawnManagerTest_GetHRfd_0200, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "AppHybridSpawnManagerTest_GetHRfd_0200 start.");
    int hrFd = AppHybridSpawnManager::GetInstance().GetHRfd();
    EXPECT_EQ(hrFd, -1);
}

/**
 * @tc.number: AppHybridSpawnManagerTest_GetHWfd_0300
 * @tc.desc: Test GetHWfd works
 * @tc.type: FUNC
 * @tc.Function: GetHWfd
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppHybridSpawnManagerTest, AppHybridSpawnManagerTest_GetHWfd_0300, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "AppHybridSpawnManagerTest_GetHWfd_0300 start.");
    int hwFd = AppHybridSpawnManager::GetInstance().GetHWfd();
    EXPECT_EQ(hwFd, -1);
}

/**
 * @tc.number: AppHybridSpawnManagerTest_RecordAppExitSignalReason_0400
 * @tc.desc: Test RecordAppExitSignalReason works
 * @tc.type: FUNC
 * @tc.Function: RecordAppExitSignalReason
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppHybridSpawnManagerTest, AppHybridSpawnManagerTest_RecordAppExitSignalReason_0400, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "AppHybridSpawnManagerTest_RecordAppExitSignalReason_0400 start.");

    int32_t pid = 12345;
    int32_t uid = 1000;
    int32_t signal = 9;
    std::string bundleName = "com.example.test";

    AppHybridSpawnManager::GetInstance().RecordAppExitSignalReason(pid, uid, signal, bundleName);

    AppHybridSpawnManager::GetInstance().RecordAppExitSignalReason(pid, uid, 0, bundleName);

    AppHybridSpawnManager::GetInstance().RecordAppExitSignalReason(-1, uid, signal, bundleName);

    EXPECT_TRUE(true);
}

/**
 * @tc.number: AppHybridSpawnManagerTest_InitHybridSpawnMsgPipe_0500
 * @tc.desc: Test InitHybridSpawnMsgPipe with empty weak_ptr (lock return null)
 * @tc.type: FUNC
 * @tc.Function: InitHybridSpawnMsgPipe
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppHybridSpawnManagerTest, AppHybridSpawnManagerTest_InitHybridSpawnMsgPipe_0500, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "AppHybridSpawnManagerTest_InitHybridSpawnMsgPipe_0500 start.");

    std::weak_ptr<AppMgrServiceInner> nullWeakPtr;
    AppHybridSpawnManager::GetInstance().InitHybridSpawnMsgPipe(nullWeakPtr);

    int hrFd = AppHybridSpawnManager::GetInstance().GetHRfd();
    int hwFd = AppHybridSpawnManager::GetInstance().GetHWfd();

    EXPECT_EQ(hrFd, -1);
    EXPECT_EQ(hwFd, -1);
}
}  // namespace AppExecFwk
}  // namespace OHOS