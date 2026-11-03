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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_MOCK_ANI_ENV_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_MOCK_ANI_ENV_H

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ani.h"
#include "want_params.h"

namespace OHOS::AbilityRuntime {
// Emulate only the ANI runtime boundary. Parameter parsing, string conversion and
// Context construction are provided by the production libraries under test.
class MockInsightIntentAniEnv : public ani_env {
public:
    struct Value {
        bool isString = false;
        std::string text;
        std::map<std::string, ani_ref> properties;
        std::map<std::string, ani_long> numbers;
        AAFwk::WantParams wantParams;
    };

    MockInsightIntentAniEnv()
    {
        c_api = &api_;
        undefined_ = NewObject();
        InitProperties();
        InitStrings();
        InitContext();
        InitWantParams();
    }

    ani_object NewObject()
    {
        values_.push_back(std::make_unique<Value>());
        return reinterpret_cast<ani_object>(values_.back().get());
    }

    ani_string NewString(const std::string &text)
    {
        auto object = NewObject();
        Get(object).isString = true;
        Get(object).text = text;
        return reinterpret_cast<ani_string>(object);
    }

    static Value &Get(ani_ref ref)
    {
        return *reinterpret_cast<Value *>(ref);
    }

    ani_ref Undefined() const
    {
        return undefined_;
    }

    ani_status stringNewStatus = ANI_OK;
    ani_status setToolCallIdStatus = ANI_OK;
    ani_ref unreadableString = nullptr;
    int stringNewCalls = 0;
    int toolCallIdSetCalls = 0;
    int contextGlobalRefCalls = 0;

private:
    static MockInsightIntentAniEnv &From(ani_env *env)
    {
        return *static_cast<MockInsightIntentAniEnv *>(env);
    }

    void InitProperties()
    {
        api_.Object_GetPropertyByName_Ref = [](ani_env *, ani_object object, const char *name, ani_ref *result) {
            const auto &properties = Get(object).properties;
            const auto it = properties.find(name);
            if (it == properties.end()) {
                return ANI_NOT_FOUND;
            }
            *result = it->second;
            return ANI_OK;
        };
        api_.Reference_IsUndefined = [](ani_env *env, ani_ref ref, ani_boolean *result) {
            *result = ref == From(env).undefined_;
            return ANI_OK;
        };
        api_.EnumItem_GetValue_Int = [](ani_env *, ani_enum_item item, ani_int *result) {
            *result = static_cast<ani_int>(Get(item).numbers.at("value"));
            return ANI_OK;
        };
    }

    void InitStrings()
    {
        api_.String_GetUTF8Size = [](ani_env *env, ani_string str, ani_size *size) {
            if (str == nullptr || str == From(env).unreadableString || !Get(str).isString) {
                return ANI_INVALID_TYPE;
            }
            *size = Get(str).text.size();
            return ANI_OK;
        };
        api_.String_GetUTF8SubString = [](ani_env *, ani_string str, ani_size offset, ani_size length,
            char *buffer, ani_size size, ani_size *copied) {
            const auto &text = Get(str).text;
            if (offset > text.size() || length > text.size() - offset || size <= length) {
                return ANI_BUFFER_TO_SMALL;
            }
            std::copy_n(text.data() + offset, length, buffer);
            buffer[length] = '\0';
            *copied = length;
            return ANI_OK;
        };
        api_.String_NewUTF8 = [](ani_env *env, const char *text, ani_size length, ani_string *result) {
            auto &mock = From(env);
            ++mock.stringNewCalls;
            if (mock.stringNewStatus != ANI_OK) {
                return mock.stringNewStatus;
            }
            *result = mock.NewString(std::string(text, length));
            return ANI_OK;
        };
    }

    void InitContext()
    {
        // Class/method handles are opaque; unlike object handles they are never dereferenced.
        api_.FindClass = [](ani_env *, const char *, ani_class *result) {
            *result = reinterpret_cast<ani_class>(1);
            return ANI_OK;
        };
        api_.Class_BindNativeMethods = [](ani_env *, ani_class, const ani_native_function *, ani_size) {
            return ANI_OK;
        };
        api_.Class_FindMethod = [](ani_env *, ani_class, const char *, const char *, ani_method *result) {
            *result = reinterpret_cast<ani_method>(1);
            return ANI_OK;
        };
        api_.Object_New_V = [](ani_env *env, ani_class, ani_method, ani_object *result, va_list) {
            auto &mock = From(env);
            *result = mock.NewObject();
            Get(*result).properties["toolCallId"] = mock.undefined_;
            mock.contextObject_ = *result;
            return ANI_OK;
        };
        api_.Class_FindField = [](ani_env *, ani_class, const char *name, ani_field *result) {
            *result = reinterpret_cast<ani_field>(const_cast<char *>(name));
            return ANI_OK;
        };
        api_.Object_SetField_Long = [](ani_env *, ani_object object, ani_field field, ani_long value) {
            Get(object).numbers[reinterpret_cast<const char *>(field)] = value;
            return ANI_OK;
        };
        api_.Object_SetField_Int = [](ani_env *, ani_object object, ani_field field, ani_int value) {
            Get(object).numbers[reinterpret_cast<const char *>(field)] = value;
            return ANI_OK;
        };
        api_.Object_SetFieldByName_Ref = [](ani_env *env, ani_object object, const char *name, ani_ref value) {
            auto &mock = From(env);
            if (std::string(name) == "toolCallId") {
                ++mock.toolCallIdSetCalls;
                if (mock.setToolCallIdStatus != ANI_OK) {
                    return mock.setToolCallIdStatus;
                }
            }
            Get(object).properties[name] = value;
            return ANI_OK;
        };
        api_.GlobalReference_Create = [](ani_env *env, ani_ref ref, ani_ref *result) {
            auto &mock = From(env);
            if (ref == mock.contextObject_) {
                ++mock.contextGlobalRefCalls;
            }
            *result = ref;
            return ANI_OK;
        };
        api_.GlobalReference_Delete = [](ani_env *, ani_ref) { return ANI_OK; };
    }

    void InitWantParams()
    {
        api_.Class_FindStaticMethod = [](ani_env *, ani_class, const char *, const char *, ani_static_method *result) {
            *result = reinterpret_cast<ani_static_method>(1);
            return ANI_OK;
        };
        // UnwrapWantParams calls the ETS Record conversion helper through ANI.
        api_.Class_CallStaticMethod_Boolean_V = [](ani_env *, ani_class, ani_static_method,
            ani_boolean *result, va_list args) {
            auto record = va_arg(args, ani_ref);
            auto destination = reinterpret_cast<AAFwk::WantParams *>(va_arg(args, ani_long));
            *destination = Get(record).wantParams;
            *result = ANI_TRUE;
            return ANI_OK;
        };
    }

    __ani_interaction_api api_ {};
    std::vector<std::unique_ptr<Value>> values_;
    ani_ref undefined_ = nullptr;
    ani_object contextObject_ = nullptr;
};
} // namespace OHOS::AbilityRuntime
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_MOCK_ANI_ENV_H
