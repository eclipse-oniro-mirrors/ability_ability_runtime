/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <memory>

#include "base_extension_record.h"
#include "extension_record_manager.h"
#include "mock_ability_token.h"
#include "session_info.h"
#include "ui_extension_record.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t PERSISTENT_ID = 10;
constexpr int32_t RECORD_ID = 5;
constexpr uint64_t COMPONENT_ID = 100;
constexpr uint32_t HOST_A = 7;
constexpr uint32_t HOST_B = 8;
constexpr uint32_t HOST_C = 9;
constexpr const char *HOST_BUNDLE = "com.example.unittest";

struct MigrationFixture {
    std::shared_ptr<ExtensionRecordManager> manager;
    std::shared_ptr<AAFwk::BaseExtensionRecord> record;
    std::shared_ptr<UIExtensionRecord> wrapper;
    sptr<IRemoteObject> tokenA;
};

MigrationFixture MakeFixture()
{
    MigrationFixture result;
    result.manager = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest initial;
    initial.appInfo.bundleName = HOST_BUNDLE;
    initial.abilityInfo.name = "MainAbility";
    initial.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto session = sptr<AAFwk::SessionInfo>::MakeSptr();
    session->uiExtensionComponentId = COMPONENT_ID;
    session->persistentId = PERSISTENT_ID;
    session->hostWindowId = HOST_A;
    result.tokenA = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    session->callerToken = result.tokenA;
    initial.sessionInfo = session;
    result.record = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(initial);
    result.record->SetSessionInfo(session);
    result.record->SetUIExtensionAbilityId(RECORD_ID);
    result.wrapper = std::make_shared<UIExtensionRecord>(result.record);
    result.wrapper->hostBundleName_ = HOST_BUNDLE;
    result.manager->AddExtensionRecord(RECORD_ID, result.wrapper);
    return result;
}

AAFwk::AbilityRequest MakeMigration(uint32_t target, const sptr<IRemoteObject> &source)
{
    AAFwk::AbilityRequest request;
    request.appInfo.bundleName = HOST_BUNDLE;
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto session = sptr<AAFwk::SessionInfo>::MakeSptr();
    session->uiExtensionComponentId = COMPONENT_ID;
    session->persistentId = PERSISTENT_ID;
    session->hostWindowId = target;
    session->originalCallerInfo.callerToken = source;
    session->originalCallerInfo.persistentId = PERSISTENT_ID;
    request.sessionInfo = session;
    return request;
}

AAFwk::AbilityRequest MakeMigrationWrongPersistentId(uint32_t target)
{
    AAFwk::AbilityRequest request;
    request.appInfo.bundleName = HOST_BUNDLE;
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto session = sptr<AAFwk::SessionInfo>::MakeSptr();
    session->uiExtensionComponentId = COMPONENT_ID;
    session->persistentId = PERSISTENT_ID;
    session->hostWindowId = target;
    session->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    session->originalCallerInfo.persistentId = PERSISTENT_ID + 1;
    request.sessionInfo = session;
    return request;
}
} // namespace

class ExtensionRecordManagerThirdTest : public testing::Test {};

