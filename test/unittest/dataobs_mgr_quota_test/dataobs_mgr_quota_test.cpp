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
#include <gmock/gmock.h>
#include <memory>
#include <string>
#include "uri.h"
#include "data_ability_observer_stub.h"
#include "dataobs_mgr_errors.h"
#include "dataobs_mgr_inner.h"
#include "dataobs_mgr_inner_ext.h"
#include "dataobs_mgr_inner_pref.h"
#include "obs_token_quota.h"

using namespace OHOS;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t USER_TEST = 100;
constexpr uint32_t TOKEN_QUOTA_TEST = 0x1001;
constexpr uint32_t TOKEN_INNER_TEST = 0x2001;
constexpr uint32_t TOKEN_INNER_OTHER = 0x2002;
constexpr uint32_t TOKEN_PREF_TEST = 0x3001;
constexpr uint32_t TOKEN_EXT_TEST = 0x4001;
constexpr uint32_t TOKEN_SHARED_TEST = 0x5001;

class MockQuotaDataAbilityObserver : public DataAbilityObserverStub {
public:
    MOCK_METHOD0(OnChange, void());
    MOCK_METHOD1(OnChangeExt, void(const ChangeInfo &));
    MOCK_METHOD1(OnChangePreferences, void(const std::string &));
};

sptr<IDataAbilityObserver> CreateObserver()
{
    return sptr<MockQuotaDataAbilityObserver>(new (std::nothrow) MockQuotaDataAbilityObserver());
}
}  // namespace

class DataObsMgrQuotaTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DataObsMgrQuotaTest::SetUpTestCase(void) {}
void DataObsMgrQuotaTest::TearDownTestCase(void) {}
void DataObsMgrQuotaTest::SetUp() {}
void DataObsMgrQuotaTest::TearDown()
{
    auto &quota = ObsTokenQuota::GetInstance();
    for (auto tokenId : { TOKEN_QUOTA_TEST, TOKEN_INNER_TEST, TOKEN_INNER_OTHER,
                          TOKEN_PREF_TEST, TOKEN_EXT_TEST, TOKEN_SHARED_TEST }) {
        uint32_t held = quota.GetCount(tokenId);
        if (held > 0) {
            quota.Release(tokenId, held);
        }
    }
}

/*
 * Feature: ObsTokenQuota
 * Function: Per-token observer quota
 * SubFunction: TryAcquire/GetCount
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: One tokenId holds at most TOKEN_OBSERVER_TOTAL_MAX slots; the request that
 * would exceed the limit fails and does not change the count.
 */
HWTEST_F(DataObsMgrQuotaTest, ObsTokenQuota_TryAcquire_0100, TestSize.Level1)
{
    auto &quota = ObsTokenQuota::GetInstance();
    for (uint32_t i = 0; i < ObsTokenQuota::TOKEN_OBSERVER_TOTAL_MAX; i++) {
        EXPECT_TRUE(quota.TryAcquire(TOKEN_QUOTA_TEST));
    }
    EXPECT_EQ(quota.GetCount(TOKEN_QUOTA_TEST), ObsTokenQuota::TOKEN_OBSERVER_TOTAL_MAX);
    EXPECT_FALSE(quota.TryAcquire(TOKEN_QUOTA_TEST));
    EXPECT_EQ(quota.GetCount(TOKEN_QUOTA_TEST), ObsTokenQuota::TOKEN_OBSERVER_TOTAL_MAX);
}

/*
 * Feature: ObsTokenQuota
 * Function: Per-token observer quota
 * SubFunction: Release/GetCount
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Release gives slots back, erases the entry at zero, and an over-release
 * never underflows the counter.
 */
