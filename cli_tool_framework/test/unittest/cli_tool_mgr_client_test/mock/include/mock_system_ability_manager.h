/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_MOCK_CLI_SYSTEM_ABILITY_MANAGER_H
#define OHOS_ABILITY_RUNTIME_MOCK_CLI_SYSTEM_ABILITY_MANAGER_H

#include "if_system_ability_manager.h"
#include "iremote_stub.h"

namespace OHOS {
namespace CliTool {
class MockSystemAbilityManager : public IRemoteStub<ISystemAbilityManager> {
public:
    int32_t LoadSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback> &callback) override;
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MOCK_CLI_SYSTEM_ABILITY_MANAGER_H
