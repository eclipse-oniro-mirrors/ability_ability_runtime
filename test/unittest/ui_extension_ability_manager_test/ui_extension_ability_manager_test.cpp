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
#include <memory>

#define private public
#define protected public
#include "ui_extension_ability_manager.h"
#undef private
#undef protected

#include "ability_config.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#include "ability_util.h"
#include "app_scheduler.h"
#include "extension_record.h"
#include "bundlemgr/mock_bundle_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_connect_callback.h"
#include "mock_sa_call.h"
#include "mock_task_handler_wrap.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "session/host/include/session.h"
#include <thread>
#include <chrono>

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using testing::_;
using testing::Invoke;
using testing::Return;
using testing::SetArgReferee;
using ::testing::DoAll;

namespace {
const std::string PARAM_RESV_CALLER_APP_ID("ohos.aafwk.param.callerAppId");
constexpr uint32_t FAKE_TOKENID = 111;
const std::string UIEXTENSION_ABILITY_ID = "ability.want.params.uiExtensionAbilityId";
}

namespace OHOS {
namespace AAFwk {
class UIExtensionAbilityManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    UIExtensionAbilityManager* GetUIExtensionAbilityManager() const;
    std::shared_ptr<MockTaskHandlerWrap> TaskHandler() const;
    std::shared_ptr<EventHandlerWrap> EventHandler() const;

    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);
    std::shared_ptr<BaseExtensionRecord> InitAbilityRecord();
    std::shared_ptr<MockTaskHandlerWrap> taskHandler_;
protected:
    AbilityRequest abilityRequest_{};
    AbilityRequest abilityRequest1_{};
    AbilityRequest abilityRequest2_{};
    std::shared_ptr<BaseExtensionRecord> serviceRecord_{ nullptr };
    std::shared_ptr<BaseExtensionRecord> serviceRecord1_{ nullptr };
    std::shared_ptr<BaseExtensionRecord> serviceRecord2_{ nullptr };
    std::shared_ptr<BaseExtensionRecord> uiExtensionAbilityRecord1_{ nullptr };
    OHOS::sptr<Token> serviceToken_{ nullptr };
    OHOS::sptr<Token> serviceToken1_{ nullptr };
    OHOS::sptr<Token> serviceToken2_{ nullptr };
    OHOS::sptr<IAbilityConnection> callbackA_{ nullptr };
    OHOS::sptr<IAbilityConnection> callbackB_{ nullptr };
    AbilityRequest uiExtensionAbilityRequest1_{};
    OHOS::sptr<Token> uiExtensionAbilityToken1_{ nullptr };

private:
    std::shared_ptr<UIExtensionAbilityManager> connectManager_;
    std::shared_ptr<EventHandlerWrap> eventHandler_;
};

AbilityRequest UIExtensionAbilityManagerTest::GenerateAbilityRequest(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
{
    ElementName element(deviceName, bundleName, abilityName, moduleName);
    Want want;
    want.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    ApplicationInfo appinfo;
    appinfo.name = appName;
    abilityInfo.applicationInfo = appinfo;
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;
    abilityInfo.process = bundleName;

    return abilityRequest;
}

sptr<SessionInfo> UIExtensionAbilityManagerTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

std::shared_ptr<BaseExtensionRecord> UIExtensionAbilityManagerTest::InitAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest);
    return abilityRecord;
}

void UIExtensionAbilityManagerTest::SetUpTestCase(void)
{}

void UIExtensionAbilityManagerTest::TearDownTestCase(void)
{}

void UIExtensionAbilityManagerTest::SetUp(void)
{
    connectManager_ = std::make_unique<UIExtensionAbilityManager>(0);
    taskHandler_ = MockTaskHandlerWrap::CreateQueueHandler("UIExtensionAbilityManagerTest");
    eventHandler_ = std::make_shared<EventHandlerWrap>(taskHandler_);
    // generate ability request
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    abilityRequest_ = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    serviceRecord_ = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    serviceToken_ = serviceRecord_->GetToken();
    std::string deviceName1 = "device";
    std::string abilityName1 = "musicServiceAbility";
    std::string appName1 = "musicservcie";
    std::string bundleName1 = "com.ix.musicservcie";
    std::string moduleName1 = "entry";
    abilityRequest1_ = GenerateAbilityRequest(deviceName1, abilityName1, appName1, bundleName1, moduleName1);
    serviceRecord1_ = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest1_);
    std::string deviceName2 = "device";
    std::string abilityName2 = "residentServiceAbility";
    std::string appName2 = "residentservcie";
    std::string bundleName2 = "com.ix.residentservcie";
    std::string moduleName2 = "entry";
    abilityRequest2_ = GenerateAbilityRequest(deviceName2, abilityName2, appName2, bundleName2, moduleName2);
    serviceRecord2_ = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest2_);
    serviceToken2_ = serviceRecord_->GetToken();
    serviceToken1_ = serviceRecord_->GetToken();
    callbackA_ = new AbilityConnectCallback();
    callbackB_ = new AbilityConnectCallback();
}

void UIExtensionAbilityManagerTest::TearDown(void)
{
    // reset the callback count
    AbilityConnectCallback::onAbilityConnectDoneCount = 0;
    AbilityConnectCallback::onAbilityDisconnectDoneCount = 0;
    serviceRecord_ = nullptr;
}

UIExtensionAbilityManager* UIExtensionAbilityManagerTest::GetUIExtensionAbilityManager() const
{
    return connectManager_.get();
}

std::shared_ptr<MockTaskHandlerWrap> UIExtensionAbilityManagerTest::TaskHandler() const
{
    return taskHandler_;
}

