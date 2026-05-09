/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "native_lib_util.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class NativeLibUtilTest : public testing::Test {
public:
    NativeLibUtilTest()
    {}
    ~NativeLibUtilTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void NativeLibUtilTest::SetUpTestCase(void)
{}

void NativeLibUtilTest::TearDownTestCase(void)
{}

void NativeLibUtilTest::SetUp(void)
{}

void NativeLibUtilTest::TearDown(void)
{}

/**
 * @tc.number: GetLibPath_0100
 * @tc.name: GetLibPath
 * @tc.desc: Test whether GetLibPath and are called normally.
 */
HWTEST_F(NativeLibUtilTest, GetLibPath_0100, TestSize.Level1)
{
    std::string hapPath = "/data/test/NativeLibUtilTest.hap";
    bool isPreInstallApp = true;
    std::string libpath = AppExecFwk::GetLibPath(hapPath, isPreInstallApp);
    EXPECT_NE(libpath, "");
}

/**
 * @tc.number: GetHapSoPath_0200
 * @tc.name: GetHapSoPath
 * @tc.desc: Test whether GetHapSoPath and are called normally.
 */
HWTEST_F(NativeLibUtilTest, GetHapSoPath_0200, TestSize.Level1)
{
    AppExecFwk::HapModuleInfo hapInfo;
    AppLibPathMap appLibPaths;
    bool isPreInstallApp = true;
    hapInfo.hapPath = "/data/test/NativeLibUtilTest.hap";
    hapInfo.nativeLibraryPath = "";
    AppExecFwk::GetHapSoPath(hapInfo, appLibPaths, isPreInstallApp);
    EXPECT_EQ(appLibPaths.empty(), true);

    hapInfo.nativeLibraryPath = "/data/test/nativeLibraryPath";
    hapInfo.compressNativeLibs = false;
    AppExecFwk::GetHapSoPath(hapInfo, appLibPaths, isPreInstallApp);
    EXPECT_EQ(appLibPaths.empty(), false);
}

/**
 * @tc.number: GetHspNativeLibPath_0300
 * @tc.name: GetHspNativeLibPath
 * @tc.desc: Test whether GetHspNativeLibPath and are called normally.
 */
HWTEST_F(NativeLibUtilTest, GetHspNativeLibPath_0300, TestSize.Level1)
{
    AppExecFwk::BaseSharedBundleInfo hspInfo;
    AppLibPathMap appLibPaths;
    bool isPreInstallApp = true;
    hspInfo.nativeLibraryPath = "";
    AppExecFwk::GetHspNativeLibPath(hspInfo, appLibPaths, isPreInstallApp);
    EXPECT_EQ(appLibPaths.empty(), true);

    hspInfo.nativeLibraryPath = "/data/test/nativeLibraryPath";
    hspInfo.bundleName = "nativeLibraryTest";
    hspInfo.moduleName = "library";

    hspInfo.compressNativeLibs = false;
    AppExecFwk::GetHspNativeLibPath(hspInfo, appLibPaths, isPreInstallApp);
    EXPECT_EQ(appLibPaths.empty(), false);

    appLibPaths.clear();
    isPreInstallApp = false;
    AppExecFwk::GetHspNativeLibPath(hspInfo, appLibPaths, isPreInstallApp);
    EXPECT_EQ(appLibPaths.empty(), false);

    appLibPaths.clear();
    hspInfo.compressNativeLibs = true;
    AppExecFwk::GetHspNativeLibPath(hspInfo, appLibPaths, isPreInstallApp);
    EXPECT_EQ(appLibPaths.empty(), false);
}

/**
 * @tc.number: GetPatchNativeLibPath_0400
 * @tc.name: GetPatchNativeLibPath
 * @tc.desc: Test whether GetPatchNativeLibPath and are called normally.
 */
