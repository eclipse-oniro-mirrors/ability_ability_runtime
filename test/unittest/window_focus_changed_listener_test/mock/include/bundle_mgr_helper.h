/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_BUNDLE_MGR_HELPER_H
#define MOCK_BUNDLE_MGR_HELPER_H

#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AppExecFwk {
class BundleMgrHelper : public std::enable_shared_from_this<BundleMgrHelper> {
public:
    static std::shared_ptr<AppExecFwk::BundleMgrHelper> GetBundleMgrHelper();

    BundleMgrHelper();

    virtual ~BundleMgrHelper();

    bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId);

    ErrCode GetSignatureInfoByBundleName(const std::string &bundleName, SignatureInfo &signatureInfo);

    void SetMockBundleInfo(const BundleInfo &bundleInfo)
    {
        mockBundleInfo_ = bundleInfo;
    }

    void SetGetBundleInfoResult(bool result)
    {
        getBundleInfoResult_ = result;
    }

    void SetMockSignatureInfo(const SignatureInfo &signatureInfo)
    {
        mockSignatureInfo_ = signatureInfo;
    }

    void SetGetSignatureInfoResult(ErrCode result)
    {
        getSignatureInfoResult_ = result;
    }

    sptr<IAppControlMgr> GetAppControlProxy()
    {
        return nullptr;
    }

    ErrCode GetNameForUid(const int32_t, std::string &name)
    {
        return 0;
    }

    bool GetApplicationInfo(const std::string &appName, const ApplicationFlag flag, const int32_t, ApplicationInfo &)
    {
        return true;
    }

    bool GetApplicationInfo(const std::string &, int32_t, int32_t, ApplicationInfo &)
    {
        return true;
    }

    bool QueryAppGalleryBundleName(std::string &)
    {
        return true;
    }

private:
    BundleInfo mockBundleInfo_;
    SignatureInfo mockSignatureInfo_;
    bool getBundleInfoResult_ = true;
    ErrCode getSignatureInfoResult_ = ERR_OK;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // MOCK_BUNDLE_MGR_HELPER_H
