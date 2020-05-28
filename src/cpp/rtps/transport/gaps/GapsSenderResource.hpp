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

#ifndef __TRANSPORT_GAPSSENDERRESOURCE_HPP__
#define __TRANSPORT_GAPSSENDERRESOURCE_HPP__

#include <fastdds/rtps/network/SenderResource.h>
#include <fastdds/rtps/transport/gaps/GapsTransport.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class GapsSenderResource : public fastrtps::rtps::SenderResource
{
    public:

        GapsSenderResource(
                GapsTransport& transport,
                int gapsDescriptor)
            : SenderResource(transport.kind())
            , gapsDescriptor_(gapsDescriptor)
        {
            // Implementation functions are bound to the right transport parameters
            clean_up = [this, &transport]()
                {
                    transport.CloseOutputChannel(gapsDescriptor_);
                };

            send_lambda_ = [this, &transport] (
                const fastrtps::rtps::octet* data,
                uint32_t dataSize,
                fastrtps::rtps::LocatorsIterator* destination_locators_begin,
                fastrtps::rtps::LocatorsIterator* destination_locators_end,
                const std::chrono::steady_clock::time_point& max_blocking_time_point) -> bool
                    {
                        return transport.send(data, dataSize, gapsDescriptor_, destination_locators_begin,
                                    destination_locators_end, max_blocking_time_point);
                    };
        }

        virtual ~GapsSenderResource()
        {
            if (clean_up)
            {
                clean_up();
            }
        }

        static GapsSenderResource* cast(TransportInterface& transport, SenderResource* sender_resource)
        {
            GapsSenderResource* returned_resource = nullptr;

            if (sender_resource->kind() == transport.kind())
            {
                returned_resource = dynamic_cast<GapsSenderResource*>(sender_resource);
            }

            return returned_resource;
        }

    private:

        GapsSenderResource() = delete;

        GapsSenderResource(const GapsSenderResource&) = delete;

        GapsSenderResource& operator=(const GapsSenderResource&) = delete;

        int gapsDescriptor_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // __TRANSPORT_GAPSSENDERRESOURCE_HPP__