HWTEST_F(NativeLibUtilTest, GetPatchNativeLibPath_0400, TestSize.Level1)
{
    AppExecFwk::HapModuleInfo hapInfo;
    std::string patchNativeLibraryPath = "";
    AppLibPathMap appLibPaths;

    hapInfo.hapPath = "/data/test/NativeLibUtilTest.hap";
    hapInfo.isLibIsolated = false;
    AppExecFwk::GetPatchNativeLibPath(hapInfo, patchNativeLibraryPath, appLibPaths);
    EXPECT_EQ(patchNativeLibraryPath, "");

    hapInfo.compressNativeLibs = true;
    patchNativeLibraryPath = "/data/test/patchNativeLibraryPath";
    AppExecFwk::GetPatchNativeLibPath(hapInfo, patchNativeLibraryPath, appLibPaths);
    EXPECT_EQ(appLibPaths.empty(), true);

    hapInfo.isLibIsolated = true;
    hapInfo.compressNativeLibs = false;
    hapInfo.bundleName = "nativeLibraryTest";
    hapInfo.moduleName = "library";
    hapInfo.hqfInfo.nativeLibraryPath = "/data/test/nativeLibraryPath";
    AppExecFwk::GetPatchNativeLibPath(hapInfo, patchNativeLibraryPath, appLibPaths);
    EXPECT_EQ(appLibPaths.empty(), false);
}

/**
 * @tc.number: GetLibrarySupportDirectory_0100
 * @tc.name: GetLibrarySupportDirectory
 * @tc.desc: Test with empty hapModuleInfos, no paths should be added.
 */
HWTEST_F(NativeLibUtilTest, GetLibrarySupportDirectory_0100, TestSize.Level1)
{
    std::vector<AppExecFwk::HapModuleInfo> hapModuleInfos;
    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    AppExecFwk::GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);
    EXPECT_TRUE(appLibPaths.empty());
}

/**
 * @tc.number: GetLibrarySupportDirectory_0200
 * @tc.name: GetLibrarySupportDirectory
 * @tc.desc: Test with hapModuleInfo that has empty librarySupportDirectory, no paths should be added.
 */
HWTEST_F(NativeLibUtilTest, GetLibrarySupportDirectory_0200, TestSize.Level1)
{
    std::vector<AppExecFwk::HapModuleInfo> hapModuleInfos;
    AppExecFwk::HapModuleInfo hapInfo;
    hapInfo.librarySupportDirectory = {};
    hapModuleInfos.push_back(hapInfo);
    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    AppExecFwk::GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);
    EXPECT_TRUE(appLibPaths.empty());
}

/**
 * @tc.number: GetLibrarySupportDirectory_0300
 * @tc.name: GetLibrarySupportDirectory
 * @tc.desc: Test with a single hapModuleInfo that has one librarySupportDirectory entry,
 *           verify exact path value.
 */
HWTEST_F(NativeLibUtilTest, GetLibrarySupportDirectory_0300, TestSize.Level1)
{
    std::vector<AppExecFwk::HapModuleInfo> hapModuleInfos;
    AppExecFwk::HapModuleInfo hapInfo;
    hapInfo.librarySupportDirectory = {"subdir1"};
    hapModuleInfos.push_back(hapInfo);
    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    AppExecFwk::GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);

    ASSERT_EQ(appLibPaths.count("default"), 1u);
    ASSERT_EQ(appLibPaths["default"].size(), 1u);
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/libs/arm/subdir1");
}

/**
 * @tc.number: GetLibrarySupportDirectory_0400
 * @tc.name: GetLibrarySupportDirectory
 * @tc.desc: Test with a single hapModuleInfo that has multiple librarySupportDirectory entries,
 *           verify exact path values.
 */
HWTEST_F(NativeLibUtilTest, GetLibrarySupportDirectory_0400, TestSize.Level1)
{
    std::vector<AppExecFwk::HapModuleInfo> hapModuleInfos;
    AppExecFwk::HapModuleInfo hapInfo;
    hapInfo.librarySupportDirectory = {"subdir1", "subdir2", "subdir3"};
    hapModuleInfos.push_back(hapInfo);
    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    AppExecFwk::GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);

    ASSERT_EQ(appLibPaths.count("default"), 1u);
    ASSERT_EQ(appLibPaths["default"].size(), 3u);
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/libs/arm/subdir1");
    EXPECT_EQ(appLibPaths["default"][1], "/data/storage/el1/bundle/libs/arm/subdir2");
    EXPECT_EQ(appLibPaths["default"][2], "/data/storage/el1/bundle/libs/arm/subdir3");
}

/**
 * @tc.number: GetLibrarySupportDirectory_0500
 * @tc.name: GetLibrarySupportDirectory
 * @tc.desc: Test with multiple hapModuleInfos. Only those with non-empty librarySupportDirectory
 *           should contribute paths. Verify exact count and path content.
 */
