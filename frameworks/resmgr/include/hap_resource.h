/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef RESOURCE_MANAGER_HAPRESOURCE_H
#define RESOURCE_MANAGER_HAPRESOURCE_H

#include <map>
#include <string>
#include <time.h>
#include <unordered_map>
#include "res_desc.h"
#include "res_config_impl.h"

namespace OHOS {
namespace Global {
namespace Resource {
/**
 * HapResource describe a resource of hap zip file.
 *
 */
class HapResource {
public:
    /**
     * Creates an HapResource.
     *
     * @param path resources.index file path
     * @param defaultConfig  match defaultConfig to keys of index file, only parse the matched keys.
     *                       'null' means parse all keys.
     * @param isSystem If `isSystem` is true, the package is marked as a system package and allows some functions to
     *     filter out this package when computing what configurations/resources are available.
     * @param isOverlay If 'isOverlay' is true, the package is marked as an overlay package and overlay resource can
     *     replace non-overlay resource.
     * @return pResource if create pResource success, else nullptr
     */
    static const HapResource *LoadFromIndex(const char *path, const ResConfigImpl *defaultConfig,
        bool isSystem = false, bool isOverlay = false);

    /**
     * Creates an HapResource.
     *
     * @param path hap file path
     * @param defaultConfig  match defaultConfig to keys of index file, only parse the matched keys.
     *     'null' means parse all keys.
     * @param isSystem If `isSystem` is true, the package is marked as a system package and allows some functions to
     *     filter out this package when computing what configurations/resources are available.
     * @param isOverlay If 'isOverlay' is true, the package is marked as an overlay package and overlay resource can
     *     replace non-overlay resource.
     * @return pResource if create pResource success, else nullptr
     */
    static const HapResource *LoadFromHap(const char *path, const ResConfigImpl *defaultConfig,
        bool isSystem = false, bool isOverlay = false);

    /**
     * Creates an HapResource.
     *
     * @param path hap file path
     * @param defaultConfig  match defaultConfig to keys of index file, only parse the matched keys.
     *     'null' means parse all keys.
     * @param isSystem If `isSystem` is true, the package is marked as a system package and allows some functions to
     *     filter out this package when computing what configurations/resources are available.
     * @param isOverlay If 'isOverlay' is true, the package is marked as an overlay package and overlay resource can
     *     replace non-overlay resource.
     * @return pResource if create pResource success, else nullptr
     */
    static const HapResource *Load(const char* path, const ResConfigImpl* defaultConfig,
        bool isSystem = false, bool isOverlay = false);

    /**
     * Load overlay resources
     * @param path the resources.index file path
     * @param overlayPath  the resource overlay path
     * @param defaultConfig the resource config
     * @param isSystem judge the overlay is system or not
     * @return the map of overlay resource path and resource info if success, else null
     */
    static const std::unordered_map<std::string, HapResource *> LoadOverlays(const std::string &path,
        const std::vector<std::string> &overlayPath, const ResConfigImpl *defaultConfig, bool isSystem = false);

    /**
     * The destructor of HapResource
     */
    ~HapResource();

    /**
     * Get the resource.index file path
     */
    inline const std::string GetIndexPath() const
    {
        return indexPath_;
    }

    /**
     * Get the resource path
     */
    inline const std::string GetResourcePath() const
    {
        return resourcePath_;
    }

    /**
     * Get the system flag of HapResource.
     *
     * @return true if isSystem_ is true, false otherwise
     */
    inline bool IsSystemResource() const
    {
        return isSystem_;
    }

    /**
     * Get the overlay flag of HapResource.
     *
     * @return true if isOverlay_ is true, false otherwise
     */
    inline bool IsOverlayResource() const
    {
        return isOverlay_;
    }

    /**
     * Get the resource information
     */
    const std::vector<std::string> GetQualifiers() const;

    /**
     * Describe limitpath and value under the path
     */
    class ValueUnderQualifierDir {
    public:
        inline const std::vector<KeyParam *> GetKeyParams() const
        {
            return keyParams_;
        }

        inline const std::string GetFolder() const
        {
            return folder_;
        }

        inline const IdItem *GetIdItem() const
        {
            return idItem_;
        }