HWTEST_F(DataObsMgrQuotaTest, ObsTokenQuota_Release_0100, TestSize.Level1)
{
    auto &quota = ObsTokenQuota::GetInstance();
    constexpr uint32_t acquired = 3;
    for (uint32_t i = 0; i < acquired; i++) {
        EXPECT_TRUE(quota.TryAcquire(TOKEN_QUOTA_TEST));
    }
    quota.Release(TOKEN_QUOTA_TEST, 2);
    EXPECT_EQ(quota.GetCount(TOKEN_QUOTA_TEST), 1);
    quota.Release(TOKEN_QUOTA_TEST);
    EXPECT_EQ(quota.GetCount(TOKEN_QUOTA_TEST), 0);
    quota.Release(TOKEN_QUOTA_TEST);
    EXPECT_EQ(quota.GetCount(TOKEN_QUOTA_TEST), 0);
}

/*
 * Feature: DataObsMgrInner
 * Function: Register observer quota
 * SubFunction: HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Registering more than TOKEN_OBSERVER_TOTAL_MAX observers for one tokenId
 * across different URIs fails with DATAOBS_SERVICE_OBS_LIMMIT.
 */
HWTEST_F(DataObsMgrQuotaTest, DataObsMgrInner_HandleRegisterObserver_0100, TestSize.Level1)
{
    auto inner = std::make_shared<DataObsMgrInner>();
    ASSERT_TRUE(inner != nullptr);
    auto observer = CreateObserver();
    for (uint32_t i = 0; i < ObsTokenQuota::TOKEN_OBSERVER_TOTAL_MAX; i++) {
        Uri uri("dataability://com.ohos.unittest.inner/uri" + std::to_string(i));
        EXPECT_EQ(inner->HandleRegisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_INNER_TEST, 0)),
            NO_ERROR);
    }
    Uri extraUri("dataability://com.ohos.unittest.inner/extra");
    EXPECT_EQ(
        inner->HandleRegisterObserver(extraUri, ObserverNode(observer, USER_TEST, TOKEN_INNER_TEST, 0)),
        DATAOBS_SERVICE_OBS_LIMMIT);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_INNER_TEST),
        ObsTokenQuota::TOKEN_OBSERVER_TOTAL_MAX);
}

/*
 * Feature: DataObsMgrInner
 * Function: Register observer quota
 * SubFunction: HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: The quota of one tokenId does not affect other tokenIds.
 */
HWTEST_F(DataObsMgrQuotaTest, DataObsMgrInner_HandleRegisterObserver_0200, TestSize.Level1)
{
    auto inner = std::make_shared<DataObsMgrInner>();
    ASSERT_TRUE(inner != nullptr);
    auto observer = CreateObserver();
    for (uint32_t i = 0; i < ObsTokenQuota::TOKEN_OBSERVER_TOTAL_MAX; i++) {
        Uri uri("dataability://com.ohos.unittest.inner/uri" + std::to_string(i));
        EXPECT_EQ(inner->HandleRegisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_INNER_TEST, 0)),
            NO_ERROR);
    }
    Uri otherUri("dataability://com.ohos.unittest.inner/other");
    EXPECT_EQ(
        inner->HandleRegisterObserver(otherUri, ObserverNode(observer, USER_TEST, TOKEN_INNER_OTHER, 0)),
        NO_ERROR);
}

/*
 * Feature: DataObsMgrInner
 * Function: Register observer quota
 * SubFunction: HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: A duplicate registration (OBS_EXIST) does not consume an extra quota slot.
 */
HWTEST_F(DataObsMgrQuotaTest, DataObsMgrInner_HandleRegisterObserver_0300, TestSize.Level1)
{
    auto inner = std::make_shared<DataObsMgrInner>();
    ASSERT_TRUE(inner != nullptr);
    auto observer = CreateObserver();
    Uri uri("dataability://com.ohos.unittest.inner/dup");
    EXPECT_EQ(inner->HandleRegisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_INNER_TEST, 0)),
        NO_ERROR);
    EXPECT_EQ(inner->HandleRegisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_INNER_TEST, 0)),
        OBS_EXIST);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_INNER_TEST), 1);
}