HWTEST_F(NativeLibUtilTest, GetLibrarySupportDirectory_0500, TestSize.Level1)
{
    std::vector<AppExecFwk::HapModuleInfo> hapModuleInfos;
    AppExecFwk::HapModuleInfo hapInfo1;
    hapInfo1.librarySupportDirectory = {"dirA"};
    hapModuleInfos.push_back(hapInfo1);

    AppExecFwk::HapModuleInfo hapInfo2;
    hapInfo2.librarySupportDirectory = {};
    hapModuleInfos.push_back(hapInfo2);

    AppExecFwk::HapModuleInfo hapInfo3;
    hapInfo3.librarySupportDirectory = {"dirB", "dirC"};
    hapModuleInfos.push_back(hapInfo3);

    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    AppExecFwk::GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);

    ASSERT_EQ(appLibPaths.count("default"), 1u);
    ASSERT_EQ(appLibPaths["default"].size(), 3u);
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/libs/arm/dirA");
    // Second module with support dirs: libPath appends nativeLibraryPath again
    EXPECT_EQ(appLibPaths["default"][1], "/data/storage/el1/bundle/libs/arm/dirB");
    EXPECT_EQ(appLibPaths["default"][2], "/data/storage/el1/bundle/libs/arm/dirC");
}

/**
 * @tc.number: GetLibrarySupportDirectory_0600
 * @tc.name: GetLibrarySupportDirectory
 * @tc.desc: Test that generated path starts with LOCAL_CODE_PATH and ends with support dir name.
 */
HWTEST_F(NativeLibUtilTest, GetLibrarySupportDirectory_0600, TestSize.Level1)
{
    std::vector<AppExecFwk::HapModuleInfo> hapModuleInfos;
    AppExecFwk::HapModuleInfo hapInfo;
    hapInfo.librarySupportDirectory = {"support"};
    hapModuleInfos.push_back(hapInfo);
    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    AppExecFwk::GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);

    ASSERT_EQ(appLibPaths["default"].size(), 1u);
    const std::string &path = appLibPaths["default"][0];
    EXPECT_EQ(path, "/data/storage/el1/bundle/libs/arm/support");
}

/**
 * @tc.number: GetHspNativeLibPath_WithSupportDirectory_0700
 * @tc.name: GetHspNativeLibPath with librarySupportDirectory
 * @tc.desc: Test that GetHspNativeLibPath adds support directory paths under bundle/module key
 *           (not "default") when librarySupportDirectory is set.
 */
HWTEST_F(NativeLibUtilTest, GetHspNativeLibPath_WithSupportDirectory_0700, TestSize.Level1)
{
    AppExecFwk::BaseSharedBundleInfo hspInfo;
    AppLibPathMap appLibPaths;
    bool isPreInstallApp = true;

    hspInfo.bundleName = "com.test.hspbundle";
    hspInfo.moduleName = "hsplib";
    hspInfo.hapPath = "/data/storage/el1/bundle/com.test.hspbundle/hsplib.hsp";
    hspInfo.nativeLibraryPath = "libs/arm";
    hspInfo.compressNativeLibs = false;
    hspInfo.librarySupportDirectory = {"support1", "support2"};

    AppExecFwk::GetHspNativeLibPath(hspInfo, appLibPaths, isPreInstallApp);

    std::string key = "com.test.hspbundle/hsplib";
    ASSERT_EQ(appLibPaths.count(key), 1u);
    ASSERT_EQ(appLibPaths[key].size(), 3u);
    EXPECT_EQ(appLibPaths[key][0], "/data/storage/el1/bundle/com.test.hspbundle/libs/arm");
    EXPECT_EQ(appLibPaths[key][1], "/data/storage/el1/bundle/com.test.hspbundle/libs/arm/support1");
    EXPECT_EQ(appLibPaths[key][2], "/data/storage/el1/bundle/com.test.hspbundle/libs/arm/support2");

    EXPECT_EQ(appLibPaths.count("default"), 0u);
}

/**
 * @tc.number: GetHspNativeLibPath_NoSupportDirectory_0800
 * @tc.name: GetHspNativeLibPath without librarySupportDirectory
 * @tc.desc: Test that GetHspNativeLibPath does not add "default" entry
 *           when librarySupportDirectory is empty.
 */
