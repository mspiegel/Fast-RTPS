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

#ifndef _FASTDDS_GAPS_TRANSPORT_DESCRIPTOR_
#define _FASTDDS_GAPS_TRANSPORT_DESCRIPTOR_

#include "fastdds/rtps/transport/TransportDescriptorInterface.h"

namespace eprosima {
namespace fastdds {
namespace rtps {

class TransportInterface;

/**
 * Gaps transport configuration
 *
 * @ingroup TRANSPORT_MODULE
 */
typedef struct GapsTransportDescriptor : public TransportDescriptorInterface
{
    std::string m_gaps_config;
    int m_gaps_flags;

    virtual ~GapsTransportDescriptor()
    {

    }

    virtual TransportInterface* create_transport() const override;
    uint32_t min_send_buffer_size() const override
    {
        return 0;
    }

    RTPS_DllAPI GapsTransportDescriptor();

    RTPS_DllAPI GapsTransportDescriptor(std::string config, int flags);

    RTPS_DllAPI GapsTransportDescriptor(
            const GapsTransportDescriptor& t);

    virtual uint32_t max_message_size() const override
    {
        return maxMessageSize;
    }

}GapsTransportDescriptor;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_GAPS_TRANSPORT_DESCRIPTOR_
