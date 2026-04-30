/**
 ********************************************************************************
 * @file    FSBProtocolTypes.hpp
 * @brief   Lightweight types shared between FSB protocol ingress and routing.
 ********************************************************************************
 */

#ifndef COMPONENTS_FSBPROTOCOL_FSBPROTOCOLTYPES_HPP_
#define COMPONENTS_FSBPROTOCOL_FSBPROTOCOLTYPES_HPP_

#include <cstdint>

#include "CommandMessage.hpp"

struct FSBProtoCommandRoute {
    Proto::Node source;
    Proto::Node target;
    uint32_t sourceSequenceNum;
    Proto::CommandMessage::FieldNumber messageType;
    uint32_t commandEnum;
    int32_t commandParam;
};

#endif /* COMPONENTS_FSBPROTOCOL_FSBPROTOCOLTYPES_HPP_ */
