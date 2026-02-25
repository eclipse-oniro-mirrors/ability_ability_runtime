/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_STATE_CALLBACK_HOST_H
#define OHOS_ABILITY_RUNTIME_APP_STATE_CALLBACK_HOST_H

#include <map>
#include "iremote_stub.h"
#include "nocopyable.h"
#include "string_ex.h"
#include "app_mgr_constants.h"
#include "appmgr/iapp_state_callback.h"
#include "bundle_info.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
class AppStateCallbackHost : public IRemoteStub<IAppStateCallback> {
public:
    AppStateCallbackHost();
    virtual ~AppStateCallbackHost();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

    /**
     * AbilityMgr's request is done.
     *
     * @param token Ability token.
     * @param state Application state.
     */
    virtual void OnAbilityRequestDone(const sptr<IRemoteObject>&, const AbilityState) override;

    /**
     * Application state changed callback.
     *
     * @param appProcessData Process data
     */
    virtual void OnAppStateChanged(const AppProcessData&) override;

    /**
     * @brief Notify abilityms start resident process.
     * @param bundleInfos resident process bundle infos.
     */
    virtual void NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos) override;

    /**
     * @brief Notify abilityms start keep-alive process.
     * @param bundleInfos resident process bundle infos.
     */
    virtual void NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
        pid_t diedPid = INVALID_DIED_PID) override;

    /**
     * @brief Notify abilityms app process pre cache
     * @param pid process pid.
     * @param userId the user id.
     */
    virtual void NotifyAppPreCache(int32_t pid, int32_t userId) override;

    virtual void RecordAppExitSignalReason(int32_t pid, int32_t uid, int32_t signal, std::string &bundleName) override;

private:
    int32_t HandleOnAppStateChanged(MessageParcel& data, MessageParcel& reply);
    int32_t HandleOnAbilityRequestDone(MessageParcel& data, MessageParcel& reply);
    int32_t HandleNotifyStartResidentProcess(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyStartKeepAliveProcess(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyAppPreCache(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRecordAppExitSignalReason(MessageParcel &data, MessageParcel &reply);

    using AppStateCallbackFunc = int32_t(AppStateCallbackHost::*)(MessageParcel& data, MessageParcel& reply);
    std::map<uint32_t, AppStateCallbackFunc> memberFuncMap_;

    DISALLOW_COPY_AND_MOVE(AppStateCallbackHost);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_STATE_CALLBACK_HOST_H
