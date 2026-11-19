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

#include "ets_native_lib_util.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class EtsNativeLibUtilTest : public testing::Test {
public:
    EtsNativeLibUtilTest()
    {}
    ~EtsNativeLibUtilTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void EtsNativeLibUtilTest::SetUpTestCase(void)
{}

void EtsNativeLibUtilTest::TearDownTestCase(void)
{}

void EtsNativeLibUtilTest::SetUp(void)
{}

void EtsNativeLibUtilTest::TearDown(void)
{}

/**
 * @tc.number: EtsGetLibrarySupportDirectory_0100
 * @tc.name: GetLibrarySupportDirectory for ets
 * @tc.desc: Test with empty hapModuleInfos, no paths should be added.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsGetLibrarySupportDirectory_0100, TestSize.Level1)
{
    std::vector<HapModuleInfo> hapModuleInfos;
    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);
    EXPECT_TRUE(appLibPaths.empty());
}

/**
 * @tc.number: EtsGetLibrarySupportDirectory_0200
 * @tc.name: GetLibrarySupportDirectory for ets
 * @tc.desc: Test with hapModuleInfo that has empty librarySupportDirectory, no paths should be added.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsGetLibrarySupportDirectory_0200, TestSize.Level1)
{
    std::vector<HapModuleInfo> hapModuleInfos;
    HapModuleInfo hapInfo;
    hapInfo.librarySupportDirectory = {};
    hapModuleInfos.push_back(hapInfo);
    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);
    EXPECT_TRUE(appLibPaths.empty());
}

/**
 * @tc.number: EtsGetLibrarySupportDirectory_0300
 * @tc.name: GetLibrarySupportDirectory for ets
 * @tc.desc: Test with a single hapModuleInfo that has one librarySupportDirectory entry,
 *           verify exact path value.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsGetLibrarySupportDirectory_0300, TestSize.Level1)
{
    std::vector<HapModuleInfo> hapModuleInfos;
    HapModuleInfo hapInfo;
    hapInfo.librarySupportDirectory = {"subdir1"};
    hapModuleInfos.push_back(hapInfo);
    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);

    ASSERT_EQ(appLibPaths.count("default"), 1u);
    ASSERT_EQ(appLibPaths["default"].size(), 1u);
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/libs/arm/subdir1");
}

/**
 * @tc.number: EtsGetLibrarySupportDirectory_0400
 * @tc.name: GetLibrarySupportDirectory for ets
 * @tc.desc: Test with a single hapModuleInfo that has multiple librarySupportDirectory entries,
 *           verify exact path values.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsGetLibrarySupportDirectory_0400, TestSize.Level1)
{
    std::vector<HapModuleInfo> hapModuleInfos;
    HapModuleInfo hapInfo;
    hapInfo.librarySupportDirectory = {"subdir1", "subdir2", "subdir3"};
    hapModuleInfos.push_back(hapInfo);
    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);

    ASSERT_EQ(appLibPaths.count("default"), 1u);
    ASSERT_EQ(appLibPaths["default"].size(), 3u);
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/libs/arm/subdir1");
    EXPECT_EQ(appLibPaths["default"][1], "/data/storage/el1/bundle/libs/arm/subdir2");
    EXPECT_EQ(appLibPaths["default"][2], "/data/storage/el1/bundle/libs/arm/subdir3");
}

/**
 * @tc.number: EtsGetLibrarySupportDirectory_0500
 * @tc.name: GetLibrarySupportDirectory for ets
 * @tc.desc: Test with multiple hapModuleInfos. Only those with non-empty librarySupportDirectory
 *           should contribute paths. Verify exact count and path content.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsGetLibrarySupportDirectory_0500, TestSize.Level1)
{
    std::vector<HapModuleInfo> hapModuleInfos;
    HapModuleInfo hapInfo1;
    hapInfo1.librarySupportDirectory = {"dirA"};
    hapModuleInfos.push_back(hapInfo1);

    HapModuleInfo hapInfo2;
    hapInfo2.librarySupportDirectory = {};
    hapModuleInfos.push_back(hapInfo2);

    HapModuleInfo hapInfo3;
    hapInfo3.librarySupportDirectory = {"dirB", "dirC"};
    hapModuleInfos.push_back(hapInfo3);

    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);

    ASSERT_EQ(appLibPaths.count("default"), 1u);
    ASSERT_EQ(appLibPaths["default"].size(), 3u);
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/libs/arm/dirA");
    EXPECT_EQ(appLibPaths["default"][1], "/data/storage/el1/bundle/libs/arm/dirB");
    EXPECT_EQ(appLibPaths["default"][2], "/data/storage/el1/bundle/libs/arm/dirC");
}

/**
 * @tc.number: EtsGetLibrarySupportDirectory_0600
 * @tc.name: GetLibrarySupportDirectory for ets
 * @tc.desc: Test that generated path equals expected full path.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsGetLibrarySupportDirectory_0600, TestSize.Level1)
{
    std::vector<HapModuleInfo> hapModuleInfos;
    HapModuleInfo hapInfo;
    hapInfo.librarySupportDirectory = {"support"};
    hapModuleInfos.push_back(hapInfo);
    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);

    ASSERT_EQ(appLibPaths["default"].size(), 1u);
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/libs/arm/support");
}

/**
 * @tc.number: EtsGetHspNativeLibPath_WithSupportDirectory_0700
 * @tc.name: GetEtsHspNativeLibPath with librarySupportDirectory
 * @tc.desc: Test that GetEtsHspNativeLibPath adds support directory paths under bundle/module key
 *           (not "default") when librarySupportDirectory is set.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsGetHspNativeLibPath_WithSupportDirectory_0700, TestSize.Level1)
{
    BaseSharedBundleInfo hspInfo;
    AppLibPathMap appLibPaths;
    bool isPreInstallApp = true;
    std::string appBundleName = "com.test.app";
    std::map<std::string, std::string> abcPathsToBundleModuleNameMap;

    hspInfo.bundleName = "com.test.hspbundle";
    hspInfo.moduleName = "hsplib";
    hspInfo.hapPath = "/data/storage/el1/bundle/com.test.hspbundle/hsplib.hsp";
    hspInfo.nativeLibraryPath = "libs/arm";
    hspInfo.compressNativeLibs = false;
    hspInfo.librarySupportDirectory = {"support1", "support2"};

    GetEtsHspNativeLibPath(hspInfo, appLibPaths, isPreInstallApp, appBundleName, abcPathsToBundleModuleNameMap);

    std::string key = "com.test.hspbundle/hsplib";
    ASSERT_EQ(appLibPaths.count(key), 1u);
    ASSERT_EQ(appLibPaths[key].size(), 3u);
    EXPECT_EQ(appLibPaths[key][0], "/data/storage/el1/bundle/com.test.hspbundle/libs/arm");
    EXPECT_EQ(appLibPaths[key][1], "/data/storage/el1/bundle/com.test.hspbundle/libs/arm/support1");
    EXPECT_EQ(appLibPaths[key][2], "/data/storage/el1/bundle/com.test.hspbundle/libs/arm/support2");

    EXPECT_EQ(appLibPaths.count("default"), 0u);
}

/**
 * @tc.number: EtsGetHspNativeLibPath_NoSupportDirectory_0800
 * @tc.name: GetEtsHspNativeLibPath without librarySupportDirectory
 * @tc.desc: Test that GetEtsHspNativeLibPath does not add "default" entry
 *           when librarySupportDirectory is empty.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsGetHspNativeLibPath_NoSupportDirectory_0800, TestSize.Level1)
{
    BaseSharedBundleInfo hspInfo;
    AppLibPathMap appLibPaths;
    bool isPreInstallApp = true;
    std::string appBundleName = "com.test.app";
    std::map<std::string, std::string> abcPathsToBundleModuleNameMap;

    hspInfo.bundleName = "com.test.hspbundle";
    hspInfo.moduleName = "hsplib";
    hspInfo.hapPath = "/data/storage/el1/bundle/com.test.hspbundle/hsplib.hsp";
    hspInfo.nativeLibraryPath = "libs/arm";
    hspInfo.compressNativeLibs = false;
    hspInfo.librarySupportDirectory = {};

    GetEtsHspNativeLibPath(hspInfo, appLibPaths, isPreInstallApp, appBundleName, abcPathsToBundleModuleNameMap);

    std::string key = "com.test.hspbundle/hsplib";
    ASSERT_EQ(appLibPaths.count(key), 1u);
    ASSERT_EQ(appLibPaths[key].size(), 1u);
    EXPECT_EQ(appLibPaths[key][0], "/data/storage/el1/bundle/com.test.hspbundle/libs/arm");
    EXPECT_EQ(appLibPaths.count("default"), 0u);
}

/**
 * @tc.number: EtsTraverseLibrarySupportDirectory_0100
 * @tc.name: TraverseLibrarySupportDirectory for ets
 * @tc.desc: Test with empty librarySupportDirectory, no paths should be added.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsTraverseLibrarySupportDirectory_0100, TestSize.Level1)
{
    std::vector<std::string> librarySupportDirectory;
    std::string prefixPath = "/data/storage/el1/bundle/libs/arm";
    std::string appLibPathKey = "default";
    AppLibPathMap appLibPaths;
    
    TraverseLibrarySupportDirectory(librarySupportDirectory, prefixPath, appLibPathKey, appLibPaths);
    
    EXPECT_TRUE(appLibPaths.empty());
}

/**
 * @tc.number: EtsTraverseLibrarySupportDirectory_0200
 * @tc.name: TraverseLibrarySupportDirectory for ets
 * @tc.desc: Test with single directory entry, verify exact path value.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsTraverseLibrarySupportDirectory_0200, TestSize.Level1)
{
    std::vector<std::string> librarySupportDirectory = {"subdir1"};
    std::string prefixPath = "/data/storage/el1/bundle/libs/arm";
    std::string appLibPathKey = "default";
    AppLibPathMap appLibPaths;
    
    TraverseLibrarySupportDirectory(librarySupportDirectory, prefixPath, appLibPathKey, appLibPaths);
    
    ASSERT_EQ(appLibPaths.count("default"), 1u);
    ASSERT_EQ(appLibPaths["default"].size(), 1u);
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/libs/arm/subdir1");
}

/**
 * @tc.number: EtsTraverseLibrarySupportDirectory_0300
 * @tc.name: TraverseLibrarySupportDirectory for ets
 * @tc.desc: Test with multiple directory entries, verify exact path values.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsTraverseLibrarySupportDirectory_0300, TestSize.Level1)
{
    std::vector<std::string> librarySupportDirectory = {"subdir1", "subdir2", "subdir3"};
    std::string prefixPath = "/data/storage/el1/bundle/libs/arm";
    std::string appLibPathKey = "default";
    AppLibPathMap appLibPaths;
    
    TraverseLibrarySupportDirectory(librarySupportDirectory, prefixPath, appLibPathKey, appLibPaths);
    
    ASSERT_EQ(appLibPaths.count("default"), 1u);
    ASSERT_EQ(appLibPaths["default"].size(), 3u);
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/libs/arm/subdir1");
    EXPECT_EQ(appLibPaths["default"][1], "/data/storage/el1/bundle/libs/arm/subdir2");
    EXPECT_EQ(appLibPaths["default"][2], "/data/storage/el1/bundle/libs/arm/subdir3");
}

/**
 * @tc.number: EtsTraverseLibrarySupportDirectory_0400
 * @tc.name: TraverseLibrarySupportDirectory for ets
 * @tc.desc: Test with custom appLibPathKey.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsTraverseLibrarySupportDirectory_0400, TestSize.Level1)
{
    std::vector<std::string> librarySupportDirectory = {"support"};
    std::string prefixPath = "/data/storage/el1/bundle/libs/arm";
    std::string appLibPathKey = "com.test.bundle/module";
    AppLibPathMap appLibPaths;
    
    TraverseLibrarySupportDirectory(librarySupportDirectory, prefixPath, appLibPathKey, appLibPaths);
    
    ASSERT_EQ(appLibPaths.count("com.test.bundle/module"), 1u);
    ASSERT_EQ(appLibPaths["com.test.bundle/module"].size(), 1u);
    EXPECT_EQ(appLibPaths["com.test.bundle/module"][0], "/data/storage/el1/bundle/libs/arm/support");
}

/**
 * @tc.number: EtsTraverseLibrarySupportDirectory_0500
 * @tc.name: TraverseLibrarySupportDirectory for ets
 * @tc.desc: Test appending to existing appLibPaths entry.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsTraverseLibrarySupportDirectory_0500, TestSize.Level1)
{
    std::vector<std::string> librarySupportDirectory = {"support1", "support2"};
    std::string prefixPath = "/data/storage/el1/bundle/libs/arm";
    std::string appLibPathKey = "default";
    AppLibPathMap appLibPaths;
    appLibPaths["default"].emplace_back("/data/storage/el1/bundle/libs/arm/existing");
    
    TraverseLibrarySupportDirectory(librarySupportDirectory, prefixPath, appLibPathKey, appLibPaths);
    
    ASSERT_EQ(appLibPaths.count("default"), 1u);
    ASSERT_EQ(appLibPaths["default"].size(), 3u);
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/libs/arm/existing");
    EXPECT_EQ(appLibPaths["default"][1], "/data/storage/el1/bundle/libs/arm/support1");
    EXPECT_EQ(appLibPaths["default"][2], "/data/storage/el1/bundle/libs/arm/support2");
}

/**
 * @tc.number: EtsGetLibrarySupportDirectory_WithIsLibIsolated_0900
 * @tc.name: GetLibrarySupportDirectory for ets with isLibIsolated
 * @tc.desc: Test that GetLibrarySupportDirectory skips modules with isLibIsolated=true.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsGetLibrarySupportDirectory_WithIsLibIsolated_0900, TestSize.Level1)
{
    std::vector<HapModuleInfo> hapModuleInfos;
    HapModuleInfo hapInfo;
    hapInfo.librarySupportDirectory = {"subdir1"};
    hapInfo.isLibIsolated = true;
    hapModuleInfos.push_back(hapInfo);
    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    
    GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);
    
    EXPECT_TRUE(appLibPaths.empty());
}

/**
 * @tc.number: EtsGetLibrarySupportDirectory_MixedIsLibIsolated_1000
 * @tc.name: GetLibrarySupportDirectory for ets with mixed isLibIsolated
 * @tc.desc: Test GetLibrarySupportDirectory with mix of isLibIsolated values.
 *           Only modules with isLibIsolated=false should contribute paths.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsGetLibrarySupportDirectory_MixedIsLibIsolated_1000, TestSize.Level1)
{
    std::vector<HapModuleInfo> hapModuleInfos;
    
    HapModuleInfo hapInfo1;
    hapInfo1.librarySupportDirectory = {"dirA"};
    hapInfo1.isLibIsolated = true;
    hapModuleInfos.push_back(hapInfo1);
    
    HapModuleInfo hapInfo2;
    hapInfo2.librarySupportDirectory = {"dirB"};
    hapInfo2.isLibIsolated = false;
    hapModuleInfos.push_back(hapInfo2);
    
    HapModuleInfo hapInfo3;
    hapInfo3.librarySupportDirectory = {"dirC", "dirD"};
    hapInfo3.isLibIsolated = true;
    hapModuleInfos.push_back(hapInfo3);
    
    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);
    
    ASSERT_EQ(appLibPaths.count("default"), 1u);
    ASSERT_EQ(appLibPaths["default"].size(), 1u);
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/libs/arm/dirB");
}

/**
 * @tc.number: EtsGetLibrarySupportDirectory_AllIsLibIsolated_1100
 * @tc.name: GetLibrarySupportDirectory for ets with all isLibIsolated
 * @tc.desc: Test GetLibrarySupportDirectory when all modules have isLibIsolated=true.
 *           All modules should be skipped.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsGetLibrarySupportDirectory_AllIsLibIsolated_1100, TestSize.Level1)
{
    std::vector<HapModuleInfo> hapModuleInfos;
    
    HapModuleInfo hapInfo1;
    hapInfo1.librarySupportDirectory = {"dirA"};
    hapInfo1.isLibIsolated = true;
    hapModuleInfos.push_back(hapInfo1);
    
    HapModuleInfo hapInfo2;
    hapInfo2.librarySupportDirectory = {"dirB", "dirC"};
    hapInfo2.isLibIsolated = true;
    hapModuleInfos.push_back(hapInfo2);
    
    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);
    
    EXPECT_TRUE(appLibPaths.empty());
}

/**
 * @tc.number: EtsGetHapSoPath_WithSupportDirectory_1200
 * @tc.name: GetEtsHapSoPath with librarySupportDirectory
 * @tc.desc: Test that GetEtsHapSoPath adds support directory paths under correct key.
 *           When compressNativeLibs=true, libPath is LOCAL_CODE_PATH based.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsGetHapSoPath_WithSupportDirectory_1200, TestSize.Level1)
{
    HapModuleInfo hapInfo;
    AppLibPathMap appLibPaths;
    bool isPreInstallApp = true;
    std::map<std::string, std::string> abcPathsToBundleModuleNameMap;
    
    hapInfo.hapPath = "/data/test/EtsNativeLibUtilTest.hap";
    hapInfo.nativeLibraryPath = "libs/arm";
    hapInfo.compressNativeLibs = true;
    hapInfo.bundleName = "com.test.bundle";
    hapInfo.moduleName = "entry";
    hapInfo.librarySupportDirectory = {"support1", "support2"};
    
    GetEtsHapSoPath(hapInfo, appLibPaths, isPreInstallApp, abcPathsToBundleModuleNameMap);
    
    std::string key = "com.test.bundle/entry";
    ASSERT_EQ(appLibPaths.count(key), 1u);
    ASSERT_EQ(appLibPaths[key].size(), 3u);
    EXPECT_EQ(appLibPaths[key][0], "/data/storage/el1/bundle/libs/arm");
    EXPECT_EQ(appLibPaths[key][1], "/data/storage/el1/bundle/libs/arm/support1");
    EXPECT_EQ(appLibPaths[key][2], "/data/storage/el1/bundle/libs/arm/support2");
}

/**
 * @tc.number: EtsGetPatchNativeLibPath_WithSupportDirectory_1300
 * @tc.name: GetEtsPatchNativeLibPath with librarySupportDirectory
 * @tc.desc: Test that GetEtsPatchNativeLibPath adds support directory paths.
 *           When isLibIsolated=true, patchNativeLibraryPath is set from hqfInfo.nativeLibraryPath.
 */
