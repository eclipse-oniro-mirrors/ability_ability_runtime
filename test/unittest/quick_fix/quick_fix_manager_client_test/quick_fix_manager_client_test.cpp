/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "mock_bundle_manager.h"
#include "mock_quick_fix_manager_stub.h"
#include "mock_quick_fix_util.h"
#include "quick_fix_error_utils.h"
#define private public
#include "quick_fix_manager_client.h"
#undef private
#include "system_ability_definition.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class QuickFixManagerClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockQuickFixManagerStub> mockQuickFixMgrService_ = nullptr;
    std::shared_ptr<QuickFixManagerClient> quickFixClient_ = nullptr;
    std::shared_ptr<QuickFixUtil> quickFixUtil_ = nullptr;
};

void QuickFixManagerClientTest::SetUpTestCase(void)
{}

void QuickFixManagerClientTest::TearDownTestCase(void)
{}

void QuickFixManagerClientTest::SetUp()
{
    mockQuickFixMgrService_ = new MockQuickFixManagerStub();
    ASSERT_NE(mockQuickFixMgrService_, nullptr);

    quickFixClient_ = std::make_shared<QuickFixManagerClient>();
    ASSERT_NE(quickFixClient_, nullptr);
    quickFixUtil_ = std::make_shared<QuickFixUtil>();
    ASSERT_NE(quickFixUtil_, nullptr);
    sptr<IRemoteObject> bundleObject = new (std::nothrow) AppExecFwk::BundleMgrService();
    quickFixUtil_->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);
}

void QuickFixManagerClientTest::TearDown()
{}

/**
 * @tc.name: ApplyQuickFix_0100
 * @tc.desc: ApplyQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerClientTest, ApplyQuickFix_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start", __func__);

    quickFixClient_->quickFixMgr_ = mockQuickFixMgrService_;
    EXPECT_CALL(*mockQuickFixMgrService_, ApplyQuickFix(_, _, _)).Times(1);

    std::vector<std::string> quickfixFiles;
    bool isDebug = false;
    bool isReplace = false;
    auto ret = quickFixClient_->ApplyQuickFix(quickfixFiles, isDebug, isReplace);
    EXPECT_EQ(ret, QUICK_FIX_OK);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end", __func__);
}

/**
 * @tc.name: GetApplyedQuickFixInfo_0100
 * @tc.desc: ApplyQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5ODCD
 */
HWTEST_F(QuickFixManagerClientTest, GetApplyedQuickFixInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start", __func__);

    quickFixClient_->quickFixMgr_ = mockQuickFixMgrService_;
    EXPECT_CALL(*mockQuickFixMgrService_, GetApplyedQuickFixInfo(_, _)).Times(1);

    std::string bundleName = "com.ohos.quickfix";
    ApplicationQuickFixInfo quickFixInfo;
    auto ret = quickFixClient_->GetApplyedQuickFixInfo(bundleName, quickFixInfo);
    EXPECT_EQ(ret, QUICK_FIX_OK);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end", __func__);
}

/**
 * @tc.name: GetQuickFixMgrProxy_0100
 * @tc.desc: ApplyQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerClientTest, GetQuickFixMgrProxy_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start", __func__);

    auto quickFixMgr = quickFixClient_->GetQuickFixMgrProxy();
    EXPECT_EQ(quickFixMgr, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end", __func__);
}

/**
 * @tc.name: LoadSystemAbility_0100
 * @tc.desc: Load system ability test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerClientTest, LoadSystemAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start", __func__);

    auto ret = quickFixClient_->LoadQuickFixMgrService();
    EXPECT_EQ(ret, false);

    quickFixClient_->OnLoadSystemAbilitySuccess(mockQuickFixMgrService_);
    quickFixClient_->OnLoadSystemAbilityFail();

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end", __func__);
}

/**
 * @tc.name: RevokeQuickFix_0100
 * @tc.desc: RevokeQuickFix
 * @tc.type: FUNC
 */
HWTEST_F(QuickFixManagerClientTest, RevokeQuickFix_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start", __func__);

    std::string bundleName = "test.bundle.name";
    quickFixClient_->quickFixMgr_ = mockQuickFixMgrService_;
    EXPECT_CALL(*mockQuickFixMgrService_, RevokeQuickFix(_)).Times(1);
    auto ret = quickFixClient_->RevokeQuickFix(bundleName);
    EXPECT_EQ(ret, QUICK_FIX_OK);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end", __func__);
}

/**
 * @tc.name: RevokeQuickFix_0200
 * @tc.desc: RevokeQuickFix
 * @tc.type: FUNC
 */
HWTEST_F(QuickFixManagerClientTest, RevokeQuickFix_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start", __func__);

    std::string bundleName = "test.bundle.name";
    quickFixClient_->quickFixMgr_ = nullptr;
    auto ret = quickFixClient_->RevokeQuickFix(bundleName);
    EXPECT_EQ(ret, QUICK_FIX_CONNECT_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end", __func__);
}

/**
 * @tc.name: GetInstance_0100
 * @tc.desc: GetInstance
 * @tc.type: FUNC
 */
HWTEST_F(QuickFixManagerClientTest, GetInstance_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start", __func__);

    auto instance_ = AAFwk::QuickFixManagerClient::GetInstance();
    EXPECT_NE(instance_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end", __func__);
}

/**
 * @tc.name: ClearProxyWithRemote_0100
 * @tc.desc: ClearProxy(remote) clears the proxy when remote matches the proxy's binder object.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerClientTest, ClearProxyWithRemote_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start", __func__);

    quickFixClient_->quickFixMgr_ = mockQuickFixMgrService_;
    wptr<IRemoteObject> diedRemote = mockQuickFixMgrService_->AsObject();
    ASSERT_NE(diedRemote.promote(), nullptr);

    quickFixClient_->ClearProxy(diedRemote);
    EXPECT_EQ(quickFixClient_->quickFixMgr_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end", __func__);
}

/**
 * @tc.name: ClearProxyWithRemote_0200
 * @tc.desc: ClearProxy(remote) keeps the proxy when remote belongs to another binder object.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerClientTest, ClearProxyWithRemote_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start", __func__);

    quickFixClient_->quickFixMgr_ = mockQuickFixMgrService_;
    sptr<MockQuickFixManagerStub> otherService = new MockQuickFixManagerStub();
    wptr<IRemoteObject> otherRemote = otherService->AsObject();

    quickFixClient_->ClearProxy(otherRemote);
    EXPECT_NE(quickFixClient_->quickFixMgr_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end", __func__);
}

/**
 * @tc.name: ClearProxyWithRemote_0300
 * @tc.desc: ClearProxy(remote) with null proxy should not crash.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerClientTest, ClearProxyWithRemote_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start", __func__);

    quickFixClient_->quickFixMgr_ = nullptr;
    wptr<IRemoteObject> diedRemote = mockQuickFixMgrService_->AsObject();

    quickFixClient_->ClearProxy(diedRemote);
    EXPECT_EQ(quickFixClient_->quickFixMgr_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end", __func__);
}
} // namespace AppExecFwk
} // namespace OHOS