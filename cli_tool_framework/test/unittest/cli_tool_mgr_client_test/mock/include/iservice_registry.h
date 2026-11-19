/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_MOCK_ISERVICE_REGISTRY_H
#define OHOS_ABILITY_RUNTIME_MOCK_ISERVICE_REGISTRY_H

#include "if_system_ability_manager.h"

namespace OHOS {
class SystemAbilityManagerClient {
public:
    static SystemAbilityManagerClient &GetInstance();
    sptr<ISystemAbilityManager> GetSystemAbilityManager();

private:
    SystemAbilityManagerClient() = default;
    ~SystemAbilityManagerClient() = default;
};
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MOCK_ISERVICE_REGISTRY_H