/*
 * Feature: DataObsMgrInner
 * Function: Unregister observer quota
 * SubFunction: HandleUnregisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: HandleUnregisterObserver releases the quota slot, so a new registration
 * succeeds afterwards.
 */
HWTEST_F(DataObsMgrQuotaTest, DataObsMgrInner_HandleUnregisterObserver_0100, TestSize.Level1)
{
    auto inner = std::make_shared<DataObsMgrInner>();
    ASSERT_TRUE(inner != nullptr);
    auto observer = CreateObserver();
    Uri uri("dataability://com.ohos.unittest.inner/release");
    EXPECT_EQ(inner->HandleRegisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_INNER_TEST, 0)),
        NO_ERROR);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_INNER_TEST), 1);
    EXPECT_EQ(inner->HandleUnregisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_INNER_TEST, 0)),
        NO_ERROR);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_INNER_TEST), 0);
    EXPECT_EQ(inner->HandleRegisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_INNER_TEST, 0)),
        NO_ERROR);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_INNER_TEST), 1);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: Register observer quota
 * SubFunction: HandleRegisterObserver/HandleUnregisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: The preferences observer store enforces the same per-token quota, and
 * unregister releases the slot.
 */
HWTEST_F(DataObsMgrQuotaTest, DataObsMgrInnerPref_HandleRegisterObserver_0100, TestSize.Level1)
{
    auto pref = std::make_shared<DataObsMgrInnerPref>();
    ASSERT_TRUE(pref != nullptr);
    auto observer = CreateObserver();
    for (uint32_t i = 0; i < ObsTokenQuota::TOKEN_OBSERVER_TOTAL_MAX; i++) {
        Uri uri("preferences://com.ohos.unittest.pref/key" + std::to_string(i));
        EXPECT_EQ(pref->HandleRegisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_PREF_TEST, 0)),
            NO_ERROR);
    }
    Uri extraUri("preferences://com.ohos.unittest.pref/extra");
    EXPECT_EQ(
        pref->HandleRegisterObserver(extraUri, ObserverNode(observer, USER_TEST, TOKEN_PREF_TEST, 0)),
        DATAOBS_SERVICE_OBS_LIMMIT);
    Uri firstUri("preferences://com.ohos.unittest.pref/key0");
    EXPECT_EQ(pref->HandleUnregisterObserver(firstUri, ObserverNode(observer, USER_TEST, TOKEN_PREF_TEST, 0)),
        NO_ERROR);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_PREF_TEST),
        ObsTokenQuota::TOKEN_OBSERVER_TOTAL_MAX - 1);
    EXPECT_EQ(
        pref->HandleRegisterObserver(extraUri, ObserverNode(observer, USER_TEST, TOKEN_PREF_TEST, 0)),
        NO_ERROR);
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: Register observer quota
 * SubFunction: HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: The observer trie enforces the same per-token quota.
 */
HWTEST_F(DataObsMgrQuotaTest, DataObsMgrInnerExt_HandleRegisterObserver_0100, TestSize.Level1)
{
    auto ext = std::make_shared<DataObsMgrInnerExt>();
    ASSERT_TRUE(ext != nullptr);
    auto observer = CreateObserver();
    for (uint32_t i = 0; i < ObsTokenQuota::TOKEN_OBSERVER_TOTAL_MAX; i++) {
        Uri uri("datashare://com.ohos.unittest.ext/seg" + std::to_string(i));
        ObserverInfo info(TOKEN_EXT_TEST, 0, 0, USER_TEST, false);
        EXPECT_EQ(ext->HandleRegisterObserver(uri, observer, info, false), SUCCESS);
    }
    Uri extraUri("datashare://com.ohos.unittest.ext/extra");
    ObserverInfo extraInfo(TOKEN_EXT_TEST, 0, 0, USER_TEST, false);
    EXPECT_EQ(ext->HandleRegisterObserver(extraUri, observer, extraInfo, false),
        DATAOBS_SERVICE_OBS_LIMMIT);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_EXT_TEST),
        ObsTokenQuota::TOKEN_OBSERVER_TOTAL_MAX);
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: Unregister observer quota
 * SubFunction: HandleUnregisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Unregistering one URI releases its slot, and unregistering the observer
 * from all URIs releases every slot it still holds.
 */