HWTEST_F(NativeLibUtilTest, GetHspNativeLibPath_NoSupportDirectory_0800, TestSize.Level1)
{
    AppExecFwk::BaseSharedBundleInfo hspInfo;
    AppLibPathMap appLibPaths;
    bool isPreInstallApp = true;

    hspInfo.bundleName = "com.test.hspbundle";
    hspInfo.moduleName = "hsplib";
    hspInfo.hapPath = "/data/storage/el1/bundle/com.test.hspbundle/hsplib.hsp";
    hspInfo.nativeLibraryPath = "libs/arm";
    hspInfo.compressNativeLibs = false;
    hspInfo.librarySupportDirectory = {};

    AppExecFwk::GetHspNativeLibPath(hspInfo, appLibPaths, isPreInstallApp);

    std::string key = "com.test.hspbundle/hsplib";
    ASSERT_EQ(appLibPaths.count(key), 1u);
    ASSERT_EQ(appLibPaths[key].size(), 1u);
    EXPECT_EQ(appLibPaths[key][0], "/data/storage/el1/bundle/com.test.hspbundle/libs/arm");
    EXPECT_EQ(appLibPaths.count("default"), 0u);
}

/**
 * @tc.number: TraverseLibrarySupportDirectory_0100
 * @tc.name: TraverseLibrarySupportDirectory
 * @tc.desc: Test with empty librarySupportDirectory, no paths should be added.
 */
HWTEST_F(NativeLibUtilTest, TraverseLibrarySupportDirectory_0100, TestSize.Level1)
{
    std::vector<std::string> librarySupportDirectory;
    std::string prefixPath = "/data/storage/el1/bundle/libs/arm";
    std::string appLibPathKey = "default";
    AppLibPathMap appLibPaths;
    
    AppExecFwk::TraverseLibrarySupportDirectory(librarySupportDirectory, prefixPath, appLibPathKey, appLibPaths);
    
    EXPECT_TRUE(appLibPaths.empty());
}

/**
 * @tc.number: TraverseLibrarySupportDirectory_0200
 * @tc.name: TraverseLibrarySupportDirectory
 * @tc.desc: Test with single directory entry, verify exact path value.
 */
HWTEST_F(NativeLibUtilTest, TraverseLibrarySupportDirectory_0200, TestSize.Level1)
{
    std::vector<std::string> librarySupportDirectory = {"subdir1"};
    std::string prefixPath = "/data/storage/el1/bundle/libs/arm";
    std::string appLibPathKey = "default";
    AppLibPathMap appLibPaths;
    
    AppExecFwk::TraverseLibrarySupportDirectory(librarySupportDirectory, prefixPath, appLibPathKey, appLibPaths);
    
    ASSERT_EQ(appLibPaths.count("default"), 1u);
    ASSERT_EQ(appLibPaths["default"].size(), 1u);
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/libs/arm/subdir1");
}

/**
 * @tc.number: TraverseLibrarySupportDirectory_0300
 * @tc.name: TraverseLibrarySupportDirectory
 * @tc.desc: Test with multiple directory entries, verify exact path values.
 */
HWTEST_F(NativeLibUtilTest, TraverseLibrarySupportDirectory_0300, TestSize.Level1)
{
    std::vector<std::string> librarySupportDirectory = {"subdir1", "subdir2", "subdir3"};
    std::string prefixPath = "/data/storage/el1/bundle/libs/arm";
    std::string appLibPathKey = "default";
    AppLibPathMap appLibPaths;
    
    AppExecFwk::TraverseLibrarySupportDirectory(librarySupportDirectory, prefixPath, appLibPathKey, appLibPaths);
    
    ASSERT_EQ(appLibPaths.count("default"), 1u);
    ASSERT_EQ(appLibPaths["default"].size(), 3u);
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/libs/arm/subdir1");
    EXPECT_EQ(appLibPaths["default"][1], "/data/storage/el1/bundle/libs/arm/subdir2");
    EXPECT_EQ(appLibPaths["default"][2], "/data/storage/el1/bundle/libs/arm/subdir3");
}

/**
 * @tc.number: TraverseLibrarySupportDirectory_0400
 * @tc.name: TraverseLibrarySupportDirectory
 * @tc.desc: Test with custom appLibPathKey.
 */
HWTEST_F(NativeLibUtilTest, TraverseLibrarySupportDirectory_0400, TestSize.Level1)
{
    std::vector<std::string> librarySupportDirectory = {"support"};
    std::string prefixPath = "/data/storage/el1/bundle/libs/arm";
    std::string appLibPathKey = "com.test.bundle/module";
    AppLibPathMap appLibPaths;
    
    AppExecFwk::TraverseLibrarySupportDirectory(librarySupportDirectory, prefixPath, appLibPathKey, appLibPaths);
    
    ASSERT_EQ(appLibPaths.count("com.test.bundle/module"), 1u);
    ASSERT_EQ(appLibPaths["com.test.bundle/module"].size(), 1u);
    EXPECT_EQ(appLibPaths["com.test.bundle/module"][0], "/data/storage/el1/bundle/libs/arm/support");
}

