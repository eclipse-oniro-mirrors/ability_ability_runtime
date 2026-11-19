/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_CCM_UTIL_H
#define OHOS_ABILITY_RUNTIME_CCM_UTIL_H

#include <cstdint>

namespace OHOS {
namespace CliTool {
class CcmUtil {
public:
    static CcmUtil &GetInstance();
    int32_t GetCliConcurrencyLimit();
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CCM_UTIL_H
