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
#include "hap_manager.h"
#include "hap_resource_manager.h"

#include <algorithm>
#include <fstream>
#include <climits>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <tuple>
#include <set>
#include "utils/errors.h"
#ifdef SUPPORT_GRAPHICS
#include <ohos/init_data.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>
#endif

#include "hilog_wrapper.h"

#include "hap_parser.h"
#include "utils/utils.h"
#include "res_common.h"

#ifdef __WINNT__
#include <shlwapi.h>
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#if !defined(__WINNT__) && !defined(__IDE_PREVIEW__) && !defined(__ARKUI_CROSS__)
#include "hitrace_meter.h"
#include "hisysevent_adapter.h"
#include "file_mapper.h"
#include "extractor.h"
#endif

namespace OHOS {
namespace Global {
namespace Resource {
#ifdef SUPPORT_GRAPHICS
constexpr uint32_t PLURAL_CACHE_MAX_COUNT = 3;
#endif
#if defined(__ARKUI_CROSS__) || defined(__WINNT__) || defined(__IDE_PREVIEW__)
const std::string RAW_FILE_PATH = "resources/rawfile/";
#endif

using ReadLock = std::shared_lock<std::shared_mutex>;
using WriteLock = std::unique_lock<std::shared_mutex>;

std::mutex g_rawFileLock;

HapManager::HapManager(std::shared_ptr<ResConfigImpl> resConfig, bool isSystem)
    : resConfig_(resConfig), isSystem_(isSystem)
{
    overrideResConfig_->SetColorMode(COLOR_MODE_NOT_SET);
}

HapManager::HapManager(std::shared_ptr<ResConfigImpl> resConfig, std::vector<std::shared_ptr<HapResource>> hapResources,
    std::unordered_map<std::string, std::vector<std::string>> loadedHapPaths, bool isSystem)
    : resConfig_(resConfig), hapResources_(hapResources), loadedHapPaths_(loadedHapPaths), isSystem_(isSystem)
{
    overrideResConfig_->SetColorMode(COLOR_MODE_NOT_SET);
}

bool HapManager::icuInitialized = HapManager::Init();

bool HapManager::Init()
{
#ifdef SUPPORT_GRAPHICS
#ifdef __IDE_PREVIEW__
#ifdef __WINNT__
    MEMORY_BASIC_INFORMATION mbi;
    if (::VirtualQuery((LPCVOID)SetHwIcuDirectory, &mbi, sizeof(mbi)) != 0) {
        char path[MAX_PATH] = { 0 };
        GetModuleFileName((HMODULE)mbi.AllocationBase, path, MAX_PATH);
        std::string tempPath(path);
        auto pos = tempPath.rfind('\\');
        if (pos != std::string::npos) {
            u_setDataDirectory(tempPath.substr(0, pos).c_str());
        }
    }
#else
    Dl_info info;
    if (dladdr((void*)SetHwIcuDirectory, &info) != 0) {
        std::string tempPath(info.dli_fname);
        auto pos = tempPath.rfind('/');
        if (pos != std::string::npos) {
            u_setDataDirectory(tempPath.substr(0, pos).c_str());
        }
    }
#endif
#else
#if !defined(__ARKUI_CROSS__)
    SetHwIcuDirectory();
#endif
#endif
#endif
    return true;
}

std::string HapManager::GetPluralRulesAndSelect(ResourceManager::Quantity quantity, bool isGetOverrideResource)
{
    std::string defaultRet("other");
#ifdef SUPPORT_GRAPHICS
    WriteLock lock(this->mutex_);
    std::shared_ptr<ResConfigImpl> config = getCompleteOverrideConfig(isGetOverrideResource);
    if (config == nullptr || config->GetResLocale() == nullptr ||
        config->GetResLocale()->GetLanguage() == nullptr) {
        RESMGR_HILOGE(RESMGR_TAG, "GetPluralRules language is null!");
        return defaultRet;
    }
    std::string language = config->GetResLocale()->GetLanguage();

    icu::PluralRules *pluralRules = nullptr;
    for (uint32_t i = 0; i < plurRulesCache_.size(); i++) {
        auto pair = plurRulesCache_[i];
        if (language == pair.first) {
            // cache hit
            pluralRules = pair.second;
            break;
        }
    }

    if (pluralRules == nullptr) {
        // no cache hit
        icu::Locale locale(language.c_str());
        if (locale.isBogus()) {
            RESMGR_HILOGE(RESMGR_TAG, "icu::Locale init error : %s", language.c_str());
            return defaultRet;
        }
        UErrorCode status = U_ZERO_ERROR;
        pluralRules = icu::PluralRules::forLocale(locale, status);
        if (status != U_ZERO_ERROR) {
            RESMGR_HILOGE(RESMGR_TAG, "icu::PluralRules::forLocale error : %d", status);
            return defaultRet;
        }
        // after PluralRules created, we add it to cache, if > 3 delete oldest one
        if (plurRulesCache_.size() >= PLURAL_CACHE_MAX_COUNT) {
            RESMGR_HILOGD(RESMGR_TAG, "cache rotate delete plurRulesMap_ %s", plurRulesCache_[0].first.c_str());
            delete (plurRulesCache_[0].second);
            plurRulesCache_.erase(plurRulesCache_.begin());
        }
        auto plPair = std::make_pair(language, pluralRules);
        plurRulesCache_.push_back(plPair);
    }
    std::string converted;
    icu::UnicodeString us = quantity.isInteger ? pluralRules->select(quantity.intValue) :
        pluralRules->select(quantity.doubleValue);
    us.toUTF8String(converted);
    return converted;
#else
    return defaultRet;
#endif
}

const std::shared_ptr<IdItem> HapManager::FindResourceById(uint32_t id, bool isGetOverrideResource)
{
    auto qualifierValue = FindQualifierValueById(id, isGetOverrideResource);
    if (qualifierValue == nullptr) {
        return nullptr;
    }
    return qualifierValue->GetIdItem();
}

const std::shared_ptr<IdItem> HapManager::FindResourceByName(
    const char *name, const ResType resType, bool isGetOverrideResource)
{
    auto qualifierValue = FindQualifierValueByName(name, resType, isGetOverrideResource);
    if (qualifierValue == nullptr) {
        return nullptr;
    }
    return qualifierValue->GetIdItem();
}

const std::shared_ptr<ValueUnderQualifierDir> HapManager::FindQualifierValueByName(
    const char *name, const ResType resType, bool isGetOverrideResource, uint32_t density)
{
    ReadLock lock(this->mutex_);
    std::vector<std::shared_ptr<IdValues>> candidates = this->GetResourceListByName(name, resType);
    if (candidates.size() == 0) {
        return nullptr;
    }
    return this->GetBestMatchResource(candidates, density, isGetOverrideResource);
}

const std::shared_ptr<ValueUnderQualifierDir> HapManager::FindQualifierValueById(uint32_t id,
    bool isGetOverrideResource, uint32_t density)
{
    ReadLock lock(this->mutex_);
    std::vector<std::shared_ptr<IdValues>> candidates = this->GetResourceList(id);
    if (candidates.size() == 0) {
        return nullptr;
    }
    return this->GetBestMatchResource(candidates, density, isGetOverrideResource);
}

std::shared_ptr<ResConfigImpl> HapManager::getCompleteOverrideConfig(bool isGetOverrideResource)
{
    if (!isGetOverrideResource) {
        return this->resConfig_;
    }

    std::shared_ptr<ResConfigImpl> completeOverrideConfig = std::make_shared<ResConfigImpl>();
    if (!completeOverrideConfig || !this->resConfig_ || !this->overrideResConfig_) {
        RESMGR_HILOGE(RESMGR_TAG, "completeOverrideConfig or resConfig_ or overrideResConfig_ is nullptr");
        return nullptr;
    }

    if (!completeOverrideConfig->Copy(*this->resConfig_, true)) {
        RESMGR_HILOGE(RESMGR_TAG, "getCompleteOverrideConfig copy failed");
        return nullptr;
    }

    if (this->overrideResConfig_->isLocaleInfoSet()
        && !completeOverrideConfig->CopyLocaleAndPreferredLocale(*this->overrideResConfig_)) {
        RESMGR_HILOGE(RESMGR_TAG, "getCompleteOverrideConfig CopyLocaleAndPreferredLocale failed");
        return nullptr;
    }
    if (this->overrideResConfig_->GetDeviceType() != DEVICE_NOT_SET) {
        completeOverrideConfig->SetDeviceType(this->overrideResConfig_->GetDeviceType());
    }
    if (this->overrideResConfig_->GetDirection() != DIRECTION_NOT_SET) {
        completeOverrideConfig->SetDirection(this->overrideResConfig_->GetDirection());
    }
    if (this->overrideResConfig_->GetColorMode() != COLOR_MODE_NOT_SET) {
        completeOverrideConfig->SetColorMode(this->overrideResConfig_->GetColorMode());
    }
    if (this->overrideResConfig_->GetInputDevice() != INPUTDEVICE_NOT_SET) {
        completeOverrideConfig->SetInputDevice(this->overrideResConfig_->GetInputDevice());
    }
    if (this->overrideResConfig_->GetMcc() != MCC_UNDEFINED) {
        completeOverrideConfig->SetMcc(this->overrideResConfig_->GetMcc());
    }
    if (this->overrideResConfig_->GetMnc() != MNC_UNDEFINED) {
        completeOverrideConfig->SetMnc(this->overrideResConfig_->GetMnc());
    }
    if (this->overrideResConfig_->GetScreenDensity() != SCREEN_DENSITY_NOT_SET) {
        completeOverrideConfig->SetScreenDensity(this->overrideResConfig_->GetScreenDensity());
    }
    return completeOverrideConfig;
}

void HapManager::MatchBestResource(std::shared_ptr<ResConfigImpl> &bestResConfig,
    std::shared_ptr<ValueUnderQualifierDir> &result,
    const std::vector<std::shared_ptr<ValueUnderQualifierDir>> &paths,
    uint32_t density, std::shared_ptr<ResConfigImpl> currentResConfig)
{
    size_t len = paths.size();
    size_t i = 0;
    for (i = 0; i < len; i++) {
        std::shared_ptr<ValueUnderQualifierDir> path = paths[i];
        const auto resConfig = path->GetResConfig();
        if (!currentResConfig->Match(resConfig)) {
            continue;
        }
        if (bestResConfig == nullptr) {
            bestResConfig = resConfig;
            result = paths[i];
            continue;
        }
        if (!bestResConfig->IsMoreSuitable(resConfig, currentResConfig, density)) {
            bestResConfig = resConfig;
            result = paths[i];
        }
    }
}

const std::shared_ptr<ValueUnderQualifierDir> HapManager::GetBestMatchResource(
    const std::vector<std::shared_ptr<IdValues>> &candidates, uint32_t density, bool isGetOverrideResource)
{
    std::shared_ptr<ResConfigImpl> bestResConfig = nullptr;
    std::shared_ptr<ResConfigImpl> bestOverlayResConfig = nullptr;
    std::shared_ptr<ValueUnderQualifierDir> result = nullptr;
    std::shared_ptr<ValueUnderQualifierDir> overlayResult = nullptr;
    const std::shared_ptr<ResConfigImpl> currentResConfig = getCompleteOverrideConfig(isGetOverrideResource);
    if (!currentResConfig) {
        return nullptr;
    }
    // When there are multiple overlays, reverse the search to find the first match resource.
    for (auto iter = candidates.rbegin(); iter != candidates.rend(); iter++) {
        const auto &paths = (*iter)->GetLimitPathsConst();
        bool isOverlayHapResource = paths[0]->IsOverlayResource();
        if (isOverlayHapResource) {
            MatchBestResource(bestOverlayResConfig, overlayResult, paths, density, currentResConfig);
        } else {
            MatchBestResource(bestResConfig, result, paths, density, currentResConfig);
        }
    }
    if (bestOverlayResConfig != nullptr && result != nullptr) {
        if (bestOverlayResConfig->IsMoreSuitable(bestResConfig, currentResConfig, density)) {
            return overlayResult;
        }
    }
    return result;
}

RState HapManager::FindRawFile(const std::string &name, std::string &outValue)
{
#ifdef __WINNT__
    char seperator = '\\';
#else
    char seperator = '/';
#endif
    ReadLock lock(this->mutex_);
    for (auto iter = hapResources_.rbegin(); iter != hapResources_.rend(); iter++) {
        std::string indexPath = (*iter)->GetIndexPath();
        auto index = indexPath.rfind(seperator);
        if (index == std::string::npos) {
            RESMGR_HILOGE(RESMGR_TAG, "index path format error by seperator");
            continue;
        }
        std::string resourcesIndexPath = indexPath.substr(0, index);
        char tmpPath[PATH_MAX] = {0};
        std::string tempName = name;
        const std::string rawFileDirName = "rawfile/";
        if (tempName.length() <= rawFileDirName.length()
            || (tempName.compare(0, rawFileDirName.length(), rawFileDirName) != 0)) {
            tempName = rawFileDirName + tempName;
        }
#ifdef __WINNT__
        if (!PathCanonicalizeA(tmpPath, (resourcesIndexPath + "/resources/" + tempName).c_str())) {
            continue;
        }
#else
        if (realpath((resourcesIndexPath + "/resources/" + tempName).c_str(), tmpPath) == nullptr) {
            RESMGR_HILOGE(RESMGR_TAG, "FindRawFile path to realpath error");
            continue;
        }
#endif
        const std::string realPath = tmpPath;
        std::fstream inputFile;
        inputFile.open(realPath, std::ios::in);
        if (inputFile) {
            outValue = realPath;
            inputFile.close();
            return SUCCESS;
        }
    }
    return ERROR_CODE_RES_PATH_INVALID;
}

RState HapManager::UpdateResConfig(ResConfig &resConfig)
{
    WriteLock lock(this->mutex_);
    bool needUpdate = !this->resConfig_->MatchLocal(resConfig);
    this->resConfig_->Copy(resConfig);
    if (needUpdate) {
        for (auto &resource : hapResources_) {
            RState state = resource->Update(this->resConfig_);
            if (state != SUCCESS) {
                return state;
            }
        }
    }
    return SUCCESS;
}

RState HapManager::UpdateOverrideResConfig(ResConfig &resConfig)
{
    WriteLock lock(this->mutex_);
    bool needUpdate = !this->overrideResConfig_->MatchLocal(resConfig);
    this->overrideResConfig_->Copy(resConfig);
    if (needUpdate) {
        for (auto &resource : hapResources_) {
            RState state = resource->Update(this->overrideResConfig_);
            if (state != SUCCESS) {
                return state;
            }
        }
    }
    return SUCCESS;
}

void HapManager::GetResConfig(ResConfig &resConfig)
{
    ReadLock lock(this->mutex_);
    resConfig.Copy(*(this->resConfig_), true);
}

RState HapManager::GetResConfigById(uint32_t resId, ResConfig &resConfig, bool isGetOverrideResource, uint32_t density)
{
    auto qualifierValue = FindQualifierValueById(resId, isGetOverrideResource, density);
    if (qualifierValue == nullptr) {
        return ERROR_CODE_RES_NOT_FOUND_BY_ID;
    }
    resConfig.Copy(*(qualifierValue->GetResConfig()), true);
    return SUCCESS;
}

RState HapManager::GetResConfigByName(const std::string &name, const ResType type, ResConfig &resConfig,
    bool isGetOverrideResource, uint32_t density)
{
    auto qualifierValue = FindQualifierValueByName(name.c_str(), type, isGetOverrideResource, density);
    if (qualifierValue == nullptr) {
        return ERROR_CODE_RES_NOT_FOUND_BY_NAME;
    }
    resConfig.Copy(*(qualifierValue->GetResConfig()), true);
    return SUCCESS;
}

void HapManager::GetOverrideResConfig(ResConfig &resConfig)
{
    ReadLock lock(this->mutex_);
    resConfig.Copy(*(this->overrideResConfig_));
}

bool HapManager::AddResource(const char *path, const uint32_t &selectedTypes, bool forceReload)
{
    WriteLock lock(this->mutex_);
    std::string sPath(path);
#if defined(__ARKUI_CROSS__)
    if (forceReload) {
        HapResourceManager::GetInstance().RemoveHapResource(sPath);
        RemoveHapResource(sPath);
    }
#endif
    auto it = loadedHapPaths_.find(sPath);
    if (it != loadedHapPaths_.end()) {
        return false;
    }
    std::shared_ptr<ResConfigImpl> config = getCompleteOverrideConfig(isOverride_);
    std::shared_ptr<HapResource> pResource =
        HapResourceManager::GetInstance().Load(path, config, isSystem_, false, selectedTypes);
    if (pResource == nullptr) {
        return false;
    }
    this->loadedHapPaths_[sPath] = std::vector<std::string>();
    this->hapResources_.push_back(pResource);
    if (pResource->HasDarkRes()) {
        this->resConfig_->SetAppDarkRes(true);
    }
    if (!pResource->IsSystemResource() && !pResource->IsOverlayResource()
        && pResource->IsThemeSystemResEnable()) {
        this->isThemeSystemResEnable_ = true;
    }
    return true;
}

bool HapManager::AddPatchResource(const char *path, const char *patchPath)
{
    WriteLock lock(this->mutex_);
    std::string sPath(path);
    auto it = loadedHapPaths_.find(sPath);
    if (it == loadedHapPaths_.end()) {
        RESMGR_HILOGW(RESMGR_TAG, "AddPatchResource hapPath not load");
        return false;
    }
    std::string sPatchPath(patchPath);
    return HapResourceManager::GetInstance().PutPatchResource(sPath, sPatchPath);
}

bool HapManager::AddResource(const std::string &path, const std::vector<std::string> &overlayPaths)
{
    WriteLock lock(this->mutex_);
    std::vector<std::string> targetOverlay = loadedHapPaths_[path];
    if (!targetOverlay.empty() && targetOverlay == overlayPaths) {
        RESMGR_HILOGI(RESMGR_TAG, "the overlay hap already been loaded");
        return true;
    }
    std::shared_ptr<ResConfigImpl> config = getCompleteOverrideConfig(isOverride_);
    std::unordered_map<std::string, std::shared_ptr<HapResource>> result =
        HapResourceManager::GetInstance().LoadOverlays(path, overlayPaths, config, isSystem_);
    if (result.size() == 0) {
        return false;
    }
    if (result.find(path) != result.end()) {
        hapResources_.push_back(result[path]);
        if (result[path]->HasDarkRes()) {
            this->resConfig_->SetAppDarkRes(true);
        }
        if (!result[path]->IsSystemResource() && !result[path]->IsOverlayResource()
            && result[path]->IsThemeSystemResEnable()) {
            this->isThemeSystemResEnable_ = true;
        }
    }
    for (auto iter = overlayPaths.rbegin(); iter != overlayPaths.rend(); iter++) {
        if (result.find(*iter) != result.end()) {
            hapResources_.push_back(result[*iter]);
        }
    }
    loadedHapPaths_[path] = overlayPaths;
    return true;
}

std::string HapManager::GetValidAppPath()
{
    std::string appPath;
    ReadLock lock(this->mutex_);
    for (auto iter = hapResources_.rbegin(); iter != hapResources_.rend(); iter++) {
        const std::string tempPath = (*iter)->GetIndexPath();
        if ((*iter)->IsSystemResource() || (*iter)->IsOverlayResource()) {
            continue;
        }
        appPath = tempPath;
    }
    return appPath;
}

bool HapManager::AddAppOverlay(const std::string &overlayPath)
{
    char outPath[PATH_MAX + 1] = {0};
    Utils::CanonicalizePath(overlayPath.c_str(), outPath, PATH_MAX);
    if (outPath[0] == '\0') {
        RESMGR_HILOGE(RESMGR_TAG, "AddAppOverlay invalid overlayPath");
        return false;
    }
    std::vector<std::string> overlayPaths;
    overlayPaths.emplace_back(outPath);
    std::string appPath = GetValidAppPath();
    return AddResource(appPath, overlayPaths);
}

bool HapManager::RemoveAppOverlay(const std::string &overlayPath)
{
    char outPath[PATH_MAX + 1] = {0};
    Utils::CanonicalizePath(overlayPath.c_str(), outPath, PATH_MAX);
    if (outPath[0] == '\0') {
        RESMGR_HILOGE(RESMGR_TAG, "RemoveAppOverlay invalid overlayPath");
        return false;
    }
    std::vector<std::string> overlayPaths;
    overlayPaths.emplace_back(outPath);
    std::string appPath = GetValidAppPath();
    return RemoveResource(appPath, overlayPaths);
}

HapManager::~HapManager()
{
#ifndef SUPPORT_ASAN
    WriteLock lock(this->mutex_);
    hapResources_.clear();
    loadedHapPaths_.clear();
#endif

#ifdef SUPPORT_GRAPHICS
    auto iter = plurRulesCache_.begin();
    for (; iter != plurRulesCache_.end(); iter++) {
        RESMGR_HILOGD(RESMGR_TAG, "delete plurRulesMap_ %s", iter->first.c_str());
        if (iter->second != nullptr) {
            auto ptr = iter->second;
            delete (ptr);
            iter->second = nullptr;
        }
    }
#endif
}

std::vector<std::shared_ptr<IdValues>> HapManager::GetResourceList(uint32_t ident) const
{
    std::vector<std::shared_ptr<IdValues>> result;
    // one id only exit in one hap
    for (size_t i = 0; i < hapResources_.size(); ++i) {
        std::shared_ptr<HapResource> pResource = hapResources_[i];
        const std::shared_ptr<IdValues>out = pResource->GetIdValues(ident);
        if (out != nullptr) {
            result.emplace_back(out);
        }
    }
    return result;
}

std::vector<std::shared_ptr<IdValues>> HapManager::GetResourceListByName(const char *name,
    const ResType resType) const
{
    std::vector<std::shared_ptr<IdValues>> result;
    // all match will return
    if (name == nullptr) {
        return result;
    }
    std::string key(name);
    for (size_t i = 0; i < hapResources_.size(); ++i) {
        std::shared_ptr<HapResource> pResource = hapResources_[i];
        const std::shared_ptr<IdValues> out = pResource->GetIdValuesByName(key, resType);
        if (out != nullptr) {
            result.emplace_back(out);
        }
    }
    return result;
}

#if defined(__ARKUI_CROSS__)
void HapManager::RemoveHapResource(const std::string &path)
{
    for (auto iter = hapResources_.begin(); iter != hapResources_.end();) {
        if ((*iter)->GetIndexPath() == path) {
            iter = hapResources_.erase(iter);
            break;
        } else {
            ++iter;
        }
    }

    auto it = loadedHapPaths_.find(path);
    if (it != loadedHapPaths_.end()) {
        loadedHapPaths_.erase(it);
    }
}
#endif

std::vector<std::string> HapManager::GetResourcePaths()
{
    std::vector<std::string> result;
    ReadLock lock(this->mutex_);
    for (auto iter = hapResources_.rbegin(); iter != hapResources_.rend(); iter++) {
        std::string indexPath = (*iter)->GetIndexPath();
        auto index = indexPath.rfind('/');
        if (index == std::string::npos) {
            RESMGR_HILOGE(RESMGR_TAG, "index path format error");
            continue;
        }

        result.emplace_back(indexPath.substr(0, index) + "/resources/");
    }

    return result;
}

std::string GetImageType(const std::string fileName)
{
    auto pos = fileName.find_last_of('.');
    std::string imgType;
    if (pos != std::string::npos) {
        imgType = fileName.substr(pos + 1);
    }
    return imgType;
}

#if !defined(__WINNT__) && !defined(__IDE_PREVIEW__) && !defined(__ARKUI_CROSS__)
std::string GetFilePathFromHap(std::shared_ptr<AbilityBase::Extractor> &extractor,
    const std::shared_ptr<ValueUnderQualifierDir> qd, const ResType resType)
{
    std::string filePath;
    if (qd == nullptr) {
        RESMGR_HILOGE(RESMGR_TAG, "GetFilePathFromHap qd is null");
        return filePath;
    }
    const std::shared_ptr<IdItem> idItem = qd->GetIdItem();
    if (idItem == nullptr || idItem->resType_ != resType) {
        std::string hapPath = qd->GetIndexPath();
        RESMGR_HILOGE(RESMGR_TAG, "actual resType = %{public}d, expect resType = %{public}d",
            idItem == nullptr ? -1 : idItem->resType_, resType);
        return filePath;
    }
    if (extractor->IsStageModel()) {
        std::string tempFilePath(idItem->value_);
        auto index = tempFilePath.find('/');
        if (index == std::string::npos) {
            RESMGR_HILOGE(RESMGR_TAG, "resource path format error");
            return filePath;
        }
        filePath = idItem->value_.substr(index + 1);
    } else {
        // FA mode
        std::string tempFilePath("assets/");
        tempFilePath.append(idItem->value_);
        filePath = tempFilePath;
    }
    return filePath;
}
#endif

#if !defined(__WINNT__) && !defined(__IDE_PREVIEW__) && !defined(__ARKUI_CROSS__)
std::shared_ptr<AbilityBase::Extractor> GetAbilityExtractor(
    const std::shared_ptr<ValueUnderQualifierDir> qd)
{
    std::string hapPath = qd->GetIndexPath();
    bool isNewExtractor = false;
    auto extractor = AbilityBase::ExtractorUtil::GetExtractor(hapPath, isNewExtractor);
    return extractor;
}
#endif

RState HapManager::GetProfileData(const std::shared_ptr<ValueUnderQualifierDir> qualifierDir, size_t &len,
    std::unique_ptr<uint8_t[]> &outValue)
{
#if !defined(__WINNT__) && !defined(__IDE_PREVIEW__) && !defined(__ARKUI_CROSS__)
    auto extractor = GetAbilityExtractor(qualifierDir);
    if (extractor == nullptr) {
        RESMGR_HILOGE(RESMGR_TAG, "failed to get extractor from ability");
        return NOT_FOUND;
    }
    std::string filePath = GetFilePathFromHap(extractor, qualifierDir, ResType::PROF);
    if (filePath.empty()) {
        RESMGR_HILOGE(RESMGR_TAG, "get file path failed in GetProfileData");
        return NOT_FOUND;
    }
    bool ret = extractor->ExtractToBufByName(filePath, outValue, len);
    if (!ret) {
        RESMGR_HILOGE(RESMGR_TAG, "failed to get config data from ability");
        return NOT_FOUND;
    }
#endif
    return SUCCESS;
}

RState HapManager::GetMediaData(const std::shared_ptr<ValueUnderQualifierDir> qualifierDir, size_t &len,
    std::unique_ptr<uint8_t[]> &outValue)
{
    std::string filePath = qualifierDir->GetIndexPath();
    RState state;
    if (Utils::ContainsTail(filePath, Utils::tailSet)) {
        state = HapManager::GetMediaDataFromHap(qualifierDir, len, outValue);
    } else {
        state = HapManager::GetMediaDataFromIndex(qualifierDir, len, outValue);
    }
    return state;
}

RState HapManager::GetMediaDataFromHap(const std::shared_ptr<ValueUnderQualifierDir> qualifierDir, size_t &len,
    std::unique_ptr<uint8_t[]> &outValue)
{
#if !defined(__WINNT__) && !defined(__IDE_PREVIEW__) && !defined(__ARKUI_CROSS__)
    HITRACE_METER_NAME_EX(HITRACE_LEVEL_INFO, HITRACE_TAG_APP, __PRETTY_FUNCTION__, nullptr);
    auto extractor = GetAbilityExtractor(qualifierDir);
    if (extractor == nullptr) {
        RESMGR_HILOGE(RESMGR_TAG, "failed to get extractor from ability");
        return NOT_FOUND;
    }
    std::string filePath = GetFilePathFromHap(extractor, qualifierDir, ResType::MEDIA);
    if (filePath.empty()) {
        RESMGR_HILOGE(RESMGR_TAG, "get file path failed in GetMediaDataFromHap");
        return NOT_FOUND;
    }
    bool ret = extractor->ExtractToBufByName(filePath, outValue, len);
    if (!ret) {
        RESMGR_HILOGE(RESMGR_TAG, "failed to get media data from ability");
        return NOT_FOUND;
    }
#endif
    return SUCCESS;
}

RState HapManager::GetMediaDataFromIndex(const std::shared_ptr<ValueUnderQualifierDir> qualifierDir, size_t &len,
    std::unique_ptr<uint8_t[]> &outValue)
{
    std::string filePath;
    RState state = HapManager::GetFilePath(qualifierDir, ResType::MEDIA, filePath);
    if (state != SUCCESS) {
        return NOT_FOUND;
    }
    outValue = Utils::LoadResourceFile(filePath, len);
    return SUCCESS;
}

RState HapManager::GetMediaBase64Data(const std::shared_ptr<ValueUnderQualifierDir> qualifierDir,
    std::string &outValue)
{
    std::string filePath = qualifierDir->GetIndexPath();
    RState state;
    if (Utils::ContainsTail(filePath, Utils::tailSet)) {
        state = HapManager::GetMediaBase64DataFromHap(qualifierDir, outValue);
    } else {
        state = HapManager::GetMediaBase64DataFromIndex(qualifierDir, outValue);
    }
    return state;
}

RState HapManager::GetMediaBase64DataFromHap(const std::shared_ptr<ValueUnderQualifierDir> qualifierDir,
    std::string &outValue)
{
#if !defined(__WINNT__) && !defined(__IDE_PREVIEW__) && !defined(__ARKUI_CROSS__)
    auto extractor = GetAbilityExtractor(qualifierDir);
    if (extractor == nullptr) {
        RESMGR_HILOGE(RESMGR_TAG, "failed to get extractor from ability");
        return NOT_FOUND;
    }
    std::string filePath = GetFilePathFromHap(extractor, qualifierDir, ResType::MEDIA);
    std::unique_ptr<uint8_t[]> buffer;
    size_t tmpLen;
    bool ret = extractor->ExtractToBufByName(filePath, buffer, tmpLen);
    if (!ret) {
        RESMGR_HILOGE(RESMGR_TAG, "failed to get mediabase64 data from ability");
        return NOT_FOUND;
    }
    std::string imgType = GetImageType(filePath);
    Utils::EncodeBase64(buffer, tmpLen, imgType, outValue);
#endif
    return SUCCESS;
}

RState HapManager::GetMediaBase64DataFromIndex(const std::shared_ptr<ValueUnderQualifierDir> qualifierDir,
    std::string &outValue)
{
    std::string filePath;
    RState state = HapManager::GetFilePath(qualifierDir, ResType::MEDIA, filePath);
    if (state != SUCCESS) {
        return NOT_FOUND;
    }
    return Utils::GetMediaBase64Data(filePath, outValue);
}

int32_t HapManager::GetValidHapPath(std::string &hapPath)
{
    ReadLock lock(this->mutex_);
    for (auto iter = hapResources_.rbegin(); iter != hapResources_.rend(); iter++) {
        if ((*iter)->IsSystemResource() || (*iter)->IsOverlayResource()) {
            continue;
        }
        const std::string tempPath = (*iter)->GetIndexPath();
        if (Utils::ContainsTail(tempPath, Utils::tailSet)) {
            hapPath = tempPath;
            return OK;
        }
    }
    return NOT_FOUND;
}

int32_t HapManager::GetValidResourceIndexPath(std::string &indexPath)
{
    ReadLock lock(this->mutex_);
    for (auto iter = hapResources_.rbegin(); iter != hapResources_.rend(); iter++) {
        const std::string tempPath = (*iter)->GetIndexPath();
        if (Utils::IsSystemPath(tempPath)) {
            continue;
        }
        indexPath = tempPath;
        return OK;
    }
    return NOT_FOUND;
}

RState HapManager::FindRawFileFromHap(const std::string &rawFileName, size_t &len,
    std::unique_ptr<uint8_t[]> &outValue)
{
    ReadLock lock(this->mutex_);
    for (auto iter = hapResources_.begin(); iter != hapResources_.end(); iter++) {
        if ((*iter)->IsSystemResource() || (*iter)->IsOverlayResource()) {
            continue;
        }
        std::string tempPath = (*iter)->GetIndexPath();
        std::string tempPatchPath;
        if ((*iter)->HasPatch()) {
            tempPatchPath = (*iter)->GetPatchPath();
        }
        if (Utils::ContainsTail(tempPath, Utils::tailSet)) { // if file path is compressed
            RState state = HapParser::ReadRawFileFromHap(tempPath, tempPatchPath, rawFileName, len, outValue);
            if (state != SUCCESS) {
                continue;
            }
        } else { // if file path is uncompressed
            std::string filePath;
            HapManager::FindRawFile(rawFileName, filePath);
            outValue = Utils::LoadResourceFile(filePath, len);
            if (outValue == nullptr) {
                continue;
            }
        }
        return SUCCESS;
    }
    return ERROR_CODE_RES_PATH_INVALID;
}

RState HapManager::FindRawFileDescriptorFromHap(const std::string &rawFileName,
    ResourceManager::RawFileDescriptor &descriptor)
{
    std::lock_guard<std::mutex> lock(g_rawFileLock);
    auto it = rawFileDescriptor_.find(rawFileName);
    if (it != rawFileDescriptor_.end()) {
        descriptor.fd = rawFileDescriptor_[rawFileName].fd;
        descriptor.length = rawFileDescriptor_[rawFileName].length;
        descriptor.offset = rawFileDescriptor_[rawFileName].offset;
        return SUCCESS;
    }
    RState state = GetRawFd(rawFileName, descriptor);
    if (state == SUCCESS) {
        rawFileDescriptor_[rawFileName] = descriptor;
    }
    return state;
}

RState HapManager::GetRawFd(const std::string &rawFileName, ResourceManager::RawFileDescriptor &descriptor)
{
    RState state;
    ReadLock lock(this->mutex_);
    for (auto iter = hapResources_.begin(); iter != hapResources_.end(); iter++) {
        if ((*iter)->IsSystemResource() || (*iter)->IsOverlayResource()) {
            continue;
        }
        std::string tempPath = (*iter)->GetIndexPath();
        std::string tempPatchPath;
        if ((*iter)->HasPatch()) {
            tempPatchPath = (*iter)->GetPatchPath();
        }
        if (Utils::ContainsTail(tempPath, Utils::tailSet)) { // if file path is compressed
            state = HapParser::ReadRawFileDescriptor(tempPath.c_str(), tempPatchPath.c_str(), rawFileName, descriptor);
        } else { // if file path is uncompressed
            state = HapManager::FindRawFileDescriptor(rawFileName, descriptor);
        }
        if (state != SUCCESS) {
            continue;
        }
        return SUCCESS;
    }
    return ERROR_CODE_RES_PATH_INVALID;
}

RState HapManager::GetRawFileList(const std::string &rawDirPath, std::vector<std::string> &fileList)
{
    std::string hapOrIndexPath;
    if (HapManager::GetValidHapPath(hapOrIndexPath) == OK) {
        std::string temPatchPath;
        {
            ReadLock lock(this->mutex_);
            for (auto iter = hapResources_.begin(); iter != hapResources_.end(); iter++) {
                if ((*iter)->IsSystemResource() || (*iter)->IsOverlayResource()) {
                    continue;
                }
                if ((*iter)->GetIndexPath() == hapOrIndexPath && (*iter)->HasPatch()) {
                    temPatchPath = (*iter)->GetPatchPath();
                }
            }
        }
        std::set<std::string> fileSet;
        RState hapState = HapParser::GetRawFileList(hapOrIndexPath, rawDirPath, fileSet);
        RState hqfState = NOT_FOUND;
        if (!temPatchPath.empty()) {
            hqfState = HapParser::GetRawFileList(temPatchPath, rawDirPath, fileSet);
        }
        for (auto it = fileSet.begin(); it != fileSet.end(); it++) {
            fileList.emplace_back(*it);
        }
        return (hapState != SUCCESS && hqfState != SUCCESS) ? ERROR_CODE_RES_PATH_INVALID : SUCCESS;
    }
    if (HapManager::GetValidResourceIndexPath(hapOrIndexPath) == OK) {
        return  HapParser::GetRawFileListUnCompressed(hapOrIndexPath, rawDirPath, fileList);
    }
    return ERROR_CODE_RES_PATH_INVALID;
}

bool HapManager::IsLoadHap(std::string &hapPath)
{
    return HapManager::GetValidHapPath(hapPath) == OK ? true : false;
}

RState HapManager::GetFilePath(const std::shared_ptr<ValueUnderQualifierDir> qualifierDir, const ResType resType,
    std::string &outValue)
{
    // not found or type invalid
    if (qualifierDir == nullptr) {
        return NOT_FOUND;
    }
    const std::shared_ptr<IdItem> idItem = qualifierDir->GetIdItem();
    if (idItem == nullptr || idItem->resType_ != resType) {
        return NOT_FOUND;
    }
    outValue = qualifierDir->GetResourcePath();
#if defined(__ARKUI_CROSS__)
    auto index = idItem->value_.find('/');
    if (index == std::string::npos) {
        RESMGR_HILOGE(RESMGR_TAG, "resource path format error");
        return NOT_FOUND;
    }
    auto nameWithoutModule = idItem->value_.substr(index + 1);
    outValue.append(nameWithoutModule);
#elif defined(__IDE_PREVIEW__)
    if (Utils::IsFileExist(idItem->value_)) {
        outValue = idItem->value_;
        return SUCCESS;
    }
    auto index = idItem->value_.find('/');
    if (index == std::string::npos) {
        RESMGR_HILOGE(RESMGR_TAG, "resource path format error");
        return NOT_FOUND;
    }
    auto nameWithoutModule = idItem->value_.substr(index + 1);
    outValue.append(nameWithoutModule);
#else
    outValue.append(idItem->value_);
#endif
    return SUCCESS;
}

RState HapManager::FindRawFileDescriptor(const std::string &name, ResourceManager::RawFileDescriptor &descriptor)
{
    std::string paths = "";
    RState rState = HapManager::FindRawFile(name, paths);
    if (rState != SUCCESS) {
        return rState;
    }
    char outPath[PATH_MAX + 1] = {0};
    Utils::CanonicalizePath(paths.c_str(), outPath, PATH_MAX);
    int fd = open(outPath, O_RDONLY);
    if (fd > 0) {
        long length = lseek(fd, 0, SEEK_END);
        if (length == -1) {
            close(fd);
            return ERROR_CODE_RES_PATH_INVALID;
        }
        long begin = lseek(fd, 0, SEEK_SET);
        if (begin == -1) {
            close(fd);
            return ERROR_CODE_RES_PATH_INVALID;
        }
        descriptor.fd = fd;
        descriptor.length = length;
        descriptor.offset = 0;
        return SUCCESS;
    }
    return ERROR_CODE_RES_PATH_INVALID;
}

RState HapManager::CloseRawFileDescriptor(const std::string &name)
{
    std::lock_guard<std::mutex> lock(g_rawFileLock);
    auto it = rawFileDescriptor_.find(name);
    if (it == rawFileDescriptor_.end()) {
        return ERROR_CODE_RES_PATH_INVALID;
    }

    int fd = rawFileDescriptor_[name].fd;
    if (fd <= 0) {
        return ERROR_CODE_RES_PATH_INVALID;
    }

#if !defined(__WINNT__) && !defined(__IDE_PREVIEW__) && !defined(__ARKUI_CROSS__)
    if (fdsan_get_owner_tag(fd) != 0) {
        rawFileDescriptor_.erase(name);
        return SUCCESS;
    }
#endif

    if (close(fd) == -1) {
        return ERROR_CODE_RES_PATH_INVALID;
    }

    rawFileDescriptor_.erase(name);
    return SUCCESS;
}

bool HapManager::RemoveResource(const std::string &path, const std::vector<std::string> &overlayPaths)
{
    WriteLock lock(this->mutex_);
    if (loadedHapPaths_.find(path) == loadedHapPaths_.end()) {
        return false;
    }
    std::vector<std::string> targetOverlay = loadedHapPaths_[path];
    if (targetOverlay.empty()) {
        RESMGR_HILOGE(RESMGR_TAG, "the hap have not overlay");
        return false;
    }
    char outPath[PATH_MAX] = {0};
    for (auto iter = overlayPaths.begin(); iter != overlayPaths.end(); iter++) {
        Utils::CanonicalizePath((*iter).c_str(), outPath, PATH_MAX);
        if (outPath[0] == '\0') {
            RESMGR_HILOGE(RESMGR_TAG, "invalid overlayPath");
            continue;
        }
        if (std::find(targetOverlay.begin(), targetOverlay.end(), outPath) != targetOverlay.end()) {
            targetOverlay.erase(std::remove(targetOverlay.begin(), targetOverlay.end(), outPath),
                targetOverlay.end());
        }
        for (auto resIter = hapResources_.begin(); resIter != hapResources_.end();) {
            if ((*resIter) == nullptr) {
                RESMGR_HILOGE(RESMGR_TAG, "hapResource is nullptr");
                return false;
            }
            std::string hapPath = (*resIter)->GetIndexPath();
            if (hapPath == outPath) {
                resIter = hapResources_.erase(resIter);
            } else {
                resIter++;
            }
        }
    }
    loadedHapPaths_[path] = targetOverlay;
    return true;
}

std::vector<std::shared_ptr<HapResource>> HapManager::GetHapResource()
{
    return hapResources_;
}

void HapManager::AddSystemResource(const std::shared_ptr<HapManager> &systemHapManager)
{
    if (systemHapManager == nullptr) {
        RESMGR_HILOGE(RESMGR_TAG, "add system resource failed, systemHapManager is nullptr");
        return;
    }
    if (!systemHapManager->isSystem_) {
        RESMGR_HILOGE(RESMGR_TAG, "add system resource failed, the added hapManager is not system");
        return;
    }
    WriteLock lock(this->mutex_);
    // add system resource to app resource vector
    const std::vector<std::shared_ptr<HapResource>> &systemResources = systemHapManager->hapResources_;
    for (size_t i = 0; i < systemResources.size(); i++) {
        this->hapResources_.push_back(systemResources[i]);
    }

    // add system loaded path to app loaded path map.
    const std::unordered_map<std::string, std::vector<std::string>> &loadedSystemPaths =
        systemHapManager->loadedHapPaths_;
    for (auto iter = loadedSystemPaths.begin(); iter != loadedSystemPaths.end(); iter++) {
        const std::vector<std::string> &overlayPaths = iter->second;
        if (this->loadedHapPaths_.find(iter->first) == this->loadedHapPaths_.end()) {
            this->loadedHapPaths_[iter->first] = overlayPaths;
        }
    }
}

uint32_t HapManager::GetResourceLimitKeys()
{
    ReadLock lock(this->mutex_);
    uint32_t limitKeysValue = 0;
    for (size_t i = 0; i < hapResources_.size(); i++) {
        limitKeysValue |= hapResources_[i]->GetLimitKeysValue();
    }
    RESMGR_HILOGD(RESMGR_TAG, "hap manager limit key is %{public}u", limitKeysValue);
    return limitKeysValue;
}

std::unordered_map<std::string, ResType> ResTypeMap {
    {"integer", INTEGER},
    {"string", STRING},
    {"strarray", STRINGARRAY},
    {"intarray", INTARRAY},
    {"boolean", BOOLEAN},
    {"color", COLOR},
    {"theme", THEME},
    {"plural", PLURALS},
    {"float", FLOAT},
    {"media", MEDIA},
    {"profile", PROF},
    {"pattern", PATTERN},
    {"symbol", SYMBOL},
};

bool IsPrefix(std::string_view prefix, std::string_view full)
{
    return prefix == full.substr(0, prefix.size());
}

uint32_t GetRealResId(const std::string &resType,
    const std::vector<std::unordered_map<ResType, uint32_t>> &candidates)
{
    for (auto candidate : candidates) {
        for (auto data : candidate) {
            if (ResTypeMap.find(resType) != ResTypeMap.end() && ResTypeMap[resType] == data.first) {
                return data.second;
            }
        }
    }
    return 0;
}

std::tuple<std::string, std::string> GetResTypeAndResName(const std::string &resTypeName)
{
    std::tuple<std::string, std::string> typeNameTuple;
    auto pos1 = resTypeName.find('.');
    auto pos2 = resTypeName.rfind('.');
    if (pos1 == std::string::npos || pos2 == std::string::npos) {
        return std::make_tuple("", "");
    }
    if (pos2 < pos1 + 1) {
        return std::make_tuple("", "");
    }
    const std::string resType = resTypeName.substr(pos1 + 1, pos2 - pos1 - 1);
    if (ResTypeMap.find(resType) == ResTypeMap.end()) {
        return std::make_tuple("", "");
    }
    const std::string resName = resTypeName.substr(pos2 + 1);
    if (resName.empty()) {
        return std::make_tuple("", "");
    }
    typeNameTuple = std::make_tuple(resType, resName);
    return typeNameTuple;
}

RState HapManager::GetResId(const std::string &resTypeName, uint32_t &resId)
{
    auto typeNameTuple = GetResTypeAndResName(resTypeName);
    const std::string resType =  std::get<0>(typeNameTuple);
    const std::string resName =  std::get<1>(typeNameTuple);
    if (resType.empty() || resName.empty()) {
        RESMGR_HILOGE(RESMGR_TAG, "invalid resTypeName = %{public}s", resTypeName.c_str());
        return NOT_FOUND;
    }
    bool isSystem = IsPrefix("sys", resTypeName);
    bool isApp = IsPrefix("app", resTypeName);
    if (!isSystem && !isApp) {
        RESMGR_HILOGE(RESMGR_TAG, "invalid resTypeName = %{public}s", resTypeName.c_str());
        return NOT_FOUND;
    }
    ReadLock lock(this->mutex_);
    for (auto iter = hapResources_.begin(); iter != hapResources_.end(); iter++) {
        bool isSystemResource = (*iter)->IsSystemResource();
        bool isOverlayResource = (*iter)->IsOverlayResource();
        if (isOverlayResource) {
            continue;
        }
        if (isSystem && !isSystemResource) {
            continue;
        }
        if (isApp && isSystemResource) {
            continue;
        }
        std::unordered_map<std::string, std::unordered_map<ResType, uint32_t>> nameTypeIdMap =
                (*iter)->BuildNameTypeIdMapping();
        std::vector<std::unordered_map<ResType, uint32_t>> candidates;
        for (auto data : nameTypeIdMap) {
            if (data.first != resName) {
                continue;
            }
            candidates.emplace_back(data.second);
        }
        resId = GetRealResId(resType, candidates);
        if (resId != 0) {
            return SUCCESS;
        }
    }
    if (resId == 0) {
        RESMGR_HILOGE(RESMGR_TAG, "GetResId name:%{public}s, resType:%{public}s", resName.c_str(), resType.c_str());
    }
    return NOT_FOUND;
}

void HapManager::GetLocales(std::vector<std::string> &outValue, bool includeSystem)
{
    if (isSystem_) {
        includeSystem = true;
    }
    std::set<std::string> result;
    ReadLock lock(this->mutex_);
    for (size_t i = 0; i < hapResources_.size(); i++) {
        hapResources_[i]->GetLocales(result, includeSystem);
    }
    outValue.assign(result.begin(), result.end());
}

RState HapManager::IsRawDirFromHap(const std::string &pathName, bool &outValue)
{
    ReadLock lock(this->mutex_);
    for (auto iter = hapResources_.begin(); iter != hapResources_.end(); iter++) {
        if ((*iter)->IsSystemResource() || (*iter)->IsOverlayResource()) {
            continue;
        }
        const std::string tempPath = (*iter)->GetIndexPath();
        if (Utils::ContainsTail(tempPath, Utils::tailSet)) { // if file path is compressed
            RState state = HapParser::IsRawDirFromHap(tempPath.c_str(), pathName, outValue);
            if (state != SUCCESS) {
                continue;
            }
        } else { // if file path is uncompressed
#if !defined(__WINNT__) && !defined(__IDE_PREVIEW__) && !defined(__ARKUI_CROSS__)
            RState state = HapParser::IsRawDirUnCompressed(pathName, outValue);
            if (state != SUCCESS) {
                continue;
            }
#else
            const std::string finalPath = (*iter)->GetResourcePath() + RAW_FILE_PATH + pathName;
            RState state = HapParser::IsRawDirUnCompressed(finalPath, outValue);
            if (state != SUCCESS) {
                continue;
            }
#endif
        }
        return SUCCESS;
    }
    return ERROR_CODE_RES_PATH_INVALID;
}

bool HapManager::IsThemeSystemResEnableHap()
{
    ReadLock lock(this->mutex_);
    return this->isThemeSystemResEnable_;
}

bool HapManager::IsSystem()
{
    return this->isSystem_;
}

std::unordered_map<std::string, std::vector<std::string>> HapManager::GetLoadedHapPaths()
{
    return this->loadedHapPaths_;
}

void HapManager::UpdateAppConfigForSysResManager(bool isAppDarkRes, bool isThemeSystemResEnable)
{
    WriteLock lock(this->mutex_);
    if (this->isSystem_ && this->isUpdateAppConfig_) {
        this->isUpdateAppConfig_ = false;
        this->resConfig_->SetAppDarkRes(isAppDarkRes);
        this->isThemeSystemResEnable_ = isThemeSystemResEnable;
    }
}
} // namespace Resource
} // namespace Global
} // namespace OHOS
