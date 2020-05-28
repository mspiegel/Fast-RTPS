// Copyright 2020 Two Six Labs.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <utility>
#include <cstring>
#include <algorithm>

#include <fastdds/rtps/transport/TransportInterface.h>
#include <fastrtps/rtps/messages/CDRMessage.h>
#include <fastrtps/log/Log.h>
#include <fastdds/rtps/network/ReceiverResource.h>
#include <fastdds/rtps/network/SenderResource.h>
#include <fastrtps/rtps/messages/MessageReceiver.h>

#include <fastdds/rtps/transport/gaps/GapsTransport.h>
#include <fastdds/rtps/transport/gaps/GapsChannelResource.h>

#include <rtps/transport/gaps/GapsSenderResource.hpp>

#define SHM_MANAGER_DOMAIN ("fastrtps")

using namespace std;

using namespace eprosima;
using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

using Locator_t = fastrtps::rtps::Locator_t;
using LocatorList_t = fastrtps::rtps::LocatorList_t;
using Log = dds::Log;
using octet = fastrtps::rtps::octet;
using SenderResource = fastrtps::rtps::SenderResource;
using LocatorSelectorEntry = fastrtps::rtps::LocatorSelectorEntry;
using LocatorSelector = fastrtps::rtps::LocatorSelector;
using PortParameters = fastrtps::rtps::PortParameters;

TransportInterface* GapsTransportDescriptor::create_transport() const
{
    return new GapsTransport(*this);
}

//*********************************************************
// GapsTransport
//*********************************************************

GapsTransport::GapsTransport(
        const GapsTransportDescriptor& descriptor)
    : TransportInterface(LOCATOR_KIND_GAPS)
    , configuration_(descriptor)
{

}

GapsTransport::GapsTransport()
    : TransportInterface(LOCATOR_KIND_GAPS)
{
}

GapsTransport::~GapsTransport()
{
    clean();
}

bool GapsTransport::getDefaultMetatrafficMulticastLocators(
        LocatorList_t& locators,
        uint32_t metatraffic_multicast_port) const
{
    return false;
}

bool GapsTransport::getDefaultMetatrafficUnicastLocators(
        LocatorList_t& locators,
        uint32_t metatraffic_unicast_port) const
{
    return false;
}

bool GapsTransport::getDefaultUnicastLocators(
        LocatorList_t& locators,
        uint32_t unicast_port) const
{
    return false;
}

void GapsTransport::AddDefaultOutputLocator(
        LocatorList_t& defaultList)
{
    (void)defaultList;
}

const GapsTransportDescriptor* GapsTransport::configuration() const
{
    return &configuration_;
}

bool GapsTransport::OpenInputChannel(
        const Locator_t& locator,
        TransportReceiverInterface* receiver,
        uint32_t maxMsgSize)
{
    std::unique_lock<std::recursive_mutex> scopedLock(input_channels_mutex_);

    if (!IsLocatorSupported(locator))
    {
        return false;
    }

    if (!IsInputChannelOpen(locator))
    {
        try
        {
            auto channel_resource = CreateInputChannelResource(locator, maxMsgSize, receiver);
            input_channels_.push_back(channel_resource);
        }
        catch (std::exception& e)
        {
            (void)e;

            logInfo(RTPS_MSG_OUT, std::string("CreateInputChannelResource failed for config ")
                    << locator.config << " msg: " << e.what());
            return false;
        }
    }

    return true;
}

bool GapsTransport::is_locator_allowed(
        const Locator_t& locator) const
{
    return IsLocatorSupported(locator);
}

LocatorList_t GapsTransport::NormalizeLocator(
        const Locator_t& locator)
{
    LocatorList_t list;

    list.push_back(locator);

    return list;
}

bool GapsTransport::is_local_locator(
        const Locator_t& locator) const
{
    return false;
}

void GapsTransport::clean()
{
    assert(input_channels_.size() == 0);
}

bool GapsTransport::CloseInputChannel(
        const Locator_t& locator)
{
    std::lock_guard<std::recursive_mutex> lock(input_channels_mutex_);

    for (auto it = input_channels_.begin(); it != input_channels_.end(); it++)
    {
        if ( (*it)->locator() == locator)
        {
            (*it)->disable();
            (*it)->release();
            (*it)->clear();
            delete (*it);
            input_channels_.erase(it);

            return true;
        }
    }

    return false;
}

void GapsTransport::CloseOutputChannel(int gapsDescriptor)
{
    // TODO pirate_close()
}

bool GapsTransport::DoInputLocatorsMatch(
        const Locator_t& left,
        const Locator_t& right) const
{
    return left.kind == right.kind && left.config == right.config;
}

bool GapsTransport::init()
{
    return true;
}