HWTEST_F(DataObsMgrQuotaTest, DataObsMgrInnerExt_HandleUnregisterObserver_0100, TestSize.Level1)
{
    constexpr uint32_t registered = 3;
    auto ext = std::make_shared<DataObsMgrInnerExt>();
    ASSERT_TRUE(ext != nullptr);
    auto observer = CreateObserver();
    for (uint32_t i = 0; i < registered; i++) {
        Uri uri("datashare://com.ohos.unittest.ext/seg" + std::to_string(i));
        ObserverInfo info(TOKEN_EXT_TEST, 0, 0, USER_TEST, false);
        EXPECT_EQ(ext->HandleRegisterObserver(uri, observer, info, false), SUCCESS);
    }
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_EXT_TEST), registered);
    Uri firstUri("datashare://com.ohos.unittest.ext/seg0");
    EXPECT_EQ(ext->HandleUnregisterObserver(firstUri, observer), SUCCESS);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_EXT_TEST), registered - 1);
    EXPECT_EQ(ext->HandleUnregisterObserver(observer), SUCCESS);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_EXT_TEST), 0);
}

/*
 * Feature: DataObsMgr
 * Function: Shared per-token quota
 * SubFunction: HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: The classic store, the preferences store and the trie share one
 * per-token budget; exhausting it in one channel blocks the others.
 */
HWTEST_F(DataObsMgrQuotaTest, DataObsMgr_QuotaShared_0100, TestSize.Level1)
{
    auto inner = std::make_shared<DataObsMgrInner>();
    auto pref = std::make_shared<DataObsMgrInnerPref>();
    auto ext = std::make_shared<DataObsMgrInnerExt>();
    ASSERT_TRUE(inner != nullptr && pref != nullptr && ext != nullptr);
    auto observer = CreateObserver();
    uint32_t half = ObsTokenQuota::TOKEN_OBSERVER_TOTAL_MAX / 2;
    for (uint32_t i = 0; i < half; i++) {
        Uri classicUri("dataability://com.ohos.unittest.shared/c" + std::to_string(i));
        EXPECT_EQ(
            inner->HandleRegisterObserver(classicUri, ObserverNode(observer, USER_TEST, TOKEN_SHARED_TEST, 0)),
            NO_ERROR);
        Uri extUri("datashare://com.ohos.unittest.shared/e" + std::to_string(i));
        ObserverInfo info(TOKEN_SHARED_TEST, 0, 0, USER_TEST, false);
        EXPECT_EQ(ext->HandleRegisterObserver(extUri, observer, info, false), SUCCESS);
    }
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_SHARED_TEST),
        ObsTokenQuota::TOKEN_OBSERVER_TOTAL_MAX);
    Uri classicExtra("dataability://com.ohos.unittest.shared/extra");
    EXPECT_EQ(
        inner->HandleRegisterObserver(classicExtra, ObserverNode(observer, USER_TEST, TOKEN_SHARED_TEST, 0)),
        DATAOBS_SERVICE_OBS_LIMMIT);
    Uri prefExtra("preferences://com.ohos.unittest.shared/extra");
    EXPECT_EQ(
        pref->HandleRegisterObserver(prefExtra, ObserverNode(observer, USER_TEST, TOKEN_SHARED_TEST, 0)),
        DATAOBS_SERVICE_OBS_LIMMIT);
    Uri extExtra("datashare://com.ohos.unittest.shared/extra");
    ObserverInfo extraInfo(TOKEN_SHARED_TEST, 0, 0, USER_TEST, false);
    EXPECT_EQ(ext->HandleRegisterObserver(extExtra, observer, extraInfo, false),
        DATAOBS_SERVICE_OBS_LIMMIT);
}

