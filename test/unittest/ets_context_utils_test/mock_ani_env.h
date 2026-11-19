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

#ifndef OHOS_ABILITY_RUNTIME_ETS_CONTEXT_UTILS_MOCK_ANI_ENV_H
#define OHOS_ABILITY_RUNTIME_ETS_CONTEXT_UTILS_MOCK_ANI_ENV_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ani.h"

namespace OHOS::AbilityRuntime {
// Emulate only the ANI runtime boundary needed by ContextUtil bind/switch functions.
// The functions under test come from the production ani_common library.
class MockAniEnv : public ani_env {
public:
    struct Value {
        ani_int enumValue = 0;
        std::map<std::string, ani_long> numbers;
        std::map<std::string, ani_ref> properties;
    };

    MockAniEnv()
    {
        c_api = &api_;
        InitClassApi();
        InitObjectApi();
        InitStringApi();
        InitEnumApi();
    }

    ani_object NewObject()
    {
        values_.push_back(std::make_unique<Value>());
        return reinterpret_cast<ani_object>(values_.back().get());
    }

    ani_enum_item NewEnumItem(ani_int value)
    {
        auto item = NewObject();
        Get(item).enumValue = value;
        return reinterpret_cast<ani_enum_item>(item);
    }

    static Value &Get(ani_ref ref)
    {
        return *reinterpret_cast<Value *>(ref);
    }

    void SetFieldLong(ani_object object, const std::string &name, ani_long value)
    {
        Get(object).numbers[name] = value;
    }

    // Call counters used to assert that guarded paths return before touching the runtime.
    int findClassCalls = 0;
    int classFindFieldCalls = 0;
    int objectSetFieldRefCalls = 0;
    int objectSetFieldByNameRefCalls = 0;
    int objectGetFieldLongCalls = 0;
    int stringNewCalls = 0;
    int enumItemGetValueCalls = 0;

private:
    static MockAniEnv &From(ani_env *env)
    {
        return *static_cast<MockAniEnv *>(env);
    }

    void InitClassApi()
    {
        // Class/method/field handles are opaque; field handles carry their name so the
        // object API can dispatch by field name.
        api_.FindClass = [](ani_env *env, const char *, ani_class *result) {
            ++From(env).findClassCalls;
            *result = reinterpret_cast<ani_class>(1);
            return ANI_OK;
        };
        api_.Class_FindField = [](ani_env *env, ani_class, const char *name, ani_field *result) {
            ++From(env).classFindFieldCalls;
            *result = reinterpret_cast<ani_field>(const_cast<char *>(name));
            return ANI_OK;
        };
    }

    void InitObjectApi()
    {
        api_.Object_GetField_Long =
            [](ani_env *env, ani_object object, ani_field field, ani_long *result) {
                auto &mock = From(env);
                ++mock.objectGetFieldLongCalls;
                const auto &numbers = Get(object).numbers;
                const auto it = numbers.find(reinterpret_cast<const char *>(field));
                *result = (it == numbers.end()) ? 0 : it->second;
                return ANI_OK;
            };
        api_.Object_SetField_Ref = [](ani_env *env, ani_object object, ani_field field, ani_ref value) {
            ++From(env).objectSetFieldRefCalls;
            Get(object).properties[reinterpret_cast<const char *>(field)] = value;
            return ANI_OK;
        };
        api_.Object_SetFieldByName_Ref =
            [](ani_env *env, ani_object object, const char *name, ani_ref value) {
                ++From(env).objectSetFieldByNameRefCalls;
                Get(object).properties[name] = value;
                return ANI_OK;
            };
    }

    void InitStringApi()
    {
        api_.String_NewUTF8 = [](ani_env *env, const char *text, ani_size length, ani_string *result) {
            ++From(env).stringNewCalls;
            *result = reinterpret_cast<ani_string>(From(env).NewObject());
            (void)text;
            (void)length;
            return ANI_OK;
        };
    }

    void InitEnumApi()
    {
        api_.EnumItem_GetValue_Int = [](ani_env *env, ani_enum_item item, ani_int *result) {
            ++From(env).enumItemGetValueCalls;
            *result = Get(item).enumValue;
            return ANI_OK;
        };
    }

    __ani_interaction_api api_ {};
    std::vector<std::unique_ptr<Value>> values_;
};
} // namespace OHOS::AbilityRuntime
#endif // OHOS_ABILITY_RUNTIME_ETS_CONTEXT_UTILS_MOCK_ANI_ENV_H
