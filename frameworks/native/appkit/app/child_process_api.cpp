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
 
#include "child_process_api.h"
#include "hilog_tag_wrapper.h"
#include <dlfcn.h>
 
namespace OHOS {
namespace AppExecFwk {
 
void ChildProcessApi::StartChild(const std::map<std::string, int32_t> &fds)
{
    TAG_LOGD(AAFwkTag::APPKIT, "ChildProcessApi StartChild, fds size:%{public}zu", fds.size());
    static void *handle = dlopen("libappkit_child.z.so", RTLD_LAZY | RTLD_LOCAL);
    if (handle == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to dlopen libappkit_child.z.so %{public}s", dlerror());
        return;
    }
 
    using StartChildFunc = void(*)(const std::map<std::string, int32_t>&);
    static StartChildFunc startChildFunc = reinterpret_cast<StartChildFunc>(dlsym(handle, "ChildMainThreadStart"));
    if (startChildFunc == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to dlsym ChildMainThreadStart %{public}s", dlerror());
        dlclose(handle);
        handle = nullptr;
        return;
    }
 
    TAG_LOGI(AAFwkTag::APPKIT, "Success to start child main thread");
    startChildFunc(fds);
}
 
}  // namespace AppExecFwk
}  // namespace OHOS