/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CHILD_PROCESS_INFO_H
#define OHOS_ABILITY_RUNTIME_CHILD_PROCESS_INFO_H

#include <string>

#include "parcel.h"
#include "bundle_info.h"
#include "shared/base_shared_bundle_info.h"

namespace OHOS {
namespace AppExecFwk {

constexpr int32_t CHILD_PROCESS_TYPE_NOT_CHILD = -1;
constexpr int32_t CHILD_PROCESS_TYPE_JS = 0;
constexpr int32_t CHILD_PROCESS_TYPE_NATIVE = 1;
constexpr int32_t CHILD_PROCESS_TYPE_ARK = 2;
constexpr int32_t CHILD_PROCESS_TYPE_NATIVE_ARGS = 3;

struct ChildProcessInfo : public Parcelable {
    bool jitEnabled = false;
    bool isDebugApp = true;
    bool isStartWithDebug = false;
    bool isStartWithNative = false;
    bool isStaticChildProcess = false;
    int32_t pid = 0;
    int32_t hostPid = 0;
    int32_t uid = -1;
    int32_t hostUid = -1;
    int32_t userId = -1;
    int32_t childProcessType = CHILD_PROCESS_TYPE_JS;
    std::string bundleName;
    std::string processName;
    std::string srcEntry;
    std::string entryFunc;
    std::string entryParams;
    std::shared_ptr<BundleInfo> bundleInfo = nullptr;
    std::vector<BaseSharedBundleInfo> hspList;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static ChildProcessInfo *Unmarshalling(Parcel &parcel);
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_CHILD_PROCESS_INFO_H
