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

#include "native_engine/native_engine.h"
#include "js_insight_intent_execute_manager.h"

/*
 * The module definition.
 */
static napi_module _module = {
    .nm_version = 0,
    .nm_filename = nullptr,
    .nm_register_func = OHOS::AbilityRuntime::CreateJsInsightIntentExecuteManager,
    .nm_modname = "app.ability.insightIntentExecuteManager",
};

/*
 * The module registration.
 */
extern "C" __attribute__((constructor)) void NAPI_application_InsightIntentExecuteManager_AutoRegister(void)
{
    napi_module_register(&_module);
}
