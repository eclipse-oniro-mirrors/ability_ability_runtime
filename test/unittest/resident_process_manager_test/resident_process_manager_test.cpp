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

#define private public
#define protected public
#include "resident_process_manager.h"
#undef private
#undef protected
#include "ability_manager_service.h"
#include "ability_resident_process_rdb.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {

class ResidentProcessManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ResidentProcessManagerTest::SetUpTestCase(void)
{}
void ResidentProcessManagerTest::TearDownTestCase(void)
{}
void ResidentProcessManagerTest::SetUp()
{}
void ResidentProcessManagerTest::TearDown()
{}

/*
 * Feature: ResidentProcessManager
 * Function: StartResidentProcessWithMainElement
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager StartResidentProcessWithMainElement
 * EnvConditions: NA
 * CaseDescription: Verify StartResidentProcessWithMainElement
 */
HWTEST_F(ResidentProcessManagerTest, StartResidentProcessWithMainElement_001, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    std::vector<BundleInfo> bundleInfos;
    BundleInfo bundleInfo1;
    BundleInfo bundleInfo2;
    HapModuleInfo hapModuleInfo1;
    HapModuleInfo hapModuleInfo2;
    hapModuleInfo1.isModuleJson = false;
    hapModuleInfo1.mainAbility = "";
    hapModuleInfo2.isModuleJson = true;
    hapModuleInfo2.mainElementName = "mainElementName";
    hapModuleInfo2.process = "process";
    bundleInfo1.isKeepAlive = true;
    bundleInfo1.applicationInfo.process = "";
    bundleInfo2.isKeepAlive = true;
    bundleInfo2.applicationInfo.process = "process";
    bundleInfo2.hapModuleInfos.emplace_back(hapModuleInfo1);
    bundleInfo2.hapModuleInfos.emplace_back(hapModuleInfo2);
    bundleInfos.emplace_back(bundleInfo1);
    bundleInfos.emplace_back(bundleInfo2);
    manager->StartResidentProcessWithMainElement(bundleInfos, 0);
    EXPECT_TRUE(manager != nullptr);
}

/*
 * Feature: ResidentProcessManager
 * Function: SetResidentProcessEnabled
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager SetResidentProcessEnabled
 * EnvConditions: NA
 * CaseDescription: Verify SetResidentProcessEnabled
 */
HWTEST_F(ResidentProcessManagerTest, SetResidentProcessEnable_001, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    ASSERT_NE(manager, nullptr);
    std::string bundleName = "com.example.resident.process";
    std::string callerName;
    EXPECT_EQ(manager->SetResidentProcessEnabled(bundleName, callerName, false), INVALID_PARAMETERS_ERR);
}

/*
 * Feature: ResidentProcessManager
 * Function: ResidentProcessManager
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager ResidentProcessManager
 * EnvConditions: NA
 * CaseDescription: Verify ResidentProcessManager
 */
HWTEST_F(ResidentProcessManagerTest, SetResidentProcessEnable_002, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    ASSERT_NE(manager, nullptr);

    std::string bundleName = "com.example.resident.process";
    std::string callerName = "resident.process.manager.test";
    EXPECT_EQ(manager->SetResidentProcessEnabled(bundleName, callerName, false), ERR_NO_RESIDENT_PERMISSION);
}

/*
 * Feature: ResidentProcessManager
 * Function: SetResidentProcessEnabledForSA
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager SetResidentProcessEnabledForSA
 * EnvConditions: NA
 * CaseDescription: Verify SetResidentProcessEnabledForSA with invalid input
 */
HWTEST_F(ResidentProcessManagerTest, SetResidentProcessEnableForSA_001, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    ASSERT_NE(manager, nullptr);
    std::string bundleName = "com.example.resident.process";
    EXPECT_EQ(manager->SetResidentProcessEnabledForSA("", 1234, false), INVALID_PARAMETERS_ERR);
    EXPECT_EQ(manager->SetResidentProcessEnabledForSA(bundleName, -1, false), INVALID_PARAMETERS_ERR);
}

/*
 * Feature: ResidentProcessManager
 * Function: SetResidentProcessEnabledForSA
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager SetResidentProcessEnabledForSA
 * EnvConditions: NA
 * CaseDescription: Verify SetResidentProcessEnabledForSA rejected when sa uid is not configured
 */