/**
 * @tc.number: TraverseLibrarySupportDirectory_0500
 * @tc.name: TraverseLibrarySupportDirectory
 * @tc.desc: Test appending to existing appLibPaths entry.
 */
HWTEST_F(NativeLibUtilTest, TraverseLibrarySupportDirectory_0500, TestSize.Level1)
{
    std::vector<std::string> librarySupportDirectory = {"support1", "support2"};
    std::string prefixPath = "/data/storage/el1/bundle/libs/arm";
    std::string appLibPathKey = "default";
    AppLibPathMap appLibPaths;
    appLibPaths["default"].emplace_back("/data/storage/el1/bundle/libs/arm/existing");
    
    AppExecFwk::TraverseLibrarySupportDirectory(librarySupportDirectory, prefixPath, appLibPathKey, appLibPaths);
    
    ASSERT_EQ(appLibPaths.count("default"), 1u);
    ASSERT_EQ(appLibPaths["default"].size(), 3u);
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/libs/arm/existing");
    EXPECT_EQ(appLibPaths["default"][1], "/data/storage/el1/bundle/libs/arm/support1");
    EXPECT_EQ(appLibPaths["default"][2], "/data/storage/el1/bundle/libs/arm/support2");
}

/**
 * @tc.number: GetLibrarySupportDirectory_WithIsLibIsolated_0900
 * @tc.name: GetLibrarySupportDirectory with isLibIsolated
 * @tc.desc: Test that GetLibrarySupportDirectory skips modules with isLibIsolated=true.
 */
HWTEST_F(NativeLibUtilTest, GetLibrarySupportDirectory_WithIsLibIsolated_0900, TestSize.Level1)
{
    std::vector<AppExecFwk::HapModuleInfo> hapModuleInfos;
    AppExecFwk::HapModuleInfo hapInfo;
    hapInfo.librarySupportDirectory = {"subdir1"};
    hapInfo.isLibIsolated = true;
    hapModuleInfos.push_back(hapInfo);
    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    
    AppExecFwk::GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);
    
    EXPECT_TRUE(appLibPaths.empty());
}

/**
 * @tc.number: GetLibrarySupportDirectory_MixedIsLibIsolated_1000
 * @tc.name: GetLibrarySupportDirectory with mixed isLibIsolated
 * @tc.desc: Test GetLibrarySupportDirectory with mix of isLibIsolated values.
 *           Only modules with isLibIsolated=false should contribute paths.
 */
HWTEST_F(NativeLibUtilTest, GetLibrarySupportDirectory_MixedIsLibIsolated_1000, TestSize.Level1)
{
    std::vector<AppExecFwk::HapModuleInfo> hapModuleInfos;
    
    AppExecFwk::HapModuleInfo hapInfo1;
    hapInfo1.librarySupportDirectory = {"dirA"};
    hapInfo1.isLibIsolated = true;
    hapModuleInfos.push_back(hapInfo1);
    
    AppExecFwk::HapModuleInfo hapInfo2;
    hapInfo2.librarySupportDirectory = {"dirB"};
    hapInfo2.isLibIsolated = false;
    hapModuleInfos.push_back(hapInfo2);
    
    AppExecFwk::HapModuleInfo hapInfo3;
    hapInfo3.librarySupportDirectory = {"dirC", "dirD"};
    hapInfo3.isLibIsolated = true;
    hapModuleInfos.push_back(hapInfo3);
    
    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    AppExecFwk::GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);
    
    ASSERT_EQ(appLibPaths.count("default"), 1u);
    ASSERT_EQ(appLibPaths["default"].size(), 1u);
    EXPECT_EQ(appLibPaths["default"][0], "/data/storage/el1/bundle/libs/arm/dirB");
}

/**
 * @tc.number: GetLibrarySupportDirectory_AllIsLibIsolated_1100
 * @tc.name: GetLibrarySupportDirectory with all isLibIsolated
 * @tc.desc: Test GetLibrarySupportDirectory when all modules have isLibIsolated=true.
 *           All modules should be skipped.
 */
