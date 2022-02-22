/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <platform/OTARequestorDriver.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>

namespace chip {

class OTARequestorInterface;
class OTAImageProcessorInterface;

namespace DeviceLayer {

class GenericOTARequestorDriver : public OTARequestorDriver
{
public:

    //// Public API methods
    /**
     * Called to perform some initialization including:
     *   - Set the OTA requestor instance used to direct download progress
     *   - Set the OTA image processor instance used to apply/abort the downloaded image
     */
    void Init(OTARequestorInterface * requestor, OTAImageProcessorInterface * processor)
    {
        mRequestor      = requestor;
        mImageProcessor = processor;
    }

    // Set the timeout (in seconds) for the Default Provider timer
    void SetDefaultProvidersTimeoutSec(uint32_t timeout) { mDefaultProvidersTimeoutSec = timeout; }

    //// Virtual methods from OTARequestorDriver
    bool CanConsent() override;
    uint16_t GetMaxDownloadBlockSize() override;
    void HandleError(UpdateFailureState state, CHIP_ERROR error) override;
    void HandleIdleState() override;
    void UpdateAvailable(const UpdateDescription & update, System::Clock::Seconds32 delay) override;
    void UpdateNotFound(UpdateNotFoundReason reason, System::Clock::Seconds32 delay) override;
    void UpdateDownloaded() override;
    void UpdateConfirmed(System::Clock::Seconds32 delay) override;
    void UpdateSuspended(System::Clock::Seconds32 delay) override;
    void UpdateDiscontinued() override;
    void UpdateCancelled() override;
    OTARequestorAction GetRequestorAction(OTARequestorIncomingEvent input) override;
    void ScheduleDelayedAction(UpdateFailureState state, System::Clock::Seconds32 delay, System::TimerCompleteCallback action, void * aAppState) override;
    void CancelDelayedAction(System::TimerCompleteCallback action, void * aAppState) override;
    void ProcessAnnounceOTAProviders(const ProviderLocationType &providerLocation, 
                                        app::Clusters::OtaSoftwareUpdateRequestor::OTAAnnouncementReason announcementReason) override;
    void DriverTriggerQuery() override;

    //// Regular methods
    void StartDefaultProvidersTimer();
    void StopDefaultProvidersTimer();
    void DefaultProviderTimerHandler(System::Layer * systemLayer, void * appState);
 
private:
    OTARequestorInterface      * mRequestor           = nullptr;
    OTAImageProcessorInterface * mImageProcessor = nullptr;
    uint32_t mOtaStartDelayMs                 = 0;
    uint32_t mDefaultProvidersTimeoutSec      = 86400;  // Timeout for the Default Provider timer

 using ProviderLocationType             = app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type;
    Optional<ProviderLocationType> mLastProviderLocation; // Provider location used for the last query or update 
};

} // namespace DeviceLayer
} // namespace chip
