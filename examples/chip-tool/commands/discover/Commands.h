/*
 *   Copyright (c) 2021 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include "DiscoverCommand.h"
#include "DiscoverCommissionablesCommand.h"
#include "DiscoverCommissionersCommand.h"
#include <controller/DeviceAddressUpdateDelegate.h>
#include <lib/address_resolve/AddressResolve.h>

class Resolve : public DiscoverCommand, public chip::AddressResolve::NodeListener
{
public:
    Resolve(CredentialIssuerCommands * credsIssuerConfig) : DiscoverCommand("resolve", credsIssuerConfig)
    {
        mNodeLookupHandle.SetListener(this);
    }

    /////////// DiscoverCommand Interface /////////
    CHIP_ERROR RunCommand(NodeId remoteId, uint64_t fabricId) override
    {
        ReturnErrorOnFailure(chip::AddressResolve::Resolver::Instance().Init(&chip::DeviceLayer::SystemLayer()));

        return chip::AddressResolve::Resolver::Instance().LookupNode(
            chip::AddressResolve::NodeLookupRequest(chip::PeerId().SetNodeId(remoteId).SetCompressedFabricId(fabricId)),
            mNodeLookupHandle);
    }

    void OnNodeAddressResolved(const PeerId & peerId, const chip::AddressResolve::ResolveResult & result) override
    {
        char addrBuffer[chip::Transport::PeerAddress::kMaxToStringSize];

        result.address.ToString(addrBuffer);

        ChipLogProgress(chipTool, "NodeId Resolution: %" PRIu64 " at %s", peerId.GetNodeId(), addrBuffer);
        ChipLogProgress(chipTool, "   MRP retry interval (idle): %" PRIu32 "ms", result.mrpConfig.mIdleRetransTimeout.count());
        ChipLogProgress(chipTool, "   MRP retry interval (active): %" PRIu32 "ms", result.mrpConfig.mActiveRetransTimeout.count());
        ChipLogProgress(chipTool, "   Supports TCP: %s", result.supportsTcp ? "yes" : "no");
        SetCommandExitStatus(CHIP_NO_ERROR);
    }

    void OnNodeAddressResolutionFailed(const chip::PeerId & peerId, CHIP_ERROR error) override
    {
        ChipLogProgress(chipTool, "NodeId %" PRIu64 " Resolution: failed!", peerId.GetNodeId());
        SetCommandExitStatus(CHIP_ERROR_INTERNAL);
    }

private:
    chip::AddressResolve::NodeLookupHandle mNodeLookupHandle;
};

class Update : public DiscoverCommand
{
public:
    Update(CredentialIssuerCommands * credsIssuerConfig) : DiscoverCommand("update", credsIssuerConfig) {}

    /////////// DiscoverCommand Interface /////////
    CHIP_ERROR RunCommand(NodeId remoteId, uint64_t fabricId) override
    {
        ChipLogProgress(chipTool, "Mdns: Updating NodeId: %" PRIx64 " Compressed FabricId: %" PRIx64 " ...", remoteId,
                        CurrentCommissioner().GetCompressedFabricId());
        return CurrentCommissioner().UpdateDevice(remoteId);
    }

    /////////// DeviceAddressUpdateDelegate Interface /////////
    void OnAddressUpdateComplete(NodeId nodeId, CHIP_ERROR error) override
    {
        if (CHIP_NO_ERROR == error)
        {
            ChipLogProgress(chipTool, "Device address updated successfully");
        }
        else
        {
            ChipLogError(chipTool, "Failed to update the device address: %s", chip::ErrorStr(error));
        }

        SetCommandExitStatus(error);
    }
};

void registerCommandsDiscover(Commands & commands, CredentialIssuerCommands * credsIssuerConfig)
{
    const char * clusterName = "Discover";

    commands_list clusterCommands = {
        make_unique<Resolve>(credsIssuerConfig),
        make_unique<Update>(credsIssuerConfig),
        make_unique<DiscoverCommissionablesCommand>(credsIssuerConfig),
        make_unique<DiscoverCommissionersCommand>(credsIssuerConfig),
    };

    commands.Register(clusterName, clusterCommands);
}
