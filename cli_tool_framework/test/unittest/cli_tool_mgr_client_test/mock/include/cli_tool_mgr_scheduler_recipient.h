/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_MOCK_CLI_TOOL_MGR_SCHEDULER_RECIPIENT_H
#define OHOS_ABILITY_RUNTIME_MOCK_CLI_TOOL_MGR_SCHEDULER_RECIPIENT_H

#include "icli_tool_manager_scheduler.h"
#include "iremote_stub.h"

namespace OHOS {
namespace CliTool {
class CliToolManagerSchedulerRecipient : public IRemoteStub<ICliToolManagerScheduler> {
public:
    int32_t SchedulerSessionEvent(
        const std::string &sessionId, const std::string &subscriptionId, const CliToolEvent &event) override;
    int32_t SchedulerInputReplyEvent(const std::string &eventId, int32_t resultCode) override;
    int32_t SchedulerExecToolReplyEvent(
        const std::string &eventId, int32_t resultCode, const CliSessionInfo &session) override;
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MOCK_CLI_TOOL_MGR_SCHEDULER_RECIPIENT_H