/*
 * Feature: DataObsMgrInner
 * Function: Register observer quota
 * SubFunction: HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: A failed registration on a fresh URI must not leave an empty node in
 * observers_; otherwise repeated failed registrations could grow the map without bound.
 * Verified indirectly via HandleNotifyChange: an erased URI returns NO_OBS_FOR_URI, while
 * a leaked empty node would return NO_ERROR.
 */
HWTEST_F(DataObsMgrQuotaTest, DataObsMgrInner_QuotaFailureNoLeak_0100, TestSize.Level1)
{
    auto inner = std::make_shared<DataObsMgrInner>();
    ASSERT_TRUE(inner != nullptr);
    auto observer = CreateObserver();
    // Exhaust the per-token quota.
    for (uint32_t i = 0; i < ObsTokenQuota::TOKEN_OBSERVER_TOTAL_MAX; i++) {
        Uri uri("dataability://com.ohos.unittest.noleak/uri" + std::to_string(i));
        EXPECT_EQ(inner->HandleRegisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_INNER_TEST, 0)),
            NO_ERROR);
    }
    // A registration on a brand-new URI fails at the quota check. The empty node inserted by
    // try_emplace must be rolled back so observers_ does not accumulate empty entries.
    Uri leakUri("dataability://com.ohos.unittest.noleak/leak");
    EXPECT_EQ(
        inner->HandleRegisterObserver(leakUri, ObserverNode(observer, USER_TEST, TOKEN_INNER_TEST, 0)),
        DATAOBS_SERVICE_OBS_LIMMIT);
    EXPECT_EQ(inner->HandleNotifyChange(leakUri, USER_TEST, "", false, 0), NO_OBS_FOR_URI);
}

/*
 * Feature: DataObsMgrInner
 * Function: Observer death callback
 * SubFunction: OnCallBackDied
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: When an observer binder dies, OnCallBackDied removes every entry for that
 * observer across all URIs and releases the corresponding per-token quota slots.
 */
HWTEST_F(DataObsMgrQuotaTest, DataObsMgrInner_OnCallBackDied_0100, TestSize.Level1)
{
    auto inner = std::make_shared<DataObsMgrInner>();
    ASSERT_TRUE(inner != nullptr);
    auto observer = CreateObserver();
    constexpr uint32_t registered = 3;
    for (uint32_t i = 0; i < registered; i++) {
        Uri uri("dataability://com.ohos.unittest.died/uri" + std::to_string(i));
        EXPECT_EQ(inner->HandleRegisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_INNER_TEST, 0)),
            NO_ERROR);
    }
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_INNER_TEST), registered);
    // Simulate observer binder death.
    auto remoteObj = observer->AsObject();
    ASSERT_TRUE(remoteObj != nullptr);
    wptr<IRemoteObject> remoteWeak(remoteObj);
    inner->OnCallBackDied(remoteWeak);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_INNER_TEST), 0);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: Observer death callback
 * SubFunction: OnCallBackDied
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: The preferences channel releases quota slots on observer death as well.
 */
HWTEST_F(DataObsMgrQuotaTest, DataObsMgrInnerPref_OnCallBackDied_0100, TestSize.Level1)
{
    auto pref = std::make_shared<DataObsMgrInnerPref>();
    ASSERT_TRUE(pref != nullptr);
    auto observer = CreateObserver();
    constexpr uint32_t registered = 3;
    for (uint32_t i = 0; i < registered; i++) {
        Uri uri("preferences://com.ohos.unittest.died/key" + std::to_string(i));
        EXPECT_EQ(pref->HandleRegisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_PREF_TEST, 0)),
            NO_ERROR);
    }
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_PREF_TEST), registered);
    auto remoteObj = observer->AsObject();
    ASSERT_TRUE(remoteObj != nullptr);
    wptr<IRemoteObject> remoteWeak(remoteObj);
    pref->OnCallBackDied(remoteWeak);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_PREF_TEST), 0);
}

/*
 * Feature: DataObsMgrInnerExt
 * Function: Observer death callback
 * SubFunction: OnCallBackDied
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: The trie channel releases quota slots on observer death across all URIs.
 */
HWTEST_F(DataObsMgrQuotaTest, DataObsMgrInnerExt_OnCallBackDied_0100, TestSize.Level1)
{
    auto ext = std::make_shared<DataObsMgrInnerExt>();
    ASSERT_TRUE(ext != nullptr);
    auto observer = CreateObserver();
    constexpr uint32_t registered = 3;
    for (uint32_t i = 0; i < registered; i++) {
        Uri uri("datashare://com.ohos.unittest.died/seg" + std::to_string(i));
        ObserverInfo info(TOKEN_EXT_TEST, 0, 0, USER_TEST, false);
        EXPECT_EQ(ext->HandleRegisterObserver(uri, observer, info, false), SUCCESS);
    }
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_EXT_TEST), registered);
    auto remoteObj = observer->AsObject();
    ASSERT_TRUE(remoteObj != nullptr);
    wptr<IRemoteObject> remoteWeak(remoteObj);
    ext->OnCallBackDied(remoteWeak);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_EXT_TEST), 0);
}

/*
 * Feature: DataObsMgrInner
 * Function: Unregister observer quota
 * SubFunction: HandleUnregisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: The unregister find predicate matches only on the observer binder object,
 * ignoring tokenId_. This is the FromExtension contract: an observer registered under the
 * first-caller token must be released by the stored token, not the unregister caller's token.
 * If this contract breaks, FromExtension quota silently leaks.
 */
HWTEST_F(DataObsMgrQuotaTest, DataObsMgrInner_UnregisterReleasesStoredToken_0100, TestSize.Level1)
{
    auto inner = std::make_shared<DataObsMgrInner>();
    ASSERT_TRUE(inner != nullptr);
    auto observer = CreateObserver();
    Uri uri("dataability://com.ohos.unittest.token/uri");
    // Register under TOKEN_INNER_TEST (simulating a stored first-caller token).
    EXPECT_EQ(inner->HandleRegisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_INNER_TEST, 0)),
        NO_ERROR);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_INNER_TEST), 1);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_INNER_OTHER), 0);
    // Unregister under TOKEN_INNER_OTHER (simulating the provider calling unregister with its
    // own token). The stored TOKEN_INNER_TEST slot must be released, not TOKEN_INNER_OTHER.
    EXPECT_EQ(inner->HandleUnregisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_INNER_OTHER, 0)),
        NO_ERROR);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_INNER_TEST), 0);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_INNER_OTHER), 0);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: Unregister observer quota
 * SubFunction: HandleUnregisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: The preferences channel shares the same FromExtension contract: unregister
 * releases the stored token, not the unregister caller's token.
 */
HWTEST_F(DataObsMgrQuotaTest, DataObsMgrInnerPref_UnregisterReleasesStoredToken_0100, TestSize.Level1)
{
    auto pref = std::make_shared<DataObsMgrInnerPref>();
    ASSERT_TRUE(pref != nullptr);
    auto observer = CreateObserver();
    Uri uri("preferences://com.ohos.unittest.token/key");
    EXPECT_EQ(pref->HandleRegisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_PREF_TEST, 0)),
        NO_ERROR);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_PREF_TEST), 1);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_INNER_OTHER), 0);
    EXPECT_EQ(pref->HandleUnregisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_INNER_OTHER, 0)),
        NO_ERROR);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_PREF_TEST), 0);
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_INNER_OTHER), 0);
}

/*
 * Feature: DataObsMgrInnerPref
 * Function: Register observer quota
 * SubFunction: HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: A failed registration on a fresh URI must not leave an empty node in
 * observers_ (preferences channel). Verified via HandleNotifyChange: an erased URI returns
 * NO_OBS_FOR_URI, while a leaked empty node would return NO_ERROR.
 */
