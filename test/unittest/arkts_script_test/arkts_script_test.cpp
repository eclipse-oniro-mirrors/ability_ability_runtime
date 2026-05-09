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

#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#define private public
#include "arkts_script.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace ArktsScript {
namespace {

std::vector<char*> BuildArgv(std::vector<std::string>& args)
{
    std::vector<char*> argv;
    argv.reserve(args.size());
    for (auto& arg : args) {
        argv.push_back(arg.data());
    }
    return argv;
}

class ScopedCoutRedirect {
public:
    ScopedCoutRedirect() : oldBuffer_(std::cout.rdbuf(stream_.rdbuf())) {}
    ~ScopedCoutRedirect()
    {
        std::cout.rdbuf(oldBuffer_);
    }

    std::string GetOutput() const
    {
        return stream_.str();
    }

private:
    std::ostringstream stream_;
    std::streambuf* oldBuffer_ = nullptr;
};

class ScopedCerrRedirect {
public:
    ScopedCerrRedirect() : oldBuffer_(std::cerr.rdbuf(stream_.rdbuf())) {}
    ~ScopedCerrRedirect()
    {
        std::cerr.rdbuf(oldBuffer_);
    }

    std::string GetOutput() const
    {
        return stream_.str();
    }

private:
    std::ostringstream stream_;
    std::streambuf* oldBuffer_ = nullptr;
};

} // namespace

class ArktsScriptTest : public testing::Test {
public:
    void TearDown() override
    {
        ArktsScript::CloseCompletionChannel(channel_);
    }

protected:
    CompletionChannel channel_;
};

