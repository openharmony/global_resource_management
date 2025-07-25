/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef RESOURCE_MANAGER_TEST_COMMON_H
#define RESOURCE_MANAGER_TEST_COMMON_H

#include "hap_resource.h"
#include "hap_resource_v1.h"
#include "hap_resource_v2.h"
#include "hilog_wrapper.h"

#include <unicode/localebuilder.h>
#include <unicode/locid.h>
#include <unicode/utypes.h>
#include <map>
#include <string>
#include <vector>

namespace OHOS {
namespace Global {
namespace Resource {
// this is relative path
static const char *g_resFilePath = "all/assets/entry/resources.index";
static const char *g_newResFilePath = "all/assets/entry/resourcesV2.index";
static const char *g_hapPath = "all.hap";
static const char *g_newModuleHapPath = "newModule.hap";
static const char *g_nonExistName = "non_existent_name";
static const char *g_overlayResFilePath = "overlay/assets/entry/resources.index";
static const char *g_newOverlayResFilePath = "overlay/assets/entry/resourcesV2.index";
static const float BASE_DPI = 160.0f;
static const int NON_EXIST_ID = 1111;
static const char *HAP_PATH_FA = "all_fa.hap";
std::string FormatFullPath(const char *fileRelativePath);

void PrintIdValues(const std::shared_ptr<IdValues> &idValues);

void PrintMapString(const std::map<std::string, std::string> &value);

void PrintVectorString(const std::vector<std::string> &value);

ResConfig *CreateResConfig(const char *language, const char *script, const char *region);

Locale GetLocale(const char *language, const char *script, const char *region);

std::shared_ptr<ResConfigImpl> InitDefaultResConfig();
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif
