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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_COMMON_WANT_H
#define OHOS_ABILITY_RUNTIME_NAPI_COMMON_WANT_H

#include <map>
#include <string>
#include <vector>

#include "native_engine/native_engine.h"
#include "want.h"
#include "want_params.h"

namespace OHOS {
namespace AppExecFwk {

napi_value WrapElementName(napi_env env, const ElementName &elementName);
bool UnwrapElementName(napi_env env, napi_value param, ElementName &elementName);

napi_value WrapWantParams(napi_env env, const AAFwk::WantParams &wantParams);
bool UnwrapWantParams(napi_env env, napi_value param, AAFwk::WantParams &wantParams);
bool UnwrapWantParams(napi_env env, napi_value param, AAFwk::WantParams &wantParams,
    const std::string &proNameNotFilter);
bool BlackListFilter(const std::string &strProName, const std::string &proNameNotFilter);

napi_value WrapWant(napi_env env, const AAFwk::Want &want);
bool UnwrapWant(napi_env env, napi_value param, AAFwk::Want &want);
bool UnwrapWant(napi_env env, napi_value param, AAFwk::Want &want, const std::string &proNameNotFilter);
void SetUriWithPassThroughFlag(const std::string &uriString, AAFwk::Want &want);

napi_value WrapWantParamsFD(napi_env env, const AAFwk::WantParams &wantParams);

napi_value WrapAbilityResult(napi_env env, const int &resultCode, const AAFwk::Want &want);
bool UnWrapAbilityResult(napi_env env, napi_value param, int &resultCode, AAFwk::Want &want);

void HandleNapiObject(napi_env env, napi_value param, napi_value jsProValue, std::string &strProName,
    AAFwk::WantParams &wantParams);

bool IsSpecialObject(napi_env env, napi_value param, std::string &strProName, std::string type,
    napi_valuetype jsValueProType);

bool HandleFdObject(napi_env env, napi_value param, std::string &strProName, AAFwk::WantParams &wantParams);

bool HandleRemoteObject(napi_env env, napi_value param, std::string &strProName, AAFwk::WantParams &wantParams);

napi_value CreateJsWant(napi_env env, const AAFwk::Want &want);
napi_value CreateJsWantParams(napi_env env, const AAFwk::WantParams &wantParams);

template<class TBase, class T, class NativeT>
bool InnerWrapJsWantParams(
    napi_env env, napi_value object, const std::string &key, const AAFwk::WantParams &wantParams);

bool InnerWrapJsWantParamsWantParams(
    napi_env env, napi_value object, const std::string &key, const AAFwk::WantParams &wantParams);

bool WrapJsWantParamsArray(
    napi_env env, napi_value object, const std::string &key, sptr<AAFwk::IArray> &ao);

template<class TBase, class T, class NativeT>
bool InnerWrapWantParamsArray(napi_env env, napi_value object, const std::string &key, sptr<AAFwk::IArray> &ao);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NAPI_COMMON_WANT_H
