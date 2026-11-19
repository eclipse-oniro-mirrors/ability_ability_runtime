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
#include <iterator>
#include <memory>
#include <chrono>
#include <thread>
#include <vector>
#include "ability_manager_errors.h"
#include "ability_business_error.h"
#include "daemon_service_manager_client.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "js_launch_service_manager.h"
using namespace testing::ext;
namespace OHOS::AbilityRuntime {
namespace {
int32_t g_registerCalls = 0;
int32_t g_unregisterCalls = 0;
int32_t g_registerResult = ERR_OK;
int32_t g_unregisterResult = ERR_OK;
}

// Isolate the JS binding from IPC while exercising the real NAPI event queue.
int32_t DaemonServiceManagerClient::RegisterObserver(const sptr<ILaunchServiceObserver> &observer)
{
    EXPECT_NE(observer, nullptr);
    ++g_registerCalls;
    return g_registerResult;
}

int32_t DaemonServiceManagerClient::UnregisterObserver(const sptr<ILaunchServiceObserver> &observer)
{
    EXPECT_NE(observer, nullptr);
    ++g_unregisterCalls;
    return g_unregisterResult;
}

class LaunchServiceJsTest : public testing::Test {
public:
    void SetUp() override
    {
        g_registerCalls = 0;
        g_unregisterCalls = 0;
        g_registerResult = ERR_OK;
        g_unregisterResult = ERR_OK;
        panda::RuntimeOption option;
        vm_ = panda::JSNApi::CreateJSVM(option);
        ASSERT_NE(vm_, nullptr);
        engine_ = std::make_unique<ArkNativeEngine>(vm_, nullptr);
        env_ = reinterpret_cast<napi_env>(engine_.get());
        ASSERT_EQ(napi_open_handle_scope(env_, &scope_), napi_ok);
    }
    void TearDown() override
    {
        if (scope_ != nullptr) {
            EXPECT_EQ(napi_close_handle_scope(env_, scope_), napi_ok);
            scope_ = nullptr;
        }
        engine_.reset();
        if (vm_ != nullptr) { panda::JSNApi::DestroyJSVM(vm_); }
    }
    struct PromiseResult {
        bool settled = false;
        bool rejected = false;
        int32_t error = 0;
    };

    napi_value InitExports()
    {
        napi_value exports = nullptr;
        EXPECT_EQ(napi_create_object(env_, &exports), napi_ok);
        JsLaunchServiceManagerInit(env_, exports);
        return exports;
    }

    napi_value CreateObserver()
    {
        napi_value observer = nullptr;
        EXPECT_EQ(napi_create_object(env_, &observer), napi_ok);
        for (const char *name : {"onLaunchServiceRegisteredChanged", "onLaunchServiceProcessStateChanged"}) {
            napi_value function = nullptr;
            EXPECT_EQ(napi_create_function(env_, name, NAPI_AUTO_LENGTH,
                [](napi_env env, napi_callback_info) -> napi_value {
                    napi_value result = nullptr;
                    napi_get_undefined(env, &result);
                    return result;
                }, nullptr, &function), napi_ok);
            EXPECT_EQ(napi_set_named_property(env_, observer, name, function), napi_ok);
        }
        return observer;
    }

    napi_value Call(napi_value exports, const char *name, napi_value argument)
    {
        napi_value function = nullptr;
        napi_value result = nullptr;
        EXPECT_EQ(napi_get_named_property(env_, exports, name, &function), napi_ok);
        EXPECT_EQ(napi_call_function(env_, exports, function, 1, &argument, &result), napi_ok);
        return result;
    }

