#pragma once

#include "base_rpc.hpp"

#include <base/property_tree.hpp>

namespace rpc
{

enum class ClientMode
{
    GRPC = 0,
    HTTP = 1
};


std::unique_ptr<BaseRpc> create_rpc_client(ClientMode mode, const std::string& connect_address);

std::unique_ptr<BaseRpcServer> create_rpc_server(const base::PropertyTree& config, std::shared_ptr<BaseRpc> interface);


} // namespace rpc
