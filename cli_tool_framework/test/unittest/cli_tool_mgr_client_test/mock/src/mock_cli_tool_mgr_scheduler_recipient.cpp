/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "cli_tool_mgr_scheduler_recipient.h"

#include "cli_event_reply_manager.h"
#include "cli_session_subscription_manager.h"

namespace OHOS {
namespace CliTool {
int32_t CliToolManagerSchedulerRecipient::SchedulerSessionEvent(
    const std::string &sessionId, const std::string &subscriptionId, const CliToolEvent &event)
{
    return CliSessionSubscriptionManager::GetInstance().HandleSessionEvent(sessionId, subscriptionId, event);
}

int32_t CliToolManagerSchedulerRecipient::SchedulerInputReplyEvent(const std::string &eventId, int32_t resultCode)
{
    CliEventReplyResult result;
    result.code = resultCode;
    return CliEventReplyManager::GetInstance().HandleEventReply(eventId, result);
}

int32_t CliToolManagerSchedulerRecipient::SchedulerExecToolReplyEvent(
    const std::string &eventId, int32_t resultCode, const CliSessionInfo &session)
{
    CliEventReplyResult result;
    result.code = resultCode;
    result.sessionInfo = session;
    return CliEventReplyManager::GetInstance().HandleEventReply(eventId, result);
}
} // namespace CliTool
} // namespace OHOS