        inline const ResConfigImpl *GetResConfig() const
        {
            return resConfig_;
        }

        inline const HapResource *GetHapResource() const
        {
            return hapResource_;
        }

        inline bool IsOverlay() const
        {
            return isOverlay_;
        }

        inline bool IsSystemResource() const
        {
            return isSystemResource_;
        }

        /**
         * The constructor of ValueUnderQualifierDir.
         *
         * @param resKey resKey, indicate every limit path item.
         * @param idItem idItem value, include type and value of id.
         * @param hapResource hapResource.
         * @param isOverlay the overlay flag, default value is false.
         * @param isSystemResource the system flag, default value is false.
         */
        ValueUnderQualifierDir(const ResKey *resKey, IdItem *idItem,
            HapResource *hapResource, bool isOverlay = false, bool isSystemResource = false);

        ~ValueUnderQualifierDir();

    private:

        /*
         * keyParams_, folder_, resConfig_ are 3 different ways to describe Qualifiers Sub-directory
         */
        std::vector<KeyParam *> keyParams_;
        // the qualifier path name
        std::string folder_;
        // this resConfig_ point to the ResKey resConfig_ and resConfig_ will be unified free in ResKey destruct.
        const ResConfigImpl *resConfig_;

        // the value
        IdItem *idItem_;

        // indicate belong to which hapresource
        const HapResource *hapResource_;

        friend class HapResource;

        bool isOverlay_;

        bool isSystemResource_;
    };

    /**
     * describe value under different Qualifiers Sub-directories
     */
    class IdValues {
    public:
        inline void AddLimitPath(ValueUnderQualifierDir *vuqd)
        {
            limitPaths_.push_back(vuqd);
        }

        inline const std::vector<ValueUnderQualifierDir *> &GetLimitPathsConst() const
        {
            return limitPaths_;
        }

        ~IdValues();

    private:
        // the folder desc
        std::vector<ValueUnderQualifierDir *> limitPaths_;
    };

    /**
     * Get the resource value by resource id
     * @param id the resource id
     * @return the resource value related to id
     */
    const IdValues *GetIdValues(const uint32_t id) const;

    /**
     * Get the resource value by resource name
     * @param name the resource name
     * @param resType the resource type
     * @return the resource value related to resource name
     */
    const IdValues *GetIdValuesByName(const std::string name, const ResType resType) const;

    /**
     * Get the resource id by resource name
     * @param name the resource name
     * @param resType the resource type
     * @return the resource id related to resource name
     */
    int GetIdByName(const char *name, const ResType resType) const;

    size_t IdSize() const
    {
        return idValuesMap_.size();
    }

    /**
     * Get the resource limit keys value which every binary bit corresponds to existing limit key {@link KeyType}
     *
     * @return the resource limit keys
     */
    uint32_t GetResourceLimitKeys() const;

private:
    HapResource(const std::string path, time_t lastModTime, ResDesc *resDes,
        bool isSystem = false, bool isOverlay = false);

    std::unordered_map<std::string, std::unordered_map<ResType, uint32_t>> BuildNameTypeIdMapping() const;

    void UpdateOverlayInfo(std::unordered_map<std::string, std::unordered_map<ResType, uint32_t>> &nameTypeId);

    uint32_t GetLimitPathsKeys(const std::vector<ValueUnderQualifierDir *> &limitPaths,
        std::vector<bool> &keyTypes) const;

    // must call Init() after constructor
    bool Init();

    // step of Init(), called in Init()
    bool InitIdList();

    // resources.index file path
    const std::string indexPath_;

    // resource path , calculated from indexPath_
    std::string resourcePath_;

    // last mod time of hap file
    time_t lastModTime_;

    // resource information stored in resDesc_
    ResDesc *resDesc_;

    std::map<uint32_t, IdValues *> idValuesMap_;

    // the key is name, each restype holds one map
    // name may conflict in same restype !
    std::vector<std::map<std::string, IdValues *> *> idValuesNameMap_;

    // judge the hap resource is system or not.
    bool isSystem_;

    // judge the hap resource is overlay or not.
    bool isOverlay_;
};
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif
