/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_MOCK_ICLI_TOOL_MANAGER_SCHEDULER_H
#define OHOS_ABILITY_RUNTIME_MOCK_ICLI_TOOL_MANAGER_SCHEDULER_H

#include "cli_session_info.h"
#include "cli_tool_event.h"
#include "iremote_broker.h"

namespace OHOS {
namespace CliTool {
class ICliToolManagerScheduler : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.CliTool.ICliToolManagerScheduler")

    virtual int32_t SchedulerSessionEvent(
        const std::string &sessionId, const std::string &subscriptionId, const CliToolEvent &event) = 0;
    virtual int32_t SchedulerInputReplyEvent(const std::string &eventId, int32_t resultCode) = 0;
    virtual int32_t SchedulerExecToolReplyEvent(
        const std::string &eventId, int32_t resultCode, const CliSessionInfo &session) = 0;
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MOCK_ICLI_TOOL_MANAGER_SCHEDULER_H
