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

#ifndef _FASTDDS_GAPS_TRANSPORT_H_
#define _FASTDDS_GAPS_TRANSPORT_H_

#include <fastdds/rtps/transport/TransportInterface.h>
#include <fastdds/rtps/transport/gaps/GapsTransportDescriptor.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class GapsChannelResource;

class GapsTransport : public TransportInterface
{
public:

    RTPS_DllAPI GapsTransport(
            const GapsTransportDescriptor&);
    void clean();
    const GapsTransportDescriptor* configuration() const;

    bool init() override;

    virtual ~GapsTransport() override;

    /**
     * Starts listening on the specified port, and if the specified address is in the
     * multicast range, it joins the specified multicast group,
     */
    bool OpenInputChannel(
        const fastrtps::rtps::Locator_t&,
        TransportReceiverInterface*, uint32_t) override;

    //! Removes the listening socket for the specified port.
    bool CloseInputChannel(
            const fastrtps::rtps::Locator_t&) override;

    //! Checks whether there are open and bound sockets for the given port.
    bool IsInputChannelOpen(
            const fastrtps::rtps::Locator_t&) const override;

    //! Reports whether Locators correspond to the same port.
    bool DoInputLocatorsMatch(
            const fastrtps::rtps::Locator_t&,
            const fastrtps::rtps::Locator_t&) const override;

    //! Checks for TCP kinds.
    bool IsLocatorSupported(
            const fastrtps::rtps::Locator_t&) const override;

    //! Opens a socket on the given address and port (as long as they are white listed).
    bool OpenOutputChannel(
            SendResourceList& sender_resource_list,
            const fastrtps::rtps::Locator_t&) override;

   void CloseOutputChannel(int gapsDescriptor);

    /**
     * Converts a given remote locator (that is, a locator referring to a remote
     * destination) to the main local locator whose channel can write to that
     * destination. In this case it will return a 0.0.0.0 address on that port.
     */
    fastrtps::rtps::Locator_t RemoteToMainLocal(
            const fastrtps::rtps::Locator_t&) const override;

    /**
     * Transforms a remote locator into a locator optimized for local communications.
     *
     * If the remote locator corresponds to one of the local interfaces, it is converted
     * to the corresponding local address.
     *
     * @param [in]  remote_locator Locator to be converted.
     * @param [out] result_locator Converted locator.
     *
     * @return false if the input locator is not supported/allowed by this transport, true otherwise.
     */
    bool transform_remote_locator(
            const fastrtps::rtps::Locator_t& remote_locator,
            fastrtps::rtps::Locator_t& result_locator) const override;

   virtual bool send(
           const fastrtps::rtps::octet* send_buffer,
           uint32_t send_buffer_size,
           int gapsDescriptor,
           fastrtps::rtps::LocatorsIterator* destination_locators_begin,
           fastrtps::rtps::LocatorsIterator* destination_locators_end,
           const std::chrono::steady_clock::time_point& max_blocking_time_point);

    fastrtps::rtps::LocatorList_t NormalizeLocator(
            const fastrtps::rtps::Locator_t& locator) override;

    bool is_local_locator(
            const fastrtps::rtps::Locator_t& locator) const override;

    TransportDescriptorInterface* get_configuration() override { return &configuration_; }

    void AddDefaultOutputLocator(
            fastrtps::rtps::LocatorList_t& defaultList) override;

    bool getDefaultMetatrafficMulticastLocators(
            fastrtps::rtps::LocatorList_t& locators,
            uint32_t metatraffic_multicast_port) const override;

    bool getDefaultMetatrafficUnicastLocators(
            fastrtps::rtps::LocatorList_t& locators,
            uint32_t metatraffic_unicast_port) const override;

    bool getDefaultUnicastLocators(
            fastrtps::rtps::LocatorList_t& locators,
            uint32_t unicast_port) const override;

    void select_locators(
            fastrtps::rtps::LocatorSelector& selector) const override;

    bool fillMetatrafficMulticastLocator(
            fastrtps::rtps::Locator_t& locator,
            uint32_t metatraffic_multicast_port) const override;

    bool fillMetatrafficUnicastLocator(
            fastrtps::rtps::Locator_t& locator,
            uint32_t metatraffic_unicast_port) const override;

    bool configureInitialPeerLocator(
            fastrtps::rtps::Locator_t& locator,
            const fastrtps::rtps::PortParameters& port_params,
            uint32_t domainId,
            fastrtps::rtps::LocatorList_t& list) const override;

    bool fillUnicastLocator(
            fastrtps::rtps::Locator_t& locator,
            uint32_t well_known_port) const override;

    uint32_t max_recv_buffer_size() const override
    {
        return (std::numeric_limits<uint32_t>::max)();
    }

private:

    //! Constructor with no descriptor is necessary for implementations derived from this class.
    GapsTransport();

    GapsTransportDescriptor configuration_;

    //! Checks for whether locator is allowed.
    bool is_locator_allowed(
            const fastrtps::rtps::Locator_t&) const override;

    mutable std::recursive_mutex input_channels_mutex_;

    std::vector<GapsChannelResource*> input_channels_;

    friend class GapsChannelResource;

protected:

    /**
     * Creates an input channel
     * @param locator Listening locator
     * @param max_msg_size Maximum message size supported by the channel
     * @throw std::exception& If the channel cannot be created
     */
    virtual GapsChannelResource* CreateInputChannelResource(
            const fastrtps::rtps::Locator_t& locator,
            uint32_t max_msg_size,
            TransportReceiverInterface* receiver);

    /**
     * Send a buffer to a destination
     */
    bool send(
        const fastrtps::rtps::octet* send_buffer,
        uint32_t send_buffer_size,
        int gapsDescriptor,
        const fastrtps::rtps::Locator_t& remote_locator,
        const std::chrono::microseconds& timeout);

private:

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_GAPS_TRANSPORT_H_
