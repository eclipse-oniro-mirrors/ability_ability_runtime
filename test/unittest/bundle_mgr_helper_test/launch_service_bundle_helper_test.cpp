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
#include <mutex>
#define private public
#include "bundle_mgr_helper.h"
#undef private
#include "bundle_mgr_proxy.h"
#include "appexecfwk_errors.h"
using namespace testing::ext;
namespace OHOS::AppExecFwk {
class LaunchServiceBundleProxyMock : public BundleMgrProxy {
public:
    LaunchServiceBundleProxyMock() : BundleMgrProxy(nullptr) {}
    int32_t result = ERR_OK;
    std::string bundle;
    std::string module;
    int32_t user = -1;
    ErrCode GetAppProvisionInfo(const std::string &bundleName, int32_t userId, AppProvisionInfo &info) override
    {
        bundle = bundleName;
        user = userId;
        if (result == ERR_OK) { info.developerId = "developer"; }
        return result;
    }
    ErrCode GetLaunchServices(const std::string &bundleName, const std::string &moduleName,
        int32_t userId, LaunchServicesResult &info) override
    {
        bundle = bundleName;
        module = moduleName;
        user = userId;
        if (result == ERR_OK) { info.developerId = "developer"; }
        return result;
    }
};
class LaunchServiceBundleHelperTest : public testing::Test {};

/**
 * @tc.name: GetAppProvisionInfo_001
 * @tc.desc: Preserve BundleMgr arguments, success data, failures and disconnected result.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceBundleHelperTest, GetAppProvisionInfo_001, TestSize.Level1)
{
    BundleMgrHelper helper;
    auto remote = sptr<LaunchServiceBundleProxyMock>::MakeSptr();
    helper.bundleMgr_ = remote;
    AppProvisionInfo info;
    EXPECT_EQ(helper.GetAppProvisionInfo("com.example.daemon", 100, info), ERR_OK);
    EXPECT_EQ(info.developerId, "developer");
    EXPECT_EQ(remote->bundle, "com.example.daemon");
    EXPECT_EQ(remote->user, 100);
    remote->result = ERR_INVALID_VALUE;
    EXPECT_EQ(helper.GetAppProvisionInfo("com.example.daemon", 100, info), ERR_INVALID_VALUE);
    helper.bundleMgr_ = nullptr;
    helper.bmsReady_ = false;
    EXPECT_EQ(helper.GetAppProvisionInfo("com.example.daemon", 100, info), ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR);
}

/**
 * @tc.name: GetLaunchServices_001
 * @tc.desc: Forward module and user to BundleMgr and preserve error codes.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceBundleHelperTest, GetLaunchServices_001, TestSize.Level1)
{
    BundleMgrHelper helper;
    auto remote = sptr<LaunchServiceBundleProxyMock>::MakeSptr();
    helper.bundleMgr_ = remote;
    LaunchServicesResult info;
    EXPECT_EQ(helper.GetLaunchServices("com.example.daemon", "entry", 100, info), ERR_OK);
    EXPECT_EQ(info.developerId, "developer");
    EXPECT_EQ(remote->bundle, "com.example.daemon");
    EXPECT_EQ(remote->module, "entry");
    EXPECT_EQ(remote->user, 100);
    remote->result = ERR_INVALID_VALUE;
    EXPECT_EQ(helper.GetLaunchServices("com.example.daemon", "entry", 100, info), ERR_INVALID_VALUE);
    helper.bundleMgr_ = nullptr;
    helper.bmsReady_ = false;
    EXPECT_EQ(helper.GetLaunchServices("com.example.daemon", "entry", 100, info),
        ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR);
}
} // namespace OHOS::AppExecFwk
