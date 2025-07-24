/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_REMOTE_CLIENT_MANAGER_H
#define OHOS_ABILITY_RUNTIME_REMOTE_CLIENT_MANAGER_H

#include "iremote_object.h"
#include "refbase.h"

#include "app_spawn_client.h"
#include "bundle_mgr_helper.h"

namespace OHOS {
namespace AppExecFwk {
class RemoteClientManager {
public:
    RemoteClientManager();
    virtual ~RemoteClientManager();

    /**
     * GetSpawnClient, Get spawn client.
     *
     * @return the spawn client instance.
     */
    std::shared_ptr<AppSpawnClient> GetSpawnClient();

    /**
     * @brief Setting spawn client instance.
     *
     * @param appSpawnClient, the spawn client instance.
     */
    void SetSpawnClient(const std::shared_ptr<AppSpawnClient> &appSpawnClient);

    /**
     * GetBundleManager, Get bundle management services.
     *
     * @return the bundle management services instance.
     */
    std::shared_ptr<BundleMgrHelper> GetBundleManagerHelper();

    /**
     * @brief Setting bundle management instance.
     *
     * @param appSpawnClient, the bundle management instance.
     */
    void SetBundleManagerHelper(const std::shared_ptr<BundleMgrHelper> &bundleMgrHelper);

    std::shared_ptr<AppSpawnClient> GetNWebSpawnClient();

    std::shared_ptr<AppSpawnClient> GetCJSpawnClient();

    std::shared_ptr<AppSpawnClient> GetNativeSpawnClient();

    std::shared_ptr<AppSpawnClient> GetHybridSpawnClient();

private:
    std::shared_ptr<AppSpawnClient> appSpawnClient_;
    std::shared_ptr<BundleMgrHelper> bundleManagerHelper_;
    std::shared_ptr<AppSpawnClient> nwebSpawnClient_;
    std::shared_ptr<AppSpawnClient> cjAppSpawnClient_;
    std::shared_ptr<AppSpawnClient> nativeSpawnClient_;
    std::shared_ptr<AppSpawnClient> hybridSpawnClient_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_REMOTE_CLIENT_MANAGER_H
