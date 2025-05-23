/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "theme_manager_test.h"

#include "resource_manager_test_common.h"
#include "theme_pack_manager.h"
using namespace OHOS::Global::Resource;
using namespace testing::ext;
namespace {
class ThemeManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();

    ThemeManagerTest()
    {}

    ~ThemeManagerTest()
    {}

public:
    std::shared_ptr<ResourceManager> rm;
    std::shared_ptr<ResourceManagerTestCommon> rmc;
    std::shared_ptr<ThemePackManager> tm;
};

void ThemeManagerTest::SetUpTestCase(void)
{
    // step 1: input testsuit setup step
    g_logLevel = LOG_DEBUG;
}

void ThemeManagerTest::TearDownTestCase(void)
{
    // step 2: input testsuit teardown step
}

void ThemeManagerTest::SetUp(void)
{
    this->tm = ThemePackManager::GetThemePackManager();
    std::string hapPath;
    std::vector<std::string> overlayPath;
    ResConfigImpl *rc = new ResConfigImpl;
    int32_t appType = 0;
    int32_t userId = 100; // userId is 100
    this->rm = CreateResourceManager("ohos.global.test.all", "entry", hapPath, overlayPath, *rc, appType, userId);
    this->rmc = std::make_shared<ResourceManagerTestCommon>(rm);
}

void ThemeManagerTest::TearDown(void)
{
    RESMGR_HILOGD(RESMGR_TAG, "ThemeManagerTest teardown");
}

