/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define private public
#include "dump_runtime_helper.h"
#undef private
#include "parameters.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class DumpRuntimeHelperTest : public testing::Test {
public:
    DumpRuntimeHelperTest()
    {}
    ~DumpRuntimeHelperTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DumpRuntimeHelperTest::SetUpTestCase(void)
{}

void DumpRuntimeHelperTest::TearDownTestCase(void)
{}

void DumpRuntimeHelperTest::SetUp(void)
{
    OHOS::system::SetParameter("hiview.oomdump.switch", "");
}

void DumpRuntimeHelperTest::TearDown(void)
{
    OHOS::system::SetParameter("hiview.oomdump.switch", "");
}

/**
 * @tc.number: DumpJsHeap_0100
 * @tc.name: DumpJsHeap
 * @tc.desc: Test whether DumpJsHeap and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTest, DumpJsHeap_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpJsHeap_0100 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.pid = 1;
    info.tid = 1;
    info.needGc = false;
    info.needSnapshot = true;
    info.needLeakobj = false;
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpJsHeap(info);
    EXPECT_NE(application, nullptr);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpJsHeap_0100 end";
}

/**
 * @tc.number: CheckOomdumpSwitch_0100
 * @tc.name: CheckOomdumpSwitch
 * @tc.desc: Test the function of CheckOomdumpSwitch.
 */
HWTEST_F(DumpRuntimeHelperTest, CheckOomdumpSwitch_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CheckOomdumpSwitch_0100 start";
    EXPECT_TRUE(DumpRuntimeHelper::CheckOomdumpSwitch());
    OHOS::system::SetParameter("hiview.oomdump.switch", "disable");
    EXPECT_FALSE(DumpRuntimeHelper::CheckOomdumpSwitch());
    OHOS::system::SetParameter("hiview.oomdump.switch", "unknown");
    EXPECT_TRUE(DumpRuntimeHelper::CheckOomdumpSwitch());
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CheckOomdumpSwitch_0100 end";
}

class CreateDirTest : public ::testing::Test {
protected:
    CreateDirTest()
    {
    }

    ~CreateDirTest()
    {
    }

    void SetUp() override
    {
        std::string testDir = "/data/local/tmp/test_dir";
        RemoveTestDir(testDir);
    }

    void TearDown() override
    {
        std::string testDir = "/data/local/tmp/test_dir";
        RemoveTestDir(testDir);
    }

    void CreateTestDir(const std::string& path)
    {
        if (DumpRuntimeHelper::IsFileExists(path)) {
            return;
        }
        constexpr mode_t defaultLogDirMode = 0770;
        if (mkdir(path.c_str(), defaultLogDirMode) != 0) {
            perror("mkdir failed");
        }
    }

    void RemoveTestDir(const std::string& path)
    {
        if (DumpRuntimeHelper::IsFileExists(path)) {
            if (rmdir(path.c_str()) != 0) {
                perror("rmdir failed");
            }
        }
    }
};

/**
 * @tc.number: CreateDir_0100
 * @tc.name: CreateDir_FileExists
 * @tc.desc: Test CreateDir when directory already exists.
 */
HWTEST_F(CreateDirTest, CreateDir_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CreateDirTest CreateDir_0100 start";

    std::string testDir = "/data/local/tmp/test_dir";
    CreateTestDir(testDir);
    EXPECT_TRUE(DumpRuntimeHelper::IsFileExists(testDir));
    EXPECT_TRUE(DumpRuntimeHelper::CreateDir(testDir));

    GTEST_LOG_(INFO) << "CreateDirTest CreateDir_0100 end";
}

/**
 * @tc.number: CreateDir_0200
 * @tc.name: CreateDir_CreateSuccess
 * @tc.desc: Test CreateDir when directory is created successfully.
 */
HWTEST_F(CreateDirTest, CreateDir_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CreateDirTest CreateDir_0200 start";

    std::string testDir = "/data/local/tmp/test_dir";
    RemoveTestDir(testDir);
    EXPECT_FALSE(DumpRuntimeHelper::IsFileExists(testDir));
    EXPECT_TRUE(DumpRuntimeHelper::CreateDir(testDir));
    EXPECT_TRUE(DumpRuntimeHelper::IsFileExists(testDir));
    RemoveTestDir(testDir);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CreateDir_0200 end";
}
}
}
