/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_INTERFACE_H

#include <variant>
#include <vector>

#include "ability_info.h"
#include "ability_manager_errors.h"
#include "start_options.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
struct AbilityInterceptorParam {
    // Core fields (read by 3+ interceptors)
    Want want;
    int32_t requestCode = 0;
    int32_t userId = 0;
    bool isWithUI = false;       // CrowdTest/Control: "should show modal/redirect?"
    bool isVisible = false;     // DisposedRule/EcologicalRule: "is this a visible start?"
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo;  // resolved target abilityInfo passed to the interceptor

    // Per-interceptor contexts, accessed via GetContext<T>()
    struct EcologicalCtx {
        bool isStartAsCaller = false;
        bool isTargetPlugin = false; // Is it pulling up a plugin
        std::string hostBundleName;
    };
    struct DisposedCtx {
        int32_t appIndex = 0;
        const StartOptions* startOptions = nullptr;
    };
    struct ScreenUnlockCtx {
        bool fromConnect = false;
    };
    // Marker context: this DoProcess call guards a remote dispatch. Interceptors that
    // require a locally-resolved target (DisposedRule / ExtensionControl / EcologicalRule)
    // defer to the remote device's own enforcement when this context is present.
    struct RemoteDispatchCtx {};

    using Context = std::variant<std::monostate, EcologicalCtx, DisposedCtx, ScreenUnlockCtx, RemoteDispatchCtx>;
    std::vector<Context> contexts;

    template<typename T>
    const T* GetContext() const
    {
        for (const auto &c : contexts) {
            if (const auto *p = std::get_if<T>(&c)) {
                return p;
            }
        }
        return nullptr;
    }
};

// Fluent builder: setters mutate an internal AbilityInterceptorParam directly, Build() moves it out.
// Defined after AbilityInterceptorParam so it can hold one by value.
class InterceptorParamBuilder {
public:
    InterceptorParamBuilder(const Want &want, int32_t requestCode, int32_t userId)
    {
        param_.want = want;
        param_.requestCode = requestCode;
        param_.userId = userId;
    }
    ~InterceptorParamBuilder() = default;
    InterceptorParamBuilder(const InterceptorParamBuilder&) = delete;
    InterceptorParamBuilder &operator=(const InterceptorParamBuilder&) = delete;
    InterceptorParamBuilder &WithUI(bool isWithUI)
    {
        param_.isWithUI = isWithUI;
        return *this;
    }
    InterceptorParamBuilder &Visible(bool isVisible)
    {
        param_.isVisible = isVisible;
        return *this;
    }
    InterceptorParamBuilder &CallerToken(const sptr<IRemoteObject> &callerToken)
    {
        param_.callerToken = callerToken;
        return *this;
    }
    InterceptorParamBuilder &AbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo)
    {
        param_.abilityInfo = abilityInfo;
        return *this;
    }

    // Push a per-interceptor context struct onto the param.
    template<typename T>
    InterceptorParamBuilder &Context(const T &ctx)
    {
        param_.contexts.push_back(ctx);
        return *this;
    }
    [[nodiscard]] AbilityInterceptorParam Build()
    {
        return std::move(param_);
    }
private:
    AbilityInterceptorParam param_;
};

/**
 * @class IAbilityInterceptor
 * IAbilityInterceptor is used to intercept a different type of start request.
 */
class IAbilityInterceptor {
public:
    virtual ~IAbilityInterceptor() = default;

    /**
     * Excute interception processing.
     */
    virtual ErrCode DoProcess(AbilityInterceptorParam &param) = 0;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_INTERFACE_H
