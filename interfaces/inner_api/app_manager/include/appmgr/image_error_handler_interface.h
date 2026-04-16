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

#ifndef OHOS_ABILITY_RUNTIME_IMAGE_ERROR_HANDLER_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_IMAGE_ERROR_HANDLER_INTERFACE_H

#include "iremote_broker.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
class IImageErrorHandler : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.IImageErrorHandler");

    virtual void OnError(int32_t errorCode) = 0;

    enum class Message {
        ON_ERROR,
    };

    enum class ImageError {
        ERR_OK = 0,
        ERR_TIMEOUT = 1,
        ERR_FORKALL_FAILED = 2,
        ERR_TEMPLATE_HAS_BEEN_USED = 3,
        ERR_INVALID_PRELOAD_TYPE = 4,
        ERR_IMAGE_INFO_EXIST = 5,
        ERR_PRELOAD_FAILED = 6,
        ERR_IMAGE_INFO_NOT_EXIST = 7,
        ERR_IMAGE_INFO_NOT_READY = 8,
        ERR_KILL_IMAGE_PROCESS_FAILED = 9,
        ERR_APP_RECORD_EXIST = 10,
        ERR_INNER = 11,
        ERR_TEMPLATE_DIED = 12,
        ERR_FORKALL_BUSY = 13,
    };
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_IMAGE_ERROR_HANDLER_INTERFACE_H