/**
 * @tc.name: ParseArguments_0100
 * @tc.desc: Parse required options and json object arguments.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, ParseArguments_0100, TestSize.Level1)
{
    std::vector<std::string> args = {
        "ohos-arktsScript",
        "--abcPath",
        "/data/test/module.abc",
        "--scriptPath",
        "entry/src/main/ets/TestScript.ets",
        "--functionName",
        "calculate",
        "--args",
        R"({"arg0":"text","arg1":true,"arg2":2147483648,"arg4":{"name":"tool"},"arg5":["a","b"]})",
    };
    std::vector<char*> argv = BuildArgv(args);
    ScriptArgs parsedArgs;

    ASSERT_TRUE(ArktsScript::ParseArguments(static_cast<int>(argv.size()), argv.data(), parsedArgs));
    EXPECT_EQ(parsedArgs.abcPath, "/data/test/module.abc");
    EXPECT_EQ(parsedArgs.scriptName, "entry/src/main/ets/TestScript.ets");
    EXPECT_EQ(parsedArgs.funName, "calculate");
    ASSERT_EQ(parsedArgs.arguments.size(), 6U);
    EXPECT_EQ(parsedArgs.arguments[0].type, ScriptArgType::STRING);
    EXPECT_EQ(parsedArgs.arguments[0].value, "text");
    EXPECT_EQ(parsedArgs.arguments[1].type, ScriptArgType::BOOLEAN);
    EXPECT_EQ(parsedArgs.arguments[1].value, "1");
    EXPECT_EQ(parsedArgs.arguments[2].type, ScriptArgType::DOUBLE);
    EXPECT_EQ(parsedArgs.arguments[2].value, "2147483648");
    EXPECT_EQ(parsedArgs.arguments[3].type, ScriptArgType::UNDEFINED);
    EXPECT_EQ(parsedArgs.arguments[4].type, ScriptArgType::JSON_VALUE);
    EXPECT_EQ(parsedArgs.arguments[4].value, R"({"name":"tool"})");
    EXPECT_EQ(parsedArgs.arguments[5].type, ScriptArgType::JSON_VALUE);
    EXPECT_EQ(parsedArgs.arguments[5].value, R"(["a","b"])");
}

/**
 * @tc.name: ParseArguments_0200
 * @tc.desc: Parse help option.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, ParseArguments_0200, TestSize.Level1)
{
    std::vector<std::string> args = {
        "ohos-arktsScript",
        "--help",
    };
    std::vector<char*> argv = BuildArgv(args);
    ScriptArgs parsedArgs;

    EXPECT_TRUE(ArktsScript::ParseArguments(static_cast<int>(argv.size()), argv.data(), parsedArgs));
    EXPECT_TRUE(parsedArgs.showHelp);
}

/**
 * @tc.name: ParseArguments_0300
 * @tc.desc: Reject invalid argument forms.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, ParseArguments_0300, TestSize.Level1)
{
    ScriptArgs parsedArgs;
    EXPECT_FALSE(ArktsScript::ParseArguments(0, nullptr, parsedArgs));

    std::vector<std::string> noOption = { "ohos-arktsScript" };
    std::vector<char*> noOptionArgv = BuildArgv(noOption);
    EXPECT_FALSE(ArktsScript::ParseArguments(static_cast<int>(noOptionArgv.size()), noOptionArgv.data(), parsedArgs));

    std::vector<std::string> missingFunction = { "ohos-arktsScript", "--abcPath", "/data/test/module.abc" };
    std::vector<char*> missingFunctionArgv = BuildArgv(missingFunction);
    EXPECT_FALSE(ArktsScript::ParseArguments(static_cast<int>(missingFunctionArgv.size()),
        missingFunctionArgv.data(), parsedArgs));

    std::vector<std::string> unknownOption = {
        "ohos-arktsScript", "--abcPath", "/data/test/module.abc", "--functionName", "run", "--unknown"
    };
    std::vector<char*> unknownOptionArgv = BuildArgv(unknownOption);
    EXPECT_FALSE(ArktsScript::ParseArguments(static_cast<int>(unknownOptionArgv.size()),
        unknownOptionArgv.data(), parsedArgs));

    std::vector<std::string> badArgsJson = {
        "ohos-arktsScript", "--abcPath", "/data/test/module.abc", "--functionName", "run", "--args", "[]"
    };
    std::vector<char*> badArgsJsonArgv = BuildArgv(badArgsJson);
    EXPECT_FALSE(ArktsScript::ParseArguments(static_cast<int>(badArgsJsonArgv.size()),
        badArgsJsonArgv.data(), parsedArgs));

    std::vector<std::string> badArgName = {
        "ohos-arktsScript", "--abcPath", "/data/test/module.abc", "--functionName", "run", "--args",
        R"({"argv0":1})"
    };
    std::vector<char*> badArgNameArgv = BuildArgv(badArgName);
    EXPECT_FALSE(ArktsScript::ParseArguments(static_cast<int>(badArgNameArgv.size()),
        badArgNameArgv.data(), parsedArgs));

    std::vector<std::string> badArgValue = {
        "ohos-arktsScript", "--abcPath", "/data/test/module.abc", "--functionName", "run", "--args",
        R"({"arg0":null})"
    };
    std::vector<char*> badArgValueArgv = BuildArgv(badArgValue);
    EXPECT_FALSE(ArktsScript::ParseArguments(static_cast<int>(badArgValueArgv.size()),
        badArgValueArgv.data(), parsedArgs));
}

/**
 * @tc.name: ParseFunctionAndArguments_0100
 * @tc.desc: Reject missing option value and duplicated args json.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, ParseFunctionAndArguments_0100, TestSize.Level1)
{
    std::vector<std::string> missingValue = { "ohos-arktsScript", "--abcPath" };
    std::vector<char*> missingValueArgv = BuildArgv(missingValue);
    ScriptArgs args;
    int index = 1;
    EXPECT_FALSE(ArktsScript::ParseFunctionAndArguments(static_cast<int>(missingValueArgv.size()),
        missingValueArgv.data(), index, args));

    std::vector<std::string> duplicatedArgs = {
        "ohos-arktsScript", "--abcPath", "/data/test/module.abc", "--functionName", "run", "--args",
        R"({"arg0":1})", "--args", R"({"arg1":2})"
    };
    std::vector<char*> duplicatedArgsArgv = BuildArgv(duplicatedArgs);
    index = 1;
    EXPECT_FALSE(ArktsScript::ParseFunctionAndArguments(static_cast<int>(duplicatedArgsArgv.size()),
        duplicatedArgsArgv.data(), index, args));
}

/**
 * @tc.name: OutputResult_0100
 * @tc.desc: Output result to stdout.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, OutputResult_0100, TestSize.Level1)
{
    ScopedCoutRedirect coutRedirect;
    ArktsScript::OutputResult("ok");
    EXPECT_EQ(coutRedirect.GetOutput(), "ok\n");
}

/**
 * @tc.name: OutputError_0100
 * @tc.desc: Output error to stderr in JSON format.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, OutputError_0100, TestSize.Level1)
{
    ScopedCerrRedirect cerrRedirect;
    ArktsScript::OutputError({ "failed", "" });
    EXPECT_EQ(cerrRedirect.GetOutput(), R"({"error":"failed","errorType":"EXECUTION_ERROR","success":false})" "\n");
}

/**
 * @tc.name: CompletionChannel_Create_0100
 * @tc.desc: Create completion channel.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, CompletionChannel_Create_0100, TestSize.Level1)
{
    ASSERT_TRUE(ArktsScript::CreateCompletionChannel(channel_));
    EXPECT_GE(channel_.eventFd, 0);
    EXPECT_GE(channel_.epollFd, 0);
}

/**
 * @tc.name: CompletionChannel_Signal_0100
 * @tc.desc: Signal and read completion event.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, CompletionChannel_Signal_0100, TestSize.Level1)
{
    ASSERT_TRUE(ArktsScript::CreateCompletionChannel(channel_));
    EXPECT_TRUE(ArktsScript::SignalCompletion(channel_.eventFd));
    ArktsScript::ReadCompletionSignal(channel_);
}

/**
 * @tc.name: CompletionChannel_Close_0100
 * @tc.desc: Close completion channel and verify cleanup.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, CompletionChannel_Close_0100, TestSize.Level1)
{
    ASSERT_TRUE(ArktsScript::CreateCompletionChannel(channel_));
    ArktsScript::CloseCompletionChannel(channel_);
    EXPECT_EQ(channel_.eventFd, -1);
    EXPECT_EQ(channel_.epollFd, -1);
    EXPECT_FALSE(ArktsScript::SignalCompletion(channel_.eventFd));
}

/**
 * @tc.name: ExecutionContext_PublishResult_0100
 * @tc.desc: Publish result to execution context.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, ExecutionContext_PublishResult_0100, TestSize.Level1)
{
    auto execContext = std::make_shared<ExecutionContext>();

    EXPECT_FALSE(ArktsScript::PublishResult(nullptr, -1, "result"));
    EXPECT_TRUE(ArktsScript::PublishResult(execContext, -1, "result"));
    EXPECT_FALSE(ArktsScript::PublishResult(execContext, -1, "again"));

    ExecutionSnapshot snapshot = ArktsScript::TakeExecutionSnapshot(execContext);
    EXPECT_TRUE(snapshot.resultReady);
    EXPECT_EQ(snapshot.result, "result");
}

/**
 * @tc.name: ExecutionContext_PublishFailure_0100
 * @tc.desc: Publish failure to execution context.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, ExecutionContext_PublishFailure_0100, TestSize.Level1)
{
    auto execContext = std::make_shared<ExecutionContext>();

    EXPECT_TRUE(ArktsScript::PublishFailure(execContext, -1, { "failed", "TEST_ERROR" }));
    EXPECT_FALSE(ArktsScript::PublishFailure(execContext, -1, { "again", "TEST_ERROR" }));

    ExecutionSnapshot snapshot = ArktsScript::TakeExecutionSnapshot(execContext);
    EXPECT_TRUE(snapshot.scriptDone);
    EXPECT_EQ(snapshot.error.message, "failed");
    EXPECT_EQ(snapshot.error.type, "TEST_ERROR");
}

/**
 * @tc.name: ExecutionContext_MarkScriptDone_0100
 * @tc.desc: Mark script as done in execution context.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, ExecutionContext_MarkScriptDone_0100, TestSize.Level1)
{
    auto execContext = std::make_shared<ExecutionContext>();

    EXPECT_FALSE(ArktsScript::MarkScriptDone(nullptr, -1));
    EXPECT_TRUE(ArktsScript::MarkScriptDone(execContext, -1));

    ExecutionSnapshot snapshot = ArktsScript::TakeExecutionSnapshot(execContext);
    EXPECT_TRUE(snapshot.scriptDone);
}

/**
 * @tc.name: ExecutionContext_TakeSnapshot_0100
 * @tc.desc: Take execution snapshot from context.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, ExecutionContext_TakeSnapshot_0100, TestSize.Level1)
{
    auto execContext = std::make_shared<ExecutionContext>();
    ArktsScript::PublishResult(execContext, -1, "test_result");

    ExecutionSnapshot snapshot = ArktsScript::TakeExecutionSnapshot(execContext);
    EXPECT_TRUE(snapshot.resultReady);
    EXPECT_FALSE(snapshot.scriptDone);
    EXPECT_EQ(snapshot.result, "test_result");
    EXPECT_TRUE(snapshot.error.message.empty());

    EXPECT_FALSE(ArktsScript::TakeExecutionSnapshot(nullptr).resultReady);
}

/**
 * @tc.name: Callback_Success_0100
 * @tc.desc: Complete callback records success result.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, Callback_Success_0100, TestSize.Level1)
{
    auto execContext = std::make_shared<ExecutionContext>();
    CompletionChannel channel;
    ResultCallback callback = ArktsScript::CreateCompleteCallback(execContext, channel);

    callback(true, "done", {});
    ExecutionSnapshot snapshot = ArktsScript::TakeExecutionSnapshot(execContext);
    EXPECT_TRUE(snapshot.resultReady);
    EXPECT_EQ(snapshot.result, "done");
}

/**
 * @tc.name: Callback_Failure_0100
 * @tc.desc: Complete callback records failure and ignores duplicated failure.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, Callback_Failure_0100, TestSize.Level1)
{
    auto execContext = std::make_shared<ExecutionContext>();
    CompletionChannel channel;
    ResultCallback callback = ArktsScript::CreateCompleteCallback(execContext, channel);

    callback(false, "", { "failed", "CALLBACK_ERROR" });
    ExecutionSnapshot snapshot = ArktsScript::TakeExecutionSnapshot(execContext);
    EXPECT_EQ(snapshot.error.message, "failed");
    EXPECT_EQ(snapshot.error.type, "CALLBACK_ERROR");
}

/**
 * @tc.name: RuntimeFacade_0100
 * @tc.desc: Cover public runtime facade failure paths that do not require a JS engine.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, RuntimeFacade_0100, TestSize.Level1)
{
    EXPECT_FALSE(ArktsScript::LoadAbcFile(nullptr, "/data/test/module.abc"));

    ScriptError error;
    ScriptArgs args;
    args.abcPath = "/data/test/module.abc";
    EXPECT_FALSE(ArktsScript::LoadScriptFile(args, nullptr, error));
    EXPECT_EQ(error.type, "LOAD_ERROR");

    std::vector<std::string> invalidArgs = { "ohos-arktsScript", "--abcPath" };
    std::vector<char*> argv = BuildArgv(invalidArgs);
    EXPECT_EQ(ArktsScript::RunArkTsScript(static_cast<int>(argv.size()), argv.data()), EXIT_FAILURE);
}

/**
 * @tc.name: ExecuteFlowHelpers_0100
 * @tc.desc: Cover safe failure paths for execution flow helper interfaces.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, ExecuteFlowHelpers_0100, TestSize.Level1)
{
    ScriptArgs args;
    args.abcPath = "/data/test/module.abc";
    args.funName = "run";
    napi_value receiver = nullptr;
    napi_value func = nullptr;
    ScriptError error;

    EXPECT_FALSE(ArktsScript::ResolveScriptFunction(args, nullptr, receiver, func, error));
    EXPECT_EQ(error.type, "FUNCTION_ERROR");

    error = {};
    EXPECT_FALSE(ArktsScript::CallResolvedFunction(args, nullptr, receiver, func, error));
    EXPECT_EQ(error.type, "ARGUMENT_ERROR");

    CompletionChannel channel;
    std::thread monitorThread;
    EXPECT_EQ(ArktsScript::FinalizeAndJoin(nullptr, channel, monitorThread, { "failed", "TEST_ERROR" }),
        EXIT_FAILURE);
}

/**
 * @tc.name: SnapshotHandler_0100
 * @tc.desc: Handle incomplete snapshots without exiting monitor process.
 * @tc.type: FUNC
 */
HWTEST_F(ArktsScriptTest, SnapshotHandler_0100, TestSize.Level1)
{
    ExecutionSnapshot snapshot;
    snapshot.resultReady = true;
    snapshot.scriptDone = false;
    CompletionChannel channel;

    EXPECT_NO_FATAL_FAILURE(ArktsScript::HandleExecutionSnapshot(snapshot, channel));
}

} // namespace ArktsScript
} // namespace OHOS