/**
 * @tc.name: MigrationReusesRecordByComponentId_0100
 * @tc.desc: A migration request with the same componentId finds the existing record and reuses it.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerThirdTest, MigrationReusesRecordByComponentId_0100, TestSize.Level1)
{
    auto fixture = MakeFixture();
    auto tokenB = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    auto request = MakeMigration(HOST_B, fixture.tokenA);
    std::shared_ptr<AAFwk::BaseExtensionRecord> accepted;
    bool isLoaded = false;
    auto result = fixture.manager->GetOrCreateExtensionRecord(request, HOST_BUNDLE, accepted, isLoaded);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(accepted, fixture.record);
    EXPECT_TRUE(isLoaded);
}

/**
 * @tc.name: MigrationRejectsUnknownComponentId_0100
 * @tc.desc: A migration request whose componentId does not match any record is rejected.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerThirdTest, MigrationRejectsUnknownComponentId_0100, TestSize.Level1)
{
    auto fixture = MakeFixture();
    auto request = MakeMigration(HOST_B, fixture.tokenA);
    request.sessionInfo->uiExtensionComponentId = COMPONENT_ID + 999;
    std::shared_ptr<AAFwk::BaseExtensionRecord> accepted;
    bool isLoaded = false;
    auto result = fixture.manager->GetOrCreateExtensionRecord(request, HOST_BUNDLE, accepted, isLoaded);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: MigrationRejectsPersistentIdMismatch_0100
 * @tc.desc: A migration request whose originalCallerInfo.persistentId differs from the record is rejected.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerThirdTest, MigrationRejectsPersistentIdMismatch_0100, TestSize.Level1)
{
    auto fixture = MakeFixture();
    auto request = MakeMigrationWrongPersistentId(HOST_B);
    std::shared_ptr<AAFwk::BaseExtensionRecord> accepted;
    bool isLoaded = false;
    auto result = fixture.manager->GetOrCreateExtensionRecord(request, HOST_BUNDLE, accepted, isLoaded);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: MigrationRejectsCredentialWithoutPersistentId_0100
 * @tc.desc: A request with callerToken but zero persistentId is rejected before lookup.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerThirdTest, MigrationRejectsCredentialWithoutPersistentId_0100, TestSize.Level1)
{
    auto fixture = MakeFixture();
    AAFwk::AbilityRequest request;
    request.appInfo.bundleName = HOST_BUNDLE;
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto session = sptr<AAFwk::SessionInfo>::MakeSptr();
    session->uiExtensionComponentId = COMPONENT_ID;
    session->persistentId = PERSISTENT_ID;
    session->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    session->originalCallerInfo.persistentId = 0;
    request.sessionInfo = session;
    std::shared_ptr<AAFwk::BaseExtensionRecord> accepted;
    bool isLoaded = false;
    auto result = fixture.manager->GetOrCreateExtensionRecord(request, HOST_BUNDLE, accepted, isLoaded);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: OrdinaryStartStillCreatesRecord_0100
 * @tc.desc: A non-migration request that does not match any componentId proceeds to create a new record.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerThirdTest, OrdinaryStartStillCreatesRecord_0100, TestSize.Level1)
{
    auto manager = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest request;
    request.appInfo.bundleName = HOST_BUNDLE;
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto session = sptr<AAFwk::SessionInfo>::MakeSptr();
    session->uiExtensionComponentId = COMPONENT_ID;
    session->persistentId = PERSISTENT_ID;
    session->hostWindowId = HOST_A;
    request.sessionInfo = session;
    std::shared_ptr<AAFwk::BaseExtensionRecord> accepted;
    bool isLoaded = false;
    auto result = manager->GetOrCreateExtensionRecord(request, HOST_BUNDLE, accepted, isLoaded);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_NE(accepted, nullptr);
    EXPECT_FALSE(isLoaded);
}

/**
 * @tc.name: UpdateStripsCallerToken_0100
 * @tc.desc: Update writes a copy of the session info with callerToken stripped.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerThirdTest, UpdateStripsCallerToken_0100, TestSize.Level1)
{
    auto fixture = MakeFixture();
    AAFwk::AbilityRequest ordinary;
    auto session = sptr<AAFwk::SessionInfo>::MakeSptr();
    session->persistentId = PERSISTENT_ID;
    session->hostWindowId = HOST_B;
    session->callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    session->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    session->realHostWindowId = 42;
    ordinary.sessionInfo = session;
    fixture.wrapper->Update(ordinary);
    auto latest = fixture.record->GetSessionInfo();
    ASSERT_NE(latest, nullptr);
    EXPECT_EQ(latest->realHostWindowId, 42);
    EXPECT_EQ(latest->hostWindowId, HOST_B);
    EXPECT_EQ(latest->originalCallerInfo.callerToken.GetRefPtr(), nullptr);
    EXPECT_NE(latest.GetRefPtr(), session.GetRefPtr());
}

/**
 * @tc.name: UpdateSharesSessionWithoutCallerToken_0100
 * @tc.desc: Update shares the incoming session when originalCallerInfo.callerToken is null.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerThirdTest, UpdateSharesSessionWithoutCallerToken_0100, TestSize.Level1)
{
    auto fixture = MakeFixture();
    AAFwk::AbilityRequest ordinary;
    auto session = sptr<AAFwk::SessionInfo>::MakeSptr();
    session->persistentId = PERSISTENT_ID;
    session->hostWindowId = HOST_B;
    session->realHostWindowId = 42;
    ordinary.sessionInfo = session;
    fixture.wrapper->Update(ordinary);
    auto latest = fixture.record->GetSessionInfo();
    ASSERT_NE(latest, nullptr);
    EXPECT_EQ(latest.GetRefPtr(), session.GetRefPtr());
}

} // namespace AbilityRuntime
} // namespace OHOS
