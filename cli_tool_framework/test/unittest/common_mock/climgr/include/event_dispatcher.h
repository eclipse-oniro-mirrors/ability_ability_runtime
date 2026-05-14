/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_EVENT_DISPATCHER_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_EVENT_DISPATCHER_H

#include <string>

#include "cli_session_info.h"
#include "icli_tool_manager_scheduler.h"

namespace OHOS {
namespace CliTool {
class EventDispatcher final {
public:
    static EventDispatcher &GetInstance();

    bool RegisterScheduler(int32_t callerPid, const sptr<ICliToolManagerScheduler> &scheduler);
    void UnregisterScheduler(int32_t callerPid);
    bool RegisterSubscriber(const std::string &sessionId, const std::string &subscriptionId, int32_t callerPid);
    bool UnregisterSubscriber(const std::string &sessionId, const std::string &subscriptionId, int32_t callerPid);
    void DispatchIOEvent(const std::string &sessionId, const std::string &eventType, const std::string &data);
    void DispatchErrorEvent(const std::string &sessionId, const std::string &error);
    void DispatchExitEvent(const std::string &sessionId, int32_t exitCode);
    bool DispatchInputReplyEvent(int32_t callerPid, const std::string &eventId, int32_t result);
    bool DispatchExecToolReplyEvent(int32_t callerPid, const std::string &eventId,
        int32_t result, const CliSessionInfo &session);
    void ClearSessionSubscribers(const std::string &sessionId);
    void ClearAll();
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_EVENT_DISPATCHER_H