std::shared_ptr<EventHandlerWrap> UIExtensionAbilityManagerTest::EventHandler() const
{
    return eventHandler_;
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionBySessionInfo
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionBySessionInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionMap_.clear();
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    auto service = connectManager->GetUIExtensionBySessionInfo(sessionInfo);
    std::string device = "device";
    std::string abilityName1 = "uiExtensionAbility1";
    std::string appName1 = "uiExtensionProvider1";
    std::string bundleName1 = "com.ix.uiExtensionProvider1";
    std::string moduleName1 = "entry";
    auto request1 = GenerateAbilityRequest(device, abilityName1, appName1, bundleName1, moduleName1);
    auto uiExtension1 = BaseExtensionRecord::CreateBaseExtensionRecord(request1);
    EXPECT_NE(uiExtension1, nullptr);
    uiExtension1->abilityInfo_.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    uiExtension1 = nullptr;
    connectManager->uiExtensionMap_.emplace(
        sessionInfo->sessionToken, UIExtensionAbilityManager::UIExtWindowMapValType(uiExtension1, sessionInfo));
    service = connectManager->GetUIExtensionBySessionInfo(sessionInfo);
    EXPECT_EQ(service, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionBySessionInfo
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionBySessionInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionMap_.clear();
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->callerToken = new Rosen::Session(info);
    std::string device = "device";
    std::string abilityName1 = "uiExtensionAbility1";
    std::string appName1 = "uiExtensionProvider1";
    std::string bundleName1 = "com.ix.uiExtensionProvider1";
    std::string moduleName1 = "entry";
    auto request1 = GenerateAbilityRequest(device, abilityName1, appName1, bundleName1, moduleName1);
    auto uiExtension1 = BaseExtensionRecord::CreateBaseExtensionRecord(request1);
    EXPECT_NE(uiExtension1, nullptr);
    uiExtension1->abilityInfo_.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    Rosen::SessionInfo infos;
    sptr<SessionInfo> sessionInfo1(new SessionInfo());
    sessionInfo1 = nullptr;
    connectManager->uiExtensionMap_.emplace(
        sessionInfo->sessionToken, UIExtensionAbilityManager::UIExtWindowMapValType(uiExtension1, sessionInfo1));
    auto service = connectManager->GetUIExtensionBySessionInfo(sessionInfo);
    EXPECT_EQ(service, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionBySessionInfo
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionBySessionInfo_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionMap_.clear();
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->callerToken = new Rosen::Session(info);
    std::string device = "device";
    std::string abilityName1 = "uiExtensionAbility1";
    std::string appName1 = "uiExtensionProvider1";
    std::string bundleName1 = "com.ix.uiExtensionProvider1";
    std::string moduleName1 = "entry";
    auto request1 = GenerateAbilityRequest(device, abilityName1, appName1, bundleName1, moduleName1);
    auto uiExtension1 = BaseExtensionRecord::CreateBaseExtensionRecord(request1);
    EXPECT_NE(uiExtension1, nullptr);
    uiExtension1->abilityInfo_.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;

    Rosen::SessionInfo infos;
    sptr<SessionInfo> sessionInfo1(new SessionInfo());
    sessionInfo1->sessionToken = new Rosen::Session(info);
    sessionInfo1->callerToken = uiExtension1->GetToken();
    connectManager->uiExtensionMap_.emplace(
        sessionInfo->sessionToken, UIExtensionAbilityManager::UIExtWindowMapValType(uiExtension1, sessionInfo1));
    auto service = connectManager->GetUIExtensionBySessionInfo(sessionInfo);
    EXPECT_EQ(service, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionBySessionInfo
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionBySessionInfo_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_004 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionMap_.clear();
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    std::string device = "device";
    std::string abilityName1 = "uiExtensionAbility1";
    std::string appName1 = "uiExtensionProvider1";
    std::string bundleName1 = "com.ix.uiExtensionProvider1";
    std::string moduleName1 = "entry";
    auto request1 = GenerateAbilityRequest(device, abilityName1, appName1, bundleName1, moduleName1);
    auto uiExtension1 = BaseExtensionRecord::CreateBaseExtensionRecord(request1);
    EXPECT_NE(uiExtension1, nullptr);
    uiExtension1->abilityInfo_.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    Rosen::SessionInfo infos;
    sptr<SessionInfo> sessionInfo1(new SessionInfo());
    sessionInfo1->sessionToken = uiExtension1->GetToken();
    sessionInfo1->callerToken = new Rosen::Session(info);
    connectManager->uiExtensionMap_.emplace(
        sessionInfo->sessionToken, UIExtensionAbilityManager::UIExtWindowMapValType(uiExtension1, sessionInfo1));
    auto service = connectManager->GetUIExtensionBySessionInfo(sessionInfo);
    EXPECT_EQ(service, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_004 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: HandleUIExtensionDied
 */
HWTEST_F(UIExtensionAbilityManagerTest, HandleUIExtensionDied_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleUIExtensionDied_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    connectManager->HandleUIExtensionDied(abilityRecord);
    EXPECT_EQ(connectManager->uiExtensionMap_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "HandleUIExtensionDied_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionSessionInfo
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionSessionInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSessionInfo_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    AbilityRequest abilityRequest;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    UIExtensionSessionInfo info;

    int32_t result = connectManager->GetUIExtensionSessionInfo(callerToken, info);
    ASSERT_EQ(result, OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSessionInfo_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionSessionInfo
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionSessionInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSessionInfo_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    ASSERT_NE(connectManager, nullptr);
    
    sptr<IRemoteObject> nullToken = nullptr;
    UIExtensionSessionInfo sessionInfo;
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    abilityRequest_ = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    serviceRecord_ = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    int32_t ret = connectManager->GetUIExtensionSessionInfo(token, sessionInfo);
    EXPECT_NE(ret, OHOS::ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSessionInfo_002 end");
}


/*
 * Feature: UIExtensionAbilityManager
 * Function: DoForegroundUIExtension
 */
HWTEST_F(UIExtensionAbilityManagerTest, DoForegroundUIExtension_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoForegroundUIExtension_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> nullRecord = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.deviceId = "id";
    abilityRequest.abilityInfo.bundleName = "bundle";
    abilityRequest.abilityInfo.name = "name";
    abilityRequest.abilityInfo.moduleName = "module";
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::UI;
    connectManager->DoForegroundUIExtension(nullRecord, abilityRequest);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest);
    abilityRecord->isReady_ = true;
    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    
    connectManager->DoForegroundUIExtension(abilityRecord, abilityRequest);
    std::string expectName = "name";
    EXPECT_EQ(expectName, abilityRequest.abilityInfo.name);
    TAG_LOGI(AAFwkTag::TEST, "DoForegroundUIExtension_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetOrCreateExtensionRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetOrCreateExtensionRecord_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetOrCreateExtensionRecord_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest);
    std::string hostBundName = "bundleName";
    bool isCreate = false;
    bool isLoadedAbility = false;
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    
    int32_t result = connectManager->GetOrCreateExtensionRecord(
        abilityRequest, isCreate, hostBundName, abilityRecord, isLoadedAbility);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "GetOrCreateExtensionRecord_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetOrCreateExtensionRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, UnloadUIExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnloadUIExtensionAbility_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER;
    abilityRequest.abilityInfo.bundleName = "com.example.test";
    abilityRequest.abilityInfo.name = "TestAbility";
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connection->SetConnectState(ConnectionState::DISCONNECTING);
    abilityRecord->abilityInfo_.type = AbilityType::PAGE;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    abilityRecord->connRecordList_.push_back(connection);
    int32_t hostPid = 10;

    int result = connectManager->UnloadUIExtensionAbility(abilityRecord, hostPid);
    EXPECT_NE(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "UnloadUIExtensionAbility_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: IsUIExtensionFocused
 */
HWTEST_F(UIExtensionAbilityManagerTest, IsUIExtensionFocused_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionFocused_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    uint32_t uiExtensionTokenId = 1;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> focusToken = abilityRecord->GetToken();
    abilityRecord->abilityInfo_.applicationInfo.accessTokenId = 5;
    connectManager->uiExtensionMap_.clear();
    connectManager->uiExtensionMap_.emplace(focusToken, std::make_pair(abilityRecord, nullptr));

    bool result = connectManager->IsUIExtensionFocused(uiExtensionTokenId, focusToken);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionFocused_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: IsUIExtensionFocused
 */
HWTEST_F(UIExtensionAbilityManagerTest, IsUIExtensionFocused_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionFocused_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    uint32_t uiExtensionTokenId = 1;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> focusToken = abilityRecord->GetToken();
    abilityRecord->abilityInfo_.applicationInfo.accessTokenId = uiExtensionTokenId;
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    sessionInfo->callerToken = focusToken;
    connectManager->uiExtensionMap_.clear();
    connectManager->uiExtensionMap_.emplace(focusToken, std::make_pair(abilityRecord, sessionInfo));

    bool result = connectManager->IsUIExtensionFocused(uiExtensionTokenId, focusToken);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionFocused_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionSourceToken
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionSourceToken_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSourceToken_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<IRemoteObject> nulltoken = nullptr;
    sptr<IRemoteObject> resultToken = connectManager->GetUIExtensionSourceToken(nulltoken);
    EXPECT_EQ(resultToken, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSourceToken_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionSourceToken
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionSourceToken_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSourceToken_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connectManager->uiExtensionMap_.emplace(token, std::make_pair(abilityRecord, nullptr));

    sptr<IRemoteObject> resultToken = connectManager->GetUIExtensionSourceToken(token);
    EXPECT_EQ(resultToken, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSourceToken_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionSourceToken
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionSourceToken_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSourceToken_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<SessionInfo> sessionInfo = new SessionInfo();
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connectManager->uiExtensionMap_.clear();
    connectManager->uiExtensionMap_.emplace(token, std::make_pair(abilityRecord, sessionInfo));

    sptr<IRemoteObject> resultToken = connectManager->GetUIExtensionSourceToken(token);
    EXPECT_EQ(resultToken, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSourceToken_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: QueryPreLoadUIExtensionRecordInner
 */
HWTEST_F(UIExtensionAbilityManagerTest, QueryPreLoadUIExtensionRecordInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryPreLoadUIExtensionRecordInner_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AppExecFwk::ElementName element("deviceId", "bundleName", "abilityName", "moduleName");
    std::string moduleName = "testModule";
    int32_t hostPid = 0;
    int32_t recordNum = 0;

    int32_t result =
        connectManager->QueryPreLoadUIExtensionRecordInner(element, moduleName, hostPid, recordNum);
    EXPECT_EQ(result, ERR_OK);

    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    result =
        connectManager->QueryPreLoadUIExtensionRecordInner(element, moduleName, hostPid, recordNum);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "QueryPreLoadUIExtensionRecordInner_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: QueryPreLoadUIExtensionRecordInner
 */
HWTEST_F(UIExtensionAbilityManagerTest, QueryPreLoadUIExtensionRecordInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryPreLoadUIExtensionRecordInner_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AppExecFwk::ElementName element("deviceId", "bundleName", "abilityName", "moduleName");
    std::string moduleName = "testModule";
    int32_t hostPid = 0;
    int32_t recordNum = 0;

    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    int result = connectManager->QueryPreLoadUIExtensionRecordInner(element, moduleName, hostPid, recordNum);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "QueryPreLoadUIExtensionRecordInner_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: PreloadUIExtensionAbilityInner
 */
HWTEST_F(UIExtensionAbilityManagerTest, PreloadUIExtensionAbilityInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadUIExtensionAbilityInner_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    std::string hostBundleName = "hostBundleName";
    int32_t hostPid = 1;

    auto res = connectManager->PreloadUIExtensionAbilityInner(abilityRequest_, hostBundleName, hostPid);
    EXPECT_EQ(res, ERR_WRONG_INTERFACE_CALL);

    abilityRequest_.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SHARE;
    res = connectManager->PreloadUIExtensionAbilityInner(abilityRequest_, hostBundleName, hostPid);
    EXPECT_EQ(res, ERR_OK);

    abilityRequest_.want.SetParam(Want::CREATE_APP_INSTANCE_KEY, true);
    res = connectManager->PreloadUIExtensionAbilityInner(abilityRequest_, hostBundleName, hostPid);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    abilityRequest_.want.SetParam(Want::CREATE_APP_INSTANCE_KEY, false);
    abilityRequest_.extensionType = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    res = connectManager->PreloadUIExtensionAbilityInner(abilityRequest_, hostBundleName, hostPid);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "PreloadUIExtensionAbilityInner_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: HandleUIExtWindowDiedTask
 * SubFunction: HandleUIExtWindowDiedTask
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UIExtensionAbilityManager HandleUIExtWindowDiedTask
 * @tc.require: AR000I8B26
 */
HWTEST_F(UIExtensionAbilityManagerTest, HandleUIExtWindowDiedTask_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    ASSERT_NE(connectManager, nullptr);
    connectManager->uiExtRecipientMap_.clear();
    connectManager->uiExtensionMap_.clear();

    connectManager->uiExtensionMap_.emplace(
        callbackA_->AsObject(), UIExtensionAbilityManager::UIExtWindowMapValType(serviceRecord_, MockSessionInfo(0)));
    connectManager->AddUIExtWindowDeathRecipient(callbackA_->AsObject());
    connectManager->HandleUIExtWindowDiedTask(nullptr);
    EXPECT_EQ(static_cast<int>(connectManager->uiExtRecipientMap_.size()), 1);
    EXPECT_EQ(static_cast<int>(connectManager->uiExtensionMap_.size()), 1);

    connectManager->HandleUIExtWindowDiedTask(callbackA_->AsObject());
    EXPECT_TRUE(connectManager->uiExtRecipientMap_.empty());
    EXPECT_TRUE(connectManager->uiExtensionMap_.empty());
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: IsUIExtensionFocused
 * SubFunction: IsUIExtensionFocused
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UIExtensionAbilityManager IsUIExtensionFocused
 */
HWTEST_F(UIExtensionAbilityManagerTest, IsUIExtensionFocused_003, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(3);
    ASSERT_NE(connectManager, nullptr);
    connectManager->uiExtensionMap_.clear();
    bool isFocused = connectManager->IsUIExtensionFocused(
        serviceRecord_->GetApplicationInfo().accessTokenId, serviceRecord1_->GetToken());
    EXPECT_EQ(isFocused, false);
    connectManager.reset();
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: IsUIExtensionFocused
 * SubFunction: IsUIExtensionFocused
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UIExtensionAbilityManager IsUIExtensionFocused
 */
HWTEST_F(UIExtensionAbilityManagerTest, IsUIExtensionFocused_004, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(3);
    ASSERT_NE(connectManager, nullptr);
    connectManager->uiExtensionMap_.clear();

    std::string device = "device";
    std::string abilityName = "uiExtensionUserAbility";
    std::string appName = "uiExtensionUser";
    std::string bundleName = "com.ix.uiExtensionUser";
    std::string moduleName = "entry";
    auto request = GenerateAbilityRequest(device, abilityName, appName, bundleName, moduleName);
    auto uiExtensionUser = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    EXPECT_NE(uiExtensionUser, nullptr);

    std::string abilityName1 = "uiExtensionAbility1";
    std::string appName1 = "uiExtensionProvider1";
    std::string bundleName1 = "com.ix.uiExtensionProvider1";
    std::string moduleName1 = "entry";
    auto request1 = GenerateAbilityRequest(device, abilityName1, appName1, bundleName1, moduleName1);
    auto uiExtension1 = BaseExtensionRecord::CreateBaseExtensionRecord(request1);
    EXPECT_NE(uiExtension1, nullptr);
    uiExtension1->abilityInfo_.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    sptr<SessionInfo> sessionInfo1 = new (std::nothrow) SessionInfo();
    sessionInfo1->callerToken = uiExtensionUser->GetToken();
    uiExtension1->sessionInfo_ = sessionInfo1;
    connectManager->uiExtensionMap_.emplace(
        callbackA_->AsObject(), UIExtensionAbilityManager::UIExtWindowMapValType(uiExtension1, sessionInfo1));
    bool isFocused1 = connectManager->IsUIExtensionFocused(
        uiExtension1->GetApplicationInfo().accessTokenId, uiExtensionUser->GetToken());
    EXPECT_EQ(isFocused1, true);
    std::string abilityName2 = "uiExtensionAbility2";
    std::string appName2 = "uiExtensionProvider2";
    std::string bundleName2 = "com.ix.uiExtensionProvider2";
    std::string moduleName2 = "entry";
    auto request2 = GenerateAbilityRequest(device, abilityName2, appName2, bundleName2, moduleName2);
    auto uiExtension2 = BaseExtensionRecord::CreateBaseExtensionRecord(request2);
    EXPECT_NE(uiExtension2, nullptr);
    uiExtension2->abilityInfo_.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    sptr<SessionInfo> sessionInfo2 = new (std::nothrow) SessionInfo();
    sessionInfo2->callerToken = uiExtension1->GetToken();
    uiExtension2->sessionInfo_ = sessionInfo2;
    connectManager->uiExtensionMap_.emplace(
        callbackA_->AsObject(), UIExtensionAbilityManager::UIExtWindowMapValType(uiExtension2, sessionInfo2));
    bool isFocused2 = connectManager->IsUIExtensionFocused(
        uiExtension2->GetApplicationInfo().accessTokenId, uiExtensionUser->GetToken());
    EXPECT_EQ(isFocused2, true);
    connectManager.reset();
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionSourceToken
 * SubFunction: GetUIExtensionSourceToken
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UIExtensionAbilityManager GetUIExtensionSourceToken
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionSourceToken_004, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(3);
    ASSERT_NE(connectManager, nullptr);
    connectManager->uiExtensionMap_.clear();
    auto sourceToken = connectManager->GetUIExtensionSourceToken(nullptr);
    EXPECT_EQ(sourceToken, nullptr);
    connectManager.reset();
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: PauseExtensions
 * SubFunction: PauseExtensions
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UIExtensionAbilityManager PauseExtensions
 */
HWTEST_F(UIExtensionAbilityManagerTest, AAFwk_AbilityMS_PauseExtensions_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord1 = serviceRecord_;
    abilityRecord1->abilityInfo_.type = AbilityType::PAGE;
    connectManager->serviceMap_.emplace("first", abilityRecord1);
    std::shared_ptr<BaseExtensionRecord> abilityRecord2 = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    abilityRecord2->abilityInfo_.type = AbilityType::EXTENSION;
    abilityRecord2->abilityInfo_.name = AbilityConfig::LAUNCHER_ABILITY_NAME;
    abilityRecord2->abilityInfo_.bundleName = AbilityConfig::LAUNCHER_BUNDLE_NAME;
    connectManager->serviceMap_.emplace("second", abilityRecord2);
    connectManager->PauseExtensions();
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: SignRestartAppFlag
 * CaseDescription: Verify UIExtensionAbilityManager SignRestartAppFlag
 */
HWTEST_F(UIExtensionAbilityManagerTest, AAFwk_AbilityMS_SignRestartAppFlag_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    ASSERT_NE(connectManager, nullptr);

    std::string bundleName = "testBundleName";
    std::shared_ptr<BaseExtensionRecord> abilityRecord1 = serviceRecord_;
    abilityRecord1->abilityInfo_.bundleName = bundleName;
    connectManager->serviceMap_.emplace("first", abilityRecord1);
    std::shared_ptr<BaseExtensionRecord> abilityRecord2 = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    abilityRecord2->abilityInfo_.bundleName = "errTestBundleName";
    connectManager->serviceMap_.emplace("second", abilityRecord2);
    int32_t uid = 100;
    connectManager->SignRestartAppFlag(uid, "");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: BuildEventInfo
 * CaseDescription: Verify UIExtensionAbilityManager BuildEventInfo
 */
HWTEST_F(UIExtensionAbilityManagerTest, AAFwk_AbilityMS_BuildEventInfo_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    ASSERT_NE(connectManager, nullptr);

    connectManager->BuildEventInfo(nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    connectManager->BuildEventInfo(abilityRecord);
    abilityRecord->SetCreateByConnectMode(true);
    connectManager->BuildEventInfo(abilityRecord);
}

/**
 * @tc.name: UpdateUIExtensionInfo_0100
 * @tc.desc: Update want params of ui extension.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionAbilityManagerTest, UpdateUIExtensionInfo_0100, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    ASSERT_NE(connectManager, nullptr);

    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<BaseExtensionRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->SetUIExtensionAbilityId(1000);
    connectManager->UpdateUIExtensionInfo(abilityRecord);
    EXPECT_EQ(abilityRecord->GetWant().HasParameter("ability.want.params.uiExtensionAbilityId"), true);
    EXPECT_EQ(abilityRecord->GetWant().GetIntParam("ability.want.params.uiExtensionAbilityId", -1), 1000);
}

/**
 * @tc.name: PreloadUIExtensionAbilityLocked_0100
 * @tc.desc: preload uiextension ability
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionAbilityManagerTest, PreloadUIExtensionAbilityLocked_0100, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    ASSERT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    abilityRequest.want.SetElement(providerElement);
    abilityRequest.abilityInfo.type = AbilityType::EXTENSION;
    std::string hostBundleName = "com.ohos.uiextensionuser";

    int32_t preloadId = AbilityRuntime::INVALID_EXTENSION_RECORD_ID;
    int32_t hostPid = DEFAULT_INVAL_VALUE;
    auto ret = connectManager->PreloadUIExtensionAbilityLocked(
        abilityRequest, hostBundleName, ERR_PRELOAD_APP_DATA_ABILITIES_FAILED);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: UnloadUIExtensionAbility_0100
 * @tc.desc: UnloadUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionAbilityManagerTest, UnloadUIExtensionAbility_0100, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    ASSERT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    abilityRequest.want.SetElement(providerElement);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest);
    int32_t hostPid = 0;
    auto ret = connectManager->UnloadUIExtensionAbility(abilityRecord, hostPid);
    EXPECT_EQ(ret, ERR_CONNECT_MANAGER_NULL_ABILITY_RECORD);
}


/**
 * @tc.name: UnPreloadUIExtensionAbilityLocked_0100
 * @tc.desc: unpreload uiextension ability with invalid id
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionAbilityManagerTest, UnPreloadUIExtensionAbilityLocked_0100, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    ASSERT_NE(connectManager, nullptr);
    int32_t extensionAbilityId = AbilityRuntime::INVALID_EXTENSION_RECORD_ID;
    auto ret = connectManager->UnPreloadUIExtensionAbilityLocked(extensionAbilityId);
    EXPECT_EQ(ret, ERR_CODE_INVALID_ID);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: ClearAllPreloadUIExtensionAbilityLocked
 * SubFunction: ClearAllPreloadUIExtensionAbilityInner
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify ClearAllPreloadUIExtensionAbilityLocked when uiExtensionAbilityRecordMgr_ is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerTest, AAFwk_AbilityMS_ClearAllPreloadUIExtensionAbilityLocked_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    
    int res = connectManager->ClearAllPreloadUIExtensionAbilityLocked();
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: ClearAllPreloadUIExtensionAbilityLocked
 * SubFunction: ClearAllPreloadUIExtensionAbilityInner
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify ClearAllPreloadUIExtensionAbilityLocked with valid uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, AAFwk_AbilityMS_ClearAllPreloadUIExtensionAbilityLocked_002, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    
    int res = connectManager->ClearAllPreloadUIExtensionAbilityLocked();
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RegisterPreloadUIExtensionHostClient
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify RegisterPreloadUIExtensionHostClient when callerToken is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerTest, AAFwk_AbilityMS_RegisterPreloadUIExtensionHostClient_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    sptr<IRemoteObject> callerToken = nullptr;
    
    int32_t res = connectManager->RegisterPreloadUIExtensionHostClient(callerToken);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RegisterPreloadUIExtensionHostClient
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify RegisterPreloadUIExtensionHostClient when uiExtensionAbilityRecordMgr_ is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerTest, AAFwk_AbilityMS_RegisterPreloadUIExtensionHostClient_002, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord = serviceRecord_;
    ASSERT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    ASSERT_NE(callerToken, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    
    int32_t res = connectManager->RegisterPreloadUIExtensionHostClient(callerToken);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RegisterPreloadUIExtensionHostClient
 * CaseDescription: Verify registration fails when a local token cannot register a death recipient
 */
HWTEST_F(UIExtensionAbilityManagerTest, AAFwk_AbilityMS_RegisterPreloadUIExtensionHostClient_003, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    sptr<IRemoteObject> callerToken = serviceRecord_->GetToken();

    EXPECT_EQ(connectManager->RegisterPreloadUIExtensionHostClient(callerToken), INNER_ERR);
    EXPECT_TRUE(connectManager->preloadUIExtensionHostClientDeathRecipients_.empty());
    EXPECT_TRUE(connectManager->uiExtensionAbilityRecordMgr_->preloadUIExtensionHostClientCallerTokens_.empty());
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UnRegisterPreloadUIExtensionHostClient
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UnRegisterPreloadUIExtensionHostClient when uiExtensionAbilityRecordMgr_ is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerTest, AAFwk_AbilityMS_UnRegisterPreloadUIExtensionHostClient_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    int32_t callerPid = 1234;
    
    int32_t res = connectManager->UnRegisterPreloadUIExtensionHostClient(callerPid);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UnRegisterPreloadUIExtensionHostClient
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UnRegisterPreloadUIExtensionHostClient with DEFAULT_INVALID_VALUE
 */
HWTEST_F(UIExtensionAbilityManagerTest, AAFwk_AbilityMS_UnRegisterPreloadUIExtensionHostClient_002, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    int32_t callerPid = DEFAULT_INVAL_VALUE;
    
    int32_t res = connectManager->UnRegisterPreloadUIExtensionHostClient(callerPid);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UnRegisterPreloadUIExtensionHostClient
 * CaseDescription: Verify unregistering an unknown process is idempotent
 */
HWTEST_F(UIExtensionAbilityManagerTest, AAFwk_AbilityMS_UnRegisterPreloadUIExtensionHostClient_003, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);

    EXPECT_EQ(connectManager->UnRegisterPreloadUIExtensionHostClient(5678), ERR_OK);
    EXPECT_TRUE(connectManager->preloadUIExtensionHostClientDeathRecipients_.empty());
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UnRegisterPreloadUIExtensionHostClient
 * CaseDescription: Verify failed registration leaves no state for unregistering
 */
HWTEST_F(UIExtensionAbilityManagerTest, AAFwk_AbilityMS_UnRegisterPreloadUIExtensionHostClient_004, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    sptr<IRemoteObject> callerToken = serviceRecord_->GetToken();

    EXPECT_EQ(connectManager->RegisterPreloadUIExtensionHostClient(callerToken), INNER_ERR);
    EXPECT_EQ(connectManager->UnRegisterPreloadUIExtensionHostClient(IPCSkeleton::GetCallingPid()), ERR_OK);
    EXPECT_TRUE(connectManager->preloadUIExtensionHostClientDeathRecipients_.empty());
    EXPECT_TRUE(connectManager->uiExtensionAbilityRecordMgr_->preloadUIExtensionHostClientCallerTokens_.empty());
}

/*
* Feature: UIExtensionAbilityManager
* Function: HandleUIExtensionDied
*/
HWTEST_F(UIExtensionAbilityManagerTest, HandleUIExtensionDied_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleUIExtensionDied_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<Token> token = serviceToken_;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> sessionToken = abilityRecord->GetToken();
    connectManager->AddUIExtWindowDeathRecipient(sessionToken);
    connectManager->uiExtensionMap_[sessionToken] = {std::weak_ptr<BaseExtensionRecord>(), nullptr};
    EXPECT_EQ(connectManager->uiExtensionMap_.size(), 1);

    connectManager->HandleUIExtensionDied(abilityRecord);
    EXPECT_EQ(connectManager->uiExtensionMap_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "HandleUIExtensionDied_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: IsUIExtensionFocused
 */
HWTEST_F(UIExtensionAbilityManagerTest, IsUIExtensionFocused_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionFocused_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    uint32_t uiExtensionTokenId = 1;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> focusToken = abilityRecord->GetToken();
    abilityRecord->abilityInfo_.applicationInfo.accessTokenId = 5;
    connectManager->uiExtensionMap_.clear();
    connectManager->uiExtensionMap_.emplace(focusToken, std::make_pair(abilityRecord, nullptr));

    bool result = connectManager->IsUIExtensionFocused(uiExtensionTokenId, focusToken);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionFocused_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: IsUIExtensionFocused
 */
HWTEST_F(UIExtensionAbilityManagerTest, IsUIExtensionFocused_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionFocused_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    uint32_t uiExtensionTokenId = 1;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> focusToken = abilityRecord->GetToken();
    abilityRecord->abilityInfo_.applicationInfo.accessTokenId = uiExtensionTokenId;
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    sessionInfo->callerToken = focusToken;
    connectManager->uiExtensionMap_.clear();
    connectManager->uiExtensionMap_.emplace(focusToken, std::make_pair(abilityRecord, sessionInfo));

    bool result = connectManager->IsUIExtensionFocused(uiExtensionTokenId, focusToken);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionFocused_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionSourceToken
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionSourceToken_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSourceToken_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<IRemoteObject> nulltoken = nullptr;
    sptr<IRemoteObject> resultToken = connectManager->GetUIExtensionSourceToken(nulltoken);
    EXPECT_EQ(resultToken, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSourceToken_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionSourceToken
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionSourceToken_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSourceToken_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connectManager->uiExtensionMap_.emplace(token, std::make_pair(abilityRecord, nullptr));

    sptr<IRemoteObject> resultToken = connectManager->GetUIExtensionSourceToken(token);
    EXPECT_EQ(resultToken, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSourceToken_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionSourceToken
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionSourceToken_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSourceToken_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<SessionInfo> sessionInfo = new SessionInfo();
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    ASSERT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connectManager->uiExtensionMap_.clear();
    connectManager->uiExtensionMap_.emplace(token, std::make_pair(abilityRecord, sessionInfo));

    sptr<IRemoteObject> resultToken = connectManager->GetUIExtensionSourceToken(token);
    EXPECT_EQ(resultToken, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSourceToken_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetActiveUIExtensionList
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the GetActiveUIExtensionList function.
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetActiveUIExtensionList_01, TestSize.Level1)
{
    int32_t pid = 1;
    std::vector<std::string> extensionList;
    auto result = GetUIExtensionAbilityManager()->GetActiveUIExtensionList(pid, extensionList);
    EXPECT_EQ(result, ERR_OK);

    std::string bundleName = "com.test.demo";
    result = GetUIExtensionAbilityManager()->GetActiveUIExtensionList(bundleName, extensionList);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: ClearAllPreloadUIExtensionAbilityLocked
 * SubFunction: ClearAllPreloadUIExtensionAbilityInner
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify ClearAllPreloadUIExtensionAbilityLocked when uiExtensionAbilityRecordMgr_ is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerTest, ClearAllPreloadUIExtensionAbilityLocked_003, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    
    int res = connectManager->ClearAllPreloadUIExtensionAbilityLocked();
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: ClearAllPreloadUIExtensionAbilityLocked
 * SubFunction: ClearAllPreloadUIExtensionAbilityInner
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify ClearAllPreloadUIExtensionAbilityLocked with valid uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, ClearAllPreloadUIExtensionAbilityLocked_004, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    
    int res = connectManager->ClearAllPreloadUIExtensionAbilityLocked();
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RegisterPreloadUIExtensionHostClient
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify RegisterPreloadUIExtensionHostClient when callerToken is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerTest, RegisterPreloadUIExtensionHostClient_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    sptr<IRemoteObject> callerToken = nullptr;
    
    int32_t res = connectManager->RegisterPreloadUIExtensionHostClient(callerToken);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RegisterPreloadUIExtensionHostClient
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify RegisterPreloadUIExtensionHostClient when uiExtensionAbilityRecordMgr_ is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerTest, RegisterPreloadUIExtensionHostClient_002, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    std::shared_ptr<AbilityRecord> abilityRecord = serviceRecord_;
    ASSERT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    ASSERT_NE(callerToken, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    
    int32_t res = connectManager->RegisterPreloadUIExtensionHostClient(callerToken);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RegisterPreloadUIExtensionHostClient
 * CaseDescription: Verify registration fails when a local token cannot register a death recipient
 */
HWTEST_F(UIExtensionAbilityManagerTest, RegisterPreloadUIExtensionHostClient_003, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    sptr<IRemoteObject> callerToken = serviceRecord_->GetToken();

    EXPECT_EQ(connectManager->RegisterPreloadUIExtensionHostClient(callerToken), INNER_ERR);
    EXPECT_TRUE(connectManager->preloadUIExtensionHostClientDeathRecipients_.empty());
    EXPECT_TRUE(connectManager->uiExtensionAbilityRecordMgr_->preloadUIExtensionHostClientCallerTokens_.empty());
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UnRegisterPreloadUIExtensionHostClient
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UnRegisterPreloadUIExtensionHostClient when uiExtensionAbilityRecordMgr_ is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerTest, UnRegisterPreloadUIExtensionHostClient_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    int32_t callerPid = 1234;
    
    int32_t res = connectManager->UnRegisterPreloadUIExtensionHostClient(callerPid);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UnRegisterPreloadUIExtensionHostClient
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UnRegisterPreloadUIExtensionHostClient with DEFAULT_INVALID_VALUE
 */
HWTEST_F(UIExtensionAbilityManagerTest, UnRegisterPreloadUIExtensionHostClient_002, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    int32_t callerPid = DEFAULT_INVAL_VALUE;
    
    int32_t res = connectManager->UnRegisterPreloadUIExtensionHostClient(callerPid);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UnRegisterPreloadUIExtensionHostClient
 * CaseDescription: Verify unregistering an unknown process is idempotent
 */
HWTEST_F(UIExtensionAbilityManagerTest, UnRegisterPreloadUIExtensionHostClient_003, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);

    EXPECT_EQ(connectManager->UnRegisterPreloadUIExtensionHostClient(5678), ERR_OK);
    EXPECT_TRUE(connectManager->preloadUIExtensionHostClientDeathRecipients_.empty());
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UnRegisterPreloadUIExtensionHostClient
 * CaseDescription: Verify failed registration leaves no state for unregistering
 */
HWTEST_F(UIExtensionAbilityManagerTest, UnRegisterPreloadUIExtensionHostClient_004, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    sptr<IRemoteObject> callerToken = serviceRecord_->GetToken();

    EXPECT_EQ(connectManager->RegisterPreloadUIExtensionHostClient(callerToken), INNER_ERR);
    EXPECT_EQ(connectManager->UnRegisterPreloadUIExtensionHostClient(IPCSkeleton::GetCallingPid()), ERR_OK);
    EXPECT_TRUE(connectManager->preloadUIExtensionHostClientDeathRecipients_.empty());
    EXPECT_TRUE(connectManager->uiExtensionAbilityRecordMgr_->preloadUIExtensionHostClientCallerTokens_.empty());
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UpdateUIExtensionBindInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UpdateUIExtensionBindInfo with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, UpdateUIExtensionBindInfo_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    connectManager->UpdateUIExtensionBindInfo(nullptr, "testBundle", 1);
    // Should not crash
    SUCCEED();
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UpdateUIExtensionBindInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UpdateUIExtensionBindInfo with non-UIExtension ability
 */
HWTEST_F(UIExtensionAbilityManagerTest, UpdateUIExtensionBindInfo_002, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord =
        BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    abilityRecord->abilityInfo_.extensionAbilityType = ExtensionAbilityType::SERVICE;
    
    connectManager->UpdateUIExtensionBindInfo(abilityRecord, "testBundle", 1);
    // Should not crash
    SUCCEED();
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UpdateUIExtensionBindInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UpdateUIExtensionBindInfo with SCENEBOARD_BUNDLE_NAME
 */
HWTEST_F(UIExtensionAbilityManagerTest, UpdateUIExtensionBindInfo_003, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord =
    BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    abilityRecord->abilityInfo_.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    
    connectManager->UpdateUIExtensionBindInfo(abilityRecord, AbilityConfig::SCENEBOARD_BUNDLE_NAME, 1);
    // Should not allow bind process
    SUCCEED();
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetActiveUIExtensionList
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetActiveUIExtensionList with valid parameters
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetActiveUIExtensionList_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    
    int32_t pid = 1234;
    std::vector<std::string> extensionList;
    int32_t result = connectManager->GetActiveUIExtensionList(pid, extensionList);
    EXPECT_EQ(result, ERR_OK);
    
    std::string bundleName = "com.test.demo";
    result = connectManager->GetActiveUIExtensionList(bundleName, extensionList);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetActiveUIExtensionList
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetActiveUIExtensionList when uiExtensionAbilityRecordMgr_ is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetActiveUIExtensionList_002, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    
    int32_t pid = 1234;
    std::vector<std::string> extensionList;
    int32_t result = connectManager->GetActiveUIExtensionList(pid, extensionList);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    
    std::string bundleName = "com.test.demo";
    result = connectManager->GetActiveUIExtensionList(bundleName, extensionList);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionRootHostInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetUIExtensionRootHostInfo with null token
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionRootHostInfo_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    
    std::shared_ptr<AAFwk::AbilityRecord> result = connectManager->GetUIExtensionRootHostInfo(nullptr);
    EXPECT_EQ(result, nullptr);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionRootHostInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetUIExtensionRootHostInfo when uiExtensionAbilityRecordMgr_ is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionRootHostInfo_002, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord =
        BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    
    std::shared_ptr<AAFwk::AbilityRecord> result = connectManager->GetUIExtensionRootHostInfo(token);
    EXPECT_EQ(result, nullptr);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: AddPreloadUIExtensionRecord
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AddPreloadUIExtensionRecord with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, AddPreloadUIExtensionRecord_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    
    int32_t result = connectManager->AddPreloadUIExtensionRecord(nullptr);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: AddPreloadUIExtensionRecord
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AddPreloadUIExtensionRecord when uiExtensionAbilityRecordMgr_ is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerTest, AddPreloadUIExtensionRecord_002, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord =
        BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    
    int32_t result = connectManager->AddPreloadUIExtensionRecord(abilityRecord);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RemoveUIExtensionBySessionInfoToken
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify RemoveUIExtensionBySessionInfoToken with null token
 */
HWTEST_F(UIExtensionAbilityManagerTest, RemoveUIExtensionBySessionInfoToken_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    
    int result = connectManager->RemoveUIExtensionBySessionInfoToken(nullptr);
    // Should not crash, just return 0
    SUCCEED();
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RemoveUIExtensionBySessionInfoToken
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify RemoveUIExtensionBySessionInfoToken with existing token
 */
HWTEST_F(UIExtensionAbilityManagerTest, RemoveUIExtensionBySessionInfoToken_002, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord =
        BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    
    connectManager->uiExtensionMap_[token] = {std::weak_ptr<BaseExtensionRecord>(), nullptr};
    int result = connectManager->RemoveUIExtensionBySessionInfoToken(token);
    EXPECT_EQ(result, 1);
    EXPECT_TRUE(connectManager->uiExtensionMap_.empty());
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionCallerTokenList
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetUIExtensionCallerTokenList with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionCallerTokenList_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    
    std::list<sptr<IRemoteObject>> callerList;
    connectManager->GetUIExtensionCallerTokenList(nullptr, callerList);
    // Should not crash
    SUCCEED();
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionCallerTokenList
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetUIExtensionCallerTokenList when uiExtensionAbilityRecordMgr_ is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionCallerTokenList_002, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord =
        BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    
    std::list<sptr<IRemoteObject>> callerList;
    connectManager->GetUIExtensionCallerTokenList(abilityRecord, callerList);
    // Should not crash
    SUCCEED();
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: HandlePreloadUIExtensionSuccess
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify HandlePreloadUIExtensionSuccess when uiExtensionAbilityRecordMgr_ is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerTest, HandlePreloadUIExtensionSuccess_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    
    connectManager->HandlePreloadUIExtensionSuccess(1234, true);
    // Should not crash
    SUCCEED();
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: IsCallerValid
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify IsCallerValid with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, IsCallerValid_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    
    bool result = connectManager->IsCallerValid(nullptr);
    EXPECT_FALSE(result);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: IsCallerValid
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify IsCallerValid with null sessionInfo
 */
HWTEST_F(UIExtensionAbilityManagerTest, IsCallerValid_002, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord =
        BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    abilityRecord->sessionInfo_ = nullptr;
    
    bool result = connectManager->IsCallerValid(abilityRecord);
    EXPECT_FALSE(result);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: SetLastExitReason
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify SetLastExitReason with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, SetLastExitReason_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    std::shared_ptr<BaseExtensionRecord> targetRecord = nullptr;
    
    connectManager->SetLastExitReason(abilityRequest, targetRecord);
    // Should not crash
    SUCCEED();
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: SetLastExitReason
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify SetLastExitReason with non-UIExtension ability
 */
HWTEST_F(UIExtensionAbilityManagerTest, SetLastExitReason_002, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    std::shared_ptr<BaseExtensionRecord> targetRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    
    connectManager->SetLastExitReason(abilityRequest, targetRecord);
    // Should not set exit reason for non-UIExtension
    SUCCEED();
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UnPreloadUIExtensionAbilityLocked
 * SubFunction: UnPreloadUIExtensionAbilityInner
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UnPreloadUIExtensionAbilityLocked with invalid extensionAbilityId
 */
HWTEST_F(UIExtensionAbilityManagerTest, UnPreloadUIExtensionAbilityLocked_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    int32_t extensionAbilityId = AbilityRuntime::INVALID_EXTENSION_RECORD_ID;
    
    int res = connectManager->UnPreloadUIExtensionAbilityLocked(extensionAbilityId);
    EXPECT_EQ(res, ERR_CODE_INVALID_ID);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UnPreloadUIExtensionAbilityLocked
 * SubFunction: UnPreloadUIExtensionAbilityInner
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UnPreloadUIExtensionAbilityLocked when uiExtensionAbilityRecordMgr_ is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerTest, UnPreloadUIExtensionAbilityLocked_002, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    int32_t extensionAbilityId = 1234;
    
    int res = connectManager->UnPreloadUIExtensionAbilityLocked(extensionAbilityId);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: PreloadUIExtensionAbilityLocked
 * SubFunction: PreloadUIExtensionAbilityInner
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify PreloadUIExtensionAbilityLocked with non-UIExtension type
 */
HWTEST_F(UIExtensionAbilityManagerTest, PreloadUIExtensionAbilityLocked_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    std::string hostBundleName = "testBundle";
    int32_t hostPid = 1234;
    
    int res = connectManager->PreloadUIExtensionAbilityLocked(abilityRequest, hostBundleName, hostPid);
    EXPECT_EQ(res, ERR_WRONG_INTERFACE_CALL);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: PreloadUIExtensionAbilityLocked
 * SubFunction: PreloadUIExtensionAbilityInner
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify PreloadUIExtensionAbilityLocked when uiExtensionAbilityRecordMgr_ is nullptr
 */
HWTEST_F(UIExtensionAbilityManagerTest, PreloadUIExtensionAbilityLocked_002, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    std::string hostBundleName = "testBundle";
    int32_t hostPid = 1234;
    
    int res = connectManager->PreloadUIExtensionAbilityLocked(abilityRequest, hostBundleName, hostPid);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: AttachAbilityThreadInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AttachAbilityThreadInner with null scheduler
 */
HWTEST_F(UIExtensionAbilityManagerTest, AttachAbilityThreadInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachAbilityThreadInner_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<IAbilityScheduler> scheduler = nullptr;
    sptr<IRemoteObject> token = serviceToken_;
    
    int res = connectManager->AttachAbilityThreadInner(scheduler, token);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AttachAbilityThreadInner_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: AttachAbilityThreadInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AttachAbilityThreadInner with null token
 */
HWTEST_F(UIExtensionAbilityManagerTest, AttachAbilityThreadInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachAbilityThreadInner_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    sptr<IRemoteObject> token = nullptr;
    
    int res = connectManager->AttachAbilityThreadInner(scheduler, token);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AttachAbilityThreadInner_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: AttachAbilityThreadInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AttachAbilityThreadInner with UIExtension ability
 */
HWTEST_F(UIExtensionAbilityManagerTest, AttachAbilityThreadInner_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachAbilityThreadInner_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    
    int res = connectManager->AttachAbilityThreadInner(scheduler, token);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AttachAbilityThreadInner_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: OnAbilityRequestDone
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify OnAbilityRequestDone with null token
 */
HWTEST_F(UIExtensionAbilityManagerTest, OnAbilityRequestDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAbilityRequestDone_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<IRemoteObject> token = nullptr;
    int32_t state = static_cast<int32_t>(AppAbilityState::ABILITY_STATE_FOREGROUND);
    
    connectManager->OnAbilityRequestDone(token, state);
    TAG_LOGI(AAFwkTag::TEST, "OnAbilityRequestDone_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: OnAbilityRequestDone
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify OnAbilityRequestDone with non-UIExtension ability
 */
HWTEST_F(UIExtensionAbilityManagerTest, OnAbilityRequestDone_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAbilityRequestDone_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    abilityRequest.abilityInfo.bundleName = "com.test.service";
    abilityRequest.abilityInfo.name = "TestService";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);

    sptr<IRemoteObject> token = abilityRecord->GetToken();
    int32_t state = static_cast<int32_t>(AppAbilityState::ABILITY_STATE_FOREGROUND);
    
    connectManager->OnAbilityRequestDone(token, state);
    TAG_LOGI(AAFwkTag::TEST, "OnAbilityRequestDone_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: OnAbilityRequestDone
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify OnAbilityRequestDone with UIExtension ability in FOREGROUNDING state
 */
HWTEST_F(UIExtensionAbilityManagerTest, OnAbilityRequestDone_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAbilityRequestDone_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUNDING);

    sptr<IRemoteObject> token = abilityRecord->GetToken();
    int32_t state = static_cast<int32_t>(AppAbilityState::ABILITY_STATE_FOREGROUND);
    
    connectManager->OnAbilityRequestDone(token, state);
    TAG_LOGI(AAFwkTag::TEST, "OnAbilityRequestDone_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: BackgroundAbilityWindowLocked
 * SubFunction: DoBackgroundAbilityWindow
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify BackgroundAbilityWindowLocked with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, BackgroundAbilityWindowLocked_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BackgroundAbilityWindowLocked_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = nullptr;
    sptr<SessionInfo> sessionInfo = MockSessionInfo(1);
    
    connectManager->BackgroundAbilityWindowLocked(abilityRecord, sessionInfo);
    TAG_LOGI(AAFwkTag::TEST, "BackgroundAbilityWindowLocked_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: BackgroundAbilityWindowLocked
 * SubFunction: DoBackgroundAbilityWindow
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify BackgroundAbilityWindowLocked with null sessionInfo
 */
HWTEST_F(UIExtensionAbilityManagerTest, BackgroundAbilityWindowLocked_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BackgroundAbilityWindowLocked_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<SessionInfo> sessionInfo = nullptr;
    
    connectManager->BackgroundAbilityWindowLocked(abilityRecord, sessionInfo);
    TAG_LOGI(AAFwkTag::TEST, "BackgroundAbilityWindowLocked_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: BackgroundAbilityWindowLocked
 * SubFunction: DoBackgroundAbilityWindow
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify BackgroundAbilityWindowLocked with FOREGROUND state
 */
HWTEST_F(UIExtensionAbilityManagerTest, BackgroundAbilityWindowLocked_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BackgroundAbilityWindowLocked_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);

    sptr<SessionInfo> sessionInfo = MockSessionInfo(1);
    
    connectManager->BackgroundAbilityWindowLocked(abilityRecord, sessionInfo);
    TAG_LOGI(AAFwkTag::TEST, "BackgroundAbilityWindowLocked_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: BackgroundAbilityWindowLocked
 * SubFunction: DoBackgroundAbilityWindow
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify BackgroundAbilityWindowLocked with INITIAL state
 */
HWTEST_F(UIExtensionAbilityManagerTest, BackgroundAbilityWindowLocked_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BackgroundAbilityWindowLocked_004 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->SetAbilityState(AbilityState::INITIAL);

    sptr<SessionInfo> sessionInfo = MockSessionInfo(1);
    
    connectManager->BackgroundAbilityWindowLocked(abilityRecord, sessionInfo);
    TAG_LOGI(AAFwkTag::TEST, "BackgroundAbilityWindowLocked_004 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: CheckUIExtensionAbilitySessionExist
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify CheckUIExtensionAbilitySessionExist with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, CheckUIExtensionAbilitySessionExist_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckUIExtensionAbilitySessionExist_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = nullptr;
    
    bool result = connectManager->CheckUIExtensionAbilitySessionExist(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "CheckUIExtensionAbilitySessionExist_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: CheckUIExtensionAbilitySessionExist
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify CheckUIExtensionAbilitySessionExist with non-UIExtension type
 */
HWTEST_F(UIExtensionAbilityManagerTest, CheckUIExtensionAbilitySessionExist_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckUIExtensionAbilitySessionExist_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    abilityRequest.abilityInfo.bundleName = "com.test.service";
    abilityRequest.abilityInfo.name = "TestService";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    
    bool result = connectManager->CheckUIExtensionAbilitySessionExist(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "CheckUIExtensionAbilitySessionExist_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: IsUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify IsUIExtensionAbility with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, IsUIExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionAbility_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = nullptr;
    
    bool result = connectManager->IsUIExtensionAbility(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionAbility_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: IsUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify IsUIExtensionAbility with UIExtension type
 */
HWTEST_F(UIExtensionAbilityManagerTest, IsUIExtensionAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionAbility_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    
    bool result = connectManager->IsUIExtensionAbility(abilityRecord);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionAbility_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: IsUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify IsUIExtensionAbility with non-UIExtension type
 */
HWTEST_F(UIExtensionAbilityManagerTest, IsUIExtensionAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionAbility_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    abilityRequest.abilityInfo.bundleName = "com.test.service";
    abilityRequest.abilityInfo.name = "TestService";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    
    bool result = connectManager->IsUIExtensionAbility(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionAbility_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: IsCallerValid
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify IsCallerValid with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, IsCallerValid_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsCallerValid_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = nullptr;
    
    bool result = connectManager->IsCallerValid(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsCallerValid_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: IsCallerValid
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify IsCallerValid with null sessionInfo
 */
HWTEST_F(UIExtensionAbilityManagerTest, IsCallerValid_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsCallerValid_004 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    
    bool result = connectManager->IsCallerValid(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsCallerValid_004 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: HandleUIExtensionDied
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify HandleUIExtensionDied with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, HandleUIExtensionDied_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleUIExtensionDied_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = nullptr;
    
    connectManager->HandleUIExtensionDied(abilityRecord);
    TAG_LOGI(AAFwkTag::TEST, "HandleUIExtensionDied_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: HandleUIExtWindowDiedTask
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify HandleUIExtWindowDiedTask with null remote
 */
HWTEST_F(UIExtensionAbilityManagerTest, HandleUIExtWindowDiedTask_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleUIExtWindowDiedTask_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<IRemoteObject> remote = nullptr;
    
    connectManager->HandleUIExtWindowDiedTask(remote);
    TAG_LOGI(AAFwkTag::TEST, "HandleUIExtWindowDiedTask_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: HandleUIExtWindowDiedTask
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify HandleUIExtWindowDiedTask with remote not found in uiExtensionMap_
 */
HWTEST_F(UIExtensionAbilityManagerTest, HandleUIExtWindowDiedTask_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleUIExtWindowDiedTask_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<IRemoteObject> remote = new AbilityScheduler();
    
    connectManager->HandleUIExtWindowDiedTask(remote);
    TAG_LOGI(AAFwkTag::TEST, "HandleUIExtWindowDiedTask_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UpdateUIExtensionInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UpdateUIExtensionInfo with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, UpdateUIExtensionInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateUIExtensionInfo_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = nullptr;
    int32_t hostPid = 1234;
    
    connectManager->UpdateUIExtensionInfo(abilityRecord, hostPid);
    TAG_LOGI(AAFwkTag::TEST, "UpdateUIExtensionInfo_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UpdateUIExtensionBindInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UpdateUIExtensionBindInfo with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, UpdateUIExtensionBindInfo_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateUIExtensionBindInfo_004 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = nullptr;
    std::string callerBundleName = "com.test.caller";
    int32_t notifyProcessBind = 1;
    
    connectManager->UpdateUIExtensionBindInfo(abilityRecord, callerBundleName, notifyProcessBind);
    TAG_LOGI(AAFwkTag::TEST, "UpdateUIExtensionBindInfo_004 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UpdateUIExtensionBindInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UpdateUIExtensionBindInfo with SCENEBOARD_BUNDLE_NAME
 */
HWTEST_F(UIExtensionAbilityManagerTest, UpdateUIExtensionBindInfo_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateUIExtensionBindInfo_005 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    
    std::string callerBundleName = AbilityConfig::SCENEBOARD_BUNDLE_NAME;
    int32_t notifyProcessBind = 1;
    
    connectManager->UpdateUIExtensionBindInfo(abilityRecord, callerBundleName, notifyProcessBind);
    TAG_LOGI(AAFwkTag::TEST, "UpdateUIExtensionBindInfo_005 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RemoveUIExtensionBySessionInfoToken
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify RemoveUIExtensionBySessionInfoToken with null token
 */
HWTEST_F(UIExtensionAbilityManagerTest, RemoveUIExtensionBySessionInfoToken_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBySessionInfoToken_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<IRemoteObject> token = nullptr;
    
    int res = connectManager->RemoveUIExtensionBySessionInfoToken(token);
    EXPECT_EQ(res, 0);
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBySessionInfoToken_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RemoveUIExtensionBySessionInfoToken
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify RemoveUIExtensionBySessionInfoToken with token not in map
 */
HWTEST_F(UIExtensionAbilityManagerTest, RemoveUIExtensionBySessionInfoToken_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBySessionInfoToken_004 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<IRemoteObject> token = new AbilityScheduler();
    
    int res = connectManager->RemoveUIExtensionBySessionInfoToken(token);
    EXPECT_EQ(res, 0);
    TAG_LOGI(AAFwkTag::TEST, "RemoveUIExtensionBySessionInfoToken_004 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: AddPreloadUIExtensionRecord
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AddPreloadUIExtensionRecord with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, AddPreloadUIExtensionRecord_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddPreloadUIExtensionRecord_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = nullptr;
    
    int res = connectManager->AddPreloadUIExtensionRecord(abilityRecord);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AddPreloadUIExtensionRecord_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: AddPreloadUIExtensionRecord
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AddPreloadUIExtensionRecord with null uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, AddPreloadUIExtensionRecord_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddPreloadUIExtensionRecord_004 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    
    int res = connectManager->AddPreloadUIExtensionRecord(abilityRecord);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AddPreloadUIExtensionRecord_004 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionCallerTokenList
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetUIExtensionCallerTokenList with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionCallerTokenList_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionCallerTokenList_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = nullptr;
    std::list<sptr<IRemoteObject>> callerList;
    
    connectManager->GetUIExtensionCallerTokenList(abilityRecord, callerList);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionCallerTokenList_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: HandlePreloadUIExtensionSuccess
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify HandlePreloadUIExtensionSuccess with null uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, HandlePreloadUIExtensionSuccess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionSuccess_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    int32_t extensionRecordId = 1234;
    bool isPreloadedSuccess = true;
    
    connectManager->HandlePreloadUIExtensionSuccess(extensionRecordId, isPreloadedSuccess);
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionSuccess_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: SetLastExitReason
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify SetLastExitReason with null targetService
 */
HWTEST_F(UIExtensionAbilityManagerTest, SetLastExitReason_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetLastExitReason_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.test.bundle";
    abilityRequest.abilityInfo.name = "TestAbility";
    std::shared_ptr<BaseExtensionRecord> targetService = nullptr;
    
    connectManager->SetLastExitReason(abilityRequest, targetService);
    TAG_LOGI(AAFwkTag::TEST, "SetLastExitReason_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: ClearPreloadUIExtensionRecord
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify ClearPreloadUIExtensionRecord with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, ClearPreloadUIExtensionRecord_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadUIExtensionRecord_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = nullptr;
    
    connectManager->ClearPreloadUIExtensionRecord(abilityRecord);
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadUIExtensionRecord_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: ClearPreloadUIExtensionRecord
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify ClearPreloadUIExtensionRecord with null uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, ClearPreloadUIExtensionRecord_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadUIExtensionRecord_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    
    connectManager->ClearPreloadUIExtensionRecord(abilityRecord);
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadUIExtensionRecord_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DoForegroundUIExtension
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DoForegroundUIExtension with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, DoForegroundUIExtension_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoForegroundUIExtension_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = nullptr;
    AbilityRequest abilityRequest;
    
    connectManager->DoForegroundUIExtension(abilityRecord, abilityRequest);
    TAG_LOGI(AAFwkTag::TEST, "DoForegroundUIExtension_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DoForegroundUIExtension
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DoForegroundUIExtension with abilityRecord not ready
 */
HWTEST_F(UIExtensionAbilityManagerTest, DoForegroundUIExtension_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoForegroundUIExtension_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->isReady_ = false;
    
    connectManager->DoForegroundUIExtension(abilityRecord, abilityRequest);
    TAG_LOGI(AAFwkTag::TEST, "DoForegroundUIExtension_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UnloadUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UnloadUIExtensionAbility with null abilityRecord
 */
HWTEST_F(UIExtensionAbilityManagerTest, UnloadUIExtensionAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnloadUIExtensionAbility_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = nullptr;
    pid_t hostPid = 1234;
    
    int res = connectManager->UnloadUIExtensionAbility(abilityRecord, hostPid);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "UnloadUIExtensionAbility_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UnloadUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UnloadUIExtensionAbility with null uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, UnloadUIExtensionAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnloadUIExtensionAbility_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    pid_t hostPid = 1234;
    
    int res = connectManager->UnloadUIExtensionAbility(abilityRecord, hostPid);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "UnloadUIExtensionAbility_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetOrCreateExtensionRecord
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetOrCreateExtensionRecord with null uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetOrCreateExtensionRecord_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetOrCreateExtensionRecord_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::string hostBundName = "bundleName";
    bool isCreate = false;
    bool isLoadedAbility = false;
    
    int32_t result = connectManager->GetOrCreateExtensionRecord(
        abilityRequest, isCreate, hostBundName, abilityRecord, isLoadedAbility);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "GetOrCreateExtensionRecord_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: QueryPreLoadUIExtensionRecordInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify QueryPreLoadUIExtensionRecordInner with null uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, QueryPreLoadUIExtensionRecordInner_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QueryPreLoadUIExtensionRecordInner_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    AppExecFwk::ElementName element("device", "bundle", "ability", "module");
    std::string moduleName = "module";
    int32_t hostPid = 1234;
    int32_t recordNum = 0;
    
    int32_t result = connectManager->QueryPreLoadUIExtensionRecordInner(element, moduleName, hostPid, recordNum);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "QueryPreLoadUIExtensionRecordInner_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: PreloadUIExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify PreloadUIExtensionAbilityInner with non-UIExtension type
 */
HWTEST_F(UIExtensionAbilityManagerTest, PreloadUIExtensionAbilityInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadUIExtensionAbilityInner_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    abilityRequest.abilityInfo.bundleName = "com.test.service";
    abilityRequest.abilityInfo.name = "TestService";
    std::string hostBundleName = "com.test.caller";
    int32_t hostPid = 1234;
    
    int res = connectManager->PreloadUIExtensionAbilityInner(abilityRequest, hostBundleName, hostPid);
    EXPECT_EQ(res, ERR_WRONG_INTERFACE_CALL);
    TAG_LOGI(AAFwkTag::TEST, "PreloadUIExtensionAbilityInner_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: PreloadUIExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify PreloadUIExtensionAbilityInner with null uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, PreloadUIExtensionAbilityInner_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadUIExtensionAbilityInner_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    std::string hostBundleName = "com.test.caller";
    int32_t hostPid = 1234;
    
    int res = connectManager->PreloadUIExtensionAbilityInner(abilityRequest, hostBundleName, hostPid);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "PreloadUIExtensionAbilityInner_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UnPreloadUIExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UnPreloadUIExtensionAbilityInner with invalid extensionAbilityId
 */
HWTEST_F(UIExtensionAbilityManagerTest, UnPreloadUIExtensionAbilityInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnPreloadUIExtensionAbilityInner_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    int32_t extensionAbilityId = AbilityRuntime::INVALID_EXTENSION_RECORD_ID;
    
    int res = connectManager->UnPreloadUIExtensionAbilityInner(extensionAbilityId);
    EXPECT_EQ(res, ERR_CODE_INVALID_ID);
    TAG_LOGI(AAFwkTag::TEST, "UnPreloadUIExtensionAbilityInner_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UnPreloadUIExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UnPreloadUIExtensionAbilityInner with null uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, UnPreloadUIExtensionAbilityInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnPreloadUIExtensionAbilityInner_002 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    int32_t extensionAbilityId = 1234;
    
    int res = connectManager->UnPreloadUIExtensionAbilityInner(extensionAbilityId);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "UnPreloadUIExtensionAbilityInner_002 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: ClearAllPreloadUIExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify ClearAllPreloadUIExtensionAbilityInner with null uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, ClearAllPreloadUIExtensionAbilityInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearAllPreloadUIExtensionAbilityInner_001 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    
    int res = connectManager->ClearAllPreloadUIExtensionAbilityInner();
    EXPECT_EQ(res, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "ClearAllPreloadUIExtensionAbilityInner_001 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RegisterPreloadUIExtensionHostClient
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify RegisterPreloadUIExtensionHostClient with null callerToken
 */
HWTEST_F(UIExtensionAbilityManagerTest, RegisterPreloadUIExtensionHostClient_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterPreloadUIExtensionHostClient_004 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<IRemoteObject> callerToken = nullptr;
    
    int32_t res = connectManager->RegisterPreloadUIExtensionHostClient(callerToken);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "RegisterPreloadUIExtensionHostClient_004 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RegisterPreloadUIExtensionHostClient
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify RegisterPreloadUIExtensionHostClient with null uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, RegisterPreloadUIExtensionHostClient_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterPreloadUIExtensionHostClient_005 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    sptr<IRemoteObject> callerToken = new AbilityScheduler();
    
    int32_t res = connectManager->RegisterPreloadUIExtensionHostClient(callerToken);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "RegisterPreloadUIExtensionHostClient_005 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: UnRegisterPreloadUIExtensionHostClient
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify UnRegisterPreloadUIExtensionHostClient with null uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, UnRegisterPreloadUIExtensionHostClient_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnRegisterPreloadUIExtensionHostClient_005 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    int32_t callerPid = 1234;
    
    int32_t res = connectManager->UnRegisterPreloadUIExtensionHostClient(callerPid);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "UnRegisterPreloadUIExtensionHostClient_005 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetActiveUIExtensionList
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetActiveUIExtensionList by pid with null uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetActiveUIExtensionList_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetActiveUIExtensionList_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    int32_t pid = 1234;
    std::vector<std::string> extensionList;
    
    int32_t res = connectManager->GetActiveUIExtensionList(pid, extensionList);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "GetActiveUIExtensionList_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetActiveUIExtensionList
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetActiveUIExtensionList by bundleName with null uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetActiveUIExtensionList_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetActiveUIExtensionList_004 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    std::string bundleName = "com.test.bundle";
    std::vector<std::string> extensionList;
    
    int32_t res = connectManager->GetActiveUIExtensionList(bundleName, extensionList);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "GetActiveUIExtensionList_004 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: IsUIExtensionFocused
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify IsUIExtensionFocused with null uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, IsUIExtensionFocused_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionFocused_007 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    uint32_t uiExtensionTokenId = 1234;
    sptr<IRemoteObject> focusToken = new AbilityScheduler();
    
    bool result = connectManager->IsUIExtensionFocused(uiExtensionTokenId, focusToken);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionFocused_007 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionRootHostInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetUIExtensionRootHostInfo with null token
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionRootHostInfo_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionRootHostInfo_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<IRemoteObject> token = nullptr;
    
    auto result = connectManager->GetUIExtensionRootHostInfo(token);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionRootHostInfo_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionRootHostInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetUIExtensionRootHostInfo with null uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionRootHostInfo_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionRootHostInfo_004 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    sptr<IRemoteObject> token = new AbilityScheduler();
    
    auto result = connectManager->GetUIExtensionRootHostInfo(token);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionRootHostInfo_004 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionSessionInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetUIExtensionSessionInfo with null token
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionSessionInfo_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSessionInfo_003 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<IRemoteObject> token = nullptr;
    UIExtensionSessionInfo sessionInfo;
    
    int32_t res = connectManager->GetUIExtensionSessionInfo(token, sessionInfo);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSessionInfo_003 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionSessionInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetUIExtensionSessionInfo with null uiExtensionAbilityRecordMgr_
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionSessionInfo_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSessionInfo_004 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;

    sptr<IRemoteObject> token = new AbilityScheduler();
    UIExtensionSessionInfo sessionInfo;
    
    int32_t res = connectManager->GetUIExtensionSessionInfo(token, sessionInfo);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSessionInfo_004 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionBySessionInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetUIExtensionBySessionInfo with null sessionInfo
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionBySessionInfo_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_005 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<SessionInfo> sessionInfo = nullptr;
    
    auto result = connectManager->GetUIExtensionBySessionInfo(sessionInfo);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_005 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionBySessionInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetUIExtensionBySessionInfo with null sessionToken
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionBySessionInfo_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_006 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->sessionToken = nullptr;
    
    auto result = connectManager->GetUIExtensionBySessionInfo(sessionInfo);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_006 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionSourceToken
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetUIExtensionSourceToken with null token
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionSourceToken_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSourceToken_008 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<IRemoteObject> token = nullptr;
    
    auto result = connectManager->GetUIExtensionSourceToken(token);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSourceToken_008 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: GetUIExtensionSourceToken
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify GetUIExtensionSourceToken with token not found in uiExtensionMap_
 */
HWTEST_F(UIExtensionAbilityManagerTest, GetUIExtensionSourceToken_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSourceToken_009 start");
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(connectManager, nullptr);

    sptr<IRemoteObject> token = new AbilityScheduler();
    
    auto result = connectManager->GetUIExtensionSourceToken(token);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionSourceToken_009 end");
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DoBackgroundAbilityWindow
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DoBackgroundAbilityWindow when AbilityState is FOREGROUND
 */
HWTEST_F(UIExtensionAbilityManagerTest, DoBackgroundAbilityWindow_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
    connectManager->DoBackgroundAbilityWindow(abilityRecord, MockSessionInfo(0));
    abilityRecord->SetAbilityState(AbilityState::INITIAL);
    connectManager->DoBackgroundAbilityWindow(abilityRecord, MockSessionInfo(0));
    abilityRecord->SetAbilityState(AbilityState::FOREGROUNDING);
    connectManager->DoBackgroundAbilityWindow(abilityRecord, MockSessionInfo(0));
    abilityRecord->SetAbilityState(AbilityState::BACKGROUNDING);
    connectManager->DoBackgroundAbilityWindow(abilityRecord, MockSessionInfo(0));
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    connectManager->DoBackgroundAbilityWindow(abilityRecord, MockSessionInfo(0));
    EXPECT_NE(connectManager->uiExtensionAbilityRecordMgr_, nullptr);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: LoadTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify LoadTimeout
 */
HWTEST_F(UIExtensionAbilityManagerTest, LoadTimeout_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRecord->sessionInfo_ = sessionInfo;
    connectManager->AddUIExtWindowDeathRecipient(sessionInfo->sessionToken);
    connectManager->LoadTimeout(abilityRecord);
    connectManager->LoadTimeout(nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    connectManager->LoadTimeout(abilityRecord);
    connectManager->LoadTimeout(nullptr);
    EXPECT_EQ(connectManager->uiExtensionAbilityRecordMgr_, nullptr);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: ForegroundTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify ForegroundTimeout
 */
HWTEST_F(UIExtensionAbilityManagerTest, ForegroundTimeout_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRecord->sessionInfo_ = sessionInfo;
    connectManager->AddUIExtWindowDeathRecipient(sessionInfo->sessionToken);
    connectManager->ForegroundTimeout(abilityRecord);
    connectManager->ForegroundTimeout(nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    connectManager->ForegroundTimeout(abilityRecord);
    connectManager->ForegroundTimeout(nullptr);
    EXPECT_EQ(connectManager->uiExtensionAbilityRecordMgr_, nullptr);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: BackgroundTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify BackgroundTimeout
 */
HWTEST_F(UIExtensionAbilityManagerTest, BackgroundTimeout_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRecord->sessionInfo_ = sessionInfo;
    connectManager->AddUIExtWindowDeathRecipient(sessionInfo->sessionToken);
    connectManager->BackgroundTimeout(abilityRecord);
    connectManager->BackgroundTimeout(nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    connectManager->BackgroundTimeout(abilityRecord);
    connectManager->BackgroundTimeout(nullptr);
    EXPECT_EQ(connectManager->uiExtensionAbilityRecordMgr_, nullptr);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: TerminateTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify TerminateTimeout
 */
HWTEST_F(UIExtensionAbilityManagerTest, TerminateTimeout_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRecord->sessionInfo_ = sessionInfo;
    connectManager->AddUIExtWindowDeathRecipient(sessionInfo->sessionToken);
    connectManager->TerminateTimeout(abilityRecord);
    connectManager->TerminateTimeout(nullptr);
    connectManager->uiExtensionAbilityRecordMgr_ = nullptr;
    connectManager->TerminateTimeout(abilityRecord);
    connectManager->TerminateTimeout(nullptr);
    EXPECT_EQ(connectManager->uiExtensionAbilityRecordMgr_, nullptr);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: HandleStartTimeoutTaskInner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify HandleStartTimeoutTaskInner
 */
HWTEST_F(UIExtensionAbilityManagerTest, HandleStartTimeoutTaskInner_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRecord->sessionInfo_ = sessionInfo;
    connectManager->AddUIExtWindowDeathRecipient(sessionInfo->sessionToken);
    connectManager->HandleStartTimeoutTaskInner(abilityRecord);
    connectManager->HandleForegroundTimeoutTaskInner(abilityRecord);
    connectManager->HandleStopTimeoutTaskInner(abilityRecord);
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::UNSPECIFIED;
    auto abilityRecord2 = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    connectManager->HandleStartTimeoutTaskInner(abilityRecord2);
    connectManager->HandleForegroundTimeoutTaskInner(abilityRecord2);
    connectManager->HandleStopTimeoutTaskInner(abilityRecord2);
    EXPECT_NE(connectManager->uiExtensionAbilityRecordMgr_, nullptr);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: TerminateDone
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify TerminateDone
 */
HWTEST_F(UIExtensionAbilityManagerTest, TerminateDone_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRecord->sessionInfo_ = sessionInfo;
    
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    connectManager->TerminateDone(abilityRecord);
    abilityRecord->SetAbilityState(AbilityState::TERMINATING);
    connectManager->TerminateDone(abilityRecord);
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::UNSPECIFIED;
    auto abilityRecord2 = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    abilityRecord2->SetAbilityState(AbilityState::INACTIVE);
    connectManager->TerminateDone(abilityRecord2);
    abilityRecord2->SetAbilityState(AbilityState::TERMINATING);
    connectManager->TerminateDone(abilityRecord2);
    EXPECT_NE(connectManager->uiExtensionAbilityRecordMgr_, nullptr);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DispatchForeground
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify DispatchForeground
 */
HWTEST_F(UIExtensionAbilityManagerTest, DispatchForeground_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRecord->sessionInfo_ = sessionInfo;
    EXPECT_EQ(connectManager->DispatchForeground(abilityRecord, nullptr), ERR_INVALID_VALUE);
    EXPECT_EQ(connectManager->DispatchBackground(abilityRecord, nullptr), ERR_INVALID_VALUE);

    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::UNSPECIFIED;
    auto abilityRecord2 = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_EQ(connectManager->DispatchForeground(abilityRecord2, nullptr), ERR_INVALID_VALUE);
    EXPECT_EQ(connectManager->DispatchBackground(abilityRecord2, nullptr), ERR_INVALID_VALUE);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: CheckAbilityStateForDisconnect
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify CheckAbilityStateForDisconnect
 */
HWTEST_F(UIExtensionAbilityManagerTest, CheckAbilityStateForDisconnect_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = sptr<Rosen::Session>(new Rosen::Session(info));
    abilityRecord->sessionInfo_ = sessionInfo;
        
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_EQ(connectManager->CheckAbilityStateForDisconnect(abilityRecord), ERR_OK);
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    EXPECT_EQ(connectManager->CheckAbilityStateForDisconnect(abilityRecord), ERR_OK);
    abilityRecord->SetAbilityState(AbilityState::BACKGROUNDING);
    EXPECT_EQ(connectManager->CheckAbilityStateForDisconnect(abilityRecord), ERR_OK);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
    EXPECT_EQ(connectManager->CheckAbilityStateForDisconnect(abilityRecord), ERR_OK);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUNDING);
    EXPECT_EQ(connectManager->CheckAbilityStateForDisconnect(abilityRecord), ERR_OK);
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::UNSPECIFIED;
    auto abilityRecord2 = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    EXPECT_EQ(connectManager->CheckAbilityStateForDisconnect(abilityRecord2), INVALID_CONNECTION_STATE);
}

}  // namespace AAFwk
}  // namespace OHOS
