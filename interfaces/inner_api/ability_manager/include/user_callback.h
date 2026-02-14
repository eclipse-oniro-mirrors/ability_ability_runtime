/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_USER_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_USER_CALLBACK_H

#include "iremote_broker.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class IUserCallback
 * user callback.
 */
class IUserCallback : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.UserCallback");

    virtual void OnStopUserDone(int userId, int errcode) = 0;

    /**
     * @brief OnStartUserDone.
     *
     * @param userId userId.
     * @param errcode errcode.
     */
    virtual void OnStartUserDone(int userId, int errcode) = 0;

    /**
     * @brief OnLogoutUserDone.
     *
     * @param userId userId.
     * @param errcode errcode.
     */
    virtual void OnLogoutUserDone(int userId, int errcode) {};

    enum UserCallbackCmd {
        // ipc id for OnStopUserDone
        ON_STOP_USER_DONE = 0,

        // ipc id for OnStartUserDone
        ON_START_USER_DONE = 1,

        // ipc id for OnLogoutUserDone
        ON_LOGOUT_USER_DONE = 2,

        // maximum of enum
        CMD_MAX
    };
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_USER_CALLBACK_H