HWTEST_F(ResidentProcessManagerTest, SetResidentProcessEnableForSA_002, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    ASSERT_NE(manager, nullptr);
    std::string bundleName = "com.example.resident.process";
    EXPECT_CALL(AmsResidentProcessRdb::GetInstance(), VerifySaConfigurationPermissions(bundleName, 1234))
        .Times(1)
        .WillOnce(Return(RdbResult::Rdb_Permissions_Err));
    EXPECT_CALL(AmsResidentProcessRdb::GetInstance(), GetSaResidentProcessRawData(bundleName, 1234))
        .Times(1)
        .WillOnce(Return(RdbResult::Rdb_Parameter_Err));
    EXPECT_EQ(manager->SetResidentProcessEnabledForSA(bundleName, 1234, false), ERR_NO_RESIDENT_PERMISSION);
}

/*
 * Feature: ResidentProcessManager
 * Function: PutResidentAbility
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager PutResidentAbility
 * EnvConditions: NA
 * CaseDescription: Verify PutResidentAbility
 */
HWTEST_F(ResidentProcessManagerTest, PutResidentAbility_001, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    ASSERT_NE(manager, nullptr);

    std::string bundleName = "com.example.resident.process";
    std::string callerName = "resident.process.manager.test";
    auto residentId = manager->PutResidentAbility(bundleName, callerName, 0);
    EXPECT_GE(residentId, 0);
    EXPECT_TRUE(manager->IsResidentAbility(bundleName, callerName, 0));
    manager->RemoveResidentAbility(residentId);
    EXPECT_TRUE(manager->residentAbilityInfos_.empty());
}

/*
 * Feature: ResidentProcessManager
 * Function: AddFailedResidentAbility
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager AddFailedResidentAbility
 * EnvConditions: NA
 * CaseDescription: Verify AddFailedResidentAbility
 */
HWTEST_F(ResidentProcessManagerTest, AddFailedResidentAbility_001, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    ASSERT_NE(manager, nullptr);

    std::string bundleName = "com.example.resident.process";
    std::string callerName = "resident.process.manager.test";
    manager->unlockedAfterBoot_ = true;
    manager->AddFailedResidentAbility(bundleName, callerName, 0);
    EXPECT_TRUE(manager->failedResidentAbilityInfos_.empty());
    manager->unlockedAfterBoot_ = false;
    manager->AddFailedResidentAbility(bundleName, callerName, 0);
    EXPECT_EQ(manager->failedResidentAbilityInfos_.size(), 1);

    manager->StartFailedResidentAbilities();
    EXPECT_TRUE(manager->unlockedAfterBoot_);
    EXPECT_TRUE(manager->failedResidentAbilityInfos_.empty());
}

/*
 * Feature: ResidentProcessManager
 * Function: StartResidentProcessWithMainElementPerBundle
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager StartResidentProcessWithMainElementPerBundle
 * EnvConditions: NA
 * CaseDescription: Verify StartResidentProcessWithMainElementPerBundle
 */
HWTEST_F(ResidentProcessManagerTest, StartResidentProcessWithMainElementPerBundle_001, TestSize.Level1)
{
    std::shared_ptr<ResidentProcessManager> manager = std::make_shared<ResidentProcessManager>();
    AppExecFwk::BundleInfo bundleInfo;
    size_t index = 1;
    std::set<uint32_t> needEraseIndexSet;
    int32_t userId = 1;
    EXPECT_EQ((int)needEraseIndexSet.size(), 0);
    manager->StartResidentProcessWithMainElementPerBundle(bundleInfo, index, needEraseIndexSet, userId);
    EXPECT_EQ((int)needEraseIndexSet.size(), 1);
}

/*
 * Feature: ResidentProcessManager
 * Function: StartResidentProcessWithMainElementPerBundle
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager StartResidentProcessWithMainElementPerBundle
 * EnvConditions: NA
 * CaseDescription: Verify StartResidentProcessWithMainElementPerBundle
 */
HWTEST_F(ResidentProcessManagerTest, StartResidentProcessWithMainElementPerBundle_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartResidentProcessWithMainElementPerBundle_002 start";

    std::shared_ptr<ResidentProcessManager> manager = std::make_shared<ResidentProcessManager>();
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.isKeepAlive = false;
    bundleInfo.name = "com.example.test";
    size_t index = 1;
    std::set<uint32_t> needEraseIndexSet;
    int32_t userId = 0;

    bool keepAliveEnable = true;
    EXPECT_CALL(AmsResidentProcessRdb::GetInstance(), GetResidentProcessEnable(_, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(keepAliveEnable), Return(RdbResult::Rdb_OK)));

    EXPECT_CALL(AmsResidentProcessRdb::GetInstance(), RemoveData(_))
        .Times(1)
        .WillOnce(Return(RdbResult::Rdb_OK));

    manager->StartResidentProcessWithMainElementPerBundle(bundleInfo, index, needEraseIndexSet, userId);


    EXPECT_EQ(needEraseIndexSet.size(), 1);

    GTEST_LOG_(INFO) << "StartResidentProcessWithMainElementPerBundle_002 end";
}

/*
 * Feature: ResidentProcessManager
 * Function: PutResidentAbility
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager PutResidentAbility
 * EnvConditions: NA
 * CaseDescription: Verify PutResidentAbility
 */
HWTEST_F(ResidentProcessManagerTest, PutResidentAbility_002, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    bool ret = manager->IsResidentAbility("", "", 0);
    EXPECT_FALSE(ret);
}

/*
 * Feature: ResidentProcessManager
 * Function: IsResidentAbility
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager IsResidentAbility
 * EnvConditions: NA
 * CaseDescription: Verify IsResidentAbility
 */
HWTEST_F(ResidentProcessManagerTest, IsResidentAbility_001, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    ASSERT_NE(manager, nullptr);
    std::string bundleName = "com.example.resident.process";
    std::string callerName = "resident.process.manager.test";
    EXPECT_FALSE(manager->IsResidentAbility(bundleName, callerName, 0));
}

/*
 * Feature: ResidentProcessManager
 * Function: GetResidentBundleInfosForUser
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager GetResidentBundleInfosForUser
 * EnvConditions: NA
 * CaseDescription: Verify GetResidentBundleInfosForUser
 */
HWTEST_F(ResidentProcessManagerTest, GetResidentBundleInfosForUser_001, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    ASSERT_NE(manager, nullptr);
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isModuleJson = false;
    hapModuleInfo.mainAbility = "";
    bundleInfo.isKeepAlive = true;
    bundleInfo.applicationInfo.process = "";
    std::vector<BundleInfo> bundleInfos;
    bundleInfos.push_back(bundleInfo);
    EXPECT_TRUE(manager->GetResidentBundleInfosForUser(bundleInfos, 0));
}

/*
 * Feature: ResidentProcessManager
 * Function: StartFailedResidentAbilities
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager StartFailedResidentAbilities
 * EnvConditions: NA
 * CaseDescription: Verify StartFailedResidentAbilities
 */
HWTEST_F(ResidentProcessManagerTest, StartFailedResidentAbilities_001, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    ASSERT_NE(manager, nullptr);
    manager->unlockedAfterBoot_ = false;
    manager->StartFailedResidentAbilities();
    EXPECT_TRUE(manager->unlockedAfterBoot_);
}

/*
 * Feature: ResidentProcessManager
 * Function: UpdateResidentProcessesStatus
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager UpdateResidentProcessesStatus
 * EnvConditions: NA
 * CaseDescription: Verify UpdateResidentProcessesStatus
 */
HWTEST_F(ResidentProcessManagerTest, UpdateResidentProcessesStatus_001, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    ASSERT_NE(manager, nullptr);
    std::string bundleName = "";
    bool localEnable = false;
    bool updateEnable = false;
    manager->UpdateResidentProcessesStatus(bundleName, localEnable, updateEnable);
    EXPECT_TRUE(bundleName.empty());
}

/*
 * Feature: ResidentProcessManager
 * Function: UpdateResidentProcessesStatus
 * SubFunction: NA
 * FunctionPoints:ResidentProcessManager UpdateResidentProcessesStatus
 * EnvConditions: NA
 * CaseDescription: Verify UpdateResidentProcessesStatus
 */
HWTEST_F(ResidentProcessManagerTest, UpdateResidentProcessesStatus_002, TestSize.Level1)
{
    auto manager = std::make_shared<ResidentProcessManager>();
    ASSERT_NE(manager, nullptr);
    std::string bundleName = "com.ohos.demo";
    bool localEnable = false;
    bool updateEnable = false;
    manager->UpdateResidentProcessesStatus(bundleName, localEnable, updateEnable);
    EXPECT_FALSE(bundleName.empty());
}
}  // namespace AAFwk
}  // namespace OHOS