HWTEST_F(NativeLibUtilTest, GetLibrarySupportDirectory_AllIsLibIsolated_1100, TestSize.Level1)
{
    std::vector<AppExecFwk::HapModuleInfo> hapModuleInfos;
    
    AppExecFwk::HapModuleInfo hapInfo1;
    hapInfo1.librarySupportDirectory = {"dirA"};
    hapInfo1.isLibIsolated = true;
    hapModuleInfos.push_back(hapInfo1);
    
    AppExecFwk::HapModuleInfo hapInfo2;
    hapInfo2.librarySupportDirectory = {"dirB", "dirC"};
    hapInfo2.isLibIsolated = true;
    hapModuleInfos.push_back(hapInfo2);
    
    std::string nativeLibraryPath = "libs/arm";
    AppLibPathMap appLibPaths;
    AppExecFwk::GetLibrarySupportDirectory(hapModuleInfos, nativeLibraryPath, appLibPaths);
    
    EXPECT_TRUE(appLibPaths.empty());
}

/**
 * @tc.number: GetHapSoPath_WithSupportDirectory_1200
 * @tc.name: GetHapSoPath with librarySupportDirectory
 * @tc.desc: Test that GetHapSoPath adds support directory paths under correct key.
 *           When compressNativeLibs=true, libPath is LOCAL_CODE_PATH based.
 */
HWTEST_F(NativeLibUtilTest, GetHapSoPath_WithSupportDirectory_1200, TestSize.Level1)
{
    AppExecFwk::HapModuleInfo hapInfo;
    AppLibPathMap appLibPaths;
    bool isPreInstallApp = true;
    
    hapInfo.hapPath = "/data/test/NativeLibUtilTest.hap";
    hapInfo.nativeLibraryPath = "libs/arm";
    hapInfo.compressNativeLibs = true;
    hapInfo.bundleName = "com.test.bundle";
    hapInfo.moduleName = "entry";
    hapInfo.librarySupportDirectory = {"support1", "support2"};
    
    AppExecFwk::GetHapSoPath(hapInfo, appLibPaths, isPreInstallApp);
    
    std::string key = "com.test.bundle/entry";
    ASSERT_EQ(appLibPaths.count(key), 1u);
    ASSERT_EQ(appLibPaths[key].size(), 3u);
    EXPECT_EQ(appLibPaths[key][0], "/data/storage/el1/bundle/libs/arm");
    EXPECT_EQ(appLibPaths[key][1], "/data/storage/el1/bundle/libs/arm/support1");
    EXPECT_EQ(appLibPaths[key][2], "/data/storage/el1/bundle/libs/arm/support2");
}

/**
 * @tc.number: GetPatchNativeLibPath_WithSupportDirectory_1300
 * @tc.name: GetPatchNativeLibPath with librarySupportDirectory
 * @tc.desc: Test that GetPatchNativeLibPath adds support directory paths.
 *           When isLibIsolated=true, patchNativeLibraryPath is set from hqfInfo.nativeLibraryPath.
 */
HWTEST_F(NativeLibUtilTest, GetPatchNativeLibPath_WithSupportDirectory_1300, TestSize.Level1)
{
    AppExecFwk::HapModuleInfo hapInfo;
    std::string patchNativeLibraryPath = "/data/test/patch";
    AppLibPathMap appLibPaths;
    
    hapInfo.hapPath = "/data/test/NativeLibUtilTest.hap";
    hapInfo.isLibIsolated = true;
    hapInfo.compressNativeLibs = false;
    hapInfo.bundleName = "com.test.bundle";
    hapInfo.moduleName = "entry";
    hapInfo.hqfInfo.nativeLibraryPath = "libs/arm";
    hapInfo.librarySupportDirectory = {"patch_support1", "patch_support2"};
    
    AppExecFwk::GetPatchNativeLibPath(hapInfo, patchNativeLibraryPath, appLibPaths);
    
    std::string key = "com.test.bundle/entry";
    ASSERT_EQ(appLibPaths.count(key), 1u);
    ASSERT_EQ(appLibPaths[key].size(), 3u);
    EXPECT_EQ(appLibPaths[key][0], "/data/storage/el1/bundle/libs/arm");
    EXPECT_EQ(appLibPaths[key][1], "/data/storage/el1/bundle/libs/arm/patch_support1");
    EXPECT_EQ(appLibPaths[key][2], "/data/storage/el1/bundle/libs/arm/patch_support2");
}
} // namespace AppExecFwk
} // namespace OHOS
