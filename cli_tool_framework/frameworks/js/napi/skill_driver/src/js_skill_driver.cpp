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

#include "js_skill_driver.h"

#include <set>
#include <string>

#include "ability_manager_client.h"
#include "array_wrapper.h"
#include "cli_error_code.h"
#include "bool_wrapper.h"
#include "cli_manager_error_utils.h"
#include "double_wrapper.h"
#include "hilog_tag_wrapper.h"
#include "int_wrapper.h"
#include "js_cli_event_handler_manager.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "long_wrapper.h"
#include "napi_common_skill_execute.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "skill/skill_execute_callback_stub.h"
#include "string_wrapper.h"
#include "want_params.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_TWO = 2;

std::string GetStringPropertyFromJs(napi_env env, napi_value obj, const std::string &key)
{
    napi_value value = nullptr;
    napi_get_named_property(env, obj, key.c_str(), &value);
    if (value == nullptr) {
        return "";
    }
    std::string result;
    if (!AppExecFwk::UnwrapStringFromJS2(env, value, result)) {
        return "";
    }
    return result;
}

std::string GetPropertyKeyFromJs(napi_env env, napi_value keyVal)
{
    size_t strLen = 0;
    napi_get_value_string_utf8(env, keyVal, nullptr, 0, &strLen);
    std::string key(strLen, '\0');
    napi_get_value_string_utf8(env, keyVal, key.data(), strLen + 1, &strLen);
    return key;
}

void SetSkillArrayString(const std::string &key, const std::vector<std::string> &values,
    AAFwk::WantParams &params)
{
    auto arr = sptr<AAFwk::IArray>(new (std::nothrow) AAFwk::Array(values.size(), AAFwk::g_IID_IString));
    if (arr == nullptr) { return; }
    for (size_t i = 0; i < values.size(); i++) {
        arr->Set(i, AAFwk::String::Box(values[i]));
    }
    params.SetParam(key, arr);
}

void SetSkillArrayBool(const std::string &key, const std::vector<bool> &values,
    AAFwk::WantParams &params)
{
    auto arr = sptr<AAFwk::IArray>(new (std::nothrow) AAFwk::Array(values.size(), AAFwk::g_IID_IBoolean));
    if (arr == nullptr) { return; }
    for (size_t i = 0; i < values.size(); i++) {
        arr->Set(i, AAFwk::Boolean::Box(values[i]));
    }
    params.SetParam(key, arr);
}

void SetSkillArrayDouble(const std::string &key, const std::vector<double> &values,
    AAFwk::WantParams &params)
{
    auto arr = sptr<AAFwk::IArray>(new (std::nothrow) AAFwk::Array(values.size(), AAFwk::g_IID_IDouble));
    if (arr == nullptr) { return; }
    for (size_t i = 0; i < values.size(); i++) {
        arr->Set(i, AAFwk::Double::Box(values[i]));
    }
    params.SetParam(key, arr);
}

void SetSkillArrayLong(const std::string &key, const std::vector<int64_t> &values,
    AAFwk::WantParams &params)
{
    auto arr = sptr<AAFwk::IArray>(new (std::nothrow) AAFwk::Array(values.size(), AAFwk::g_IID_ILong));
    if (arr == nullptr) { return; }
    for (size_t i = 0; i < values.size(); i++) {
        arr->Set(i, AAFwk::Long::Box(values[i]));
    }
    params.SetParam(key, arr);
}

void SetSkillArrayParam(napi_env env, const std::string &key, napi_value val,
    AAFwk::WantParams &params)
{
    uint32_t size = 0;
    if (!AppExecFwk::IsArrayForNapiValue(env, val, size) || size == 0) {
        return;
    }
    napi_value elem = nullptr;
    napi_get_element(env, val, 0, &elem);
    if (elem == nullptr) { return; }

    napi_valuetype elemType = napi_undefined;
    napi_typeof(env, elem, &elemType);
    switch (elemType) {
        case napi_string: {
            std::vector<std::string> values;
            if (AppExecFwk::UnwrapArrayStringFromJS(env, val, values)) {
                SetSkillArrayString(key, values, params);
            }
            break;
        }
        case napi_number: {
            std::vector<double> dblValues;
            if (AppExecFwk::UnwrapArrayDoubleFromJS(env, val, dblValues)) {
                SetSkillArrayDouble(key, dblValues, params);
            }
            break;
        }
        case napi_boolean: {
            std::vector<bool> values;
            if (AppExecFwk::UnwrapArrayBoolFromJS(env, val, values)) {
                SetSkillArrayBool(key, values, params);
            }
            break;
        }
        case napi_bigint: {
            std::vector<int64_t> values;
            if (AppExecFwk::UnwrapArrayInt64FromJS(env, val, values)) {
                SetSkillArrayLong(key, values, params);
            }
            break;
        }
        default:
            break;
    }
}

