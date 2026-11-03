/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "hilog_tag_wrapper.h"
#define private public
#include "app_spawn_client.h"
#undef private
#include "app_spawn_client.cpp"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppSpawnClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppSpawnClientTest::SetUpTestCase(void)
{}

void AppSpawnClientTest::TearDownTestCase(void)
{}

void AppSpawnClientTest::SetUp()
{}

void AppSpawnClientTest::TearDown()
{}

/**
 * @tc.name: PreStartNWebSpawnProcess_001
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, PreStartNWebSpawnProcess_001, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    int ret = asc->PreStartNWebSpawnProcess();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: PreStartNWebSpawnProcessImpl_002
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, PreStartNWebSpawnProcessImpl_002, TestSize.Level2)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    asc->OpenConnection();
    int ret = asc->PreStartNWebSpawnProcess();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: AppSpawnClient_001
 * @tc.desc: new AppSpawnClient object
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, AppSpawnClient_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppSpawnClient_001 start");
    const char* serviceName = "appspawn";
    auto asc = std::make_shared<AppSpawnClient>(serviceName);
    auto ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppSpawnClient_001 end");
}

/**
 * @tc.name: AppSpawnClient_002
 * @tc.desc: new AppSpawnClient object
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, AppSpawnClient_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppSpawnClient_002 start");
    const char* serviceName = "cjappspawn";
    auto asc = std::make_shared<AppSpawnClient>(serviceName);
    auto ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppSpawnClient_002 end");
}

/**
 * @tc.name: AppSpawnClient_003
 * @tc.desc: new AppSpawnClient object
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, AppSpawnClient_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppSpawnClient_003 start");
    const char* serviceName = "nwebRestart";
    auto asc = std::make_shared<AppSpawnClient>(serviceName);
    auto ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppSpawnClient_003 end");
}

/**
 * @tc.name: AppSpawnClient_004
 * @tc.desc: new AppSpawnClient object
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, AppSpawnClient_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppSpawnClient_004 start");
    const char* serviceName = "nativespawn";
    auto asc = std::make_shared<AppSpawnClient>(serviceName);
    auto ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppSpawnClient_004 end");
}

/**
 * @tc.name: AppSpawnClient_005
 * @tc.desc: new AppSpawnClient object
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, AppSpawnClient_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppSpawnClient_005 start");
    const char* serviceName = "nwebspawn";
    auto asc = std::make_shared<AppSpawnClient>(serviceName);
    auto ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppSpawnClient_005 end");
}

/**
 * @tc.name: OpenConnection_001
 * @tc.desc: appspawn OpenConnection
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, OpenConnection_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenConnection_001 start");
    auto asc = std::make_shared<AppSpawnClient>(true);
    auto ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "OpenConnection_001 end");
}

/**
 * @tc.name: OpenConnection_002
 * @tc.desc: appspawn OpenConnection
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, OpenConnection_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenConnection_001 start");
    auto asc = std::make_shared<AppSpawnClient>(false);
    auto ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "OpenConnection_001 end");
}

/**
 * @tc.name: CloseConnection_001
 * @tc.desc: appspawn CloseConnection
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, CloseConnection_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenConnection_001 start");
    auto asc = std::make_shared<AppSpawnClient>(true);
    auto ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "OpenConnection_001 end");
}

/**
 * @tc.name: CloseConnection_002
 * @tc.desc: appspawn CloseConnection
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, CloseConnection_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenConnection_001 start");
    auto asc = std::make_shared<AppSpawnClient>(false);
    auto ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "OpenConnection_001 end");
}

/**
 * @tc.name: SetDacInfo_001
 * @tc.desc: appspawn SetDacInfo
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, SetDacInfo_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetDacInfo_001 start");
    AppSpawnStartMsg startMsg = {0};
    startMsg.uid = 1001;
    startMsg.gid = 2001;
    startMsg.gids = {1001, 1002, 1003};
    std::string procName("test_proc_name");
    startMsg.procName = procName;
    std::string permission1("permission_for_test_1");
    std::string permission2("permission_for_test_2");
    std::string permission3("permission_for_test_3");
    startMsg.permissions.insert(permission1);
    startMsg.permissions.insert(permission2);
    startMsg.permissions.insert(permission3);
    startMsg.flags = 0x1000;
    startMsg.strictMode = false;
    startMsg.childProcessType = 1;
    std::string provisionType("test_provisionType");
    startMsg.provisionType = provisionType;
    std::string processType("test_processType");
    startMsg.processType = processType;
    startMsg.maxChildProcess = 1;
    std::string extensionSandboxPath("test_extensionSandboxPath");
    startMsg.extensionSandboxPath = extensionSandboxPath;

    auto asc = std::make_shared<AppSpawnClient>(false);
    int32_t ret = 0;
    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    ret = AppSpawnReqMsgCreate(static_cast<AppSpawnMsgType>(startMsg.code), startMsg.procName.c_str(), &reqHandle);
    EXPECT_EQ(ret, ERR_OK);
    ret = asc->SetDacInfo(startMsg, reqHandle);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "SetDacInfo_001 end");
}

/**
 * @tc.name: SetMountPermission_001
 * @tc.desc: appspawn SetMountPermission
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, SetMountPermission_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetMountPermission_001 start");
    AppSpawnStartMsg startMsg = {0};
    startMsg.uid = 1001;
    startMsg.gid = 2001;
    startMsg.gids = {1001, 1002, 1003};
    std::string procName("test_proc_name");
    startMsg.procName = procName;
    std::string permission1("permission_for_test_1");
    std::string permission2("permission_for_test_2");
    std::string permission3("permission_for_test_3");
    startMsg.permissions.insert(permission1);
    startMsg.permissions.insert(permission2);
    startMsg.permissions.insert(permission3);
    startMsg.flags = 0x1000;
    startMsg.strictMode = false;
    startMsg.childProcessType = 1;
    std::string provisionType("test_provisionType");
    startMsg.provisionType = provisionType;
    std::string processType("test_processType");
    startMsg.processType = processType;
    startMsg.maxChildProcess = 1;
    std::string extensionSandboxPath("test_extensionSandboxPath");
    startMsg.extensionSandboxPath = extensionSandboxPath;

    auto asc = std::make_shared<AppSpawnClient>(false);
    int32_t ret = 0;
    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    ret = AppSpawnReqMsgCreate(static_cast<AppSpawnMsgType>(startMsg.code), startMsg.procName.c_str(), &reqHandle);
    EXPECT_EQ(ret, ERR_OK);
    ret = asc->SetMountPermission(startMsg, reqHandle);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "SetMountPermission_001 end");
}

/**
 * @tc.name: SetStartFlags_001
 * @tc.desc: appspawn SetStartFlags
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, SetStartFlags_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetStartFlags_001 start");
    AppSpawnStartMsg startMsg = {0};
    startMsg.uid = 1001;
    startMsg.gid = 2001;
    startMsg.gids = {1001, 1002, 1003};
    std::string procName("test_proc_name");
    startMsg.procName = procName;
    std::string permission1("permission_for_test_1");
    std::string permission2("permission_for_test_2");
    std::string permission3("permission_for_test_3");
    startMsg.permissions.insert(permission1);
    startMsg.permissions.insert(permission2);
    startMsg.permissions.insert(permission3);
    startMsg.flags = 0x1000;
    startMsg.strictMode = false;
    startMsg.childProcessType = 1;
    std::string provisionType("test_provisionType");
    startMsg.provisionType = provisionType;
    std::string processType("test_processType");
    startMsg.processType = processType;
    startMsg.maxChildProcess = 1;
    std::string extensionSandboxPath("test_extensionSandboxPath");
    startMsg.extensionSandboxPath = extensionSandboxPath;

    auto asc = std::make_shared<AppSpawnClient>(false);
    int32_t ret = 0;
    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    ret = AppSpawnReqMsgCreate(static_cast<AppSpawnMsgType>(startMsg.code), startMsg.procName.c_str(), &reqHandle);
    EXPECT_EQ(ret, ERR_OK);
    ret = asc->SetStartFlags(startMsg, reqHandle);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "SetStartFlags_001 end");
}


/**
 * @tc.name: AppspawnSetExtMsgMore_001
 * @tc.desc: appspawn AppspawnSetExtMsgMore
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, AppspawnSetExtMsgMore_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppspawnSetExtMsgMore_001 start");
    AppSpawnStartMsg startMsg = {0};
    startMsg.uid = 1001;
    startMsg.gid = 2001;
    startMsg.gids = {1001, 1002, 1003};
    std::string procName("test_proc_name");
    startMsg.procName = procName;
    std::string permission1("permission_for_test_1");
    std::string permission2("permission_for_test_2");
    std::string permission3("permission_for_test_3");
    startMsg.permissions.insert(permission1);
    startMsg.permissions.insert(permission2);
    startMsg.permissions.insert(permission3);
    startMsg.flags = 0x1000;
    startMsg.strictMode = false;
    startMsg.childProcessType = 1;
    std::string provisionType("test_provisionType");
    startMsg.provisionType = provisionType;
    std::string processType("test_processType");
    startMsg.processType = processType;
    startMsg.maxChildProcess = 1;
    std::string extensionSandboxPath("test_extensionSandboxPath");
    startMsg.extensionSandboxPath = extensionSandboxPath;

    auto asc = std::make_shared<AppSpawnClient>(false);
    int32_t ret = 0;
    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    ret = AppSpawnReqMsgCreate(static_cast<AppSpawnMsgType>(startMsg.code), startMsg.procName.c_str(), &reqHandle);
    EXPECT_EQ(ret, ERR_OK);
    ret = asc->AppspawnSetExtMsgMore(startMsg, reqHandle);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppspawnSetExtMsgMore_001 end");
}


/**
 * @tc.name: AppspawnCreateDefaultMsg_001
 * @tc.desc: appspawn AppspawnCreateDefaultMsg
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, AppspawnCreateDefaultMsg_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppspawnCreateDefaultMsg_001 start");
    AppSpawnStartMsg startMsg = {0};
    startMsg.uid = 1001;
    startMsg.gid = 2001;
    startMsg.gids = {1001, 1002, 1003};
    std::string procName("test_proc_name");
    startMsg.procName = procName;
    std::string apl("test_apl");
    startMsg.apl = apl;
    std::string ownerId("test_owner_id");
    startMsg.ownerId = ownerId;
    std::string bundleName("test_bundle_name");
    startMsg.bundleName = bundleName;
    std::string permission1("permission_for_test_1");
    std::string permission2("permission_for_test_2");
    std::string permission3("permission_for_test_3");
    startMsg.permissions.insert(permission1);
    startMsg.permissions.insert(permission2);
    startMsg.permissions.insert(permission3);
    startMsg.flags = 0x1000;
    startMsg.strictMode = false;
    startMsg.childProcessType = 1;
    std::string provisionType("test_provisionType");
    startMsg.provisionType = provisionType;
    std::string processType("test_processType");
    startMsg.processType = processType;
    startMsg.maxChildProcess = 1;
    std::string extensionSandboxPath("test_extensionSandboxPath");
    startMsg.extensionSandboxPath = extensionSandboxPath;

    auto asc = std::make_shared<AppSpawnClient>(false);
    int32_t ret = 0;
    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    ret = AppSpawnReqMsgCreate(static_cast<AppSpawnMsgType>(startMsg.code), startMsg.procName.c_str(), &reqHandle);
    EXPECT_EQ(ret, ERR_OK);
    ret = asc->AppspawnCreateDefaultMsg(startMsg, reqHandle);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppspawnCreateDefaultMsg_001 end");
}


/**
 * @tc.name: VerifyMsg_001
 * @tc.desc: appspawn VerifyMsg
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, VerifyMsg_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyMsg_001 start");
    AppSpawnStartMsg startMsg = {0};
    startMsg.uid = 1001;
    startMsg.gid = 2001;
    startMsg.gids = {1001, 1002, 1003};
    std::string procName("test_proc_name");
    startMsg.procName = procName;
    std::string permission1("permission_for_test_1");
    std::string permission2("permission_for_test_2");
    std::string permission3("permission_for_test_3");
    startMsg.permissions.insert(permission1);
    startMsg.permissions.insert(permission2);
    startMsg.permissions.insert(permission3);
    startMsg.flags = 0x1000;
    startMsg.strictMode = false;
    startMsg.childProcessType = 1;
    std::string provisionType("test_provisionType");
    startMsg.provisionType = provisionType;
    std::string processType("test_processType");
    startMsg.processType = processType;
    startMsg.maxChildProcess = 1;
    std::string extensionSandboxPath("test_extensionSandboxPath");
    startMsg.extensionSandboxPath = extensionSandboxPath;

    auto asc = std::make_shared<AppSpawnClient>(false);
    int32_t ret = 0;
    bool result = false;
    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    ret = AppSpawnReqMsgCreate(static_cast<AppSpawnMsgType>(startMsg.code), startMsg.procName.c_str(), &reqHandle);
    EXPECT_EQ(ret, ERR_OK);

    result = asc->VerifyMsg(startMsg);
    EXPECT_EQ(result, true);

    TAG_LOGI(AAFwkTag::TEST, "VerifyMsg_001 end");
}

/**
 * @tc.name: VerifyMsg_002
 * @tc.desc: appspawn VerifyMsg
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, VerifyMsg_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyMsg_002 start");
    AppSpawnStartMsg startMsg = {0};
    startMsg.uid = -1;
    startMsg.gid = 2001;
    startMsg.gids = {1001, 1002, 1003};
    std::string procName("test_proc_name");
    startMsg.procName = procName;
    std::string permission1("permission_for_test_1");
    std::string permission2("permission_for_test_2");
    std::string permission3("permission_for_test_3");
    startMsg.permissions.insert(permission1);
    startMsg.permissions.insert(permission2);
    startMsg.permissions.insert(permission3);
    startMsg.flags = 0x1000;
    startMsg.strictMode = false;
    startMsg.childProcessType = 1;
    std::string provisionType("test_provisionType");
    startMsg.provisionType = provisionType;
    std::string processType("test_processType");
    startMsg.processType = processType;
    startMsg.maxChildProcess = 1;
    std::string extensionSandboxPath("test_extensionSandboxPath");
    startMsg.extensionSandboxPath = extensionSandboxPath;

    auto asc = std::make_shared<AppSpawnClient>(false);
    int32_t ret = 0;
    bool result = false;
    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    ret = AppSpawnReqMsgCreate(static_cast<AppSpawnMsgType>(startMsg.code), startMsg.procName.c_str(), &reqHandle);
    EXPECT_EQ(ret, ERR_OK);

    result = asc->VerifyMsg(startMsg);
    EXPECT_EQ(result, false);

    TAG_LOGI(AAFwkTag::TEST, "VerifyMsg_002 end");
}

/**
 * @tc.name: VerifyMsg_003
 * @tc.desc: appspawn VerifyMsg
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, VerifyMsg_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyMsg_003 start");
    AppSpawnStartMsg startMsg = {0};
    startMsg.uid = 1001;
    startMsg.gid = -1;
    startMsg.gids = {1001, 1002, 1003};
    std::string procName("test_proc_name");
    startMsg.procName = procName;
    std::string permission1("permission_for_test_1");
    std::string permission2("permission_for_test_2");
    std::string permission3("permission_for_test_3");
    startMsg.permissions.insert(permission1);
    startMsg.permissions.insert(permission2);
    startMsg.permissions.insert(permission3);
    startMsg.flags = 0x1000;
    startMsg.strictMode = false;
    startMsg.childProcessType = 1;
    std::string provisionType("test_provisionType");
    startMsg.provisionType = provisionType;
    std::string processType("test_processType");
    startMsg.processType = processType;
    startMsg.maxChildProcess = 1;
    std::string extensionSandboxPath("test_extensionSandboxPath");
    startMsg.extensionSandboxPath = extensionSandboxPath;

    auto asc = std::make_shared<AppSpawnClient>(false);
    int32_t ret = 0;
    bool result = false;
    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    ret = AppSpawnReqMsgCreate(static_cast<AppSpawnMsgType>(startMsg.code), startMsg.procName.c_str(), &reqHandle);
    EXPECT_EQ(ret, ERR_OK);

    result = asc->VerifyMsg(startMsg);
    EXPECT_EQ(result, false);

    TAG_LOGI(AAFwkTag::TEST, "VerifyMsg_003 end");
}

/**
 * @tc.name: VerifyMsg_004
 * @tc.desc: appspawn VerifyMsg
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, VerifyMsg_004, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyMsg_004 start");
    AppSpawnStartMsg startMsg = {0};
    startMsg.uid = 1001;
    startMsg.gid = 2001;
    startMsg.gids = {1001, 1002, 1003, -1};
    std::string procName("test_proc_name");
    startMsg.procName = procName;
    std::string permission1("permission_for_test_1");
    std::string permission2("permission_for_test_2");
    std::string permission3("permission_for_test_3");
    startMsg.permissions.insert(permission1);
    startMsg.permissions.insert(permission2);
    startMsg.permissions.insert(permission3);
    startMsg.flags = 0x1000;
    startMsg.strictMode = false;
    startMsg.childProcessType = 1;
    std::string provisionType("test_provisionType");
    startMsg.provisionType = provisionType;
    std::string processType("test_processType");
    startMsg.processType = processType;
    startMsg.maxChildProcess = 1;
    std::string extensionSandboxPath("test_extensionSandboxPath");
    startMsg.extensionSandboxPath = extensionSandboxPath;

    auto asc = std::make_shared<AppSpawnClient>(false);
    int32_t ret = 0;
    bool result = false;
    ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    result = asc->VerifyMsg(startMsg);
    EXPECT_EQ(result, false);

    TAG_LOGI(AAFwkTag::TEST, "VerifyMsg_004 end");
}

/**
 * @tc.name: VerifyMsg_005
 * @tc.desc: appspawn VerifyMsg
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, VerifyMsg_005, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyMsg_005 start");
    AppSpawnStartMsg startMsg = {0};
    startMsg.uid = 1001;
    startMsg.gid = 2001;
    startMsg.gids = {1001, 1002};
    std::string procName("");
    startMsg.procName = procName;
    std::string permission1("permission_for_test_1");
    std::string permission2("permission_for_test_2");
    std::string permission3("permission_for_test_3");
    startMsg.permissions.insert(permission1);
    startMsg.permissions.insert(permission2);
    startMsg.permissions.insert(permission3);
    startMsg.flags = 0x1000;
    startMsg.strictMode = false;
    startMsg.childProcessType = 1;
    std::string provisionType("test_provisionType");
    startMsg.provisionType = provisionType;
    std::string processType("test_processType");
    startMsg.processType = processType;
    startMsg.maxChildProcess = 1;
    std::string extensionSandboxPath("test_extensionSandboxPath");
    startMsg.extensionSandboxPath = extensionSandboxPath;

    auto asc = std::make_shared<AppSpawnClient>(false);
    int32_t ret = 0;
    bool result = false;
    ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    result = asc->VerifyMsg(startMsg);
    EXPECT_EQ(result, false);

    TAG_LOGI(AAFwkTag::TEST, "VerifyMsg_005 end");
}

/**
 * @tc.name: VerifyMsg_006
 * @tc.desc: appspawn VerifyMsg
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientTest, VerifyMsg_006, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyMsg_006 start");
    AppSpawnStartMsg startMsg = {0};
    startMsg.uid = 1001;
    startMsg.gid = 2001;
    startMsg.gids = {1001, 1002};
    std::string procName("");
    startMsg.procName = procName;
    std::string permission1("permission_for_test_1");
    startMsg.permissions.insert(permission1);
    startMsg.flags = 0x1000;
    startMsg.strictMode = false;
    startMsg.childProcessType = 1;
    startMsg.maxChildProcess = 1;
    auto asc = std::make_shared<AppSpawnClient>(false);
    int32_t ret = 0;
    bool result = false;
    ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    result = asc->VerifyMsg(startMsg);
    EXPECT_EQ(result, false);

    startMsg.code = MSG_APP_SPAWN;
    result = asc->VerifyMsg(startMsg);
    EXPECT_EQ(result, false);
    startMsg.code = MSG_SPAWN_NATIVE_PROCESS;
    result = asc->VerifyMsg(startMsg);
    EXPECT_EQ(result, false);

    startMsg.code = MSG_GET_RENDER_TERMINATION_STATUS;
    startMsg.pid = -1;
    result = asc->VerifyMsg(startMsg);
    EXPECT_EQ(result, false);
    startMsg.pid = 1;
    result = asc->VerifyMsg(startMsg);
    EXPECT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "VerifyMsg_006 end");
}


/**
 * @tc.name: VerifyMsg_007
 * @tc.desc: appspawn VerifyMsg
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientTest, VerifyMsg_007, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyMsg_007 start");
    AppSpawnStartMsg startMsg = {0};
    startMsg.uid = 1001;
    startMsg.gid = 2001;
    startMsg.gids = {1001, 1002};
    std::string procName("");
    startMsg.procName = procName;
    std::string permission1("permission_for_test_1");
    startMsg.permissions.insert(permission1);
    startMsg.flags = 0x1000;
    startMsg.strictMode = false;
    startMsg.childProcessType = 1;
    startMsg.maxChildProcess = 1;
    auto asc = std::make_shared<AppSpawnClient>(false);
    int32_t ret = 0;
    bool result = false;
    ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    result = asc->VerifyMsg(startMsg);
    EXPECT_EQ(result, false);

    startMsg.code = MSG_APP_SPAWN;
    startMsg.uid = -1;
    result = asc->VerifyMsg(startMsg);
    EXPECT_EQ(result, false);

    startMsg.gid = -1;
    result = asc->VerifyMsg(startMsg);
    EXPECT_EQ(result, false);

    startMsg.gids = {1001, -1};
    result = asc->VerifyMsg(startMsg);
    EXPECT_EQ(result, false);

    TAG_LOGI(AAFwkTag::TEST, "VerifyMsg_007 end");
}

/**
 * @tc.name: SetChildProcessTypeStartFlag_001
 * @tc.desc: appspawn SetChildProcessTypeStartFlag
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, SetChildProcessTypeStartFlag_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetChildProcessTypeStartFlag_001 start");
    AppSpawnStartMsg startMsg = {0};
    std::string procName("test_proc_name");
    startMsg.procName = procName;
    int32_t childProcessType = 1;
    auto asc = std::make_shared<AppSpawnClient>(false);
    int32_t ret = 0;
    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    ret = AppSpawnReqMsgCreate(static_cast<AppSpawnMsgType>(startMsg.code), startMsg.procName.c_str(), &reqHandle);
    EXPECT_EQ(ret, ERR_OK);
    ret = asc->SetChildProcessTypeStartFlag(reqHandle, childProcessType);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "SetChildProcessTypeStartFlag_001 end");
}

/**
 * @tc.name: SetChildProcessTypeStartFlag_002
 * @tc.desc: appspawn SetChildProcessTypeStartFlag
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, SetChildProcessTypeStartFlag_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetChildProcessTypeStartFlag_002 start");
    AppSpawnStartMsg startMsg = {0};
    std::string procName("test_proc_name");
    startMsg.procName = procName;
    int32_t childProcessType = -1;
    auto asc = std::make_shared<AppSpawnClient>(false);
    int32_t ret = 0;
    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    ret = AppSpawnReqMsgCreate(static_cast<AppSpawnMsgType>(startMsg.code), startMsg.procName.c_str(), &reqHandle);
    EXPECT_EQ(ret, ERR_OK);
    ret = asc->SetChildProcessTypeStartFlag(reqHandle, childProcessType);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "SetChildProcessTypeStartFlag_002 end");
}

/**
 * @tc.name: SetExtMsgFds_001
 * @tc.desc: appspawn SetExtMsgFds
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, SetExtMsgFds_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetExtMsgFds_001 start");
    AppSpawnStartMsg startMsg = {0};
    std::string procName("test_proc_name");
    startMsg.procName = procName;
    startMsg.fds.emplace("test_fd1", 100);
    startMsg.fds.emplace("test_fd2", 101);
    startMsg.fds.emplace("test_fd3", 102);
    
    auto asc = std::make_shared<AppSpawnClient>(false);
    int32_t ret = 0;
    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    ret = AppSpawnReqMsgCreate(static_cast<AppSpawnMsgType>(startMsg.code), startMsg.procName.c_str(), &reqHandle);
    EXPECT_EQ(ret, ERR_OK);
    ret = asc->SetExtMsgFds(reqHandle, startMsg.fds);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "SetExtMsgFds_001 end");
}

/**
 * @tc.name: SetExtMsgFds_002
 * @tc.desc: appspawn SetExtMsgFds
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, SetExtMsgFds_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "SetExtMsgFds_002 start");
    AppSpawnStartMsg startMsg = {0};
    std::string procName("test_proc_name");
    startMsg.procName = procName;
    startMsg.fds.emplace("test_fd1", 100);
    startMsg.fds.emplace("test_fd2", 101);
    startMsg.fds.emplace("test_fd3", -1); 
    auto asc = std::make_shared<AppSpawnClient>(false);
    int32_t ret = 0;
    AppSpawnReqMsgHandle reqHandle = nullptr;
    ret = asc->OpenConnection();
    EXPECT_EQ(ret, ERR_OK);
    ret = AppSpawnReqMsgCreate(static_cast<AppSpawnMsgType>(startMsg.code), startMsg.procName.c_str(), &reqHandle);
    EXPECT_EQ(ret, ERR_OK);
    ret = asc->SetExtMsgFds(reqHandle, startMsg.fds);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "SetExtMsgFds_002 end");
}

/**
 * @tc.name: StartProcess_001
 * @tc.desc: appspawn StartProcess
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, StartProcess_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartProcess_001 start");
    AppSpawnStartMsg startMsg = {0};
    startMsg.uid = 1001;
    startMsg.gid = 2001;
    startMsg.gids = {1001, 1002, 1003};
    std::string procName("test_proc_name");
    startMsg.procName = procName;
    std::string permission1("permission_for_test_1");
    std::string permission2("permission_for_test_2");
    std::string permission3("permission_for_test_3");
    startMsg.permissions.insert(permission1);
    startMsg.permissions.insert(permission2);
    startMsg.permissions.insert(permission3);
    startMsg.flags = 0x1000;
    startMsg.strictMode = false;
    startMsg.childProcessType = 1;
    std::string provisionType("test_provisionType");
    startMsg.provisionType = provisionType;
    std::string processType("test_processType");
    startMsg.processType = processType;
    startMsg.maxChildProcess = 1;
    std::string extensionSandboxPath("test_extensionSandboxPath");
    startMsg.extensionSandboxPath = extensionSandboxPath;

    int32_t ret = 0;
    auto asc = std::make_shared<AppSpawnClient>(false);
    pid_t pid = 0;
    ret = asc->StartProcess(startMsg, pid);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartProcess_001 end");
}

/**
 * @tc.name: StartProcess_002
 * @tc.desc: appspawn StartProcess
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppSpawnClientTest, StartProcess_002, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "StartProcess_002 start");
    AppSpawnStartMsg startMsg = {0};
    startMsg.uid = 1001;
    startMsg.gid = 2001;
    startMsg.gids = {1001, 1002, 1003, -1};
    std::string procName("test_proc_name");
    startMsg.procName = procName;
    std::string permission1("permission_for_test_1");
    std::string permission2("permission_for_test_2");
    std::string permission3("permission_for_test_3");
    startMsg.permissions.insert(permission1);
    startMsg.permissions.insert(permission2);
    startMsg.permissions.insert(permission3);
    startMsg.flags = 0x1000;
    startMsg.strictMode = false;
    startMsg.childProcessType = 1;
    std::string provisionType("test_provisionType");
    startMsg.provisionType = provisionType;
    std::string processType("test_processType");
    startMsg.processType = processType;
    startMsg.maxChildProcess = 1;
    std::string extensionSandboxPath("test_extensionSandboxPath");
    startMsg.extensionSandboxPath = extensionSandboxPath;

    int32_t ret = 0;
    auto asc = std::make_shared<AppSpawnClient>(false);
    pid_t pid = 0;
    ret = asc->StartProcess(startMsg, pid);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartProcess_002 end");
}

/**
 * @tc.name: DumpJITPermissionListToJson_001
 * @tc.desc: appspawn client DumpJITPermissionListToJson_001
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientTest, DumpJITPermissionListToJson_001, TestSize.Level2)
{
    std::map<std::string, std::string> jitPermissionsMap = {
        {"ohos.permission.jit1", "1"}, {"ohos.permission.jit2", "{\"cont\": \"1\"}"}
    };
    std::string jsonJITPermissions = DumpJITPermissionListToJson(jitPermissionsMap);

    size_t pos = jsonJITPermissions.find("ohos.permission.jit1");
    ASSERT_NE(pos, std::string::npos);
    pos = jsonJITPermissions.find("ohos.permission.jit2");
    ASSERT_NE(pos, std::string::npos);
}

/**
 * @tc.name: DumpJITPermissionListToJson_002
 * @tc.desc: appspawn client DumpJITPermissionListToJson_002
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientTest, DumpJITPermissionListToJson_002, TestSize.Level2)
{
    std::map<std::string, std::string> jitPermissionsMap;
    std::string jsonJITPermissions = DumpJITPermissionListToJson(jitPermissionsMap);

    size_t pos = jsonJITPermissions.find("\"ohos.encaps.count\":0");
    ASSERT_NE(pos, std::string::npos);
}

/**
 * @tc.name: DumpJITPermissionListToJson_003
 * @tc.desc: appspawn client DumpJITPermissionListToJson_003
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientTest, DumpJITPermissionListToJson_003, TestSize.Level2)
{
    std::map<std::string, std::string> jitPermissionsMap = { { "ohos.permission.jit1", "" } };
    std::string jsonJITPermissions = DumpJITPermissionListToJson(jitPermissionsMap);

    size_t pos = jsonJITPermissions.find("ohos.permission.jit1");
    ASSERT_NE(pos, std::string::npos);
}

/**
 * @tc.name: SendAppSpawnUninstallDebugHapMsg_001
 * @tc.desc: SendAppSpawnUninstallDebugHapMsg
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientTest, SendAppSpawnUninstallDebugHapMsg_001, TestSize.Level2)
{
    auto appSpawnClient = std::make_shared<AppSpawnClient>(false);
    int32_t userId = 0;
    auto ret = appSpawnClient->SendAppSpawnUninstallDebugHapMsg(userId);
    ASSERT_EQ(ret, ERR_OK);
}
} // namespace AppExecFwk
} // namespace OHOS
