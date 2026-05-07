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

#ifndef MOCK_EXTENSION_CONTEXT_H
#define MOCK_EXTENSION_CONTEXT_H

#include <memory>
#include "iremote_broker.h"
#include "refbase.h"
#include "iremote_object.h"
#include "errors.h"

namespace OHOS {
namespace AbilityRuntime {

class Context : public std::enable_shared_from_this<Context> {
public:
    Context() = default;
    virtual ~Context() = default;
};

} // namespace AbilityRuntime

namespace AAFwk {
class Want {};

class StartOptions {};

class AbilityManagerClient {
public:
    static std::shared_ptr<AbilityManagerClient> GetInstance()
    {
        static auto instance = std::make_shared<AbilityManagerClient>();
        return instance;
    }

    ErrCode StartSelfUIAbility(const Want &want)
    {
        return g_startSelfUIAbilityResult;
    }

    ErrCode StartSelfUIAbilityWithStartOptions(const Want &want, const StartOptions &options)
    {
        return g_startSelfUIAbilityWithStartOptionsResult;
    }

    ErrCode TerminateAbility(const sptr<IRemoteObject> &token, int32_t resultCode, const Want *resultWant)
    {
        g_terminateCalled = true;
        g_lastToken = token.GetRefPtr();
        return g_terminateResult;
    }

    static ErrCode g_startSelfUIAbilityResult;
    static ErrCode g_startSelfUIAbilityWithStartOptionsResult;
    static ErrCode g_terminateResult;
    static bool g_terminateCalled;
    static IRemoteObject *g_lastToken;

    static void Reset()
    {
        g_startSelfUIAbilityResult = ERR_OK;
        g_startSelfUIAbilityWithStartOptionsResult = ERR_OK;
        g_terminateResult = ERR_OK;
        g_terminateCalled = false;
        g_lastToken = nullptr;
    }
};

} // namespace AAFwk

namespace AbilityRuntime {

class ExtensionContext : public Context {
public:
    sptr<IRemoteObject> token_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // MOCK_EXTENSION_CONTEXT_H