    PromiseResult *ObservePromise(napi_value promise)
    {
        bool isPromise = false;
        EXPECT_EQ(napi_is_promise(env_, promise, &isPromise), napi_ok);
        EXPECT_TRUE(isPromise);
        results_.emplace_back(std::make_unique<PromiseResult>());
        auto result = results_.back().get();
        napi_value callbacks[] = {nullptr, nullptr};
        EXPECT_EQ(napi_create_function(env_, "resolved", NAPI_AUTO_LENGTH,
            [](napi_env env, napi_callback_info info) -> napi_value {
                void *data = nullptr;
                napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data);
                static_cast<PromiseResult *>(data)->settled = true;
                return nullptr;
            }, result, &callbacks[0]), napi_ok);
        EXPECT_EQ(napi_create_function(env_, "rejected", NAPI_AUTO_LENGTH,
            [](napi_env env, napi_callback_info info) -> napi_value {
                size_t argc = 1;
                napi_value error = nullptr;
                void *data = nullptr;
                napi_get_cb_info(env, info, &argc, &error, nullptr, &data);
                auto result = static_cast<PromiseResult *>(data);
                result->settled = true;
                result->rejected = true;
                napi_value code = nullptr;
                EXPECT_EQ(napi_get_named_property(env, error, "code", &code), napi_ok);
                EXPECT_EQ(napi_get_value_int32(env, code, &result->error), napi_ok);
                return nullptr;
            }, result, &callbacks[1]), napi_ok);
        napi_value then = nullptr;
        napi_value ignored = nullptr;
        EXPECT_EQ(napi_get_named_property(env_, promise, "then", &then), napi_ok);
        EXPECT_EQ(napi_call_function(env_, promise, then, std::size(callbacks), callbacks, &ignored), napi_ok);
        return result;
    }

    void Wait(PromiseResult *result)
    {
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (!result->settled && std::chrono::steady_clock::now() < deadline) {
            engine_->Loop(LOOP_NOWAIT);
            std::this_thread::yield();
        }
        EXPECT_TRUE(result->settled);
    }

    // Keep callback data alive until TearDown destroys the runtime.
    std::vector<std::unique_ptr<PromiseResult>> results_;
    panda::ecmascript::EcmaVM *vm_ = nullptr;
    std::unique_ptr<ArkNativeEngine> engine_;
    napi_env env_ = nullptr;
    napi_handle_scope scope_ = nullptr;
};

