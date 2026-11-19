/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "context.h"
#include "ets_context_utils.h"
#include "mock_ani_env.h"

using namespace testing::ext;

namespace OHOS::AbilityRuntime::ContextUtil {
// BindParentProperty is exported from ani_common but not declared in
// ets_context_utils.h; declare the linkage locally for test coverage.
void BindParentProperty(ani_env *aniEnv, ani_class contextClass, ani_object contextObj,
    std::shared_ptr<Context> context);
} // namespace OHOS::AbilityRuntime::ContextUtil

namespace OHOS::AbilityRuntime {
namespace {
// Minimal Context implementation. It only records the accesses the functions
// under test must NOT perform on guarded (null env / null context) paths.
class StubContext : public Context {
public:
    StubContext() = default;
    ~StubContext() override = default;

    mutable int applicationInfoAccess = 0;
    mutable int resourceManagerAccess = 0;
    mutable int processNameAccess = 0;
    int switchAreaCalls = 0;
    int lastAreaMode = 0;

    std::string GetBundleName() const override
    {
        return "";
    }

    std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName) override
    {
        return nullptr;
    }

    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override
    {
        ++applicationInfoAccess;
        return nullptr;
    }

    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override
    {
        ++resourceManagerAccess;
        return nullptr;
    }

    std::string GetBundleCodePath() const override
    {
        return "";
    }

    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override
    {
        return nullptr;
    }

    std::string GetBundleCodeDir() override
    {
        return "";
    }

    std::string GetCacheDir() override
    {
        return "";
    }

    std::string GetTempDir() override
    {
        return "";
    }

    std::string GetFilesDir() override
    {
        return "";
    }

    std::string GetResourceDir(const std::string &moduleName) override
    {
        return "";
    }

    bool IsUpdatingConfigurations() override
    {
        return false;
    }

    bool PrintDrawnCompleted() override
    {
        return false;
    }

    std::string GetDatabaseDir() override
    {
        return "";
    }

    int32_t GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir) override
    {
        return ERR_INVALID_VALUE;
    }

    std::string GetPreferencesDir() override
    {
        return "";
    }

    int32_t GetSystemPreferencesDir(const std::string &groupId, bool checkExist, std::string &preferencesDir) override
    {
        return ERR_INVALID_VALUE;
    }

    std::string GetGroupDir(std::string groupId) override
    {
        return "";
    }

    std::string GetDistributedFilesDir() override
    {
        return "";
    }

    std::string GetCloudFileDir() override
    {
        return "";
    }

    std::string GetLogFileDir() override
    {
        return "";
    }

    sptr<IRemoteObject> GetToken() override
    {
        return nullptr;
    }

    void SetToken(const sptr<IRemoteObject> &token) override {}

    void SwitchArea(int mode) override
    {
        ++switchAreaCalls;
        lastAreaMode = mode;
    }

    std::shared_ptr<Context> CreateModuleContext(const std::string &moduleName) override
    {
        return nullptr;
    }

    std::shared_ptr<Context> CreateModuleContext(const std::string &bundleName, const std::string &moduleName) override
    {
        return nullptr;
    }

    std::shared_ptr<Global::Resource::ResourceManager> CreateModuleResourceManager(
        const std::string &bundleName, const std::string &moduleName) override
    {
        return nullptr;
    }

    int32_t CreateSystemHspModuleResourceManager(const std::string &bundleName, const std::string &moduleName,
        std::shared_ptr<Global::Resource::ResourceManager> &resourceManager) override
    {
        return ERR_INVALID_VALUE;
    }

    int GetArea() override
    {
        return 0;
    }

    std::string GetProcessName() override
    {
        ++processNameAccess;
        return "";
    }

    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() const override
    {
        return nullptr;
    }

    std::string GetBaseDir() const override
    {
        return "";
    }

    Global::Resource::DeviceType GetDeviceType() const override
    {
        return static_cast<Global::Resource::DeviceType>(0);
    }

    std::shared_ptr<Context> CreateAreaModeContext(int areaMode) override
    {
        return nullptr;
    }

    std::shared_ptr<Context> CreateModuleOrPluginContext(
        const std::string &bundleName, const std::string &moduleName) override
    {
        return nullptr;
    }

    std::shared_ptr<Context> CreateDisplayContext(uint64_t displayId) override
    {
        return nullptr;
    }
};
} // namespace

class EtsContextUtilsTest : public testing::Test {
protected:
    void SetUp() override
    {
        context_ = std::make_shared<StubContext>();
        weakContext_ = context_;
        contextClass_ = reinterpret_cast<ani_class>(1);
        contextObj_ = env_.NewObject();
        // GetBaseContext reads "nativeContext" as a std::weak_ptr<Context>* stored in the object.
        env_.SetFieldLong(contextObj_, "nativeContext", reinterpret_cast<ani_long>(&weakContext_));
    }

    MockAniEnv env_;
    std::shared_ptr<StubContext> context_;
    std::weak_ptr<Context> weakContext_;
    ani_class contextClass_ = nullptr;
    ani_object contextObj_ = nullptr;
};

/**
 * @tc.name: BindApplicationInfoNullAniEnv0100
 * @tc.desc: BindApplicationInfo returns early when aniEnv is null and must not touch the context.
 */
