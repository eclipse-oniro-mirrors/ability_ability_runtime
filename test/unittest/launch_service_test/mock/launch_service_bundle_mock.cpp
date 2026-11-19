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

#include "launch_service_bundle_mock.h"
namespace OHOS::AppExecFwk {
namespace {
constexpr int32_t TEST_DATA_GROUP_GID = 123;
}
LaunchServiceBundleMock &LaunchServiceBundleMock::Instance()
{
    static LaunchServiceBundleMock mock;
    return mock;
}
void LaunchServiceBundleMock::Reset()
{
    *this = LaunchServiceBundleMock{};
    services.developerId = developerId;
    LaunchService service;
    service.name = "worker";
    service.type = LaunchServiceType::DAEMON;
    services.launchServices.push_back(service);
}
ErrCode BundleMgrHelper::GetUidByBundleName(const std::string &, int32_t, int32_t)
{
    return LaunchServiceBundleMock::Instance().uid;
}
ErrCode BundleMgrHelper::GetNameForUid(const int32_t, std::string &name)
{
    auto &mock = LaunchServiceBundleMock::Instance();
    name = mock.callerName;
    return mock.result;
}
ErrCode BundleMgrHelper::GetNameAndIndexForUid(const int32_t, std::string &name, int32_t &index)
{
    auto &mock = LaunchServiceBundleMock::Instance();
    name = mock.callerName;
    index = 0;
    return mock.result;
}
ErrCode BundleMgrHelper::GetAppProvisionInfo(const std::string &, int32_t, AppProvisionInfo &info)
{
    auto &mock = LaunchServiceBundleMock::Instance();
    info.developerId = mock.developerId;
    return mock.provisionResult;
}
ErrCode BundleMgrHelper::GetLaunchServices(const std::string &, const std::string &, int32_t,
    LaunchServicesResult &result)
{
    auto &mock = LaunchServiceBundleMock::Instance();
    result = mock.services;
    return mock.result;
}
bool BundleMgrHelper::GetBundleInfo(const std::string &, int32_t, BundleInfo &info, int32_t)
{
    auto &mock = LaunchServiceBundleMock::Instance();
    info = mock.bundle;
    return mock.bundleAvailable;
}
ErrCode BundleMgrHelper::GetBaseSharedBundleInfos(const std::string &,
    std::vector<BaseSharedBundleInfo> &infos, GetDependentBundleInfoFlag)
{
    auto result = LaunchServiceBundleMock::Instance().hspResult;
    if (result == ERR_OK) { infos.resize(1); }
    return result;
}
bool BundleMgrHelper::QueryDataGroupInfos(const std::string &, int32_t, std::vector<DataGroupInfo> &infos)
{
    infos.resize(1);
    infos[0].gid = TEST_DATA_GROUP_GID;
    return true;
}
sptr<IOverlayManager> BundleMgrHelper::GetOverlayManagerProxy()
{
    return nullptr;
}
} // namespace OHOS::AppExecFwk
