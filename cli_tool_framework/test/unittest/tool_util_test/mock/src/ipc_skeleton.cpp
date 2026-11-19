/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "ipc_skeleton.h"

namespace OHOS {
pid_t IPCSkeleton::callingUid = 0;
pid_t IPCSkeleton::callingPid = 0;
uint64_t IPCSkeleton::callingFullTokenId = 0;
std::string IPCSkeleton::callingIdentity;
uint32_t IPCSkeleton::callingTokenId = 0;

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
    callingIdentity.clear();
    callingTokenId = 0;
    setCallingIdentityRet = true;
}

std::string IPCSkeleton::ResetCallingIdentity()
{
    return callingIdentity;
}

bool IPCSkeleton::setCallingIdentityRet = true;

bool IPCSkeleton::SetCallingIdentity(const std::string &identity)
{
    callingIdentity = identity;
    return setCallingIdentityRet;
}

uint32_t IPCSkeleton::GetCallingTokenID()
{
    return callingTokenId;
}
} // namespace OHOS
