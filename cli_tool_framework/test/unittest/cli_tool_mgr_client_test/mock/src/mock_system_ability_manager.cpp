/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "mock_system_ability_manager.h"

#include "cli_error_code.h"
#include "mock_cli_tool_mgr_client_flag.h"

namespace OHOS {
namespace CliTool {
int32_t MockSystemAbilityManager::LoadSystemAbility(
    int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback> &callback)
{
    if (CliToolMgrClientFlag::retLoadSystemAbility != ERR_OK) {
        return CliToolMgrClientFlag::retLoadSystemAbility;
    }
    if (CliToolMgrClientFlag::shouldCallback) {
        callback->OnLoadSystemAbilitySuccess(systemAbilityId, CliToolMgrClientFlag::cliToolMgr);
    }
    return ERR_OK;
}
} // namespace CliTool
} // namespace OHOS
