/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "ipc_skeleton.h"

namespace OHOS {
pid_t IPCSkeleton::callingUid = 0;
pid_t IPCSkeleton::callingPid = 0;
uint64_t IPCSkeleton::callingFullTokenId = 0;

pid_t IPCSkeleton::GetCallingUid()
{
    return callingUid;
}

pid_t IPCSkeleton::GetCallingPid()
{
    return callingPid;
}

uint64_t IPCSkeleton::GetCallingFullTokenID()
{
    return callingFullTokenId;
}

void IPCSkeleton::Reset()
{
    callingUid = 0;
    callingPid = 0;
    callingFullTokenId = 0;
}
} // namespace OHOS
