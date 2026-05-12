/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "bundle_mgr_helper.h"

namespace OHOS {
namespace AppExecFwk {
ErrCode BundleMgrHelper::getBundleInfoResult = ERR_OK;
ErrCode BundleMgrHelper::getCloneBundleInfoResult = ERR_OK;
int32_t BundleMgrHelper::gid = 0;
std::string BundleMgrHelper::appId;
std::string BundleMgrHelper::bundleName;

BundleMgrHelper::BundleMgrHelper() = default;

BundleMgrHelper::~BundleMgrHelper() = default;

ErrCode BundleMgrHelper::GetBundleInfoV9(
    const std::string &, int32_t, BundleInfo &bundleInfoResult, int32_t)
{
    bundleInfoResult.gid = gid;
    bundleInfoResult.appId = appId;
    bundleInfoResult.name = bundleName;
    return getBundleInfoResult;
}

ErrCode BundleMgrHelper::GetCloneBundleInfo(
    const std::string &, int32_t, int32_t, BundleInfo &bundleInfoResult, int32_t)
{
    bundleInfoResult.gid = gid;
    bundleInfoResult.appId = appId;
    bundleInfoResult.name = bundleName;
    return getCloneBundleInfoResult;
}

void BundleMgrHelper::Reset()
{
    getBundleInfoResult = ERR_OK;
    getCloneBundleInfoResult = ERR_OK;
    gid = 0;
    appId.clear();
    bundleName.clear();
}
} // namespace AppExecFwk
} // namespace OHOS