/**
 * @tc.name: JsLaunchServiceManagerInit_001
 * @tc.desc: Export all current functions and enum values through the real NAPI runtime.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceJsTest, JsLaunchServiceManagerInit_001, TestSize.Level1)
{
    napi_value exports = nullptr;
    ASSERT_EQ(napi_create_object(env_, &exports), napi_ok);
    EXPECT_EQ(JsLaunchServiceManagerInit(env_, exports), exports);
    for (const char *name : {"register", "unregister", "start", "stop", "getInformation",
        "getAllRegisteredInformation", "setLaunchServicesEnabled", "onLaunchServiceObserver",
        "offLaunchServiceObserver"}) {
        napi_value function = nullptr;
        ASSERT_EQ(napi_get_named_property(env_, exports, name, &function), napi_ok);
        napi_valuetype type = napi_undefined;
        ASSERT_EQ(napi_typeof(env_, function, &type), napi_ok);
        EXPECT_EQ(type, napi_function);
    }
    bool exists = true;
    ASSERT_EQ(napi_has_named_property(env_, exports, "setLaunchServicesEnable", &exists), napi_ok);
    EXPECT_FALSE(exists);
    napi_value state = nullptr;
    napi_value enabled = nullptr;
    int32_t value = -1;
    ASSERT_EQ(napi_get_named_property(env_, exports, "LaunchServiceState", &state), napi_ok);
    ASSERT_EQ(napi_get_named_property(env_, state, "ENABLED", &enabled), napi_ok);
    ASSERT_EQ(napi_get_value_int32(env_, enabled, &value), napi_ok);
    EXPECT_EQ(value, 1);
}
/**
 * @tc.name: Observer_SharedRegistration_001
 * @tc.desc: More than ten callbacks share one server observer; only the last off unregisters it.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceJsTest, Observer_SharedRegistration_001, TestSize.Level1)
{
    auto exports = InitExports();
    auto observer = CreateObserver();
    std::vector<napi_value> ids;
    for (int i = 0; i < 11; ++i) {
        ids.push_back(Call(exports, "onLaunchServiceObserver", observer));
    }
    EXPECT_EQ(g_registerCalls, 1);
    for (size_t i = 0; i < ids.size(); ++i) {
        auto result = ObservePromise(Call(exports, "offLaunchServiceObserver", ids[i]));
        EXPECT_FALSE(result->settled);
        EXPECT_EQ(g_unregisterCalls, 0);
        Wait(result);
        EXPECT_FALSE(result->rejected);
        EXPECT_EQ(g_unregisterCalls, i + 1 == ids.size() ? 1 : 0);
    }
    Call(exports, "onLaunchServiceObserver", observer);
    EXPECT_EQ(g_registerCalls, 2);
}

/**
 * @tc.name: Observer_DuplicateOff_001
 * @tc.desc: Queued duplicate off rejects the stale ID without removing another callback.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceJsTest, Observer_DuplicateOff_001, TestSize.Level1)
{
    auto exports = InitExports();
    auto observer = CreateObserver();
    auto first = Call(exports, "onLaunchServiceObserver", observer);
    auto second = Call(exports, "onLaunchServiceObserver", observer);
    auto removed = ObservePromise(Call(exports, "offLaunchServiceObserver", first));
    auto duplicate = ObservePromise(Call(exports, "offLaunchServiceObserver", first));
    Wait(removed);
    Wait(duplicate);
    EXPECT_FALSE(removed->rejected);
    EXPECT_TRUE(duplicate->rejected);
    EXPECT_EQ(duplicate->error, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_LAUNCH_SERVICE_INVALID_ID));
    EXPECT_EQ(g_unregisterCalls, 0);
    auto last = ObservePromise(Call(exports, "offLaunchServiceObserver", second));
    Wait(last);
    EXPECT_FALSE(last->rejected);
    EXPECT_EQ(g_unregisterCalls, 1);
}

/**
 * @tc.name: Observer_UnregisterRetry_001
 * @tc.desc: A server failure rejects off and preserves the callback for a successful retry.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceJsTest, Observer_UnregisterRetry_001, TestSize.Level1)
{
    auto exports = InitExports();
    auto id = Call(exports, "onLaunchServiceObserver", CreateObserver());
    g_unregisterResult = AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    auto failed = ObservePromise(Call(exports, "offLaunchServiceObserver", id));
    Wait(failed);
    EXPECT_TRUE(failed->rejected);
    EXPECT_EQ(g_unregisterCalls, 1);
    g_unregisterResult = ERR_OK;
    auto retry = ObservePromise(Call(exports, "offLaunchServiceObserver", id));
    Wait(retry);
    EXPECT_FALSE(retry->rejected);
    EXPECT_EQ(g_unregisterCalls, 2);
}

/**
 * @tc.name: Observer_RegisterRetry_001
 * @tc.desc: Failed first registration retains no callback; retry registers with the server again.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceJsTest, Observer_RegisterRetry_001, TestSize.Level1)
{
    auto exports = InitExports();
    auto observer = CreateObserver();
    g_registerResult = AAFwk::ERR_LAUNCH_SERVICE_OBSERVER_LIMIT;
    napi_value function = nullptr;
    napi_value ignored = nullptr;
    ASSERT_EQ(napi_get_named_property(env_, exports, "onLaunchServiceObserver", &function), napi_ok);
    EXPECT_EQ(napi_call_function(env_, exports, function, 1, &observer, &ignored), napi_pending_exception);
    napi_value error = nullptr;
    ASSERT_EQ(napi_get_and_clear_last_exception(env_, &error), napi_ok);
    EXPECT_EQ(g_registerCalls, 1);
    g_registerResult = ERR_OK;
    auto id = Call(exports, "onLaunchServiceObserver", observer);
    EXPECT_EQ(g_registerCalls, 2);
    auto removed = ObservePromise(Call(exports, "offLaunchServiceObserver", id));
    Wait(removed);
    EXPECT_FALSE(removed->rejected);
    EXPECT_EQ(g_unregisterCalls, 1);
}

/**
 * @tc.name: Observer_Cleanup_001
 * @tc.desc: Environment cleanup unregisters one shared observer regardless of callback count.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceJsTest, Observer_Cleanup_001, TestSize.Level1)
{
    auto exports = InitExports();
    auto observer = CreateObserver();
    Call(exports, "onLaunchServiceObserver", observer);
    Call(exports, "onLaunchServiceObserver", observer);
    EXPECT_EQ(g_registerCalls, 1);
    ASSERT_EQ(napi_close_handle_scope(env_, scope_), napi_ok);
    scope_ = nullptr;
    engine_.reset();
    env_ = nullptr;
    EXPECT_EQ(g_unregisterCalls, 1);
}
} // namespace OHOS::AbilityRuntime
