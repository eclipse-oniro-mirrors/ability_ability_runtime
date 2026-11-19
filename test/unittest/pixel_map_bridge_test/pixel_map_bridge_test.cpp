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

#include "parcel.h"
#include "pixel_map.h"

// Expose PixelMapBridge internals so tests can drive both the success path
// (force a fresh real dlopen) and the degradation path (pretend the library
// failed to load) deterministically, independent of test execution order.
#include "pixel_map_bridge.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t PIXELMAP_WIDTH = 2;
constexpr int32_t PIXELMAP_HEIGHT = 2;

// Builds a minimal RGBA_8888 PixelMap for round-trip testing.
std::unique_ptr<Media::PixelMap> MakePixelMap()
{
    Media::InitializationOptions options;
    options.size.width = PIXELMAP_WIDTH;
    options.size.height = PIXELMAP_HEIGHT;
    options.pixelFormat = Media::PixelFormat::RGBA_8888;
    return std::unique_ptr<Media::PixelMap>(Media::PixelMap::Create(options));
}

// Forces the bridge into a clean state so the next call triggers a real
// dlopen of libimage_native_wrap.z.so.
void ForceReload(PixelMapBridge &bridge)
{
    bridge.loadAttempted_ = false;
    bridge.handle_ = nullptr;
    bridge.readFunc_ = nullptr;
    bridge.writeFunc_ = nullptr;
    bridge.destroyFunc_ = nullptr;
}

// Forces the bridge into the "library unavailable" degradation state:
// LoadLibrary() returns false immediately without touching dlopen.
void ForceDegrade(PixelMapBridge &bridge)
{
    bridge.loadAttempted_ = true;
    bridge.handle_ = nullptr;
    bridge.readFunc_ = nullptr;
    bridge.writeFunc_ = nullptr;
    bridge.destroyFunc_ = nullptr;
}
}  // namespace

class PixelMapBridgeTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void PixelMapBridgeTest::SetUpTestCase(void) {}
void PixelMapBridgeTest::TearDownTestCase(void) {}
void PixelMapBridgeTest::SetUp(void) {}
void PixelMapBridgeTest::TearDown(void) {}

/**
 * @tc.number: PixelMapBridgeTest_LoadLibrary_success_0100
 * @tc.desc: On the system image, dlopen of libimage_native_wrap.z.so succeeds
 *          and the handle becomes non-null.
 * @tc.type: FUNC
 */
HWTEST_F(PixelMapBridgeTest, PixelMapBridgeTest_LoadLibrary_success_0100, TestSize.Level1)
{
    auto &bridge = PixelMapBridge::GetInstance();
    ForceReload(bridge);
    Parcel parcel;
    // Triggers LoadLibrary internally.
    bridge.ReadPixelMapFromParcel(&parcel);
    EXPECT_TRUE(bridge.loadAttempted_);
    EXPECT_NE(bridge.handle_, nullptr);
}

/**
 * @tc.number: PixelMapBridgeTest_RoundTrip_0100
 * @tc.desc: A PixelMap written via the bridge can be read back with matching
 *          width/height.
 * @tc.type: FUNC
 */
HWTEST_F(PixelMapBridgeTest, PixelMapBridgeTest_RoundTrip_0100, TestSize.Level1)
{
    auto &bridge = PixelMapBridge::GetInstance();
    ForceReload(bridge);

    auto pixelMap = MakePixelMap();

    Parcel parcel;
    EXPECT_TRUE(bridge.WritePixelMapToParcel(pixelMap.get(), &parcel));
    // Read back from the same parcel (read cursor is at the start of the data).
    Media::PixelMap *rawPtr = bridge.ReadPixelMapFromParcel(&parcel);
    EXPECT_NE(rawPtr, nullptr);
    EXPECT_EQ(rawPtr->GetWidth(), PIXELMAP_WIDTH);
    EXPECT_EQ(rawPtr->GetHeight(), PIXELMAP_HEIGHT);
    // Ownership released via the bridge to keep alloc/free matched.
    bridge.DestroyPixelMap(rawPtr);
}

/**
 * @tc.number: PixelMapBridgeTest_DestroyPixelMap_0100
 * @tc.desc: Destroying a bridge-produced PixelMap does not leak or crash.
 * @tc.type: FUNC
 */
HWTEST_F(PixelMapBridgeTest, PixelMapBridgeTest_DestroyPixelMap_0100, TestSize.Level1)
{
    auto &bridge = PixelMapBridge::GetInstance();
    ForceReload(bridge);

    Parcel parcel;
    auto pixelMap = MakePixelMap();
    EXPECT_TRUE(bridge.WritePixelMapToParcel(pixelMap.get(), &parcel));

    Media::PixelMap *rawPtr = bridge.ReadPixelMapFromParcel(&parcel);
    EXPECT_NE(rawPtr, nullptr);
    bridge.DestroyPixelMap(rawPtr);  // must not crash; ownership released
    EXPECT_TRUE(bridge.loadAttempted_);
}

/**
 * @tc.number: PixelMapBridgeTest_Degrade_read_0100
 * @tc.desc: When the library fails to load, ReadPixelMapFromParcel degrades to
 *          nullptr without crashing.
 * @tc.type: FUNC
 */
HWTEST_F(PixelMapBridgeTest, PixelMapBridgeTest_Degrade_read_0100, TestSize.Level1)
{
    auto &bridge = PixelMapBridge::GetInstance();
    ForceDegrade(bridge);
    Parcel parcel;
    EXPECT_EQ(bridge.ReadPixelMapFromParcel(&parcel), nullptr);
}

/**
 * @tc.number: PixelMapBridgeTest_Degrade_write_0100
 * @tc.desc: When the library fails to load, WritePixelMapToParcel degrades to
 *          false without crashing.
 * @tc.type: FUNC
 */
HWTEST_F(PixelMapBridgeTest, PixelMapBridgeTest_Degrade_write_0100, TestSize.Level1)
{
    auto &bridge = PixelMapBridge::GetInstance();
    ForceDegrade(bridge);
    auto pixelMap = MakePixelMap();
    Parcel parcel;
    EXPECT_FALSE(bridge.WritePixelMapToParcel(pixelMap.get(), &parcel));
    EXPECT_TRUE(bridge.loadAttempted_);
}

/**
 * @tc.number: PixelMapBridgeTest_Degrade_destroy_0100
 * @tc.desc: When the library fails to load, DestroyPixelMap is a no-op.
 * @tc.type: FUNC
 */
HWTEST_F(PixelMapBridgeTest, PixelMapBridgeTest_Degrade_destroy_0100, TestSize.Level1)
{
    auto &bridge = PixelMapBridge::GetInstance();
    ForceDegrade(bridge);
    auto pixelMap = MakePixelMap();
    // Hand ownership to the bridge; in degrade mode it must not free the memory
    // (LoadLibrary returns early), so keep a raw pointer to delete manually.
    Media::PixelMap *rawPtr = pixelMap.release();
    bridge.DestroyPixelMap(rawPtr);  // no-op in degrade mode
    delete rawPtr;                   // still valid, bridge did not free it
    EXPECT_TRUE(bridge.loadAttempted_);
}

/**
 * @tc.number: PixelMapBridgeTest_NullSafety_write_0100
 * @tc.desc: Writing with a null Parcel returns false without crashing.
 * @tc.type: FUNC
 */
HWTEST_F(PixelMapBridgeTest, PixelMapBridgeTest_NullSafety_write_0100, TestSize.Level1)
{
    auto &bridge = PixelMapBridge::GetInstance();
    ForceReload(bridge);
    auto pixelMap = MakePixelMap();
    EXPECT_FALSE(bridge.WritePixelMapToParcel(pixelMap.get(), nullptr));
    EXPECT_TRUE(bridge.loadAttempted_);
}

/**
 * @tc.number: PixelMapBridgeTest_NullSafety_destroy_0100
 * @tc.desc: Destroying a null pointer is a no-op.
 * @tc.type: FUNC
 */
HWTEST_F(PixelMapBridgeTest, PixelMapBridgeTest_NullSafety_destroy_0100, TestSize.Level1)
{
    auto &bridge = PixelMapBridge::GetInstance();
    ForceReload(bridge);
    bridge.DestroyPixelMap(nullptr);  // must not crash
    EXPECT_TRUE(bridge.loadAttempted_);
}
}  // namespace AAFwk
}  // namespace OHOS
