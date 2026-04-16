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

#ifndef OHOS_ABILITY_RUNTIME_APP_REFRESH_RECIPIENT_H
#define OHOS_ABILITY_RUNTIME_APP_REFRESH_RECIPIENT_H

#include "iremote_object.h"

#include "task_handler_wrap.h"

namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceInner;

class AppRefreshRecipient : public IRemoteObject::RefreshRecipient {
public:
    virtual void OnRemoteRefreshed(const wptr<IRemoteObject> &remote) override;

    /**
     * @brief Setting event handler instance.
     *
     * @param handler, event handler instance.
     */
    void SetTaskHandler(const std::shared_ptr<AAFwk::TaskHandlerWrap> &handler);

    /**
     * @brief Setting application service internal handler instance.
     *
     * @param serviceInner, application service internal handler instance.
     */
    void SetAppMgrServiceInner(const std::shared_ptr<AppMgrServiceInner> &serviceInner);

private:
    std::weak_ptr<AAFwk::TaskHandlerWrap> handler_;
    std::weak_ptr<AppMgrServiceInner> appMgrServiceInner_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_REFRESH_RECIPIENT_H