bool GapsTransport::IsInputChannelOpen(
        const Locator_t& locator) const
{
    std::lock_guard<std::recursive_mutex> lock(input_channels_mutex_);

    return IsLocatorSupported(locator) && (std::find_if(
               input_channels_.begin(), input_channels_.end(),
               [&](const GapsChannelResource* resource) {
        return locator == resource->locator();
    }) != input_channels_.end());
}

bool GapsTransport::IsLocatorSupported(
        const Locator_t& locator) const
{
    return locator.kind == transport_kind_;
}

GapsChannelResource* GapsTransport::CreateInputChannelResource(
        const Locator_t& locator,
        uint32_t maxMsgSize,
        TransportReceiverInterface* receiver)
{
    (void) maxMsgSize;
    int gd = 0;
    // TODO: pirate_open
    // TODO: delete this function
    return new GapsChannelResource(this, gd, maxMsgSize, locator, receiver);
}

bool GapsTransport::OpenOutputChannel(
        SendResourceList& sender_resource_list,
        const Locator_t& locator)
{
    if (!IsLocatorSupported(locator))
    {
        return false;
    }

    // We try to find a SenderResource that can be reuse to this locator.
    // Note: This is done in this level because if we do in NetworkFactory level, we have to mantain what transport
    // already reuses a SenderResource.
    for (auto& sender_resource : sender_resource_list)
    {
        GapsSenderResource* sm_sender_resource = GapsSenderResource::cast(*this, sender_resource.get());

        if (sm_sender_resource)
        {
            return true;
        }
    }

    try
    {
        int gd = 0;
        // TODO: pirate_open()
        sender_resource_list.emplace_back(
            static_cast<SenderResource*>(new GapsSenderResource(*this, gd)));
    }
    catch (std::exception& e)
    {
        logError(RTPS_MSG_OUT, "GapsTransport error opening port " << std::to_string(locator.port)
                                                                        << " with msg: " << e.what());

        return false;
    }

    return true;
}

Locator_t GapsTransport::RemoteToMainLocal(
        const Locator_t& remote) const
{
    return false;
}

bool GapsTransport::transform_remote_locator(
        const Locator_t& remote_locator,
        Locator_t& result_locator) const
{
    return false;
}

bool GapsTransport::send(
        const octet* send_buffer,
        uint32_t send_buffer_size,
        int gapsDescriptor,
        fastrtps::rtps::LocatorsIterator* destination_locators_begin,
        fastrtps::rtps::LocatorsIterator* destination_locators_end,
        const std::chrono::steady_clock::time_point& max_blocking_time_point)
{
    fastrtps::rtps::LocatorsIterator& it = *destination_locators_begin;

    bool ret = true;

    auto time_out = std::chrono::duration_cast<std::chrono::microseconds>(
        max_blocking_time_point - std::chrono::steady_clock::now());

    while (it != *destination_locators_end)
    {
        if (IsLocatorSupported(*it))
        {
            ret &= send(send_buffer, 
                send_buffer_size, 
                gapsDescriptor,
                *it,
                time_out);
        }

        ++it;
    }

    return ret;
}

bool GapsTransport::send(
        const octet* send_buffer,
        uint32_t send_buffer_size,
        int gapsDescriptor,
        const fastrtps::rtps::Locator_t& remote_locator,
        const std::chrono::microseconds& timeout)
{
    // TODO pirate_write()
    return false;
}

void GapsTransport::select_locators(
        LocatorSelector& selector) const
{
    fastrtps::ResourceLimitedVector<LocatorSelectorEntry*>& entries = selector.transport_starts();

    for (size_t i = 0; i < entries.size(); ++i)
    {
        LocatorSelectorEntry* entry = entries[i];
        if (entry->transport_should_process)
        {
            bool selected = false;

            for (size_t j = 0; j < entry->unicast.size(); ++j)
            {
                if (IsLocatorSupported(entry->unicast[j]) && !selector.is_selected(entry->unicast[j]))
                {
                    entry->state.unicast.push_back(j);
                    selected = true;
                }
            }

            // Select this entry if necessary
            if (selected)
            {
                selector.select(i);
            }
        }
    }
}

bool GapsTransport::fillMetatrafficMulticastLocator(
        Locator_t& locator,
        uint32_t metatraffic_multicast_port) const
{
    return false;
}

bool GapsTransport::fillMetatrafficUnicastLocator(
        Locator_t& locator,
        uint32_t metatraffic_unicast_port) const
{
    return false;
}

bool GapsTransport::configureInitialPeerLocator(
        Locator_t& locator,
        const PortParameters& port_params,
        uint32_t domainId,
        LocatorList_t& list) const
{
    return false;
}

bool GapsTransport::fillUnicastLocator(
        Locator_t& locator,
        uint32_t well_known_port) const
{
    return false;
}