/*
 * @tc.name: ThemeManagerTestLoadThemeSkinResourceTest001
 * @tc.desc: Test LoadThemeSkinResource function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestLoadThemeSkinResourceTest001, TestSize.Level1)
{
   // success cases
    bool ret = rm->AddResource(FormatFullPath(g_hapPath).c_str());
    ASSERT_TRUE(ret);
    int id = rmc->GetResId("base_only", ResType::COLOR);
    uint32_t outValue;
    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/skin/ohos.global.test.all";
    rootDirs.emplace_back(rootDir);
    int32_t userId = 100; // userId is 100
    // the bundlename is invalid
    tm->LoadThemeSkinResource("ohos.global.test", "entry", rootDirs, userId);
    rm->GetColorById(id, outValue);
    ASSERT_EQ(0, outValue); // base_only APP value is #000000
}

/*
 * @tc.name: ThemeManagerTestLoadThemeSkinResourceTest002
 * @tc.desc: Test AddResource function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestLoadThemeSkinResourceTest002, TestSize.Level1)
{
   // success cases
    bool ret = rm->AddResource(FormatFullPath(g_hapPath).c_str());
    ASSERT_TRUE(ret);
    int id = rmc->GetResId("base_only", ResType::COLOR);
    uint32_t outValue;
    rm->GetColorById(id, outValue);
    ASSERT_EQ(0, outValue); // base_only APP value is #000000

    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/skin/ohos.global.test.all";
    rootDirs.emplace_back(rootDir);
    int32_t userId = 100; // userId is 100
    tm->LoadThemeSkinResource("ohos.global.test.all", "entry", rootDirs, userId);
    rm->GetColorById(id, outValue);
    ASSERT_EQ(4294967295, outValue); // base_only theme value is #ffffff(4294967295)
}

/*
 * @tc.name: ThemeManagerTestChangeSkinResourceStatusTest001
 * @tc.desc: Test ChangeSkinResourceStatus function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestChangeSkinResourceStatusTest001, TestSize.Level1)
{
   // success cases
    bool ret = rm->AddResource(FormatFullPath(g_hapPath).c_str());
    ASSERT_TRUE(ret);

    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/skin/ohos.global.test.all";
    rootDirs.emplace_back(rootDir);
    int32_t userId = 100; // userId is 100
    tm->LoadThemeSkinResource("ohos.global.test.all", "entry", rootDirs, userId);
    uint32_t outValue;
    int id = rmc->GetResId("base_only", ResType::COLOR);
    rm->GetColorById(id, outValue);
    ASSERT_EQ(4294967295, outValue); // base_only theme value is #ffffff(4294967295)

    tm->ChangeSkinResourceStatus(userId);
    tm->ClearSkinResource();

    rm->GetColorById(id, outValue);
    ASSERT_EQ(0, outValue); // base_only APP value is #000000
}

/*
 * @tc.name: ThemeManagerTestLoadThemeSkinResourceTest003
 * @tc.desc: Test GetThemeDataById function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestLoadThemeSkinResourceTest003, TestSize.Level1)
{
   // success cases
    bool ret = rm->AddResource(FormatFullPath(g_hapPath).c_str());
    ASSERT_TRUE(ret);
    std::vector<std::string> rootDirs;
    int32_t userId = 100; // userId is 100
    tm->LoadThemeSkinResource("ohos.global.test.all", "entry", rootDirs, userId);
    int id = rmc->GetResId("ohos_device_theme", ResType::THEME);
    std::map<std::string, ResourceManager::ResData> outValue;
    rm->GetThemeDataById(id, outValue);
    ASSERT_EQ(ResType::FLOAT, outValue["width1"].resType);
    ASSERT_EQ(ResType::STRING, outValue["width2"].resType);
    ASSERT_EQ(ResType::FLOAT, outValue["height1"].resType);
    ASSERT_EQ(ResType::STRING, outValue["height2"].resType);
    ASSERT_EQ(ResType::COLOR, outValue["textColor1"].resType);
    ASSERT_EQ(ResType::STRING, outValue["textColor2"].resType);
    ASSERT_EQ(ResType::PATTERN, outValue["button_pattern"].resType);
    ASSERT_EQ(ResType::THEME, outValue["ohosTheme"].resType);
    ASSERT_EQ(ResType::PATTERN, outValue["text_pattern"].resType);
    ASSERT_EQ(ResType::PATTERN, outValue["list_pattern"].resType);
    ASSERT_EQ("100fp", outValue["width1"].value);
    ASSERT_EQ("100fp", outValue["width2"].value);
    ASSERT_EQ("100vp", outValue["height1"].value);
    ASSERT_EQ("100vp", outValue["height2"].value);
    ASSERT_EQ("#000000FF", outValue["textColor1"].value);
    ASSERT_EQ("#000000FF", outValue["textColor2"].value);
    ASSERT_EQ("16777333", outValue["button_pattern"].value);
    ASSERT_EQ("16777328", outValue["ohosTheme"].value);
    ASSERT_EQ("16777332", outValue["text_pattern"].value);
    ASSERT_EQ("16777331", outValue["list_pattern"].value);

    std::string rootDir = "/data/test/theme/skin/ohos.global.test.all";
    rootDirs.emplace_back(rootDir);
    tm->LoadThemeSkinResource("ohos.global.test.all", "entry", rootDirs, userId);
    rm->GetThemeDataById(id, outValue);
    ASSERT_EQ(ResType::FLOAT, outValue["width1"].resType);
    ASSERT_EQ(ResType::STRING, outValue["width2"].resType);
    ASSERT_EQ(ResType::FLOAT, outValue["height1"].resType);
    ASSERT_EQ(ResType::STRING, outValue["height2"].resType);
    ASSERT_EQ(ResType::COLOR, outValue["textColor1"].resType);
    ASSERT_EQ(ResType::STRING, outValue["textColor2"].resType);
    ASSERT_EQ("1000fp", outValue["width1"].value);
    ASSERT_EQ("100fp", outValue["width2"].value);
    ASSERT_EQ("1000vp", outValue["height1"].value);
    ASSERT_EQ("100vp", outValue["height2"].value);
    ASSERT_EQ("#000000AA", outValue["textColor1"].value);
    ASSERT_EQ("#000000FF", outValue["textColor2"].value);
    ASSERT_EQ("16777333", outValue["button_pattern"].value);
    ASSERT_EQ("16777328", outValue["ohosTheme"].value);
    ASSERT_EQ("16777332", outValue["text_pattern"].value);
    ASSERT_EQ("16777331", outValue["list_pattern"].value);
}

/*
 * @tc.name: ThemeManagerTestLoadThemeSkinResourceTest004
 * @tc.desc: Test GetThemeDataByName function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestLoadThemeSkinResourceTest004, TestSize.Level1)
{
   // success cases
    bool ret = rm->AddResource(FormatFullPath(g_hapPath).c_str());
    ASSERT_TRUE(ret);
    std::vector<std::string> rootDirs;
    int32_t userId = 100; // userId is 100
    tm->LoadThemeSkinResource("ohos.global.test.all", "entry", rootDirs, userId);
    std::map<std::string, ResourceManager::ResData> outValue;
    rm->GetThemeDataByName("ohos_device_theme", outValue);
    ASSERT_EQ(ResType::FLOAT, outValue["width1"].resType);
    ASSERT_EQ(ResType::STRING, outValue["width2"].resType);
    ASSERT_EQ(ResType::FLOAT, outValue["height1"].resType);
    ASSERT_EQ(ResType::STRING, outValue["height2"].resType);
    ASSERT_EQ(ResType::COLOR, outValue["textColor1"].resType);
    ASSERT_EQ(ResType::STRING, outValue["textColor2"].resType);
    ASSERT_EQ(ResType::PATTERN, outValue["button_pattern"].resType);
    ASSERT_EQ(ResType::THEME, outValue["ohosTheme"].resType);
    ASSERT_EQ(ResType::PATTERN, outValue["text_pattern"].resType);
    ASSERT_EQ(ResType::PATTERN, outValue["list_pattern"].resType);
    ASSERT_EQ("100fp", outValue["width1"].value);
    ASSERT_EQ("100fp", outValue["width2"].value);
    ASSERT_EQ("100vp", outValue["height1"].value);
    ASSERT_EQ("100vp", outValue["height2"].value);
    ASSERT_EQ("#000000FF", outValue["textColor1"].value);
    ASSERT_EQ("#000000FF", outValue["textColor2"].value);
    ASSERT_EQ("16777333", outValue["button_pattern"].value);
    ASSERT_EQ("16777328", outValue["ohosTheme"].value);
    ASSERT_EQ("16777332", outValue["text_pattern"].value);
    ASSERT_EQ("16777331", outValue["list_pattern"].value);

    std::string rootDir = "/data/test/theme/skin/ohos.global.test.all";
    rootDirs.emplace_back(rootDir);
    tm->LoadThemeSkinResource("ohos.global.test.all", "entry", rootDirs, userId);
    rm->GetThemeDataByName("ohos_device_theme", outValue);
    ASSERT_EQ(ResType::FLOAT, outValue["width1"].resType);
    ASSERT_EQ(ResType::STRING, outValue["width2"].resType);
    ASSERT_EQ(ResType::FLOAT, outValue["height1"].resType);
    ASSERT_EQ(ResType::STRING, outValue["height2"].resType);
    ASSERT_EQ(ResType::COLOR, outValue["textColor1"].resType);
    ASSERT_EQ(ResType::STRING, outValue["textColor2"].resType);
    ASSERT_EQ("1000fp", outValue["width1"].value);
    ASSERT_EQ("100fp", outValue["width2"].value);
    ASSERT_EQ("1000vp", outValue["height1"].value);
    ASSERT_EQ("100vp", outValue["height2"].value);
    ASSERT_EQ("#000000AA", outValue["textColor1"].value);
    ASSERT_EQ("#000000FF", outValue["textColor2"].value);
    ASSERT_EQ("16777333", outValue["button_pattern"].value);
    ASSERT_EQ("16777328", outValue["ohosTheme"].value);
    ASSERT_EQ("16777332", outValue["text_pattern"].value);
    ASSERT_EQ("16777331", outValue["list_pattern"].value);
}

/*
 * @tc.name: ThemeManagerTestLoadThemeSkinResourceTest005
 * @tc.desc: Test GetPatternDataByName function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestLoadThemeSkinResourceTest005, TestSize.Level1)
{
   // success cases
    bool ret = rm->AddResource(FormatFullPath(g_hapPath).c_str());
    ASSERT_TRUE(ret);
    std::vector<std::string> rootDirs;
    int32_t userId = 100; // userId is 100
    tm->LoadThemeSkinResource("ohos.global.test.all", "entry", rootDirs, userId);
    std::map<std::string, ResourceManager::ResData> outValue;
    rm->GetPatternDataByName("ohos_test_button_pattern", outValue);
    ASSERT_EQ(ResType::STRING, outValue["width"].resType);
    ASSERT_EQ(ResType::FLOAT, outValue["height"].resType);
    ASSERT_EQ(ResType::STRING, outValue["bgColor"].resType);
    ASSERT_EQ(ResType::STRING, outValue["fgColor"].resType);
    ASSERT_EQ(ResType::PATTERN, outValue["fontPattern"].resType);
    ASSERT_EQ(ResType::STRING, outValue["marginColor"].resType);
    ASSERT_EQ(ResType::PATTERN, outValue["baseFontPattern"].resType);
    ASSERT_EQ("10vp", outValue["width"].value);
    ASSERT_EQ("66vp", outValue["height"].value);
    ASSERT_EQ("#FF0000BB", outValue["bgColor"].value);
    ASSERT_EQ("#FF0000CC", outValue["fgColor"].value);
    ASSERT_EQ("16777334", outValue["fontPattern"].value);
    ASSERT_EQ("#FF00AA00", outValue["marginColor"].value);
    ASSERT_EQ("16777335", outValue["baseFontPattern"].value);

    std::string rootDir = "/data/test/theme/skin/ohos.global.test.all";
    rootDirs.emplace_back(rootDir);
    tm->LoadThemeSkinResource("ohos.global.test.all", "entry", rootDirs, userId);
    rm->GetPatternDataByName("ohos_test_button_pattern", outValue);
    ASSERT_EQ(ResType::STRING, outValue["width"].resType);
    ASSERT_EQ(ResType::FLOAT, outValue["height"].resType);
    ASSERT_EQ(ResType::STRING, outValue["bgColor"].resType);
    ASSERT_EQ(ResType::STRING, outValue["fgColor"].resType);
    ASSERT_EQ(ResType::PATTERN, outValue["fontPattern"].resType);
    ASSERT_EQ(ResType::STRING, outValue["marginColor"].resType);
    ASSERT_EQ(ResType::PATTERN, outValue["baseFontPattern"].resType);
    ASSERT_EQ("10vp", outValue["width"].value);
    ASSERT_EQ("666vp", outValue["height"].value);
    ASSERT_EQ("#FF0000BB", outValue["bgColor"].value);
    ASSERT_EQ("#FF0000CC", outValue["fgColor"].value);
    ASSERT_EQ("16777334", outValue["fontPattern"].value);
    ASSERT_EQ("#FF00AA00", outValue["marginColor"].value);
    ASSERT_EQ("16777335", outValue["baseFontPattern"].value);
}

/*
 * @tc.name: ThemeManagerTestLoadThemeSkinResourceTest006
 * @tc.desc: Test GetPatternDataById function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestLoadThemeSkinResourceTest006, TestSize.Level1)
{
   // success cases
    bool ret = rm->AddResource(FormatFullPath(g_hapPath).c_str());
    ASSERT_TRUE(ret);
    std::vector<std::string> rootDirs;
    int32_t userId = 100; // userId is 100
    tm->LoadThemeSkinResource("ohos.global.test.all", "entry", rootDirs, userId);
    int id = rmc->GetResId("ohos_test_button_pattern", ResType::PATTERN);
    std::map<std::string, ResourceManager::ResData> outValue;
    rm->GetPatternDataById(id, outValue);
    ASSERT_EQ(ResType::STRING, outValue["width"].resType);
    ASSERT_EQ(ResType::FLOAT, outValue["height"].resType);
    ASSERT_EQ(ResType::STRING, outValue["bgColor"].resType);
    ASSERT_EQ(ResType::STRING, outValue["fgColor"].resType);
    ASSERT_EQ(ResType::PATTERN, outValue["fontPattern"].resType);
    ASSERT_EQ(ResType::STRING, outValue["marginColor"].resType);
    ASSERT_EQ(ResType::PATTERN, outValue["baseFontPattern"].resType);
    ASSERT_EQ("10vp", outValue["width"].value);
    ASSERT_EQ("66vp", outValue["height"].value);
    ASSERT_EQ("#FF0000BB", outValue["bgColor"].value);
    ASSERT_EQ("#FF0000CC", outValue["fgColor"].value);
    ASSERT_EQ("16777334", outValue["fontPattern"].value);
    ASSERT_EQ("#FF00AA00", outValue["marginColor"].value);
    ASSERT_EQ("16777335", outValue["baseFontPattern"].value);

    std::string rootDir = "/data/test/theme/skin/ohos.global.test.all";
    rootDirs.emplace_back(rootDir);
    tm->LoadThemeSkinResource("ohos.global.test.all", "entry", rootDirs, userId);
    rm->GetPatternDataById(id, outValue);
    ASSERT_EQ(ResType::STRING, outValue["width"].resType);
    ASSERT_EQ(ResType::FLOAT, outValue["height"].resType);
    ASSERT_EQ(ResType::STRING, outValue["bgColor"].resType);
    ASSERT_EQ(ResType::STRING, outValue["fgColor"].resType);
    ASSERT_EQ(ResType::PATTERN, outValue["fontPattern"].resType);
    ASSERT_EQ(ResType::STRING, outValue["marginColor"].resType);
    ASSERT_EQ(ResType::PATTERN, outValue["baseFontPattern"].resType);
    ASSERT_EQ("10vp", outValue["width"].value);
    ASSERT_EQ("666vp", outValue["height"].value);
    ASSERT_EQ("#FF0000BB", outValue["bgColor"].value);
    ASSERT_EQ("#FF0000CC", outValue["fgColor"].value);
    ASSERT_EQ("16777334", outValue["fontPattern"].value);
    ASSERT_EQ("#FF00AA00", outValue["marginColor"].value);
    ASSERT_EQ("16777335", outValue["baseFontPattern"].value);
}

/*
 * @tc.name: ThemeManagerTestLoadThemeIconsResourceTest001
 * @tc.desc: Test GetThemeIcons function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestLoadThemeIconsResourceTest001, TestSize.Level1)
{
    bool ret = rm->AddResource(FormatFullPath(g_hapPath).c_str());
    ASSERT_TRUE(ret);
    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/icons/ohos.global.test.all";
    uint32_t resId = 16777306;
    std::pair<std::unique_ptr<uint8_t[]>, size_t> foregroundInfo;
    std::pair<std::unique_ptr<uint8_t[]>, size_t> backgroundInfo;
    RState state = rm->GetThemeIcons(resId, foregroundInfo, backgroundInfo);
    EXPECT_TRUE(state == ERROR_CODE_RES_ID_NOT_FOUND);

    rootDirs.emplace_back(rootDir);
    int32_t userId = 100; // userId is 100
    tm->LoadThemeIconsResource("ohos.global.test.all", "entry", rootDirs, userId);
    state = rm->GetThemeIcons(resId, foregroundInfo, backgroundInfo);
    EXPECT_TRUE(state == SUCCESS);
    tm->LoadThemeIconsResource("ohos.global.test.all", "entry", {}, userId);
}

/*
 * @tc.name: ThemeManagerTestLoadThemeIconsResourceTest002
 * @tc.desc: Test GetThemeIcons function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestLoadThemeIconsResourceTest002, TestSize.Level1)
{
    bool ret = rm->AddResource(FormatFullPath(g_hapPath).c_str());
    ASSERT_TRUE(ret);
    uint32_t resId = 16777306;
    std::pair<std::unique_ptr<uint8_t[]>, size_t> foregroundInfo;
    std::pair<std::unique_ptr<uint8_t[]>, size_t> backgroundInfo;
    RState state = rm->GetThemeIcons(resId, foregroundInfo, backgroundInfo);
    EXPECT_EQ(state, ERROR_CODE_RES_ID_NOT_FOUND);

    std::vector<std::string> rootDirs;
    rootDirs.emplace_back("/data/test/theme/icons/dynamic_icons");
    int32_t userId = 100; // userId is 100
    tm->LoadThemeIconsResource("ohos.global.test.all", "entry", rootDirs, userId);
    state = rm->GetThemeIcons(resId, foregroundInfo, backgroundInfo);
    EXPECT_EQ(state, SUCCESS);
    tm->LoadThemeIconsResource("ohos.global.test.all", "entry", {}, userId);
}

/*
 * @tc.name: ThemeManagerTestLoadThemeIconsResourceTest003
 * @tc.desc: Test GetThemeIcons function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestLoadThemeIconsResourceTest003, TestSize.Level1)
{
    bool ret = rm->AddResource(FormatFullPath(g_hapPath).c_str());
    ASSERT_TRUE(ret);
    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/icons/ohos.global.test.all";
    uint32_t resId = 16777306;
    std::pair<std::unique_ptr<uint8_t[]>, size_t> foregroundInfo;
    std::pair<std::unique_ptr<uint8_t[]>, size_t> backgroundInfo;
    RState state = rm->GetThemeIcons(resId, foregroundInfo, backgroundInfo, 0, "ohos.global.test.all.EntryAbility");
    EXPECT_EQ(state, ERROR_CODE_RES_ID_NOT_FOUND);

    rootDirs.emplace_back(rootDir);
    int32_t userId = 100; // userId is 100
    tm->LoadThemeIconsResource("ohos.global.test.all", "entry", rootDirs, userId);
    state = rm->GetThemeIcons(resId, foregroundInfo, backgroundInfo, 0, "ohos.global.test.all.EntryAbility");
    EXPECT_EQ(state, SUCCESS);
    tm->LoadThemeIconsResource("ohos.global.test.all", "entry", {}, userId);
}

/*
 * @tc.name: ThemeManagerTestLoadThemeIconsResourceTest004
 * @tc.desc: Test FindThemeIconResource function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestLoadThemeIconsResourceTest004, TestSize.Level1)
{
    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/theme1/icons/ohos.global.test1";
    rootDirs.emplace_back(rootDir);
    int32_t userId = 100; // userId is 100
    tm->LoadThemeIconsResource("ohos.global.test1", "entry", rootDirs, userId);
    std::pair<std::string, std::string> bundleInfo;
    bundleInfo.first = "ohos.global.test1";
    std::string iconName("foreground");
    std::string iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "/data/test/theme/theme1/icons/ohos.global.test1/entry/foreground.png");
    iconName = "background";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "/data/test/theme/theme1/icons/ohos.global.test1/entry/background.png");
}

/*
 * @tc.name: ThemeManagerTestLoadThemeIconsResourceTest005
 * @tc.desc: Test FindThemeIconResource function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestLoadThemeIconsResourceTest005, TestSize.Level1)
{
    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/theme1/icons/ohos.global.test1";
    rootDirs.emplace_back(rootDir);
    int32_t userId = 100; // userId is 100
    tm->LoadThemeIconsResource("ohos.global.test1", "entry", rootDirs, userId);
    std::pair<std::string, std::string> bundleInfo;
    bundleInfo.first = "ohos.global.test1";
    std::string iconName("foreground");
    std::string iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "/data/test/theme/theme1/icons/ohos.global.test1/entry/foreground.png");
    iconName = "background";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "/data/test/theme/theme1/icons/ohos.global.test1/entry/background.png");

    rootDirs.clear();
    rootDirs.emplace_back("/data/test/theme/theme2/icons/ohos.global.test2");
    tm->LoadThemeIconsResource("ohos.global.test2", "entry", rootDirs, userId);
    bundleInfo.first = "ohos.global.test2";
    iconName = "foreground";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "/data/test/theme/theme2/icons/ohos.global.test2/entry/foreground.png");
    iconName = "background";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "/data/test/theme/theme2/icons/ohos.global.test2/entry/background.png");
}

/*
 * @tc.name: ThemeManagerTestLoadThemeIconsResourceTest006
 * @tc.desc: Test FindThemeIconResource function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestLoadThemeIconsResourceTest006, TestSize.Level1)
{
    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/theme1/icons/ohos.global.test1";
    rootDirs.emplace_back(rootDir);
    int32_t userId = 100; // userId is 100
    tm->LoadThemeIconsResource("ohos.global.test1", "entry", rootDirs, userId);
    std::pair<std::string, std::string> bundleInfo;
    bundleInfo.first = "ohos.global.test1";
    std::string iconName("foreground");
    std::string iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "/data/test/theme/theme1/icons/ohos.global.test1/entry/foreground.png");
    iconName = "background";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "/data/test/theme/theme1/icons/ohos.global.test1/entry/background.png");

    rootDirs.clear();
    rootDirs.emplace_back("/data/test/theme/theme2/icons/ohos.global.test2");
    tm->LoadThemeIconsResource("ohos.global.test2", "entry", rootDirs, userId);
    iconName = "foreground";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "");
    iconName = "background";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "");
}

/*
 * @tc.name: ThemeManagerTestLoadThemeIconsResourceTest007
 * @tc.desc: Test FindThemeIconResource function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestLoadThemeIconsResourceTest007, TestSize.Level1)
{
    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/theme1/icons";
    rootDirs = tm->GetRootDir(rootDir);
    int32_t userId = 100; // userId is 100
    tm->LoadThemeIconsResource("ohos.global.test1", "entry", rootDirs, userId);
    std::pair<std::string, std::string> bundleInfo;
    bundleInfo.first = "ohos.global.test1";
    std::string iconName("foreground");
    std::string iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "/data/test/theme/theme1/icons/ohos.global.test1/entry/foreground.png");
    iconName = "background";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "/data/test/theme/theme1/icons/ohos.global.test1/entry/background.png");

    tm->LoadThemeIconRes("ohos.global.test1", "entry", userId);
    iconName = "foreground";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "");
    iconName = "background";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "");
}

/*
 * @tc.name: ThemeManagerTestLoadThemeIconsResourceTest008
 * @tc.desc: Test FindThemeIconResource function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestLoadThemeIconsResourceTest008, TestSize.Level1)
{
    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/theme1/icons";
    rootDirs = tm->GetRootDir(rootDir);
    int32_t userId = 100; // userId is 100
    tm->LoadThemeIconsResource("ohos.global.test1", "entry", rootDirs, userId);
    std::pair<std::string, std::string> bundleInfo;
    bundleInfo.first = "ohos.global.test1";
    std::string iconName("foreground");
    std::string iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "/data/test/theme/theme1/icons/ohos.global.test1/entry/foreground.png");
    iconName = "background";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "/data/test/theme/theme1/icons/ohos.global.test1/entry/background.png");

    tm->LoadThemeIconRes("ohos.global.test1", "entry", userId);
    iconName = "foreground";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "");
    iconName = "background";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "");

    rootDirs.clear();
    rootDirs = tm->GetRootDir("/data/test/theme/theme2/icons");
    tm->LoadThemeIconsResource("ohos.global.test2", "entry", rootDirs, userId);
    bundleInfo.first = "ohos.global.test2";
    iconName = "foreground";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "/data/test/theme/theme2/icons/ohos.global.test2/entry/foreground.png");
    iconName = "background";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "/data/test/theme/theme2/icons/ohos.global.test2/entry/background.png");
}

/*
 * @tc.name: ThemeManagerTestChangeIconResourceStatusTest001
 * @tc.desc: Test ChangeIconResourceStatus function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestChangeIconResourceStatusTest001, TestSize.Level1)
{
    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/theme1/icons";
    rootDirs = tm->GetRootDir(rootDir);
    int32_t userId = 100; // userId is 100
    tm->LoadThemeIconsResource("ohos.global.test1", "entry", rootDirs, userId);
    std::pair<std::string, std::string> bundleInfo;
    bundleInfo.first = "ohos.global.test1";
    std::string iconName("foreground");
    std::string iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "/data/test/theme/theme1/icons/ohos.global.test1/entry/foreground.png");
    iconName = "background";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "/data/test/theme/theme1/icons/ohos.global.test1/entry/background.png");

    tm->ChangeIconResourceStatus(userId);
    tm->ClearIconResource();

    iconName = "foreground";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "");
    iconName = "background";
    iconPath = tm->FindThemeIconResource(bundleInfo, iconName, userId);
    EXPECT_EQ(iconPath, "");
}

/*
 * @tc.name: ThemeManagerTestHasIconInThemeTest001
 * @tc.desc: Test HasIconInTheme function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestHasIconInThemeTest001, TestSize.Level1)
{
    bool ret = rm->AddResource(FormatFullPath(g_hapPath).c_str());
    ASSERT_TRUE(ret);
    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/icons/ohos.global.test.all";
    rootDirs.emplace_back(rootDir);
    int32_t userId = 100; // userId is 100
    tm->LoadThemeIconsResource("ohos.global.test.all", "entry", rootDirs, userId);
    bool result = rm->HasIconInTheme("ohos.global.test");
    EXPECT_TRUE(result == false);

    result = rm->HasIconInTheme("ohos.global.test.all");
    EXPECT_TRUE(result == true);
}

/*
 * @tc.name: ThemeManagerTestGetOtherIconsInfoTest001
 * @tc.desc: Test GetOtherIconsInfo function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestGetOtherIconsInfoTest001, TestSize.Level1)
{
    bool ret = rm->AddResource(FormatFullPath(g_hapPath).c_str());
    ASSERT_TRUE(ret);
    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/icons/other_icons";
    rootDirs.emplace_back(rootDir);
    int32_t userId = 100; // userId is 100
    tm->LoadThemeIconsResource("other_icons", "", rootDirs, userId);
    std::unique_ptr<uint8_t[]> outValue;
    size_t len;
    RState state = rm->GetOtherIconsInfo("other_icons", outValue, len, true);
    EXPECT_EQ(state, ERROR_CODE_RES_NOT_FOUND_BY_NAME);

    state = rm->GetOtherIconsInfo("foreground", outValue, len, false);
    EXPECT_EQ(state, SUCCESS);
}

/*
 * @tc.name: ThemeManagerTestGetOtherIconsInfoTest002
 * @tc.desc: Test GetOtherIconsInfo function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestGetOtherIconsInfoTest002, TestSize.Level1)
{
    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/icons/";
    rootDirs = tm->GetRootDir(rootDir);
    int32_t userId = 100; // userId is 100
    tm->LoadThemeIconsResource("other_icons", "", rootDirs, userId);
    std::unique_ptr<uint8_t[]> outValue;
    size_t len;
    RState state = rm->GetOtherIconsInfo("icon_mask", outValue, len, true);
    EXPECT_EQ(state, SUCCESS);

    state = rm->GetOtherIconsInfo("icon_mask", outValue, len, false);
    EXPECT_EQ(state, ERROR_CODE_RES_NOT_FOUND_BY_NAME);
}

/*
 * @tc.name: ThemeManagerTestGetOtherIconsInfoTest003
 * @tc.desc: Test GetOtherIconsInfo function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestGetOtherIconsInfoTest003, TestSize.Level1)
{
    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/icons/";
    rootDirs = tm->GetRootDir(rootDir);
    int32_t userId = 100; // userId is 100
    tm->LoadThemeIconsResource("other_icons", "", rootDirs, userId);
    std::unique_ptr<uint8_t[]> outValue;
    size_t len;
    RState state = rm->GetOtherIconsInfo("icon_mask", outValue, len, true);
    EXPECT_EQ(state, SUCCESS);

    RState state1 = tm->GetThemeIconFromCache("global_icon_mask", outValue, len);
    EXPECT_EQ(state1, SUCCESS);

    RState state2 = rm->GetOtherIconsInfo("icon_mask", outValue, len, true);
    EXPECT_EQ(state2, SUCCESS);
}

/*
 * @tc.name: ThemeManagerTestGetThemeIconFromCacheTest001
 * @tc.desc: Test GetThemeIconFromCache function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestGetThemeIconFromCacheTest001, TestSize.Level1)
{
    bool ret = rm->AddResource(FormatFullPath(g_hapPath).c_str());
    ASSERT_TRUE(ret);
    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/icons/other_icons";
    rootDirs.emplace_back(rootDir);
    int32_t userId = 100; // userId is 100
    tm->LoadThemeIconsResource("other_icons", "", rootDirs, userId);
    std::unique_ptr<uint8_t[]> outValue;
    size_t len;
    RState state = tm->GetThemeIconFromCache("other_icons_background", outValue, len);
    EXPECT_EQ(state, NOT_FOUND);
    state = tm->GetOtherIconsInfo("background", outValue, len, false, userId);
    EXPECT_EQ(state, SUCCESS);
    state = tm->GetThemeIconFromCache("other_icons_background", outValue, len);
    EXPECT_EQ(state, SUCCESS);
}

/*
 * @tc.name: ThemeManagerTestGetThemeIconFromCacheTest002
 * @tc.desc: Test GetThemeIconFromCache function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestGetThemeIconFromCacheTest002, TestSize.Level1)
{
    std::vector<std::string> rootDirs;
    std::string rootDir = "/data/test/theme/icons/other_icons";
    rootDirs.emplace_back(rootDir);
    int32_t userId = 100; // userId is 100
    tm->LoadThemeIconsResource("other_icons", "", rootDirs, userId);
    std::unique_ptr<uint8_t[]> outValue;
    size_t len;
    RState state = tm->GetOtherIconsInfo("background", outValue, len, false, userId);
    EXPECT_EQ(state, SUCCESS);
    state = tm->GetThemeIconFromCache("global_icon_mask", outValue, len);
    EXPECT_EQ(state, NOT_FOUND);
    state = tm->GetOtherIconsInfo("icon_mask", outValue, len, true, userId);
    EXPECT_EQ(state, SUCCESS);
    state = tm->GetThemeIconFromCache("global_icon_mask", outValue, len);
    EXPECT_EQ(state, SUCCESS);
}

/*
 * @tc.name: ThemeManagerTestIsUpdateByUserIdTest001
 * @tc.desc: Test IsUpdateByUserId function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestIsUpdateByUserIdTest001, TestSize.Level1)
{
    int32_t userId = 100;
    tm->UpdateUserId(userId);
    bool result = tm->IsUpdateByUserId(userId);
    ASSERT_FALSE(result);

    userId = 101;
    result = tm->IsUpdateByUserId(userId);
    ASSERT_TRUE(result);
}

/*
 * @tc.name: ThemeManagerTestIsSameResourceByUserIdTest001
 * @tc.desc: Test IsSameResourceByUserId function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestIsSameResourceByUserIdTest001, TestSize.Level1)
{
    std::string path("/data/service/el1/public/themes/100/a/app/icons/ohos.global.test2/entry/foreground.png");
    int32_t userId = 100;
    bool result = tm->IsSameResourceByUserId(path, userId);
    EXPECT_TRUE(result == true);

    userId = 101;
    result = tm->IsSameResourceByUserId(path, userId);
    EXPECT_TRUE(result == false);
}

/*
 * @tc.name: ThemeManagerTestIsSameResourceByUserIdTest002
 * @tc.desc: Test IsSameResourceByUserId function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestIsSameResourceByUserIdTest002, TestSize.Level1)
{
    std::string path("/data/service/el1/public/themes/100");
    int32_t userId = 100;
    bool result = tm->IsSameResourceByUserId(path, userId);
    EXPECT_TRUE(result);
}

/*
 * @tc.name: ThemeManagerTestIsSameResourceByUserIdTest003
 * @tc.desc: Test IsSameResourceByUserId function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestIsSameResourceByUserIdTest003, TestSize.Level1)
{
    std::string path("/data/service/el1/public/themes/abc/");
    int32_t userId = 100;
    bool result = tm->IsSameResourceByUserId(path, userId);
    EXPECT_TRUE(result);
}

/*
 * @tc.name: ThemeManagerTestGetMaskStringTest001
 * @tc.desc: Test GetMaskString function, file case.
 * @tc.type: FUNC
 */
HWTEST_F(ThemeManagerTest, ThemeManagerTestGetMaskStringTest001, TestSize.Level1)
{
    std::string path("/data/service/el1/public/themes/100/a/app/icons/ohos.global.test2/entry/foreground.png");
    std::string maskPath = tm->GetMaskString(path);
    EXPECT_EQ(maskPath, "100/a/app/icons/ohos.global.test2/entry/foreground.png");

    path = "/data/test/theme/theme2/icons/ohos.global.test2/entry/foreground.png";
    maskPath = tm->GetMaskString(path);
    EXPECT_EQ(maskPath, path);
}
}
