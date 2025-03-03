/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/* This file contains the declaration for the OTA Requestor interface.
 * Any implementation of the OTA Requestor must implement this interface.
 */

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/util/af-enums.h>

#pragma once

namespace chip {

/**
 * A class to represent a list of the provider locations
 */
class ProviderLocationList
{
public:
    /**
     * A class to iterate over an instance of ProviderLocationList
     */
    class Iterator
    {
    public:
        /**
         * Initialize iterator values, must be called before calling Next()/GetValue()
         */
        Iterator(const Optional<app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type> * const list,
                 size_t size)
        {
            mList         = list;
            mListSize     = size;
            mCurrentIndex = 0;
            mNextIndex    = 0;
        }

        /**
         * Search for the next provider location found in the list
         */
        bool Next()
        {
            for (size_t i = mNextIndex; i < mListSize; i++)
            {
                if (mList[i].HasValue())
                {
                    mCurrentIndex = i;
                    mNextIndex    = mCurrentIndex + 1;
                    return true;
                }
            }

            return false;
        }

        /**
         * Retrieves a reference to the provider location found on a previous call to Next()
         */
        const app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type & GetValue() const
        {
            return mList[mCurrentIndex].Value();
        }

    private:
        const Optional<app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type> * mList;
        size_t mListSize;
        size_t mCurrentIndex;
        size_t mNextIndex;
    };

    /**
     * Retrieve an iterator to the list
     */
    Iterator Begin() const { return Iterator(mList, CHIP_CONFIG_MAX_FABRICS); }

    /**
     * Add a provider location to the list if there is space available
     */
    CHIP_ERROR Add(const app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type & providerLocation)
    {
        for (size_t i = 0; i < mMaxSize; i++)
        {
            if (!mList[i].HasValue())
            {
                mList[i].SetValue(providerLocation);
                mListSize++;
                return CHIP_NO_ERROR;
            }
        }

        return CHIP_ERROR_NO_MEMORY;
    }

    /**
     * Delete a provider location from the list if found
     */
    CHIP_ERROR Delete(const app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type & providerLocation)
    {
        for (size_t i = 0; i < mMaxSize; i++)
        {
            if (mList[i].HasValue())
            {
                const app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type & pl = mList[i].Value();
                if ((pl.GetFabricIndex() == providerLocation.GetFabricIndex()) &&
                    (pl.providerNodeID == providerLocation.providerNodeID) && (pl.endpoint == providerLocation.endpoint))
                {
                    mList[i].ClearValue();
                    mListSize--;
                    return CHIP_NO_ERROR;
                }
            }
        }

        return CHIP_ERROR_NOT_FOUND;
    }

private:
    Optional<app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type> mList[CHIP_CONFIG_MAX_FABRICS];
    size_t mListSize = 0;
    size_t mMaxSize  = CHIP_CONFIG_MAX_FABRICS;
};

// Interface class to connect the OTA Software Update Requestor cluster command processing
// with the core OTA Requestor logic
class OTARequestorInterface
{
public:
    // Return value for various trigger-type APIs
    enum OTATriggerResult
    {
        kTriggerSuccessful = 0,
        kNoProviderKnown   = 1
    };

    // Handler for the AnnounceOTAProvider command
    virtual EmberAfStatus HandleAnnounceOTAProvider(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData) = 0;

    // Destructor
    virtual ~OTARequestorInterface() = default;

    // Send QueryImage command
    virtual OTATriggerResult TriggerImmediateQuery() = 0;

    // Download image
    virtual void DownloadUpdate() = 0;

    // Send ApplyImage command
    virtual void ApplyUpdate() = 0;

    // Send NotifyUpdateApplied command
    virtual void NotifyUpdateApplied(uint32_t version) = 0;

    // Get image update progress in percents unit
    virtual CHIP_ERROR GetUpdateProgress(EndpointId endpointId, chip::app::DataModel::Nullable<uint8_t> & progress) = 0;

    // Get requestor state
    virtual CHIP_ERROR GetState(EndpointId endpointId,
                                chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum & state) = 0;

    // Application directs the Requestor to cancel image update in progress. All the Requestor state is
    // cleared, UpdateState is reset to Idle
    virtual void CancelImageUpdate() = 0;

    // Clear all entries with the specified fabric index in the default OTA provider list
    virtual CHIP_ERROR ClearDefaultOtaProviderList(FabricIndex fabricIndex) = 0;

    // Add a default OTA provider to the cached list
    virtual CHIP_ERROR
    AddDefaultOtaProvider(const app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type & providerLocation) = 0;

    // Retrieve an iterator to the cached default OTA provider list
    virtual ProviderLocationList::Iterator GetDefaultOTAProviderListIterator(void) = 0;
};

// The instance of the class implementing OTARequestorInterface must be managed through
// the following global getter and setter functions.

// Set the object implementing OTARequestorInterface
void SetRequestorInstance(OTARequestorInterface * instance);

// Get the object implementing OTARequestorInterface
OTARequestorInterface * GetRequestorInstance();

} // namespace chip
