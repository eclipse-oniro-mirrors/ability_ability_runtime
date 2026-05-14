/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_MOCK_IF_SYSTEM_ABILITY_MANAGER_H
#define OHOS_ABILITY_RUNTIME_MOCK_IF_SYSTEM_ABILITY_MANAGER_H

#include "iremote_broker.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
class ISystemAbilityManager : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.ISystemAbilityManager")
    virtual int32_t LoadSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback> &callback) = 0;
};
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MOCK_IF_SYSTEM_ABILITY_MANAGER_H
