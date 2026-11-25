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

#include <gtest/gtest.h>
#include <set>
#include <string>
#include "ets_launch_service_manager.h"
using namespace testing::ext;
namespace OHOS::AbilityRuntime {
class LaunchServiceAniEnv : public ani_env {
public:
    LaunchServiceAniEnv()
    {
        c_api = &api;
        api.FindNamespace = [](ani_env *env, const char *, ani_namespace *result) {
            auto &mock = *static_cast<LaunchServiceAniEnv *>(env);
            *result = mock.findResult == ANI_OK ? reinterpret_cast<ani_namespace>(env) : nullptr;
            return mock.findResult;
        };
        api.Namespace_BindNativeFunctions = [](ani_env *env, ani_namespace,
            const ani_native_function *methods, ani_size size) {
            auto &mock = *static_cast<LaunchServiceAniEnv *>(env);
            ++mock.bindCalls;
            for (ani_size i = 0; i < size; ++i) {
                mock.names.insert(methods[i].name);
                EXPECT_NE(methods[i].pointer, nullptr);
            }
            return mock.bindResult;
        };
    }
    __ani_interaction_api api {};
    ani_status findResult = ANI_OK;
    ani_status bindResult = ANI_OK;
    int32_t bindCalls = 0;
    std::set<std::string> names;
};
class LaunchServiceAniTest : public testing::Test {};

/**
 * @tc.name: EtsLaunchServiceManagerInit_001
 * @tc.desc: Register the current API names and stop on unavailable namespace.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceAniTest, EtsLaunchServiceManagerInit_001, TestSize.Level1)
{
    EtsLaunchServiceManagerInit(nullptr);
    LaunchServiceAniEnv env;
    env.findResult = ANI_NOT_FOUND;
    EtsLaunchServiceManagerInit(&env);
    EXPECT_EQ(env.bindCalls, 0);
    env.findResult = ANI_OK;
    EtsLaunchServiceManagerInit(&env);
    EXPECT_EQ(env.bindCalls, 1);
    EXPECT_EQ(env.names.size(), 9);
    EXPECT_EQ(env.names.count("nativeSetLaunchServicesEnabled"), 1);
    EXPECT_EQ(env.names.count("nativeGetAllRegisteredInformation"), 1);
    EXPECT_EQ(env.names.count("nativeSetLaunchServicesEnable"), 0);
    env.bindResult = ANI_ERROR;
    EtsLaunchServiceManagerInit(&env);
    EXPECT_EQ(env.bindCalls, 2);
}
} // namespace OHOS::AbilityRuntime
