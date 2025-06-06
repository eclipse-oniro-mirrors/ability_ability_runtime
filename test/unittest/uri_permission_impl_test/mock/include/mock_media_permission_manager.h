/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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
 
#ifndef OHOS_ABILITY_RUNTIME_MEDIA_PERMISSION_MANAGER_H
#define OHOS_ABILITY_RUNTIME_MEDIA_PERMISSION_MANAGER_H
 
#include <sys/types.h>
#include <vector>
#include <string>
 
namespace OHOS {
namespace AAFwk {
class MediaPermissionManager {
public:
    static int32_t GrantPhotoUriPermission(const std::string &bundleName, const std::vector<std::string> &uris,
        uint32_t flag)
    {
        return 0;
    }
 
    static std::vector<bool> CheckPhotoUriPermission(const std::vector<std::string> &uriVec,
        uint32_t tokenId, uint32_t flag)
    {
        return std::vector<bool>(uriVec.size(), 0);
    }
};
 
} // OHOS
} // AAFwk
#endif // OHOS_ABILITY_RUNTIME_MEDIA_PERMISSION_MANAGER_H