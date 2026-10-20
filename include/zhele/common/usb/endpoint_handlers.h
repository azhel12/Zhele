/**
 * @file
 * Implement endpoint`s handlers dispatching
 * 
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license MIT
 */

#ifndef ZHELE_COMMON_USB_ENDPOINT_HANDLERS_H
#define ZHELE_COMMON_USB_ENDPOINT_HANDLERS_H

#include "endpoint.h"

#include <zhele/common/template_utils/type_list.h>

#include <array>
#include <type_traits>

namespace Zhele::Usb
{
    /**
     * @brief Endpoint transfer complete callback
     */
    using EpRequestHandler = std::add_pointer_t<void()>;

    /**
     * @brief Implements endpoint`s handlers management
     * 
     * @tparam Endpoints All endpoints
     */
    template<typename... Endpoints>
    class EndpointHandlers
    {
        /**
         * @brief Builds indexes array for given interfaces
         * 
         * @details I have been use template_utils::type_unbox<> for parameters because on element of Endpoints
         *  is Typebox without inherit - it is template_utils::type_box<DeviceBase<XXX>>, where DeviceBase<XXX> is incomplete type 
         * 
         * @returns std::array with indexes
        */
        static consteval auto BuildIndexesArray()
        {
            constexpr auto endpoints = template_utils::type_list<Endpoints...>{};
            constexpr auto maxEndpointsNumber = template_utils::type_unbox<endpoints.sort([](auto a, auto b) {
                return template_utils::type_unbox<a>::Number < template_utils::type_unbox<b>::Number;
            }).back()>::Number;

            std::array<int8_t, (maxEndpointsNumber + 1) * 2> indexes {};

            for(auto& i : indexes) {
                i = -1;
            }

            endpoints.foreach([&indexes, i{0}](auto endpoint) mutable {
                if (template_utils::type_unbox<endpoint>::Direction == EndpointDirection::In || template_utils::type_unbox<endpoint>::Direction == EndpointDirection::Bidirectional)
                    indexes[2 * template_utils::type_unbox<endpoint>::Number] = i;
                if (template_utils::type_unbox<endpoint>::Direction == EndpointDirection::Out || template_utils::type_unbox<endpoint>::Direction == EndpointDirection::Bidirectional)
                    indexes[1 + 2 * template_utils::type_unbox<endpoint>::Number] = i;
                ++i;
            });

            return indexes;
        }

        static constexpr EpRequestHandler _handlers[] = {Endpoints::Handler...};
        static constexpr auto _handlersIndexes = BuildIndexesArray();
    public:

        template<typename... Ep>
        constexpr EndpointHandlers(template_utils::type_list<Ep...> endpoints) {}

        inline static void Handle(uint8_t number, EndpointDirection direction)
        {
            _handlers[_handlersIndexes[2 * number + (direction == EndpointDirection::Out ? 1 : 0)]]();
        }
    };
    /// deduction guid for EndpointHandlers
    template<typename... Ep>
    EndpointHandlers(template_utils::type_list<Ep...> endpoints) -> EndpointHandlers<Ep...>;

}
#endif // ZHELE_COMMON_USB_ENDPOINT_HANDLERS_H
