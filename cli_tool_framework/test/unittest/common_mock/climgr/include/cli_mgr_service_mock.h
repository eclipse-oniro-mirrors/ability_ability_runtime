/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_CLI_MGR_SERVICE_MOCK_H
#define OHOS_ABILITY_RUNTIME_CLI_MGR_SERVICE_MOCK_H

#include <cstdint>
#include <string>
#include <vector>

namespace OHOS {
namespace AppExecFwk {
// Provide AppMgrResultCode enum for tests
enum AppMgrResultCode {
    RESULT_OK = 0,
    ERROR_SERVICE_NOT_READY = -1,
};
} // namespace AppExecFwk

namespace CliTool {
class CliMgrServiceMock {
public:
    static int32_t createChildProcessResult;
    static int32_t createShellProcessResult;
    static bool killpgResult;
    static int32_t registerSessionResult;
    static int32_t unregisterSessionCount;
    static int32_t stopCount;
    static int32_t sendMessageCount;
    static int32_t ensureToolsLoadedResult;
    static int32_t getToolByNameResult;
    static int32_t connectAppMgrResult;
    static int32_t registerAppObserverResult;
    static int32_t querySkillTypeResult;
    static int32_t executeSkillResult;
    static int32_t skillType;
    static std::string lastSkillName;
    static bool toolHasSubCommand;
    static std::string subCommandName;
    static std::vector<std::string> toolPermissions;
    static std::vector<std::string> subCommandPermissions;

    static void Reset();
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_MGR_SERVICE_MOCK_H
