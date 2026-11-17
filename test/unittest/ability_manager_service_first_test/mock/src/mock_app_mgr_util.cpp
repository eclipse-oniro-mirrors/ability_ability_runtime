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

#include "utils/app_mgr_util.h"

namespace OHOS {
namespace AAFwk {
sptr<OHOS::AppExecFwk::IAppMgr> AppMgrUtil::appMgr_ = nullptr;

// Link-time replacement of AppMgrUtil::GetAppMgr so the test binary never touches
// samgr: it just returns the cached appMgr_, which test cases seed directly (the test
// TU includes the header with `#define private public`), or nullptr when unseeded.
OHOS::sptr<OHOS::AppExecFwk::IAppMgr> AppMgrUtil::GetAppMgr()
{
    return appMgr_;
}
}  // namespace AAFwk
}  // namespace OHOS
