/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "mock_my_flag.h"
#include "js_runtime.h"
#include <gmock/gmock.h>
#include "file_mapper.h"
#include "native_reference.h"
#include "ohos_js_environment_impl.h"
#include "js_environment.h"

namespace OHOS {
namespace AbilityRuntime {
class NativeReferenceMock : public NativeReference {
public:
    NativeReferenceMock() = default;
    virtual ~NativeReferenceMock() = default;
    MOCK_METHOD0(Ref, uint32_t());
    MOCK_METHOD0(Unref, uint32_t());
    MOCK_METHOD0(Get, napi_value());
    MOCK_METHOD0(GetData, void*());
    virtual operator napi_value() override
    {
        return reinterpret_cast<napi_value>(this);
    }
    MOCK_METHOD0(SetDeleteSelf, void());
    MOCK_METHOD0(GetRefCount, uint32_t());
    MOCK_METHOD0(GetFinalRun, bool());
    napi_value GetNapiValue() override
    {
        return nullptr;
    }
};

napi_value VirtualFunc(napi_env env, napi_callback_info info)
{
    return nullptr;
}

JsRuntime::JsRuntime()
{
    jsEnv_ = std::make_shared<JsEnv::JsEnvironment>(std::make_unique<OHOSJsEnvironmentImpl>());
}

JsRuntime::~JsRuntime()
{
}

std::unique_ptr<NativeReference> JsRuntime::LoadSystemModuleByEngine(
    napi_env env, const std::string& moduleName, const napi_value* argv, size_t argc)
{
    if (MyFlag::isLoadSystemModuleByEngine_) {
        return std::make_unique<NativeReferenceMock>();
    }
    return nullptr;
}

std::unique_ptr<NativeReference> JsRuntime::LoadSystemModule(
    const std::string& moduleName, const napi_value* argv, size_t argc)
{
    return nullptr;
}

napi_value JsRuntime::GetExportObjectFromOhmUrl(const std::string &srcEntrance, const std::string &key)
{
    if (MyFlag::isGetExportObjectFromOhmUrlNullptr_) {
        return nullptr;
    }
    napi_value fn;
    napi_create_function(GetNapiEnv(), NULL, 0, VirtualFunc, NULL, &fn);
    return fn;
}

std::unique_ptr<AbilityBase::FileMapper> JsRuntime::ExecuteSecureWithOhmUrl(const std::string &moduleName,
    const std::string &hapPath, const std::string &srcEntrance)
{
    if (MyFlag::isExecuteSecureWithOhmUrl_) {
        return std::make_unique<AbilityBase::FileMapper>();
    }
    return nullptr;
}

napi_env JsRuntime::GetNapiEnv() const
{
    if (MyFlag::isGetNapiEnvNullptr_) {
        return nullptr;
    }
    return reinterpret_cast<napi_env>(jsEnv_->GetNativeEngine());
}
}  // namespace AbilityRuntime
}  // namespace OHOS