HWTEST_F(EtsContextUtilsTest, BindApplicationInfoNullAniEnv0100, TestSize.Level1)
{
    ContextUtil::BindApplicationInfo(nullptr, contextClass_, contextObj_, context_);
    // The guard must return before GetApplicationInfo is called.
    EXPECT_EQ(context_->applicationInfoAccess, 0);
}

/**
 * @tc.name: BindApplicationInfoNullContext0200
 * @tc.desc: BindApplicationInfo returns early when context is null without any ANI call.
 */
HWTEST_F(EtsContextUtilsTest, BindApplicationInfoNullContext0200, TestSize.Level1)
{
    ContextUtil::BindApplicationInfo(&env_, contextClass_, contextObj_, nullptr);
    // The guard must return before the first ANI interaction (Class_FindField).
    EXPECT_EQ(env_.classFindFieldCalls, 0);
}

/**
 * @tc.name: BindResourceManagerNullAniEnv0300
 * @tc.desc: BindResourceManager returns early when aniEnv is null and must not touch the context.
 */
HWTEST_F(EtsContextUtilsTest, BindResourceManagerNullAniEnv0300, TestSize.Level1)
{
    ContextUtil::BindResourceManager(nullptr, contextClass_, contextObj_, context_);
    EXPECT_EQ(context_->resourceManagerAccess, 0);
}

/**
 * @tc.name: BindResourceManagerNullContext0400
 * @tc.desc: BindResourceManager returns early when context is null without any ANI call.
 */
HWTEST_F(EtsContextUtilsTest, BindResourceManagerNullContext0400, TestSize.Level1)
{
    ContextUtil::BindResourceManager(&env_, contextClass_, contextObj_, nullptr);
    EXPECT_EQ(env_.classFindFieldCalls, 0);
}

/**
 * @tc.name: BindParentPropertyNullAniEnv0500
 * @tc.desc: BindParentProperty returns early when aniEnv is null and must not touch the context.
 */
HWTEST_F(EtsContextUtilsTest, BindParentPropertyNullAniEnv0500, TestSize.Level1)
{
    ContextUtil::BindParentProperty(nullptr, contextClass_, contextObj_, context_);
    EXPECT_EQ(context_->applicationInfoAccess, 0);
    EXPECT_EQ(context_->resourceManagerAccess, 0);
    EXPECT_EQ(context_->processNameAccess, 0);
}

/**
 * @tc.name: BindParentPropertyNullContext0600
 * @tc.desc: BindParentProperty returns early when context is null without any ANI call.
 */
HWTEST_F(EtsContextUtilsTest, BindParentPropertyNullContext0600, TestSize.Level1)
{
    ContextUtil::BindParentProperty(&env_, contextClass_, contextObj_, nullptr);
    EXPECT_EQ(env_.classFindFieldCalls, 0);
    EXPECT_EQ(env_.stringNewCalls, 0);
}

/**
 * @tc.name: SwitchAreaNullAniEnv0700
 * @tc.desc: SwitchArea returns early when env is null and must not switch area on the context.
 */
HWTEST_F(EtsContextUtilsTest, SwitchAreaNullAniEnv0700, TestSize.Level1)
{
    auto areaModeItem = env_.NewEnumItem(1);
    ContextUtil::SwitchArea(nullptr, contextObj_, areaModeItem);
    // The guard must return before the enum is parsed and the context is switched.
    EXPECT_EQ(context_->switchAreaCalls, 0);
}

/**
 * @tc.name: SwitchAreaNullNativeContext0800
 * @tc.desc: SwitchArea returns early when the object has no native context bound.
 */
HWTEST_F(EtsContextUtilsTest, SwitchAreaNullNativeContext0800, TestSize.Level1)
{
    auto areaModeItem = env_.NewEnumItem(1);
    auto emptyObj = env_.NewObject(); // no "nativeContext" field set
    ContextUtil::SwitchArea(&env_, emptyObj, areaModeItem);
    EXPECT_EQ(env_.enumItemGetValueCalls, 1); // enum is parsed before the context lookup
    EXPECT_EQ(context_->switchAreaCalls, 0);
}

/**
 * @tc.name: SwitchAreaValidParams0900
 * @tc.desc: SwitchArea switches the area on the context and rebinds all context directories.
 */
HWTEST_F(EtsContextUtilsTest, SwitchAreaValidParams0900, TestSize.Level1)
{
    constexpr ani_int EXPECTED_AREA = 1;
    auto areaModeItem = env_.NewEnumItem(EXPECTED_AREA);
    ContextUtil::SwitchArea(&env_, contextObj_, areaModeItem);
    EXPECT_EQ(context_->switchAreaCalls, 1);
    EXPECT_EQ(context_->lastAreaMode, EXPECTED_AREA);
    EXPECT_EQ(env_.enumItemGetValueCalls, 1);
    // BindContextDir rebinds 9 directories: preferences/database/cache/files/temp
    // plus cloudFile/distributedFiles/bundleCode/resource in BindContextDirInner.
    EXPECT_EQ(env_.objectSetFieldByNameRefCalls, 9);
}
} // namespace OHOS::AbilityRuntime
