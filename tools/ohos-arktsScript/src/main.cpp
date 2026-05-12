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

#include "arkts_script.h"

#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"

#include <unistd.h>

int main(int argc, char* argv[])
{
    TAG_LOGD(AAFwkTag::APPKIT,
        "ohos-arkts process identity: processUid=%{public}d, processPid=%{public}d, callingUid=%{public}d, "
        "callingPid=%{public}d, tokenId=%{public}u, fullTokenId=%{public}llu",
        getuid(), getpid(), OHOS::IPCSkeleton::GetCallingUid(), OHOS::IPCSkeleton::GetCallingPid(),
        OHOS::IPCSkeleton::GetCallingTokenID(),
        static_cast<unsigned long long>(OHOS::IPCSkeleton::GetCallingFullTokenID()));
    return OHOS::ArktsScript::ArktsScript::RunArkTsScript(argc, argv);
}
