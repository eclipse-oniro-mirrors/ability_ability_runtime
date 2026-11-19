/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_IO_MONITOR_H
#define OHOS_ABILITY_RUNTIME_IO_MONITOR_H

#include <functional>
#include <memory>
#include <string>

namespace OHOS {
namespace CliTool {
class IOMonitor : public std::enable_shared_from_this<IOMonitor> {
public:
    using OutputCallback = std::function<void(const std::string &, bool, const std::string &)>;
    using InputReplyCallback = std::function<void(const std::string &, const std::string &, bool)>;
    using SessionClosedCallback = std::function<void(const std::string &, bool)>;
    using SessionDrainedCallback = std::function<void(const std::string &)>;

    static std::shared_ptr<IOMonitor> Create();
    bool Start();
    void Stop();
    bool RegisterSession(const std::string &sessionId, int stdoutFd, int stderrFd, int stdinFd);
    void UnregisterSession(const std::string &sessionId);
    void SetOutputCallback(OutputCallback callback);
    void SetInputReplyCallback(InputReplyCallback callback);
    void SetSessionClosedCallback(SessionClosedCallback callback);
    void SetSessionDrainedCallback(SessionDrainedCallback callback);
    void SendMessage(const std::string &sessionId, const std::string &message, const std::string &eventId);
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_IO_MONITOR_H
