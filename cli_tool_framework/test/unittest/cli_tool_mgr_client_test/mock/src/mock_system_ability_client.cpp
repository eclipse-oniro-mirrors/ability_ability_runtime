/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "iservice_registry.h"
#include "mock_cli_tool_mgr_client_flag.h"
#include "mock_system_ability_manager.h"

namespace OHOS {
SystemAbilityManagerClient &SystemAbilityManagerClient::GetInstance()
{
    static SystemAbilityManagerClient instance;
    return instance;
}

sptr<ISystemAbilityManager> SystemAbilityManagerClient::GetSystemAbilityManager()
{
    if (CliTool::CliToolMgrClientFlag::nullSystemAbility) {
        return nullptr;
    }
    return sptr<CliTool::MockSystemAbilityManager>::MakeSptr();
}
} // namespace OHOS