void SetSkillParamByType(napi_env env, const std::string &key, napi_value val, AAFwk::WantParams &params)
{
    napi_valuetype type = napi_undefined;
    napi_typeof(env, val, &type);
    switch (type) {
        case napi_string: {
            std::string str;
            if (AppExecFwk::UnwrapStringFromJS2(env, val, str)) {
                params.SetParam(key, AAFwk::String::Box(str));
            }
            break;
        }
        case napi_number: {
            double dblVal = 0.0;
            napi_get_value_double(env, val, &dblVal);
            int32_t intVal = static_cast<int32_t>(dblVal);
            if (static_cast<double>(intVal) == dblVal) {
                params.SetParam(key, AAFwk::Integer::Box(intVal));
            } else {
                params.SetParam(key, AAFwk::Double::Box(dblVal));
            }
            break;
        }
        case napi_boolean: {
            bool boolVal = false;
            napi_get_value_bool(env, val, &boolVal);
            params.SetParam(key, AAFwk::Boolean::Box(boolVal));
            break;
        }
        case napi_bigint: {
            int64_t int64Val = 0;
            bool lossless = true;
            napi_get_value_bigint_int64(env, val, &int64Val, &lossless);
            params.SetParam(key, AAFwk::Long::Box(int64Val));
            break;
        }
        case napi_object: {
            SetSkillArrayParam(env, key, val, params);
            break;
        }
        default:
            break;
    }
}

std::shared_ptr<AAFwk::WantParams> ExtractSkillArgs(napi_env env, napi_value obj)
{
    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    napi_value propertyNames = nullptr;
    napi_get_property_names(env, obj, &propertyNames);
    if (propertyNames == nullptr) {
        return skillArgs;
    }
    const std::set<std::string> reservedKeys = {
        "skillToolType", "bundleName", "moduleName", "skillName", "arkTSPath", "funcName"
    };
    uint32_t length = 0;
    napi_get_array_length(env, propertyNames, &length);
    for (uint32_t i = 0; i < length; i++) {
        napi_value keyVal = nullptr;
        napi_get_element(env, propertyNames, i, &keyVal);
        if (keyVal == nullptr) { continue; }
        std::string key = GetPropertyKeyFromJs(env, keyVal);
        if (key.empty() || reservedKeys.count(key) > 0) { continue; }
        napi_value val = nullptr;
        napi_get_named_property(env, obj, key.c_str(), &val);
        if (val == nullptr) { continue; }
        SetSkillParamByType(env, key, val, *skillArgs);
    }
    return skillArgs;
}

class SkillExecuteCallbackImpl : public AAFwk::SkillExecuteCallbackStub {
public:
    explicit SkillExecuteCallbackImpl(napi_env env, napi_deferred deferred)
        : env_(env), deferred_(deferred) {}

    void OnExecuteDone(const std::string &requestCode, int32_t resultCode,
        const AppExecFwk::SkillExecuteResult &result) override
    {
        TAG_LOGD(AAFwkTag::CLI_TOOL,
            "SkillExecuteCallbackImpl::OnExecuteDone req:%{public}s code:%{public}d",
            requestCode.c_str(), resultCode);
        auto resultCopy = result;
        auto deferred = deferred_;
        JsCliEventHandlerManager::GetInstance().PostTask(
            [env = env_, deferred, resultCopy]() {
                HandleScope handleScope(env);
                napi_value jsResult = WrapSkillExecuteResult(env, resultCopy);
                napi_resolve_deferred(env, deferred, jsResult);
            });
    }

private:
    napi_env env_ = nullptr;
    napi_deferred deferred_ = nullptr;
};

