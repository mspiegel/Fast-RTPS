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

#include <fastdds/rtps/transport/TransportInterface.h>
#include <fastdds/rtps/transport/gaps/GapsTransportDescriptor.h>

using namespace eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace rtps {

static constexpr uint32_t gaps_default_segment_size = 0;

} // rtps
} // fastdds
} // eprosima

//*********************************************************
// GapsTransportDescriptor
//*********************************************************
GapsTransportDescriptor::GapsTransportDescriptor()
    : TransportDescriptorInterface(gaps_default_segment_size, s_maximumInitialPeersRange)
{
    maxMessageSize = s_maximumMessageSize;
    m_gaps_flags = -1;
}

GapsTransportDescriptor::GapsTransportDescriptor(std::string config, int flags)
    : TransportDescriptorInterface(gaps_default_segment_size, s_maximumInitialPeersRange)
{
    maxMessageSize = s_maximumMessageSize;
    m_gaps_config = config;
    m_gaps_flags = flags;
}

GapsTransportDescriptor::GapsTransportDescriptor(
        const GapsTransportDescriptor& t)
    : TransportDescriptorInterface(gaps_default_segment_size, s_maximumInitialPeersRange)
{
    maxMessageSize = t.max_message_size();
    m_gaps_config = t.m_gaps_config;
    m_gaps_flags = t.m_gaps_flags;
}
