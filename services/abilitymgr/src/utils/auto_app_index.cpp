/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "auto_app_index.h"

#include "clone_for_account_util.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
AutoAppIndex::AutoAppIndex(Want &want, sptr<IRemoteObject> callerToken, int32_t userId)
{
    auto bundleName = want.GetBundle();
    if (!bundleName.empty()) {
        int32_t cachedAppIndex = 0;
        if (CloneForAccountUtil::GetCachedAppIndex(bundleName, cachedAppIndex)) {
            want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, cachedAppIndex);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "AutoAppIndex cache hit, bundleName=%{public}s, appIndex=%{public}d",
                bundleName.c_str(), cachedAppIndex);
            return;
        }
    }

    CloneForAccountUtil::ProcessAppIndex(want, callerToken, userId);

    if (!bundleName.empty() && want.HasParameter(Want::PARAM_APP_CLONE_INDEX_KEY)) {
        int32_t resolvedAppIndex = want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, 0);
        CloneForAccountUtil::CacheAppIndex(bundleName, resolvedAppIndex);
        bundleName_ = bundleName;
        TAG_LOGI(AAFwkTag::ABILITYMGR, "AutoAppIndex cached, bundleName=%{public}s, appIndex=%{public}d",
            bundleName.c_str(), resolvedAppIndex);
    }
}

AutoAppIndex::~AutoAppIndex()
{
    if (!bundleName_.empty()) {
        CloneForAccountUtil::RemoveCachedAppIndex(bundleName_);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "AutoAppIndex removed cache for bundleName=%{public}s", bundleName_.c_str());
    }
}
}  // namespace AAFwk
}  // namespace OHOS
