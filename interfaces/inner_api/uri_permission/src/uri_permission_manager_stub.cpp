/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "uri_permission_manager_stub.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
int UriPermissionManagerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != IUriPermissionManager::GetDescriptor()) {
        HILOG_ERROR("InterfaceToken not equal IUriPermissionManager's descriptor.");
        return ERR_INVALID_VALUE;
    }
    ErrCode errCode = ERR_OK;
    switch (code) {
        case UriPermMgrCmd::ON_GRANT_URI_PERMISSION : {
            std::unique_ptr<Uri> uri(data.ReadParcelable<Uri>());
            if (!uri) {
                errCode = ERR_DEAD_OBJECT;
                HILOG_ERROR("To read uri failed.");
                break;
            }
            auto flag = data.ReadInt32();
            auto targetBundleName = data.ReadString();
            auto autoremove = data.ReadInt32();
            auto appIndex = data.ReadInt32();
            int result = GrantUriPermission(*uri, flag, targetBundleName, autoremove, appIndex);
            reply.WriteInt32(result);
            break;
        }
        case UriPermMgrCmd::ON_REVOKE_URI_PERMISSION : {
            auto tokenId = data.ReadInt32();
            RevokeUriPermission(tokenId);
            break;
        }
        case UriPermMgrCmd::ON_REVOKE_URI_PERMISSION_MANUALLY : {
            std::unique_ptr<Uri> uri(data.ReadParcelable<Uri>());
            if (!uri) {
                errCode = ERR_DEAD_OBJECT;
                HILOG_ERROR("To read uri failed.");
                break;
            }
            auto bundleName = data.ReadString();
            int result = RevokeUriPermissionManually(*uri, bundleName);
            reply.WriteInt32(result);
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return errCode;
}
}  // namespace AAFwk
}  // namespace OHOS