HWTEST_F(DataObsMgrQuotaTest, DataObsMgrInnerPref_QuotaFailureNoLeak_0100, TestSize.Level1)
{
    auto pref = std::make_shared<DataObsMgrInnerPref>();
    ASSERT_TRUE(pref != nullptr);
    auto observer = CreateObserver();
    for (uint32_t i = 0; i < ObsTokenQuota::TOKEN_OBSERVER_TOTAL_MAX; i++) {
        Uri uri("preferences://com.ohos.unittest.noleakpref/key" + std::to_string(i));
        EXPECT_EQ(pref->HandleRegisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_PREF_TEST, 0)),
            NO_ERROR);
    }
    // A registration on a brand-new URI fails at the quota check. The empty node inserted by
    // try_emplace must be rolled back so observers_ does not accumulate empty entries.
    Uri leakUri("preferences://com.ohos.unittest.noleakpref/leak");
    EXPECT_EQ(
        pref->HandleRegisterObserver(leakUri, ObserverNode(observer, USER_TEST, TOKEN_PREF_TEST, 0)),
        DATAOBS_SERVICE_OBS_LIMMIT);
    // HandleNotifyChange requires a query string; the observerKey is the part before '?'.
    // With the rollback the key is absent → NO_OBS_FOR_URI; without it the empty node → NO_ERROR.
    Uri notifyUri("preferences://com.ohos.unittest.noleakpref/leak?x=y");
    EXPECT_EQ(pref->HandleNotifyChange(notifyUri, USER_TEST, 0), NO_OBS_FOR_URI);
}

/*
 * Feature: DataObsMgrInner
 * Function: Register observer quota
 * SubFunction: HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: When TryAcquire fails on a URI that already has observers, the existing
 * entries must not be erased. Only empty (freshly try_emplace'd) nodes are rolled back.
 */
HWTEST_F(DataObsMgrQuotaTest, DataObsMgrInner_QuotaFailurePreservesExisting_0100, TestSize.Level1)
{
    auto inner = std::make_shared<DataObsMgrInner>();
    ASSERT_TRUE(inner != nullptr);
    auto observer = CreateObserver();
    // Register one observer on existingUri, then exhaust the remaining quota on other URIs.
    Uri existingUri("dataability://com.ohos.unittest.existing/uri0");
    EXPECT_EQ(inner->HandleRegisterObserver(existingUri, ObserverNode(observer, USER_TEST, TOKEN_INNER_TEST, 0)),
        NO_ERROR);
    for (uint32_t i = 1; i < ObsTokenQuota::TOKEN_OBSERVER_TOTAL_MAX; i++) {
        Uri uri("dataability://com.ohos.unittest.existing/uri" + std::to_string(i));
        EXPECT_EQ(inner->HandleRegisterObserver(uri, ObserverNode(observer, USER_TEST, TOKEN_INNER_TEST, 0)),
            NO_ERROR);
    }
    EXPECT_EQ(ObsTokenQuota::GetInstance().GetCount(TOKEN_INNER_TEST),
        ObsTokenQuota::TOKEN_OBSERVER_TOTAL_MAX);
    // A second observer on the existing URI fails at the quota check. The URI already has
    // content from the first observer; the rollback must NOT erase the existing entry.
    auto observer2 = CreateObserver();
    EXPECT_EQ(
        inner->HandleRegisterObserver(existingUri, ObserverNode(observer2, USER_TEST, TOKEN_INNER_TEST, 0)),
        DATAOBS_SERVICE_OBS_LIMMIT);
    // The existing observer's entry is still there: HandleNotifyChange returns NO_ERROR
    // (would return NO_OBS_FOR_URI if the entry had been erased).
    EXPECT_EQ(inner->HandleNotifyChange(existingUri, USER_TEST, "", false, 0), NO_ERROR);
}
}  // namespace AAFwk
}  // namespace OHOS
