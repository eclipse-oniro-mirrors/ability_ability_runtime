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

#ifndef OHOS_ABILITY_RUNTIME_ANI_COMMON_WANT_H
#define OHOS_ABILITY_RUNTIME_ANI_COMMON_WANT_H

#include <map>
#include <string>
#include <vector>

#include "ani_common_util.h"
#include "want.h"
#include "want_params.h"

namespace OHOS {
namespace AppExecFwk {
ani_object WrapWant(ani_env *env, const AAFwk::Want &want);
bool UnwrapWant(ani_env *env, ani_object param, AAFwk::Want &want);

ani_ref WrapWantParams(ani_env *env, const AAFwk::WantParams &wantParams);
bool UnwrapWantParams(ani_env *env, ani_ref param, AAFwk::WantParams &wantParams);

ani_ref WrapWantParamsFD(ani_env *env, const AAFwk::WantParams &wantParams);

bool UnWrapAbilityResult(ani_env *env, ani_object param, int &resultCode, AAFwk::Want &want);

ani_object WrapElementName(ani_env *env, const AppExecFwk::ElementName &elementNameParam);
ani_object WrapElementNameInner(ani_env *env, ani_class elementNameObj, ani_object object,
    const AppExecFwk::ElementName &elementNameParam);

ani_object CreateAniWant(ani_env *env, const AAFwk::Want &want);
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ANI_COMMON_WANT_H