int32_t DispatchExecuteSkill(const std::string &bundleName, const std::string &moduleName,
    const std::string &skillName, const std::string &arkTSPath, const std::string &funcName,
    const std::shared_ptr<AAFwk::WantParams> &skillArgs,
    const sptr<SkillExecuteCallbackImpl> &callback)
{
    constexpr int32_t SKILL_TYPE_INDEPENDENT = -1;
    int32_t skillType = 0;
    auto queryRet = AAFwk::AbilityManagerClient::GetInstance()->QuerySkillType(
        bundleName, moduleName, skillName, skillType);
    if (queryRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "querySkillType failed:%{public}d", queryRet);
        return queryRet;
    }
    if (skillType == SKILL_TYPE_INDEPENDENT) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "independent skill not supported yet");
        return ERR_TOOL_NOT_EXIST;
    }
    return AAFwk::AbilityManagerClient::GetInstance()->ExecuteInAppSkill(
        bundleName, moduleName, skillName, arkTSPath, funcName, skillArgs, callback);
}
} // namespace

void JSSkillDriver::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSSkillDriver::Finalizer is called");
    std::unique_ptr<JSSkillDriver>(static_cast<JSSkillDriver *>(data));
}

napi_value JSSkillDriver::ExecSkillTool(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSSkillDriver, OnExecSkillTool);
}

napi_value JSSkillDriver::OnExecSkillTool(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSSkillDriver::OnExecSkillTool called");
    HandleEscape handleEscape(env);
    if (argc < INDEX_TWO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[INDEX_ZERO], &valueType);
    if (valueType != napi_object) {
        ThrowInvalidParamError(env, "skillToolParam must be an object");
        return CreateJsUndefined(env);
    }
    auto skillToolType = GetStringPropertyFromJs(env, argv[INDEX_ZERO], "skillToolType");
    if (skillToolType.empty()) {
        ThrowInvalidParamError(env, "skillToolType is required");
        return CreateJsUndefined(env);
    }
    auto bundleName = GetStringPropertyFromJs(env, argv[INDEX_ZERO], "bundleName");
    auto moduleName = GetStringPropertyFromJs(env, argv[INDEX_ZERO], "moduleName");
    auto skillName = GetStringPropertyFromJs(env, argv[INDEX_ZERO], "skillName");
    if (bundleName.empty() || moduleName.empty() || skillName.empty()) {
        ThrowInvalidParamError(env, "bundleName, moduleName, skillName are required");
        return CreateJsUndefined(env);
    }
    auto arkTSPath = GetStringPropertyFromJs(env, argv[INDEX_ZERO], "arkTSPath");
    auto funcName = GetStringPropertyFromJs(env, argv[INDEX_ZERO], "funcName");
    auto skillArgs = ExtractSkillArgs(env, argv[INDEX_ZERO]);
    TAG_LOGD(AAFwkTag::CLI_TOOL,
        "execSkillTool bundle:%{public}s module:%{public}s skill:%{public}s "
        "type:%{public}s",
        bundleName.c_str(), moduleName.c_str(), skillName.c_str(),
        skillToolType.c_str());

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto callback = sptr<SkillExecuteCallbackImpl>::MakeSptr(env, deferred);

    NapiAsyncTask::ExecuteCallback execute =
        [innerErrCode, bundleName, moduleName, skillName,
         arkTSPath, funcName, skillArgs, callback]() {
        *innerErrCode = DispatchExecuteSkill(
            bundleName, moduleName, skillName, arkTSPath, funcName, skillArgs, callback);
    };

    NapiAsyncTask::CompleteCallback complete =
        [innerErrCode, deferred](napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            napi_reject_deferred(env, deferred,
                CreateCliJsErrorByNativeErr(env, *innerErrCode));
        }
    };

    auto asyncTask = std::make_unique<NapiAsyncTask>(deferred,
        std::make_unique<NapiAsyncTask::ExecuteCallback>(std::move(execute)),
        std::make_unique<NapiAsyncTask::CompleteCallback>(std::move(complete)));
    NapiAsyncTask::Schedule("JSSkillDriver::OnExecSkillTool", env, std::move(asyncTask));
    return handleEscape.Escape(promise);
}

napi_value JSSkillDriverInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "Init JSSkillDriver");

    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Null env or exportObj");
        return nullptr;
    }

    std::unique_ptr<JSSkillDriver> jsSkillDriver = std::make_unique<JSSkillDriver>();
    napi_wrap(env, exportObj, jsSkillDriver.release(), JSSkillDriver::Finalizer, nullptr, nullptr);

    const char *moduleName = "SkillDriver";
    BindNativeFunction(env, exportObj, "execSkillTool", moduleName, JSSkillDriver::ExecSkillTool);

    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSSkillDriverInit end");
    return CreateJsUndefined(env);
}

} // namespace CliTool
} // namespace OHOS
