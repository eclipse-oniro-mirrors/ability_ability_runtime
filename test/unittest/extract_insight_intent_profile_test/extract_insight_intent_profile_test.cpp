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

#include <gtest/gtest.h>

#include "insight_intent/extract_insight_intent_profile.cpp"
#include "extract_insight_intent_profile.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
    const std::string errProfileJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"game\","
            "\"schema\": \"GameList\","
            "\"keywords\": [\"123234345\",\"12345654321\"],"
            "\"intentName\": \"123\","
            "\"displayName\": \"Home\","
            "\"decoratorClass\": \"base\","
            "\"icon\": \"$r('app.media.startIcon')\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"uri\": \"/data/app/base\","
            "\"example\": \"exampleAAA\","
            "\"paramMappings\": ["
              "{"
                "\"paramCategory\": \"dddd\","
                "\"paramMappingName\": \"ccc\","
                "\"paramName\": \"aaa\""
              "}"
            "],"
            "\"decoratorType\": \"@InsightIntentLinkErr\","
            "\"llmDescription\": \"123111321\","
            "\"domain\": \"game\","
            "\"intentVersion\": \"1.0.2\","
            "\"bundleName\": \"com.example.instent\","
            "\"parameters\": {"
              "\"type\": \"object\","
              "\"items\": {"
                "\"type\": \"array\","
                "\"items\": {"
                  "\"propertyNames\": {"
                    "\"enum\": [\"entityId\",\"entityGroupId\",\"gameType\"]"
                  "},"
                  "\"type\": \"object\","
                  "\"required\": [\"entityId\"],"
                  "\"properties\": {"
                    "\"gameType\": {"
                      "\"description\": \"游戏类型\","
                      "\"type\": \"string\","
                      "\"enum\": [\"3D\",\"2D\",\"RPG\"]"
                    "},"
                    "\"entityId\": {"
                      "\"description\": \"游戏唯一实体 id\","
                      "\"type\": \"string\""
                    "},"
                    "\"entityGroupId\": {"
                      "\"description\": \"用于确定游戏的更新形式（每日游戏）\","
                      "\"type\": \"string\""
                    "}"
                  "}"
                "}"
              "}"
            "}"
        "},"
        "{"
            "\"displayDescription\": \"music\","
            "\"schema\": \"ControlPlayback\","
            "\"keywords\": [\"ControlPlayback\"],"
            "\"intentName\": \"123\","
            "\"displayName\": \"Home\","
            "\"decoratorClass\": \"base\","
            "\"icon\": \"$r('app.media.startIcon')\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"example\": \"exampleBBB\","
            "\"uri\": \"/data/app/base\","
            "\"paramMappings\": ["
              "{"
                "\"paramCategory\": \"dddd\","
                "\"paramMappingName\": \"ccc\","
                "\"paramName\": \"aaa\""
              "}"
            "],"
            "\"decoratorType\": \"@InsightIntentLinkErr2\","
            "\"llmDescription\": \"播放音乐控制\","
            "\"domain\": \"control\","
            "\"intentVersion\": \"1.0.1\","
            "\"bundleName\": \"com.example.instent\","
            "\"parameters\": {"
              "\"oneOf\": ["
                "{"
                  "\"required\": [\"playbackSpeed\"]"
                "},"
                "{"
                  "\"required\": [\"playbackProgress\"]"
                "}"
              "],"
              "\"propertyNames\": {"
                "\"enum\": [\"playbackSpeed\",\"playbackProgress\"]"
              "},"
              "\"type\": \"object\","
              "\"properties\": {"
                "\"playbackSpeed\": {"
                  "\"description\": \"播放倍速\","
                  "\"type\": \"number\","
                  "\"enum\": [0.5,0.75,1,1.25,1.5,2]"
                "},"
                "\"playbackProgress\": {"
                  "\"description\": \"播放进度,单位秒\","
                  "\"type\": \"number\""
                "}"
              "}"
            "}"
        "}"
        "]"
    "}";

    const std::string profileJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"game\","
            "\"schema\": \"GameList\","
            "\"keywords\": [\"123234345\",\"12345654321\"],"
            "\"intentName\": \"123\","
            "\"displayName\": \"Home\","
            "\"decoratorClass\": \"base\","
            "\"icon\": \"$r('app.media.startIcon')\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"uri\": \"/data/app/base\","
            "\"example\": \"exampleAAA\","
            "\"paramMappings\": ["
              "{"
                "\"paramCategory\": \"dddd\","
                "\"paramMappingName\": \"ccc\","
                "\"paramName\": \"aaa\""
              "}"
            "],"
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"llmDescription\": \"123111321\","
            "\"domain\": \"game\","
            "\"intentVersion\": \"1.0.2\","
            "\"bundleName\": \"com.example.instent\","
            "\"parameters\": {"
              "\"type\": \"object\","
              "\"items\": {"
                "\"type\": \"array\","
                "\"items\": {"
                  "\"propertyNames\": {"
                    "\"enum\": [\"entityId\",\"entityGroupId\",\"gameType\"]"
                  "},"
                  "\"type\": \"object\","
                  "\"required\": [\"entityId\"],"
                  "\"properties\": {"
                    "\"gameType\": {"
                      "\"description\": \"游戏类型\","
                      "\"type\": \"string\","
                      "\"enum\": [\"3D\",\"2D\",\"RPG\"]"
                    "},"
                    "\"entityId\": {"
                      "\"description\": \"游戏唯一实体 id\","
                      "\"type\": \"string\""
                    "},"
                    "\"entityGroupId\": {"
                      "\"description\": \"用于确定游戏的更新形式（每日游戏）\","
                      "\"type\": \"string\""
                    "}"
                  "}"
                "}"
              "}"
            "},"
            "\"result\": {"
              "\"type\": \"object\","
              "\"items\": {"
                "\"type\": \"array\","
                "\"items\": {"
                  "\"propertyNames\": {"
                    "\"enum\": [\"entityId\",\"entityGroupId\",\"gameType\"]"
                  "},"
                  "\"type\": \"object\","
                  "\"required\": [\"entityId\"],"
                  "\"properties\": {"
                    "\"gameType\": {"
                      "\"description\": \"游戏类型\","
                      "\"type\": \"string\","
                      "\"enum\": [\"3D\",\"2D\",\"RPG\"]"
                    "},"
                    "\"entityId\": {"
                      "\"description\": \"游戏唯一实体 id\","
                      "\"type\": \"string\""
                    "},"
                    "\"entityGroupId\": {"
                      "\"description\": \"用于确定游戏的更新形式（每日游戏）\","
                      "\"type\": \"string\""
                    "}"
                  "}"
                "}"
              "}"
            "},"
            "\"entities\": ["
              "{"
                "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
                "\"className\": \"SongPlayState\","
                "\"decoratorType\": \"@IntentEntityDecorator\","
                "\"entityId\": \"11\","
                "\"entityCategory\": \"entity Category\","
                "\"parentClassName\": \"base\","
                "\"parameters\": {"
                  "\"type\": \"object\","
                  "\"items\": {"
                    "\"type\": \"array\","
                    "\"items\": {"
                      "\"propertyNames\": {"
                        "\"enum\": [\"entityId\",\"entityGroupId\",\"gameType\"]"
                      "},"
                    "\"type\": \"object\","
                    "\"required\": [\"entityId\"]"
                    "}"
                  "}"
                "}"
              "},"
              "{"
                "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
                "\"className\": \"base\","
                "\"decoratorType\": \"@IntentEntityDecorator\","
                "\"entityId\": \"12\","
                "\"entityCategory\": \"entity1 Category\","
                "\"parameters\": {"
                  "\"type\": \"object\","
                  "\"items\": {"
                    "\"type\": \"array\","
                    "\"items\": {"
                      "\"propertyNames\": {"
                        "\"enum\": [\"entityId\",\"entityGroupId\",\"gameType\"]"
                      "},"
                      "\"type\": \"object\","
                      "\"required\": [\"entityId\"]"
                    "}"
                  "}"
                "}"
              "}"
            "]"
        "},"
        "{"
            "\"displayDescription\": \"music\","
            "\"schema\": \"ControlPlayback\","
            "\"keywords\": [\"ControlPlayback\"],"
            "\"intentName\": \"InsightIntent2\","
            "\"displayName\": \"Home\","
            "\"decoratorClass\": \"base\","
            "\"icon\": \"$r('app.media.startIcon')\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"uri\": \"/data/app/base\","
            "\"example\": \"exampleBBB\","
            "\"paramMappings\": ["
              "{"
                "\"paramCategory\": \"dddd\","
                "\"paramMappingName\": \"ccc\","
                "\"paramName\": \"aaa\""
              "}"
            "],"
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"llmDescription\": \"播放音乐控制\","
            "\"domain\": \"control\","
            "\"intentVersion\": \"1.0.1\","
            "\"bundleName\": \"com.example.instent\","
            "\"parameters\": {"
              "\"oneOf\": ["
                "{"
                  "\"required\": [\"playbackSpeed\"]"
                "},"
                "{"
                  "\"required\": [\"playbackProgress\"]"
                "}"
              "],"
              "\"propertyNames\": {"
                "\"enum\": [\"playbackSpeed\",\"playbackProgress\"]"
              "},"
              "\"type\": \"object\","
              "\"properties\": {"
                "\"playbackSpeed\": {"
                  "\"description\": \"播放倍速\","
                  "\"type\": \"number\","
                  "\"enum\": [0.5,0.75,1,1.25,1.5,2]"
                "},"
                "\"playbackProgress\": {"
                  "\"description\": \"播放进度,单位秒\","
                  "\"type\": \"number\""
                "}"
              "}"
            "},"
            "\"result\": {"
              "\"oneOf\": ["
                "{"
                  "\"required\": [\"playbackSpeed\"]"
                "},"
                "{"
                  "\"required\": [\"playbackProgress\"]"
                "}"
              "],"
              "\"propertyNames\": {"
                "\"enum\": [\"playbackSpeed\",\"playbackProgress\"]"
              "},"
              "\"type\": \"object\","
              "\"properties\": {"
                "\"playbackSpeed\": {"
                  "\"description\": \"播放倍速\","
                  "\"type\": \"number\","
                  "\"enum\": [0.5,0.75,1,1.25,1.5,2]"
                "},"
                "\"playbackProgress\": {"
                  "\"description\": \"播放进度,单位秒\","
                  "\"type\": \"number\""
                "}"
              "}"
            "}"
        "}"
        "]"
    "}";

    // ========== @InsightIntentLink 测试数据 ==========
    // 正确数据：@InsightIntentLink
    const std::string linkProfileJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"game\","
            "\"schema\": \"GameList\","
            "\"keywords\": [\"123234345\",\"12345654321\"],"
            "\"intentName\": \"GameListIntent\","
            "\"displayName\": \"游戏列表\","
            "\"decoratorClass\": \"GameLink\","
            "\"icon\": \"$r('app.media.startIcon')\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"uri\": \"/data/app/game/list\","
            "\"example\": \"打开游戏列表\","
            "\"paramMappings\": ["
              "{"
                "\"paramCategory\": \"page\","
                "\"paramMappingName\": \"pageNum\","
                "\"paramName\": \"page\""
              "}"
            "],"
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"llmDescription\": \"打开游戏列表页面\","
            "\"domain\": \"game\","
            "\"intentVersion\": \"1.0.2\","
            "\"bundleName\": \"com.example.instent\","
            "\"parameters\": {"
              "\"type\": \"object\","
              "\"properties\": {"
                "\"page\": {"
                  "\"description\": \"页码\","
                  "\"type\": \"number\""
                "}"
              "}"
            "}"
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentLink 缺少 decoratorFile
    const std::string linkMissingDecoratorFile = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"game\","
            "\"schema\": \"GameList\","
            "\"keywords\": [\"123234345\"],"
            "\"intentName\": \"GameListIntent\","
            "\"displayName\": \"游戏列表\","
            "\"decoratorClass\": \"GameLink\","
            "\"moduleName\": \"entry\","
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"llmDescription\": \"打开游戏列表页面\","
            "\"domain\": \"game\","
            "\"intentVersion\": \"1.0.2\","
            "\"bundleName\": \"com.example.instent\","
            "\"uri\": \"/data/app/game/list\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentLink 缺少 decoratorClass
    const std::string linkMissingDecoratorClass = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"game\","
            "\"schema\": \"GameList\","
            "\"keywords\": [\"123234345\"],"
            "\"intentName\": \"GameListIntent\","
            "\"displayName\": \"游戏列表\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"llmDescription\": \"打开游戏列表页面\","
            "\"domain\": \"game\","
            "\"intentVersion\": \"1.0.2\","
            "\"bundleName\": \"com.example.instent\","
            "\"uri\": \"/data/app/game/list\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentLink 缺少 decoratorType
    const std::string linkMissingDecoratorType = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"game\","
            "\"schema\": \"GameList\","
            "\"keywords\": [\"123234345\"],"
            "\"intentName\": \"GameListIntent\","
            "\"displayName\": \"游戏列表\","
            "\"decoratorClass\": \"GameLink\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"llmDescription\": \"打开游戏列表页面\","
            "\"domain\": \"game\","
            "\"intentVersion\": \"1.0.2\","
            "\"bundleName\": \"com.example.instent\","
            "\"uri\": \"/data/app/game/list\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentLink 缺少 bundleName
    const std::string linkMissingBundleName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"game\","
            "\"schema\": \"GameList\","
            "\"keywords\": [\"123234345\"],"
            "\"intentName\": \"GameListIntent\","
            "\"displayName\": \"游戏列表\","
            "\"decoratorClass\": \"GameLink\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"llmDescription\": \"打开游戏列表页面\","
            "\"domain\": \"game\","
            "\"intentVersion\": \"1.0.2\","
            "\"uri\": \"/data/app/game/list\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentLink 缺少 moduleName
    const std::string linkMissingModuleName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"game\","
            "\"schema\": \"GameList\","
            "\"keywords\": [\"123234345\"],"
            "\"intentName\": \"GameListIntent\","
            "\"displayName\": \"游戏列表\","
            "\"decoratorClass\": \"GameLink\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"llmDescription\": \"打开游戏列表页面\","
            "\"domain\": \"game\","
            "\"intentVersion\": \"1.0.2\","
            "\"bundleName\": \"com.example.instent\","
            "\"uri\": \"/data/app/game/list\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentLink 缺少 intentName
    const std::string linkMissingIntentName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"game\","
            "\"schema\": \"GameList\","
            "\"keywords\": [\"123234345\"],"
            "\"displayName\": \"游戏列表\","
            "\"decoratorClass\": \"GameLink\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"llmDescription\": \"打开游戏列表页面\","
            "\"domain\": \"game\","
            "\"intentVersion\": \"1.0.2\","
            "\"bundleName\": \"com.example.instent\","
            "\"uri\": \"/data/app/game/list\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentLink 缺少 domain
    const std::string linkMissingDomain = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"game\","
            "\"schema\": \"GameList\","
            "\"keywords\": [\"123234345\"],"
            "\"intentName\": \"GameListIntent\","
            "\"displayName\": \"游戏列表\","
            "\"decoratorClass\": \"GameLink\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"llmDescription\": \"打开游戏列表页面\","
            "\"intentVersion\": \"1.0.2\","
            "\"bundleName\": \"com.example.instent\","
            "\"uri\": \"/data/app/game/list\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentLink 缺少 intentVersion
    const std::string linkMissingIntentVersion = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"game\","
            "\"schema\": \"GameList\","
            "\"keywords\": [\"123234345\"],"
            "\"intentName\": \"GameListIntent\","
            "\"displayName\": \"游戏列表\","
            "\"decoratorClass\": \"GameLink\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"llmDescription\": \"打开游戏列表页面\","
            "\"domain\": \"game\","
            "\"bundleName\": \"com.example.instent\","
            "\"uri\": \"/data/app/game/list\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentLink 缺少 displayName
    const std::string linkMissingDisplayName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"game\","
            "\"schema\": \"GameList\","
            "\"keywords\": [\"123234345\"],"
            "\"intentName\": \"GameListIntent\","
            "\"decoratorClass\": \"GameLink\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"llmDescription\": \"打开游戏列表页面\","
            "\"domain\": \"game\","
            "\"intentVersion\": \"1.0.2\","
            "\"bundleName\": \"com.example.instent\","
            "\"uri\": \"/data/app/game/list\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentLink 缺少 uri
    const std::string linkMissingUri = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"game\","
            "\"schema\": \"GameList\","
            "\"keywords\": [\"123234345\"],"
            "\"intentName\": \"GameListIntent\","
            "\"displayName\": \"游戏列表\","
            "\"decoratorClass\": \"GameLink\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/Index&\","
            "\"decoratorType\": \"@InsightIntentLink\","
            "\"llmDescription\": \"打开游戏列表页面\","
            "\"domain\": \"game\","
            "\"intentVersion\": \"1.0.2\","
            "\"bundleName\": \"com.example.instent\""
        "}"
        "]"
    "}";

    // ========== @InsightIntentPage 测试数据 ==========
    // 正确数据：@InsightIntentPage
    const std::string pageProfileJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"打开商品详情页面\","
            "\"schema\": \"ProductDetail\","
            "\"keywords\": [\"商品详情\", \"product\"],"
            "\"intentName\": \"OpenProductDetail\","
            "\"displayName\": \"商品详情\","
            "\"decoratorClass\": \"ProductPage\","
            "\"icon\": \"$r('app.media.product_icon')\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/ProductDetail&\","
            "\"decoratorType\": \"@InsightIntentPage\","
            "\"llmDescription\": \"打开商品详情页面\","
            "\"domain\": \"shopping\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.shopping\","
            "\"pagePath\": \"pages/ProductDetail\","
            "\"uiAbility\": \"EntryAbility\","
            "\"parameters\": {"
              "\"type\": \"object\""
            "}"
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentPage 缺少 decoratorFile
    const std::string pageMissingDecoratorFile = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"打开商品详情页面\","
            "\"schema\": \"ProductDetail\","
            "\"keywords\": [\"商品详情\"],"
            "\"intentName\": \"OpenProductDetail\","
            "\"displayName\": \"商品详情\","
            "\"decoratorClass\": \"ProductPage\","
            "\"moduleName\": \"entry\","
            "\"decoratorType\": \"@InsightIntentPage\","
            "\"llmDescription\": \"打开商品详情页面\","
            "\"domain\": \"shopping\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.shopping\","
            "\"pagePath\": \"pages/ProductDetail\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentPage 缺少 decoratorClass
    const std::string pageMissingDecoratorClass = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"打开商品详情页面\","
            "\"schema\": \"ProductDetail\","
            "\"keywords\": [\"商品详情\"],"
            "\"intentName\": \"OpenProductDetail\","
            "\"displayName\": \"商品详情\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/ProductDetail&\","
            "\"decoratorType\": \"@InsightIntentPage\","
            "\"llmDescription\": \"打开商品详情页面\","
            "\"domain\": \"shopping\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.shopping\","
            "\"pagePath\": \"pages/ProductDetail\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentPage 缺少 decoratorType
    const std::string pageMissingDecoratorType = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"打开商品详情页面\","
            "\"schema\": \"ProductDetail\","
            "\"keywords\": [\"商品详情\"],"
            "\"intentName\": \"OpenProductDetail\","
            "\"displayName\": \"商品详情\","
            "\"decoratorClass\": \"ProductPage\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/ProductDetail&\","
            "\"llmDescription\": \"打开商品详情页面\","
            "\"domain\": \"shopping\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.shopping\","
            "\"pagePath\": \"pages/ProductDetail\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentPage 缺少 bundleName
    const std::string pageMissingBundleName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"打开商品详情页面\","
            "\"schema\": \"ProductDetail\","
            "\"keywords\": [\"商品详情\"],"
            "\"intentName\": \"OpenProductDetail\","
            "\"displayName\": \"商品详情\","
            "\"decoratorClass\": \"ProductPage\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/ProductDetail&\","
            "\"decoratorType\": \"@InsightIntentPage\","
            "\"llmDescription\": \"打开商品详情页面\","
            "\"domain\": \"shopping\","
            "\"intentVersion\": \"1.0.0\","
            "\"pagePath\": \"pages/ProductDetail\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentPage 缺少 moduleName
    const std::string pageMissingModuleName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"打开商品详情页面\","
            "\"schema\": \"ProductDetail\","
            "\"keywords\": [\"商品详情\"],"
            "\"intentName\": \"OpenProductDetail\","
            "\"displayName\": \"商品详情\","
            "\"decoratorClass\": \"ProductPage\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/ProductDetail&\","
            "\"decoratorType\": \"@InsightIntentPage\","
            "\"llmDescription\": \"打开商品详情页面\","
            "\"domain\": \"shopping\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.shopping\","
            "\"pagePath\": \"pages/ProductDetail\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentPage 缺少 intentName
    const std::string pageMissingIntentName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"打开商品详情页面\","
            "\"schema\": \"ProductDetail\","
            "\"keywords\": [\"商品详情\"],"
            "\"displayName\": \"商品详情\","
            "\"decoratorClass\": \"ProductPage\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/ProductDetail&\","
            "\"decoratorType\": \"@InsightIntentPage\","
            "\"llmDescription\": \"打开商品详情页面\","
            "\"domain\": \"shopping\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.shopping\","
            "\"pagePath\": \"pages/ProductDetail\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentPage 缺少 domain
    const std::string pageMissingDomain = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"打开商品详情页面\","
            "\"schema\": \"ProductDetail\","
            "\"keywords\": [\"商品详情\"],"
            "\"intentName\": \"OpenProductDetail\","
            "\"displayName\": \"商品详情\","
            "\"decoratorClass\": \"ProductPage\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/ProductDetail&\","
            "\"decoratorType\": \"@InsightIntentPage\","
            "\"llmDescription\": \"打开商品详情页面\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.shopping\","
            "\"pagePath\": \"pages/ProductDetail\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentPage 缺少 intentVersion
    const std::string pageMissingIntentVersion = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"打开商品详情页面\","
            "\"schema\": \"ProductDetail\","
            "\"keywords\": [\"商品详情\"],"
            "\"intentName\": \"OpenProductDetail\","
            "\"displayName\": \"商品详情\","
            "\"decoratorClass\": \"ProductPage\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/ProductDetail&\","
            "\"decoratorType\": \"@InsightIntentPage\","
            "\"llmDescription\": \"打开商品详情页面\","
            "\"domain\": \"shopping\","
            "\"bundleName\": \"com.example.shopping\","
            "\"pagePath\": \"pages/ProductDetail\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentPage 缺少 displayName
    const std::string pageMissingDisplayName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"打开商品详情页面\","
            "\"schema\": \"ProductDetail\","
            "\"keywords\": [\"商品详情\"],"
            "\"intentName\": \"OpenProductDetail\","
            "\"decoratorClass\": \"ProductPage\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/ProductDetail&\","
            "\"decoratorType\": \"@InsightIntentPage\","
            "\"llmDescription\": \"打开商品详情页面\","
            "\"domain\": \"shopping\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.shopping\","
            "\"pagePath\": \"pages/ProductDetail\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentPage 缺少 pagePath
    const std::string pageMissingPagePath = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"打开商品详情页面\","
            "\"schema\": \"ProductDetail\","
            "\"keywords\": [\"商品详情\"],"
            "\"intentName\": \"OpenProductDetail\","
            "\"displayName\": \"商品详情\","
            "\"decoratorClass\": \"ProductPage\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/pages/ProductDetail&\","
            "\"decoratorType\": \"@InsightIntentPage\","
            "\"llmDescription\": \"打开商品详情页面\","
            "\"domain\": \"shopping\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.shopping\","
            "\"uiAbility\": \"EntryAbility\""
        "}"
        "]"
    "}";

    // ========== @InsightIntentEntry 测试数据 ==========
    // 正确数据：@InsightIntentEntry
    const std::string entryProfileJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"启动音乐播放器\","
            "\"schema\": \"MusicPlayer\","
            "\"keywords\": [\"音乐\", \"播放器\"],"
            "\"intentName\": \"StartMusicPlayer\","
            "\"displayName\": \"音乐播放器\","
            "\"decoratorClass\": \"MusicEntry\","
            "\"icon\": \"$r('app.media.music_icon')\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/MusicAbility&\","
            "\"decoratorType\": \"@InsightIntentEntry\","
            "\"llmDescription\": \"启动音乐播放器应用\","
            "\"domain\": \"music\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.music\","
            "\"abilityName\": \"MusicAbility\","
            "\"executeMode\": [\"foreground\", \"background\"],"
            "\"parameters\": {"
              "\"type\": \"object\""
            "}"
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentEntry 缺少 decoratorFile
    const std::string entryMissingDecoratorFile = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"启动音乐播放器\","
            "\"schema\": \"MusicPlayer\","
            "\"keywords\": [\"音乐\"],"
            "\"intentName\": \"StartMusicPlayer\","
            "\"displayName\": \"音乐播放器\","
            "\"decoratorClass\": \"MusicEntry\","
            "\"moduleName\": \"entry\","
            "\"decoratorType\": \"@InsightIntentEntry\","
            "\"llmDescription\": \"启动音乐播放器应用\","
            "\"domain\": \"music\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.music\","
            "\"abilityName\": \"MusicAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentEntry 缺少 decoratorClass
    const std::string entryMissingDecoratorClass = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"启动音乐播放器\","
            "\"schema\": \"MusicPlayer\","
            "\"keywords\": [\"音乐\"],"
            "\"intentName\": \"StartMusicPlayer\","
            "\"displayName\": \"音乐播放器\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/MusicAbility&\","
            "\"decoratorType\": \"@InsightIntentEntry\","
            "\"llmDescription\": \"启动音乐播放器应用\","
            "\"domain\": \"music\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.music\","
            "\"abilityName\": \"MusicAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentEntry 缺少 decoratorType
    const std::string entryMissingDecoratorType = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"启动音乐播放器\","
            "\"schema\": \"MusicPlayer\","
            "\"keywords\": [\"音乐\"],"
            "\"intentName\": \"StartMusicPlayer\","
            "\"displayName\": \"音乐播放器\","
            "\"decoratorClass\": \"MusicEntry\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/MusicAbility&\","
            "\"llmDescription\": \"启动音乐播放器应用\","
            "\"domain\": \"music\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.music\","
            "\"abilityName\": \"MusicAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentEntry 缺少 bundleName
    const std::string entryMissingBundleName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"启动音乐播放器\","
            "\"schema\": \"MusicPlayer\","
            "\"keywords\": [\"音乐\"],"
            "\"intentName\": \"StartMusicPlayer\","
            "\"displayName\": \"音乐播放器\","
            "\"decoratorClass\": \"MusicEntry\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/MusicAbility&\","
            "\"decoratorType\": \"@InsightIntentEntry\","
            "\"llmDescription\": \"启动音乐播放器应用\","
            "\"domain\": \"music\","
            "\"intentVersion\": \"1.0.0\","
            "\"abilityName\": \"MusicAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentEntry 缺少 moduleName
    const std::string entryMissingModuleName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"启动音乐播放器\","
            "\"schema\": \"MusicPlayer\","
            "\"keywords\": [\"音乐\"],"
            "\"intentName\": \"StartMusicPlayer\","
            "\"displayName\": \"音乐播放器\","
            "\"decoratorClass\": \"MusicEntry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/MusicAbility&\","
            "\"decoratorType\": \"@InsightIntentEntry\","
            "\"llmDescription\": \"启动音乐播放器应用\","
            "\"domain\": \"music\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.music\","
            "\"abilityName\": \"MusicAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentEntry 缺少 intentName
    const std::string entryMissingIntentName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"启动音乐播放器\","
            "\"schema\": \"MusicPlayer\","
            "\"keywords\": [\"音乐\"],"
            "\"displayName\": \"音乐播放器\","
            "\"decoratorClass\": \"MusicEntry\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/MusicAbility&\","
            "\"decoratorType\": \"@InsightIntentEntry\","
            "\"llmDescription\": \"启动音乐播放器应用\","
            "\"domain\": \"music\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.music\","
            "\"abilityName\": \"MusicAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentEntry 缺少 domain
    const std::string entryMissingDomain = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"启动音乐播放器\","
            "\"schema\": \"MusicPlayer\","
            "\"keywords\": [\"音乐\"],"
            "\"intentName\": \"StartMusicPlayer\","
            "\"displayName\": \"音乐播放器\","
            "\"decoratorClass\": \"MusicEntry\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/MusicAbility&\","
            "\"decoratorType\": \"@InsightIntentEntry\","
            "\"llmDescription\": \"启动音乐播放器应用\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.music\","
            "\"abilityName\": \"MusicAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentEntry 缺少 intentVersion
    const std::string entryMissingIntentVersion = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"启动音乐播放器\","
            "\"schema\": \"MusicPlayer\","
            "\"keywords\": [\"音乐\"],"
            "\"intentName\": \"StartMusicPlayer\","
            "\"displayName\": \"音乐播放器\","
            "\"decoratorClass\": \"MusicEntry\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/MusicAbility&\","
            "\"decoratorType\": \"@InsightIntentEntry\","
            "\"llmDescription\": \"启动音乐播放器应用\","
            "\"domain\": \"music\","
            "\"bundleName\": \"com.example.music\","
            "\"abilityName\": \"MusicAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentEntry 缺少 displayName
    const std::string entryMissingDisplayName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"启动音乐播放器\","
            "\"schema\": \"MusicPlayer\","
            "\"keywords\": [\"音乐\"],"
            "\"intentName\": \"StartMusicPlayer\","
            "\"decoratorClass\": \"MusicEntry\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/MusicAbility&\","
            "\"decoratorType\": \"@InsightIntentEntry\","
            "\"llmDescription\": \"启动音乐播放器应用\","
            "\"domain\": \"music\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.music\","
            "\"abilityName\": \"MusicAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentEntry 缺少 abilityName
    const std::string entryMissingAbilityName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"启动音乐播放器\","
            "\"schema\": \"MusicPlayer\","
            "\"keywords\": [\"音乐\"],"
            "\"intentName\": \"StartMusicPlayer\","
            "\"displayName\": \"音乐播放器\","
            "\"decoratorClass\": \"MusicEntry\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/MusicAbility&\","
            "\"decoratorType\": \"@InsightIntentEntry\","
            "\"llmDescription\": \"启动音乐播放器应用\","
            "\"domain\": \"music\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.music\","
            "\"executeMode\": [\"foreground\"]"
        "}"
        "]"
    "}";

    // ========== @InsightIntentFunctionMethod 测试数据 ==========
    // 正确数据：@InsightIntentFunctionMethod
    const std::string functionProfileJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"设置闹钟\","
            "\"schema\": \"SetAlarm\","
            "\"keywords\": [\"闹钟\", \"提醒\"],"
            "\"intentName\": \"SetAlarmIntent\","
            "\"displayName\": \"设置闹钟\","
            "\"decoratorClass\": \"AlarmFunction\","
            "\"icon\": \"$r('app.media.alarm_icon')\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/AlarmManager&\","
            "\"decoratorType\": \"@InsightIntentFunctionMethod\","
            "\"llmDescription\": \"设置闹钟提醒\","
            "\"domain\": \"alarm\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.alarm\","
            "\"functionName\": \"setAlarm\","
            "\"functionParamList\": [\"time\", \"label\"],"
            "\"parameters\": {"
              "\"type\": \"object\""
            "}"
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentFunctionMethod 缺少 decoratorFile
    const std::string functionMissingDecoratorFile = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"设置闹钟\","
            "\"schema\": \"SetAlarm\","
            "\"keywords\": [\"闹钟\"],"
            "\"intentName\": \"SetAlarmIntent\","
            "\"displayName\": \"设置闹钟\","
            "\"decoratorClass\": \"AlarmFunction\","
            "\"moduleName\": \"entry\","
            "\"decoratorType\": \"@InsightIntentFunctionMethod\","
            "\"llmDescription\": \"设置闹钟提醒\","
            "\"domain\": \"alarm\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.alarm\","
            "\"functionName\": \"setAlarm\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentFunctionMethod 缺少 decoratorClass
    const std::string functionMissingDecoratorClass = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"设置闹钟\","
            "\"schema\": \"SetAlarm\","
            "\"keywords\": [\"闹钟\"],"
            "\"intentName\": \"SetAlarmIntent\","
            "\"displayName\": \"设置闹钟\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/AlarmManager&\","
            "\"decoratorType\": \"@InsightIntentFunctionMethod\","
            "\"llmDescription\": \"设置闹钟提醒\","
            "\"domain\": \"alarm\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.alarm\","
            "\"functionName\": \"setAlarm\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentFunctionMethod 缺少 decoratorType
    const std::string functionMissingDecoratorType = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"设置闹钟\","
            "\"schema\": \"SetAlarm\","
            "\"keywords\": [\"闹钟\"],"
            "\"intentName\": \"SetAlarmIntent\","
            "\"displayName\": \"设置闹钟\","
            "\"decoratorClass\": \"AlarmFunction\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/AlarmManager&\","
            "\"llmDescription\": \"设置闹钟提醒\","
            "\"domain\": \"alarm\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.alarm\","
            "\"functionName\": \"setAlarm\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentFunctionMethod 缺少 bundleName
    const std::string functionMissingBundleName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"设置闹钟\","
            "\"schema\": \"SetAlarm\","
            "\"keywords\": [\"闹钟\"],"
            "\"intentName\": \"SetAlarmIntent\","
            "\"displayName\": \"设置闹钟\","
            "\"decoratorClass\": \"AlarmFunction\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/AlarmManager&\","
            "\"decoratorType\": \"@InsightIntentFunctionMethod\","
            "\"llmDescription\": \"设置闹钟提醒\","
            "\"domain\": \"alarm\","
            "\"intentVersion\": \"1.0.0\","
            "\"functionName\": \"setAlarm\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentFunctionMethod 缺少 moduleName
    const std::string functionMissingModuleName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"设置闹钟\","
            "\"schema\": \"SetAlarm\","
            "\"keywords\": [\"闹钟\"],"
            "\"intentName\": \"SetAlarmIntent\","
            "\"displayName\": \"设置闹钟\","
            "\"decoratorClass\": \"AlarmFunction\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/AlarmManager&\","
            "\"decoratorType\": \"@InsightIntentFunctionMethod\","
            "\"llmDescription\": \"设置闹钟提醒\","
            "\"domain\": \"alarm\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.alarm\","
            "\"functionName\": \"setAlarm\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentFunctionMethod 缺少 intentName
    const std::string functionMissingIntentName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"设置闹钟\","
            "\"schema\": \"SetAlarm\","
            "\"keywords\": [\"闹钟\"],"
            "\"displayName\": \"设置闹钟\","
            "\"decoratorClass\": \"AlarmFunction\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/AlarmManager&\","
            "\"decoratorType\": \"@InsightIntentFunctionMethod\","
            "\"llmDescription\": \"设置闹钟提醒\","
            "\"domain\": \"alarm\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.alarm\","
            "\"functionName\": \"setAlarm\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentFunctionMethod 缺少 domain
    const std::string functionMissingDomain = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"设置闹钟\","
            "\"schema\": \"SetAlarm\","
            "\"keywords\": [\"闹钟\"],"
            "\"intentName\": \"SetAlarmIntent\","
            "\"displayName\": \"设置闹钟\","
            "\"decoratorClass\": \"AlarmFunction\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/AlarmManager&\","
            "\"decoratorType\": \"@InsightIntentFunctionMethod\","
            "\"llmDescription\": \"设置闹钟提醒\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.alarm\","
            "\"functionName\": \"setAlarm\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentFunctionMethod 缺少 intentVersion
    const std::string functionMissingIntentVersion = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"设置闹钟\","
            "\"schema\": \"SetAlarm\","
            "\"keywords\": [\"闹钟\"],"
            "\"intentName\": \"SetAlarmIntent\","
            "\"displayName\": \"设置闹钟\","
            "\"decoratorClass\": \"AlarmFunction\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/AlarmManager&\","
            "\"decoratorType\": \"@InsightIntentFunctionMethod\","
            "\"llmDescription\": \"设置闹钟提醒\","
            "\"domain\": \"alarm\","
            "\"bundleName\": \"com.example.alarm\","
            "\"functionName\": \"setAlarm\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentFunctionMethod 缺少 displayName
    const std::string functionMissingDisplayName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"设置闹钟\","
            "\"schema\": \"SetAlarm\","
            "\"keywords\": [\"闹钟\"],"
            "\"intentName\": \"SetAlarmIntent\","
            "\"decoratorClass\": \"AlarmFunction\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/AlarmManager&\","
            "\"decoratorType\": \"@InsightIntentFunctionMethod\","
            "\"llmDescription\": \"设置闹钟提醒\","
            "\"domain\": \"alarm\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.alarm\","
            "\"functionName\": \"setAlarm\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentFunctionMethod 缺少 functionName
    const std::string functionMissingFunctionName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"设置闹钟\","
            "\"schema\": \"SetAlarm\","
            "\"keywords\": [\"闹钟\"],"
            "\"intentName\": \"SetAlarmIntent\","
            "\"displayName\": \"设置闹钟\","
            "\"decoratorClass\": \"AlarmFunction\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/AlarmManager&\","
            "\"decoratorType\": \"@InsightIntentFunctionMethod\","
            "\"llmDescription\": \"设置闹钟提醒\","
            "\"domain\": \"alarm\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.alarm\","
            "\"functionParamList\": [\"time\"]"
        "}"
        "]"
    "}";

    // ========== @InsightIntentForm 测试数据 ==========
    // 正确数据：@InsightIntentForm
    const std::string formProfileJsonStr = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"天气卡片\","
            "\"schema\": \"WeatherWidget\","
            "\"keywords\": [\"天气\", \"卡片\"],"
            "\"intentName\": \"WeatherFormIntent\","
            "\"displayName\": \"天气卡片\","
            "\"decoratorClass\": \"WeatherForm\","
            "\"icon\": \"$r('app.media.weather_icon')\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/form/WeatherForm&\","
            "\"decoratorType\": \"@InsightIntentForm\","
            "\"llmDescription\": \"显示天气信息卡片\","
            "\"domain\": \"weather\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.weather\","
            "\"formName\": \"weatherCard\","
            "\"abilityName\": \"FormAbility\","
            "\"parameters\": {"
              "\"type\": \"object\""
            "}"
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentForm 缺少 decoratorFile
    const std::string formMissingDecoratorFile = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"天气卡片\","
            "\"schema\": \"WeatherWidget\","
            "\"keywords\": [\"天气\"],"
            "\"intentName\": \"WeatherFormIntent\","
            "\"displayName\": \"天气卡片\","
            "\"decoratorClass\": \"WeatherForm\","
            "\"moduleName\": \"entry\","
            "\"decoratorType\": \"@InsightIntentForm\","
            "\"llmDescription\": \"显示天气信息卡片\","
            "\"domain\": \"weather\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.weather\","
            "\"formName\": \"weatherCard\","
            "\"abilityName\": \"FormAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentForm 缺少 decoratorClass
    const std::string formMissingDecoratorClass = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"天气卡片\","
            "\"schema\": \"WeatherWidget\","
            "\"keywords\": [\"天气\"],"
            "\"intentName\": \"WeatherFormIntent\","
            "\"displayName\": \"天气卡片\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/form/WeatherForm&\","
            "\"decoratorType\": \"@InsightIntentForm\","
            "\"llmDescription\": \"显示天气信息卡片\","
            "\"domain\": \"weather\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.weather\","
            "\"formName\": \"weatherCard\","
            "\"abilityName\": \"FormAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentForm 缺少 decoratorType
    const std::string formMissingDecoratorType = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"天气卡片\","
            "\"schema\": \"WeatherWidget\","
            "\"keywords\": [\"天气\"],"
            "\"intentName\": \"WeatherFormIntent\","
            "\"displayName\": \"天气卡片\","
            "\"decoratorClass\": \"WeatherForm\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/form/WeatherForm&\","
            "\"llmDescription\": \"显示天气信息卡片\","
            "\"domain\": \"weather\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.weather\","
            "\"formName\": \"weatherCard\","
            "\"abilityName\": \"FormAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentForm 缺少 bundleName
    const std::string formMissingBundleName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"天气卡片\","
            "\"schema\": \"WeatherWidget\","
            "\"keywords\": [\"天气\"],"
            "\"intentName\": \"WeatherFormIntent\","
            "\"displayName\": \"天气卡片\","
            "\"decoratorClass\": \"WeatherForm\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/form/WeatherForm&\","
            "\"decoratorType\": \"@InsightIntentForm\","
            "\"llmDescription\": \"显示天气信息卡片\","
            "\"domain\": \"weather\","
            "\"intentVersion\": \"1.0.0\","
            "\"formName\": \"weatherCard\","
            "\"abilityName\": \"FormAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentForm 缺少 moduleName
    const std::string formMissingModuleName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"天气卡片\","
            "\"schema\": \"WeatherWidget\","
            "\"keywords\": [\"天气\"],"
            "\"intentName\": \"WeatherFormIntent\","
            "\"displayName\": \"天气卡片\","
            "\"decoratorClass\": \"WeatherForm\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/form/WeatherForm&\","
            "\"decoratorType\": \"@InsightIntentForm\","
            "\"llmDescription\": \"显示天气信息卡片\","
            "\"domain\": \"weather\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.weather\","
            "\"formName\": \"weatherCard\","
            "\"abilityName\": \"FormAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentForm 缺少 intentName
    const std::string formMissingIntentName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"天气卡片\","
            "\"schema\": \"WeatherWidget\","
            "\"keywords\": [\"天气\"],"
            "\"displayName\": \"天气卡片\","
            "\"decoratorClass\": \"WeatherForm\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/form/WeatherForm&\","
            "\"decoratorType\": \"@InsightIntentForm\","
            "\"llmDescription\": \"显示天气信息卡片\","
            "\"domain\": \"weather\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.weather\","
            "\"formName\": \"weatherCard\","
            "\"abilityName\": \"FormAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentForm 缺少 domain
    const std::string formMissingDomain = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"天气卡片\","
            "\"schema\": \"WeatherWidget\","
            "\"keywords\": [\"天气\"],"
            "\"intentName\": \"WeatherFormIntent\","
            "\"displayName\": \"天气卡片\","
            "\"decoratorClass\": \"WeatherForm\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/form/WeatherForm&\","
            "\"decoratorType\": \"@InsightIntentForm\","
            "\"llmDescription\": \"显示天气信息卡片\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.weather\","
            "\"formName\": \"weatherCard\","
            "\"abilityName\": \"FormAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentForm 缺少 intentVersion
    const std::string formMissingIntentVersion = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"天气卡片\","
            "\"schema\": \"WeatherWidget\","
            "\"keywords\": [\"天气\"],"
            "\"intentName\": \"WeatherFormIntent\","
            "\"displayName\": \"天气卡片\","
            "\"decoratorClass\": \"WeatherForm\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/form/WeatherForm&\","
            "\"decoratorType\": \"@InsightIntentForm\","
            "\"llmDescription\": \"显示天气信息卡片\","
            "\"domain\": \"weather\","
            "\"bundleName\": \"com.example.weather\","
            "\"formName\": \"weatherCard\","
            "\"abilityName\": \"FormAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentForm 缺少 displayName
    const std::string formMissingDisplayName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"天气卡片\","
            "\"schema\": \"WeatherWidget\","
            "\"keywords\": [\"天气\"],"
            "\"intentName\": \"WeatherFormIntent\","
            "\"decoratorClass\": \"WeatherForm\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/form/WeatherForm&\","
            "\"decoratorType\": \"@InsightIntentForm\","
            "\"llmDescription\": \"显示天气信息卡片\","
            "\"domain\": \"weather\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.weather\","
            "\"formName\": \"weatherCard\","
            "\"abilityName\": \"FormAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentForm 缺少 formName
    const std::string formMissingFormName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"天气卡片\","
            "\"schema\": \"WeatherWidget\","
            "\"keywords\": [\"天气\"],"
            "\"intentName\": \"WeatherFormIntent\","
            "\"displayName\": \"天气卡片\","
            "\"decoratorClass\": \"WeatherForm\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/form/WeatherForm&\","
            "\"decoratorType\": \"@InsightIntentForm\","
            "\"llmDescription\": \"显示天气信息卡片\","
            "\"domain\": \"weather\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.weather\","
            "\"abilityName\": \"FormAbility\""
        "}"
        "]"
    "}";

    // 错误数据：@InsightIntentForm 缺少 abilityName
    const std::string formMissingAbilityName = "{"
        "\"extractInsightIntents\": ["
        "{"
            "\"displayDescription\": \"天气卡片\","
            "\"schema\": \"WeatherWidget\","
            "\"keywords\": [\"天气\"],"
            "\"intentName\": \"WeatherFormIntent\","
            "\"displayName\": \"天气卡片\","
            "\"decoratorClass\": \"WeatherForm\","
            "\"moduleName\": \"entry\","
            "\"decoratorFile\": \"@normalized:N&&&entry/src/main/ets/form/WeatherForm&\","
            "\"decoratorType\": \"@InsightIntentForm\","
            "\"llmDescription\": \"显示天气信息卡片\","
            "\"domain\": \"weather\","
            "\"intentVersion\": \"1.0.0\","
            "\"bundleName\": \"com.example.weather\","
            "\"formName\": \"weatherCard\""
        "}"
        "]"
    "}";

}  // namespace

class ExtractInsightIntentProfileTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ExtractInsightIntentProfileTest::SetUpTestCase()
{}

void ExtractInsightIntentProfileTest::TearDownTestCase()
{}

void ExtractInsightIntentProfileTest::SetUp()
{}

void ExtractInsightIntentProfileTest::TearDown()
{}

/**
 * @tc.number: TransformTo_0100
 * @tc.name: TransformTo
 * @tc.desc: Test TransformTo invalid param profileStr.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_0100, TestSize.Level0)
{
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(errProfileJsonStr, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_0200
 * @tc.name: TransformTo, ToJson, ProfileInfoFormat
 * @tc.desc: Test TransformTo profileStr success.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_0200, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_0200 called. start");
    TAG_LOGI(AAFwkTag::TEST, "profileJsonStr: %{public}s", profileJsonStr.c_str());
    ExtractInsightIntentProfileInfoVec profileInfos;
    bool result = ExtractInsightIntentProfile::TransformTo(profileJsonStr, profileInfos);
    EXPECT_EQ(result, true);
    EXPECT_EQ(profileInfos.insightIntents.size(), 2);
    EXPECT_EQ(profileInfos.insightIntents[0].decoratorType, "@InsightIntentLink");
    EXPECT_EQ(profileInfos.insightIntents[0].intentName, "123");
    EXPECT_EQ(profileInfos.insightIntents[0].example, "exampleAAA");
    EXPECT_NE(profileInfos.insightIntents[0].result, "");
    EXPECT_EQ(profileInfos.insightIntents[1].decoratorType, "@InsightIntentLink");
    EXPECT_EQ(profileInfos.insightIntents[1].intentName, "InsightIntent2");
    EXPECT_EQ(profileInfos.insightIntents[1].example, "exampleBBB");
    EXPECT_NE(profileInfos.insightIntents[1].result, "");

    EXPECT_EQ(profileInfos.insightIntents[0].entities.size(), 2);
    EXPECT_EQ(profileInfos.insightIntents[0].entities[0].className, "SongPlayState");
    EXPECT_EQ(profileInfos.insightIntents[0].entities[0].decoratorType, "@IntentEntityDecorator");
    EXPECT_EQ(profileInfos.insightIntents[0].entities[0].entityId, "11");
    EXPECT_EQ(profileInfos.insightIntents[0].entities[0].parentClassName, "base");
    EXPECT_NE(profileInfos.insightIntents[0].entities[0].parameters, "");
    EXPECT_EQ(profileInfos.insightIntents[0].entities[1].className, "base");
    EXPECT_EQ(profileInfos.insightIntents[0].entities[1].decoratorType, "@IntentEntityDecorator");
    EXPECT_EQ(profileInfos.insightIntents[0].entities[1].entityId, "12");
    EXPECT_EQ(profileInfos.insightIntents[0].entities[1].parentClassName, "");
    EXPECT_NE(profileInfos.insightIntents[0].entities[1].parameters, "");
    EXPECT_EQ(profileInfos.insightIntents[1].entities.size(), 0);

    nlohmann::json jsonObject1;
    result = ExtractInsightIntentProfile::ToJson(profileInfos.insightIntents[0], jsonObject1);
    EXPECT_EQ(result, true);
    ExtractInsightIntentProfileInfoVec profileInfos1;
    result = ExtractInsightIntentProfile::TransformTo(jsonObject1.dump(), profileInfos1);
    EXPECT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "jsonObject1 dump: %{public}s", jsonObject1.dump().c_str());
    EXPECT_EQ(profileInfos1.insightIntents.size(), 1);
    EXPECT_EQ(profileInfos1.insightIntents[0].decoratorType, "@InsightIntentLink");
    EXPECT_EQ(profileInfos1.insightIntents[0].intentName, "123");
    EXPECT_EQ(profileInfos1.insightIntents[0].example, "exampleAAA");
    EXPECT_EQ(profileInfos1.insightIntents[0].entities.size(), 2);
    EXPECT_EQ(profileInfos1.insightIntents[0].entities[0].className, "SongPlayState");
    EXPECT_EQ(profileInfos1.insightIntents[0].entities[0].decoratorType, "@IntentEntityDecorator");
    EXPECT_EQ(profileInfos1.insightIntents[0].entities[0].entityId, "11");
    EXPECT_EQ(profileInfos1.insightIntents[0].entities[0].parentClassName, "base");
    EXPECT_NE(profileInfos1.insightIntents[0].entities[0].parameters, "");
    EXPECT_EQ(profileInfos1.insightIntents[0].entities[1].className, "base");
    EXPECT_EQ(profileInfos1.insightIntents[0].entities[1].decoratorType, "@IntentEntityDecorator");
    EXPECT_EQ(profileInfos1.insightIntents[0].entities[1].entityId, "12");
    EXPECT_EQ(profileInfos1.insightIntents[0].entities[1].parentClassName, "");
    EXPECT_NE(profileInfos1.insightIntents[0].entities[1].parameters, "");

    nlohmann::json jsonObject2;
    result = ExtractInsightIntentProfile::ToJson(profileInfos.insightIntents[1], jsonObject2);
    EXPECT_EQ(result, true);
    ExtractInsightIntentProfileInfoVec profileInfos2;
    result = ExtractInsightIntentProfile::TransformTo(jsonObject2.dump(), profileInfos2);
    EXPECT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "jsonObject2 dump: %{public}s", jsonObject2.dump().c_str());
    EXPECT_EQ(profileInfos2.insightIntents.size(), 1);
    EXPECT_EQ(profileInfos2.insightIntents[0].decoratorType, "@InsightIntentLink");
    EXPECT_EQ(profileInfos2.insightIntents[0].intentName, "InsightIntent2");
    EXPECT_EQ(profileInfos2.insightIntents[0].example, "exampleBBB");
    EXPECT_EQ(profileInfos2.insightIntents[0].entities.size(), 0);

    ExtractInsightIntentInfo info1;
    result = ExtractInsightIntentProfile::ProfileInfoFormat(profileInfos1.insightIntents[0], info1);
    EXPECT_EQ(result, true);
    EXPECT_EQ(info1.domain, "game");
    EXPECT_NE(info1.result, "");
    EXPECT_EQ(info1.entities.size(), 2);
    EXPECT_EQ(info1.entities[0].className, "SongPlayState");
    EXPECT_EQ(info1.entities[0].decoratorType, "@IntentEntityDecorator");
    EXPECT_EQ(info1.entities[0].entityId, "11");
    EXPECT_EQ(info1.entities[0].parentClassName, "base");
    EXPECT_NE(info1.entities[0].parameters, "");
    EXPECT_EQ(info1.entities[1].className, "base");
    EXPECT_EQ(info1.entities[1].decoratorType, "@IntentEntityDecorator");
    EXPECT_EQ(info1.entities[1].entityId, "12");
    EXPECT_EQ(info1.entities[1].parentClassName, "");
    EXPECT_NE(info1.entities[1].parameters, "");
    EXPECT_EQ(info1.genericInfo.decoratorType, "@InsightIntentLink");
    InsightIntentLinkInfo linkInfo1 = info1.genericInfo.get<InsightIntentLinkInfo>();
    EXPECT_EQ(linkInfo1.uri, "/data/app/base");
    EXPECT_EQ(linkInfo1.paramMapping.size(), 1);

    ExtractInsightIntentInfo info2;
    result = ExtractInsightIntentProfile::ProfileInfoFormat(profileInfos2.insightIntents[0], info2);
    EXPECT_EQ(result, true);
    EXPECT_EQ(info2.domain, "control");
    EXPECT_NE(info2.result, "");
    EXPECT_EQ(info2.entities.size(), 0);
    EXPECT_EQ(info2.genericInfo.decoratorType, "@InsightIntentLink");
    InsightIntentLinkInfo linkInfo2 = info2.genericInfo.get<InsightIntentLinkInfo>();
    EXPECT_EQ(linkInfo2.uri, "/data/app/base");
    EXPECT_EQ(linkInfo2.paramMapping.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_0200 called. end");
}

// ========== @InsightIntentLink 测试用例 ==========

/**
 * @tc.number: TransformTo_Link_Valid_0100
 * @tc.name: TransformTo for @InsightIntentLink with valid data
 * @tc.desc: Test TransformTo with valid @InsightIntentLink data.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Link_Valid_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Link_Valid_0100 called.");
    ExtractInsightIntentProfileInfoVec profileInfos;
    bool result = ExtractInsightIntentProfile::TransformTo(linkProfileJsonStr, profileInfos);
    EXPECT_EQ(result, true);
    EXPECT_EQ(profileInfos.insightIntents.size(), 1);
    EXPECT_EQ(profileInfos.insightIntents[0].decoratorType, "@InsightIntentLink");
    EXPECT_EQ(profileInfos.insightIntents[0].intentName, "GameListIntent");
    EXPECT_EQ(profileInfos.insightIntents[0].uri, "/data/app/game/list");
}

/**
 * @tc.number: TransformTo_Link_Missing_DecoratorFile_0100
 * @tc.name: TransformTo for @InsightIntentLink missing decoratorFile
 * @tc.desc: Test TransformTo with @InsightIntentLink missing decoratorFile.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Link_Missing_DecoratorFile_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Link_Missing_DecoratorFile_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(linkMissingDecoratorFile, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Link_Missing_DecoratorClass_0100
 * @tc.name: TransformTo for @InsightIntentLink missing decoratorClass
 * @tc.desc: Test TransformTo with @InsightIntentLink missing decoratorClass.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Link_Missing_DecoratorClass_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Link_Missing_DecoratorClass_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(linkMissingDecoratorClass, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Link_Missing_DecoratorType_0100
 * @tc.name: TransformTo for @InsightIntentLink missing decoratorType
 * @tc.desc: Test TransformTo with @InsightIntentLink missing decoratorType.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Link_Missing_DecoratorType_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Link_Missing_DecoratorType_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(linkMissingDecoratorType, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Link_Missing_BundleName_0100
 * @tc.name: TransformTo for @InsightIntentLink missing bundleName
 * @tc.desc: Test TransformTo with @InsightIntentLink missing bundleName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Link_Missing_BundleName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Link_Missing_BundleName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(linkMissingBundleName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Link_Missing_ModuleName_0100
 * @tc.name: TransformTo for @InsightIntentLink missing moduleName
 * @tc.desc: Test TransformTo with @InsightIntentLink missing moduleName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Link_Missing_ModuleName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Link_Missing_ModuleName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(linkMissingModuleName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Link_Missing_IntentName_0100
 * @tc.name: TransformTo for @InsightIntentLink missing intentName
 * @tc.desc: Test TransformTo with @InsightIntentLink missing intentName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Link_Missing_IntentName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Link_Missing_IntentName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(linkMissingIntentName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Link_Missing_Domain_0100
 * @tc.name: TransformTo for @InsightIntentLink missing domain
 * @tc.desc: Test TransformTo with @InsightIntentLink missing domain.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Link_Missing_Domain_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Link_Missing_Domain_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(linkMissingDomain, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Link_Missing_IntentVersion_0100
 * @tc.name: TransformTo for @InsightIntentLink missing intentVersion
 * @tc.desc: Test TransformTo with @InsightIntentLink missing intentVersion.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Link_Missing_IntentVersion_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Link_Missing_IntentVersion_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(linkMissingIntentVersion, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Link_Missing_DisplayName_0100
 * @tc.name: TransformTo for @InsightIntentLink missing displayName
 * @tc.desc: Test TransformTo with @InsightIntentLink missing displayName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Link_Missing_DisplayName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Link_Missing_DisplayName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(linkMissingDisplayName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Link_Missing_Uri_0100
 * @tc.name: TransformTo for @InsightIntentLink missing uri
 * @tc.desc: Test TransformTo with @InsightIntentLink missing uri.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Link_Missing_Uri_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Link_Missing_Uri_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(linkMissingUri, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Link_Entity_Missing_EntityId_NotQueryable_0100
 * @tc.name: TransformTo for entity empty entityId and not queryable
 * @tc.desc: Test TransformTo hit CheckProfileInfo line560 branch.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Link_Entity_Missing_EntityId_NotQueryable_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Link_Entity_Missing_EntityId_NotQueryable_0100 called.");
    auto jsonObject = nlohmann::json::parse(profileJsonStr, nullptr, false);
    ASSERT_FALSE(jsonObject.is_discarded());
    jsonObject["extractInsightIntents"][0]["entities"][1]["entityId"] = "";
    jsonObject["extractInsightIntents"][0]["entities"][1]["parentClassName"] = "base";

    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(jsonObject.dump(), infos);
    EXPECT_EQ(result, false);

    jsonObject["extractInsightIntents"][0]["entities"][1]["parentClassName"] = "insightIntent.AppIntentEntity";
    result = ExtractInsightIntentProfile::TransformTo(jsonObject.dump(), infos);
    EXPECT_EQ(result, true);
}

// ========== @InsightIntentPage 测试用例 ==========

/**
 * @tc.number: TransformTo_Page_Valid_0100
 * @tc.name: TransformTo for @InsightIntentPage with valid data
 * @tc.desc: Test TransformTo with valid @InsightIntentPage data.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Page_Valid_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Page_Valid_0100 called.");
    ExtractInsightIntentProfileInfoVec profileInfos;
    bool result = ExtractInsightIntentProfile::TransformTo(pageProfileJsonStr, profileInfos);
    EXPECT_EQ(result, true);
    EXPECT_EQ(profileInfos.insightIntents.size(), 1);
    EXPECT_EQ(profileInfos.insightIntents[0].decoratorType, "@InsightIntentPage");
    EXPECT_EQ(profileInfos.insightIntents[0].intentName, "OpenProductDetail");
    EXPECT_EQ(profileInfos.insightIntents[0].pagePath, "pages/ProductDetail");
}

/**
 * @tc.number: TransformTo_Page_Missing_DecoratorFile_0100
 * @tc.name: TransformTo for @InsightIntentPage missing decoratorFile
 * @tc.desc: Test TransformTo with @InsightIntentPage missing decoratorFile.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Page_Missing_DecoratorFile_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Page_Missing_DecoratorFile_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(pageMissingDecoratorFile, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Page_Missing_DecoratorClass_0100
 * @tc.name: TransformTo for @InsightIntentPage missing decoratorClass
 * @tc.desc: Test TransformTo with @InsightIntentPage missing decoratorClass.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Page_Missing_DecoratorClass_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Page_Missing_DecoratorClass_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(pageMissingDecoratorClass, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Page_Missing_DecoratorType_0100
 * @tc.name: TransformTo for @InsightIntentPage missing decoratorType
 * @tc.desc: Test TransformTo with @InsightIntentPage missing decoratorType.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Page_Missing_DecoratorType_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Page_Missing_DecoratorType_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(pageMissingDecoratorType, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Page_Missing_BundleName_0100
 * @tc.name: TransformTo for @InsightIntentPage missing bundleName
 * @tc.desc: Test TransformTo with @InsightIntentPage missing bundleName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Page_Missing_BundleName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Page_Missing_BundleName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(pageMissingBundleName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Page_Missing_ModuleName_0100
 * @tc.name: TransformTo for @InsightIntentPage missing moduleName
 * @tc.desc: Test TransformTo with @InsightIntentPage missing moduleName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Page_Missing_ModuleName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Page_Missing_ModuleName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(pageMissingModuleName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Page_Missing_IntentName_0100
 * @tc.name: TransformTo for @InsightIntentPage missing intentName
 * @tc.desc: Test TransformTo with @InsightIntentPage missing intentName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Page_Missing_IntentName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Page_Missing_IntentName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(pageMissingIntentName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Page_Missing_Domain_0100
 * @tc.name: TransformTo for @InsightIntentPage missing domain
 * @tc.desc: Test TransformTo with @InsightIntentPage missing domain.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Page_Missing_Domain_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Page_Missing_Domain_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(pageMissingDomain, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Page_Missing_IntentVersion_0100
 * @tc.name: TransformTo for @InsightIntentPage missing intentVersion
 * @tc.desc: Test TransformTo with @InsightIntentPage missing intentVersion.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Page_Missing_IntentVersion_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Page_Missing_IntentVersion_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(pageMissingIntentVersion, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Page_Missing_DisplayName_0100
 * @tc.name: TransformTo for @InsightIntentPage missing displayName
 * @tc.desc: Test TransformTo with @InsightIntentPage missing displayName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Page_Missing_DisplayName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Page_Missing_DisplayName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(pageMissingDisplayName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Page_Missing_PagePath_0100
 * @tc.name: TransformTo for @InsightIntentPage missing pagePath
 * @tc.desc: Test TransformTo with @InsightIntentPage missing pagePath.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Page_Missing_PagePath_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Page_Missing_PagePath_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(pageMissingPagePath, infos);
    EXPECT_EQ(result, false);
}

// ========== @InsightIntentEntry 测试用例 ==========

/**
 * @tc.number: TransformTo_Entry_Valid_0100
 * @tc.name: TransformTo for @InsightIntentEntry with valid data
 * @tc.desc: Test TransformTo with valid @InsightIntentEntry data.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Entry_Valid_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Entry_Valid_0100 called.");
    ExtractInsightIntentProfileInfoVec profileInfos;
    bool result = ExtractInsightIntentProfile::TransformTo(entryProfileJsonStr, profileInfos);
    EXPECT_EQ(result, true);
    EXPECT_EQ(profileInfos.insightIntents.size(), 1);
    EXPECT_EQ(profileInfos.insightIntents[0].decoratorType, "@InsightIntentEntry");
    EXPECT_EQ(profileInfos.insightIntents[0].intentName, "StartMusicPlayer");
    EXPECT_EQ(profileInfos.insightIntents[0].abilityName, "MusicAbility");
}

/**
 * @tc.number: TransformTo_Entry_Missing_DecoratorFile_0100
 * @tc.name: TransformTo for @InsightIntentEntry missing decoratorFile
 * @tc.desc: Test TransformTo with @InsightIntentEntry missing decoratorFile.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Entry_Missing_DecoratorFile_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Entry_Missing_DecoratorFile_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(entryMissingDecoratorFile, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Entry_Missing_DecoratorClass_0100
 * @tc.name: TransformTo for @InsightIntentEntry missing decoratorClass
 * @tc.desc: Test TransformTo with @InsightIntentEntry missing decoratorClass.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Entry_Missing_DecoratorClass_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Entry_Missing_DecoratorClass_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(entryMissingDecoratorClass, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Entry_Missing_DecoratorType_0100
 * @tc.name: TransformTo for @InsightIntentEntry missing decoratorType
 * @tc.desc: Test TransformTo with @InsightIntentEntry missing decoratorType.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Entry_Missing_DecoratorType_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Entry_Missing_DecoratorType_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(entryMissingDecoratorType, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Entry_Missing_BundleName_0100
 * @tc.name: TransformTo for @InsightIntentEntry missing bundleName
 * @tc.desc: Test TransformTo with @InsightIntentEntry missing bundleName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Entry_Missing_BundleName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Entry_Missing_BundleName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(entryMissingBundleName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Entry_Missing_ModuleName_0100
 * @tc.name: TransformTo for @InsightIntentEntry missing moduleName
 * @tc.desc: Test TransformTo with @InsightIntentEntry missing moduleName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Entry_Missing_ModuleName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Entry_Missing_ModuleName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(entryMissingModuleName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Entry_Missing_IntentName_0100
 * @tc.name: TransformTo for @InsightIntentEntry missing intentName
 * @tc.desc: Test TransformTo with @InsightIntentEntry missing intentName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Entry_Missing_IntentName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Entry_Missing_IntentName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(entryMissingIntentName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Entry_Missing_Domain_0100
 * @tc.name: TransformTo for @InsightIntentEntry missing domain
 * @tc.desc: Test TransformTo with @InsightIntentEntry missing domain.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Entry_Missing_Domain_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Entry_Missing_Domain_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(entryMissingDomain, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Entry_Missing_IntentVersion_0100
 * @tc.name: TransformTo for @InsightIntentEntry missing intentVersion
 * @tc.desc: Test TransformTo with @InsightIntentEntry missing intentVersion.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Entry_Missing_IntentVersion_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Entry_Missing_IntentVersion_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(entryMissingIntentVersion, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Entry_Missing_DisplayName_0100
 * @tc.name: TransformTo for @InsightIntentEntry missing displayName
 * @tc.desc: Test TransformTo with @InsightIntentEntry missing displayName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Entry_Missing_DisplayName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Entry_Missing_DisplayName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(entryMissingDisplayName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Entry_Missing_AbilityName_0100
 * @tc.name: TransformTo for @InsightIntentEntry missing abilityName
 * @tc.desc: Test TransformTo with @InsightIntentEntry missing abilityName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Entry_Missing_AbilityName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Entry_Missing_AbilityName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(entryMissingAbilityName, infos);
    EXPECT_EQ(result, false);
}

// ========== @InsightIntentFunctionMethod 测试用例 ==========

/**
 * @tc.number: TransformTo_Function_Valid_0100
 * @tc.name: TransformTo for @InsightIntentFunctionMethod with valid data
 * @tc.desc: Test TransformTo with valid @InsightIntentFunctionMethod data.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Function_Valid_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Function_Valid_0100 called.");
    ExtractInsightIntentProfileInfoVec profileInfos;
    bool result = ExtractInsightIntentProfile::TransformTo(functionProfileJsonStr, profileInfos);
    EXPECT_EQ(result, true);
    EXPECT_EQ(profileInfos.insightIntents.size(), 1);
    EXPECT_EQ(profileInfos.insightIntents[0].decoratorType, "@InsightIntentFunctionMethod");
    EXPECT_EQ(profileInfos.insightIntents[0].intentName, "SetAlarmIntent");
    EXPECT_EQ(profileInfos.insightIntents[0].functionName, "setAlarm");
}

/**
 * @tc.number: TransformTo_Function_Missing_DecoratorFile_0100
 * @tc.name: TransformTo for @InsightIntentFunctionMethod missing decoratorFile
 * @tc.desc: Test TransformTo with @InsightIntentFunctionMethod missing decoratorFile.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Function_Missing_DecoratorFile_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Function_Missing_DecoratorFile_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(functionMissingDecoratorFile, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Function_Missing_DecoratorClass_0100
 * @tc.name: TransformTo for @InsightIntentFunctionMethod missing decoratorClass
 * @tc.desc: Test TransformTo with @InsightIntentFunctionMethod missing decoratorClass.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Function_Missing_DecoratorClass_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Function_Missing_DecoratorClass_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(functionMissingDecoratorClass, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Function_Missing_DecoratorType_0100
 * @tc.name: TransformTo for @InsightIntentFunctionMethod missing decoratorType
 * @tc.desc: Test TransformTo with @InsightIntentFunctionMethod missing decoratorType.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Function_Missing_DecoratorType_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Function_Missing_DecoratorType_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(functionMissingDecoratorType, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Function_Missing_BundleName_0100
 * @tc.name: TransformTo for @InsightIntentFunctionMethod missing bundleName
 * @tc.desc: Test TransformTo with @InsightIntentFunctionMethod missing bundleName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Function_Missing_BundleName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Function_Missing_BundleName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(functionMissingBundleName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Function_Missing_ModuleName_0100
 * @tc.name: TransformTo for @InsightIntentFunctionMethod missing moduleName
 * @tc.desc: Test TransformTo with @InsightIntentFunctionMethod missing moduleName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Function_Missing_ModuleName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Function_Missing_ModuleName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(functionMissingModuleName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Function_Missing_IntentName_0100
 * @tc.name: TransformTo for @InsightIntentFunctionMethod missing intentName
 * @tc.desc: Test TransformTo with @InsightIntentFunctionMethod missing intentName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Function_Missing_IntentName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Function_Missing_IntentName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(functionMissingIntentName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Function_Missing_Domain_0100
 * @tc.name: TransformTo for @InsightIntentFunctionMethod missing domain
 * @tc.desc: Test TransformTo with @InsightIntentFunctionMethod missing domain.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Function_Missing_Domain_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Function_Missing_Domain_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(functionMissingDomain, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Function_Missing_IntentVersion_0100
 * @tc.name: TransformTo for @InsightIntentFunctionMethod missing intentVersion
 * @tc.desc: Test TransformTo with @InsightIntentFunctionMethod missing intentVersion.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Function_Missing_IntentVersion_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Function_Missing_IntentVersion_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(functionMissingIntentVersion, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Function_Missing_DisplayName_0100
 * @tc.name: TransformTo for @InsightIntentFunctionMethod missing displayName
 * @tc.desc: Test TransformTo with @InsightIntentFunctionMethod missing displayName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Function_Missing_DisplayName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Function_Missing_DisplayName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(functionMissingDisplayName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Function_Missing_FunctionName_0100
 * @tc.name: TransformTo for @InsightIntentFunctionMethod missing functionName
 * @tc.desc: Test TransformTo with @InsightIntentFunctionMethod missing functionName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Function_Missing_FunctionName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Function_Missing_FunctionName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(functionMissingFunctionName, infos);
    EXPECT_EQ(result, false);
}

// ========== @InsightIntentForm 测试用例 ==========

/**
 * @tc.number: TransformTo_Form_Valid_0100
 * @tc.name: TransformTo for @InsightIntentForm with valid data
 * @tc.desc: Test TransformTo with valid @InsightIntentForm data.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Form_Valid_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Form_Valid_0100 called.");
    ExtractInsightIntentProfileInfoVec profileInfos;
    bool result = ExtractInsightIntentProfile::TransformTo(formProfileJsonStr, profileInfos);
    EXPECT_EQ(result, true);
    EXPECT_EQ(profileInfos.insightIntents.size(), 1);
    EXPECT_EQ(profileInfos.insightIntents[0].decoratorType, "@InsightIntentForm");
    EXPECT_EQ(profileInfos.insightIntents[0].intentName, "WeatherFormIntent");
    EXPECT_EQ(profileInfos.insightIntents[0].formName, "weatherCard");
    EXPECT_EQ(profileInfos.insightIntents[0].abilityName, "FormAbility");
}

/**
 * @tc.number: TransformTo_Form_Missing_DecoratorFile_0100
 * @tc.name: TransformTo for @InsightIntentForm missing decoratorFile
 * @tc.desc: Test TransformTo with @InsightIntentForm missing decoratorFile.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Form_Missing_DecoratorFile_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Form_Missing_DecoratorFile_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(formMissingDecoratorFile, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Form_Missing_DecoratorClass_0100
 * @tc.name: TransformTo for @InsightIntentForm missing decoratorClass
 * @tc.desc: Test TransformTo with @InsightIntentForm missing decoratorClass.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Form_Missing_DecoratorClass_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Form_Missing_DecoratorClass_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(formMissingDecoratorClass, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Form_Missing_DecoratorType_0100
 * @tc.name: TransformTo for @InsightIntentForm missing decoratorType
 * @tc.desc: Test TransformTo with @InsightIntentForm missing decoratorType.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Form_Missing_DecoratorType_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Form_Missing_DecoratorType_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(formMissingDecoratorType, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Form_Missing_BundleName_0100
 * @tc.name: TransformTo for @InsightIntentForm missing bundleName
 * @tc.desc: Test TransformTo with @InsightIntentForm missing bundleName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Form_Missing_BundleName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Form_Missing_BundleName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(formMissingBundleName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Form_Missing_ModuleName_0100
 * @tc.name: TransformTo for @InsightIntentForm missing moduleName
 * @tc.desc: Test TransformTo with @InsightIntentForm missing moduleName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Form_Missing_ModuleName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Form_Missing_ModuleName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(formMissingModuleName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Form_Missing_IntentName_0100
 * @tc.name: TransformTo for @InsightIntentForm missing intentName
 * @tc.desc: Test TransformTo with @InsightIntentForm missing intentName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Form_Missing_IntentName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Form_Missing_IntentName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(formMissingIntentName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Form_Missing_Domain_0100
 * @tc.name: TransformTo for @InsightIntentForm missing domain
 * @tc.desc: Test TransformTo with @InsightIntentForm missing domain.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Form_Missing_Domain_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Form_Missing_Domain_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(formMissingDomain, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Form_Missing_IntentVersion_0100
 * @tc.name: TransformTo for @InsightIntentForm missing intentVersion
 * @tc.desc: Test TransformTo with @InsightIntentForm missing intentVersion.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Form_Missing_IntentVersion_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Form_Missing_IntentVersion_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(formMissingIntentVersion, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Form_Missing_DisplayName_0100
 * @tc.name: TransformTo for @InsightIntentForm missing displayName
 * @tc.desc: Test TransformTo with @InsightIntentForm missing displayName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Form_Missing_DisplayName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Form_Missing_DisplayName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(formMissingDisplayName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Form_Missing_FormName_0100
 * @tc.name: TransformTo for @InsightIntentForm missing formName
 * @tc.desc: Test TransformTo with @InsightIntentForm missing formName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Form_Missing_FormName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Form_Missing_FormName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(formMissingFormName, infos);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: TransformTo_Form_Missing_AbilityName_0100
 * @tc.name: TransformTo for @InsightIntentForm missing abilityName
 * @tc.desc: Test TransformTo with @InsightIntentForm missing abilityName.
 */
HWTEST_F(ExtractInsightIntentProfileTest, TransformTo_Form_Missing_AbilityName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "TransformTo_Form_Missing_AbilityName_0100 called.");
    ExtractInsightIntentProfileInfoVec infos;
    bool result = ExtractInsightIntentProfile::TransformTo(formMissingAbilityName, infos);
    EXPECT_EQ(result, false);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
