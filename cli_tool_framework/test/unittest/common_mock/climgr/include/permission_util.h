/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_PERMISSION_UTIL_H
#define OHOS_ABILITY_RUNTIME_PERMISSION_UTIL_H

#include <string>
#include <vector>

#include "access_token.h"

namespace OHOS {
namespace CliTool {

using namespace OHOS::Security;

class PermissionUtil {
public:
    PermissionUtil() = default;
    ~PermissionUtil() = default;

    static bool VerifyAccessToken(AccessToken::AccessTokenID tokenId,
        const std::vector<std::string> &requirePermissions);
    static bool VerifyAccessToken(AccessToken::AccessTokenID tokenId,
        const std::string &requirePermission);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_PERMISSION_UTIL_H
