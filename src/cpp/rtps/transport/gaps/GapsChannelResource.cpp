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

#include "libpirate.h"

#include <fastdds/rtps/transport/gaps/GapsTransport.h>
#include <fastdds/rtps/transport/gaps/GapsChannelResource.h>
#include <fastdds/rtps/messages/MessageReceiver.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Locator_t = fastrtps::rtps::Locator_t;
using octet = fastrtps::rtps::octet;

GapsChannelResource::GapsChannelResource(
        GapsTransport* transport,
        int gapsDescriptor,
        uint32_t maxMsgSize,
        const Locator_t& locator,
        TransportReceiverInterface* receiver)
    : ChannelResource(maxMsgSize)
    , message_receiver_(receiver)
    , gapsDescriptor_(gapsDescriptor)
    , transport_(transport)
    , locator_(locator)
{
    thread(std::thread(&GapsChannelResource::perform_listen_operation, this, locator));
}

GapsChannelResource::~GapsChannelResource()
{
    message_receiver_ = nullptr;
}

void GapsChannelResource::perform_listen_operation(Locator_t input_locator)
{
    Locator_t remote_locator;

    while (alive())
    {
        // Blocking receive.
        auto& msg = message_buffer();
        if (!Receive(msg.buffer, msg.max_size, msg.length, remote_locator))
        {
            continue;
        }

        // Processes the data through the CDR Message interface.
        if (message_receiver() != nullptr)
        {
            message_receiver()->OnDataReceived(msg.buffer, msg.length, input_locator, remote_locator);
        }
        else if (alive())
        {
            logWarning(RTPS_MSG_IN, "Received Message, but no receiver attached");
        }
    }

    message_receiver(nullptr);
}

bool GapsChannelResource::Receive(
        octet* receive_buffer,
        uint32_t receive_buffer_capacity,
        uint32_t& receive_buffer_size,
        Locator_t& remote_locator)
{
    ssize_t rv = pirate_read(gapsDescriptor_, receive_buffer, receive_buffer_capacity);
    if (rv < 0) {
        logError(RTPS_MSG_OUT, "GapsTransport error reading from channel " << remote_locator.config
            << " with msg: " << strerror(errno));
        return false;
    }
    receive_buffer_size = (uint32_t) rv;
    return (receive_buffer_size > 0);
}

void GapsChannelResource::release()
{
    pirate_close(gapsDescriptor_);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