HWTEST_F(EtsNativeLibUtilTest, EtsGetPatchNativeLibPath_WithSupportDirectory_1300, TestSize.Level1)
{
    HapModuleInfo hapInfo;
    std::string patchNativeLibraryPath = "/data/test/patch";
    AppLibPathMap appLibPaths;
    std::map<std::string, std::string> abcPathsToBundleModuleNameMap;
    
    hapInfo.hapPath = "/data/test/EtsNativeLibUtilTest.hap";
    hapInfo.isLibIsolated = true;
    hapInfo.compressNativeLibs = false;
    hapInfo.bundleName = "com.test.bundle";
    hapInfo.moduleName = "entry";
    hapInfo.hqfInfo.nativeLibraryPath = "libs/arm";
    hapInfo.librarySupportDirectory = {"patch_support1", "patch_support2"};
    
    GetEtsPatchNativeLibPath(hapInfo, patchNativeLibraryPath, appLibPaths, abcPathsToBundleModuleNameMap);
    
    std::string key = "com.test.bundle/entry";
    ASSERT_EQ(appLibPaths.count(key), 1u);
    ASSERT_EQ(appLibPaths[key].size(), 3u);
    EXPECT_EQ(appLibPaths[key][0], "/data/storage/el1/bundle/libs/arm");
    EXPECT_EQ(appLibPaths[key][1], "/data/storage/el1/bundle/libs/arm/patch_support1");
    EXPECT_EQ(appLibPaths[key][2], "/data/storage/el1/bundle/libs/arm/patch_support2");
}
} // namespace AppExecFwk
} // namespace OHOS
