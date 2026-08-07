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

#include "hilog_tag_wrapper.h"
#include "mock_bundle_manager.h"
#include "mock_quick_fix_manager_stub.h"
#include "mock_quick_fix_util.h"
#include "system_ability_definition.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t INVALID_SA_ID = 99999;
constexpr int32_t CUSTOM_SA_ID_FIRST = 80001;
constexpr int32_t CUSTOM_SA_ID_SECOND = 80002;
} // namespace

class QuickFixUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<QuickFixUtil> quickFixUtil_ = nullptr;
};

void QuickFixUtilsTest::SetUpTestCase(void) {}

void QuickFixUtilsTest::TearDownTestCase(void) {}

void QuickFixUtilsTest::SetUp()
{
    quickFixUtil_ = std::make_shared<QuickFixUtil>();
    ASSERT_NE(quickFixUtil_, nullptr);
    QuickFixUtil::setAppManagerProxyNull_ = false;
    QuickFixUtil::setBundleMgrProxyNull_ = false;
}

void QuickFixUtilsTest::TearDown()
{
    QuickFixUtil::setAppManagerProxyNull_ = false;
    QuickFixUtil::setBundleMgrProxyNull_ = false;
}

/**
 * @tc.name: GetRemoteObjectOfSystemAbility_0100
 * @tc.desc: registered system ability should return the registered object.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixUtilsTest, GetRemoteObjectOfSystemAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto obj = sptr<IRemoteObject>(new (std::nothrow) MockQuickFixManagerStub());
    ASSERT_NE(obj, nullptr);
    quickFixUtil_->RegisterSystemAbility(CUSTOM_SA_ID_FIRST, obj);

    auto ret = QuickFixUtil::GetRemoteObjectOfSystemAbility(CUSTOM_SA_ID_FIRST);
    EXPECT_EQ(ret, obj);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetRemoteObjectOfSystemAbility_0200
 * @tc.desc: unregistered system ability should return nullptr when SAM unavailable.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixUtilsTest, GetRemoteObjectOfSystemAbility_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto ret = QuickFixUtil::GetRemoteObjectOfSystemAbility(INVALID_SA_ID);
    EXPECT_EQ(ret, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetRemoteObjectOfSystemAbility_0300
 * @tc.desc: repeated query of registered ability should return the same cached object.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixUtilsTest, GetRemoteObjectOfSystemAbility_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto obj = sptr<IRemoteObject>(new (std::nothrow) MockQuickFixManagerStub());
    ASSERT_NE(obj, nullptr);
    quickFixUtil_->RegisterSystemAbility(CUSTOM_SA_ID_FIRST, obj);

    auto first = QuickFixUtil::GetRemoteObjectOfSystemAbility(CUSTOM_SA_ID_FIRST);
    auto second = QuickFixUtil::GetRemoteObjectOfSystemAbility(CUSTOM_SA_ID_FIRST);
    EXPECT_EQ(first, obj);
    EXPECT_EQ(second, obj);
    EXPECT_EQ(first, second);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetRemoteObjectOfSystemAbility_0400
 * @tc.desc: re-registering the same ability id should overwrite the previous object.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixUtilsTest, GetRemoteObjectOfSystemAbility_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto first = sptr<IRemoteObject>(new (std::nothrow) MockQuickFixManagerStub());
    auto second = sptr<IRemoteObject>(new (std::nothrow) MockQuickFixManagerStub());
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    quickFixUtil_->RegisterSystemAbility(CUSTOM_SA_ID_FIRST, first);
    EXPECT_EQ(QuickFixUtil::GetRemoteObjectOfSystemAbility(CUSTOM_SA_ID_FIRST), first);

    quickFixUtil_->RegisterSystemAbility(CUSTOM_SA_ID_FIRST, second);
    EXPECT_EQ(QuickFixUtil::GetRemoteObjectOfSystemAbility(CUSTOM_SA_ID_FIRST), second);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetRemoteObjectOfSystemAbility_0500
 * @tc.desc: multiple distinct ability ids should be independently retrievable.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixUtilsTest, GetRemoteObjectOfSystemAbility_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto objA = sptr<IRemoteObject>(new (std::nothrow) MockQuickFixManagerStub());
    auto objB = sptr<IRemoteObject>(new (std::nothrow) MockQuickFixManagerStub());
    ASSERT_NE(objA, nullptr);
    ASSERT_NE(objB, nullptr);
    quickFixUtil_->RegisterSystemAbility(CUSTOM_SA_ID_FIRST, objA);
    quickFixUtil_->RegisterSystemAbility(CUSTOM_SA_ID_SECOND, objB);

    EXPECT_EQ(QuickFixUtil::GetRemoteObjectOfSystemAbility(CUSTOM_SA_ID_FIRST), objA);
    EXPECT_EQ(QuickFixUtil::GetRemoteObjectOfSystemAbility(CUSTOM_SA_ID_SECOND), objB);
    EXPECT_NE(QuickFixUtil::GetRemoteObjectOfSystemAbility(CUSTOM_SA_ID_FIRST),
        QuickFixUtil::GetRemoteObjectOfSystemAbility(CUSTOM_SA_ID_SECOND));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetAppManagerProxy_0100
 * @tc.desc: when setAppManagerProxyNull_ flag is true, GetAppManagerProxy returns nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixUtilsTest, GetAppManagerProxy_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    QuickFixUtil::setAppManagerProxyNull_ = true;
    auto ret = QuickFixUtil::GetAppManagerProxy();
    EXPECT_EQ(ret, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetAppManagerProxy_0200
 * @tc.desc: when the underlying ability is not registered, GetAppManagerProxy returns nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixUtilsTest, GetAppManagerProxy_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    QuickFixUtil::setAppManagerProxyNull_ = false;
    quickFixUtil_->RegisterSystemAbility(APP_MGR_SERVICE_ID, nullptr);
    auto ret = QuickFixUtil::GetAppManagerProxy();
    EXPECT_EQ(ret, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetAppManagerProxy_0300
 * @tc.desc: flag reset to false after being true should not short-circuit to nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixUtilsTest, GetAppManagerProxy_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    QuickFixUtil::setAppManagerProxyNull_ = true;
    EXPECT_EQ(QuickFixUtil::GetAppManagerProxy(), nullptr);
    QuickFixUtil::setAppManagerProxyNull_ = false;
    quickFixUtil_->RegisterSystemAbility(APP_MGR_SERVICE_ID, nullptr);
    EXPECT_EQ(QuickFixUtil::GetAppManagerProxy(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetBundleManagerProxy_0100
 * @tc.desc: when setBundleMgrProxyNull_ flag is true, GetBundleManagerProxy returns nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixUtilsTest, GetBundleManagerProxy_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    QuickFixUtil::setBundleMgrProxyNull_ = true;
    auto ret = QuickFixUtil::GetBundleManagerProxy();
    EXPECT_EQ(ret, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetBundleManagerProxy_0200
 * @tc.desc: registered BundleMgrService should return a non-null bundle manager proxy.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixUtilsTest, GetBundleManagerProxy_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    QuickFixUtil::setBundleMgrProxyNull_ = false;
    sptr<IRemoteObject> bundleObject = new (std::nothrow) AppExecFwk::BundleMgrService();
    ASSERT_NE(bundleObject, nullptr);
    quickFixUtil_->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);

    auto ret = QuickFixUtil::GetBundleManagerProxy();
    EXPECT_NE(ret, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetBundleManagerProxy_0300
 * @tc.desc: when setBundleMgrProxyNull_ flag is true, GetBundleManagerProxy returns nullptr even if registered.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixUtilsTest, GetBundleManagerProxy_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    sptr<IRemoteObject> bundleObject = new (std::nothrow) AppExecFwk::BundleMgrService();
    ASSERT_NE(bundleObject, nullptr);
    quickFixUtil_->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);
    QuickFixUtil::setBundleMgrProxyNull_ = true;
    auto ret = QuickFixUtil::GetBundleManagerProxy();
    EXPECT_EQ(ret, nullptr);
    QuickFixUtil::setBundleMgrProxyNull_ = false;
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetBundleQuickFixMgrProxy_0100
 * @tc.desc: when bundleMgr is null (flag set), GetBundleQuickFixMgrProxy returns nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixUtilsTest, GetBundleQuickFixMgrProxy_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    QuickFixUtil::setBundleMgrProxyNull_ = true;
    auto ret = QuickFixUtil::GetBundleQuickFixMgrProxy();
    EXPECT_EQ(ret, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetBundleQuickFixMgrProxy_0200
 * @tc.desc: when bundleMgr is registered, GetBundleQuickFixMgrProxy returns a non-null quick fix manager.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixUtilsTest, GetBundleQuickFixMgrProxy_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    QuickFixUtil::setBundleMgrProxyNull_ = false;
    sptr<IRemoteObject> bundleObject = new (std::nothrow) AppExecFwk::BundleMgrService();
    ASSERT_NE(bundleObject, nullptr);
    quickFixUtil_->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);

    auto ret = QuickFixUtil::GetBundleQuickFixMgrProxy();
    EXPECT_NE(ret, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetBundleQuickFixMgrProxy_0300
 * @tc.desc: when setBundleMgrProxyNull_ flag is true, GetBundleQuickFixMgrProxy returns nullptr even if registered.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixUtilsTest, GetBundleQuickFixMgrProxy_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    sptr<IRemoteObject> bundleObject = new (std::nothrow) AppExecFwk::BundleMgrService();
    ASSERT_NE(bundleObject, nullptr);
    quickFixUtil_->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);
    QuickFixUtil::setBundleMgrProxyNull_ = true;
    auto ret = QuickFixUtil::GetBundleQuickFixMgrProxy();
    EXPECT_EQ(ret, nullptr);
    QuickFixUtil::setBundleMgrProxyNull_ = false;
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: RegisterSystemAbility_0100
 * @tc.desc: registering an ability and querying it should return the same object.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixUtilsTest, RegisterSystemAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    auto obj = sptr<IRemoteObject>(new (std::nothrow) MockQuickFixManagerStub());
    ASSERT_NE(obj, nullptr);
    quickFixUtil_->RegisterSystemAbility(CUSTOM_SA_ID_FIRST, obj);
    EXPECT_EQ(QuickFixUtil::GetRemoteObjectOfSystemAbility(CUSTOM_SA_ID_FIRST), obj);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: RegisterSystemAbility_0200
 * @tc.desc: registering a nullptr object should be stored and queried as nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixUtilsTest, RegisterSystemAbility_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    quickFixUtil_->RegisterSystemAbility(CUSTOM_SA_ID_FIRST, nullptr);
    auto ret = QuickFixUtil::GetRemoteObjectOfSystemAbility(CUSTOM_SA_ID_FIRST);
    EXPECT_EQ(ret, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}
} // namespace AAFwk
} // namespace OHOS
