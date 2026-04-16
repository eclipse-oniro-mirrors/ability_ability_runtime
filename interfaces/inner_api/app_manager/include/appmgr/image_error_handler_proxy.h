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

#ifndef OHOS_ABILITY_RUNTIME_IMAGE_ERROR_HANDLER_PROXY_H
#define OHOS_ABILITY_RUNTIME_IMAGE_ERROR_HANDLER_PROXY_H

#include "image_error_handler_interface.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {
class ImageErrorHandlerProxy : public IRemoteProxy<IImageErrorHandler> {
public:
    explicit ImageErrorHandlerProxy(const sptr<IRemoteObject> &impl);
    virtual ~ImageErrorHandlerProxy() = default;

    void OnError(int32_t errorCode) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    static inline BrokerDelegator<ImageErrorHandlerProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_IMAGE_ERROR_HANDLER_PROXY_H