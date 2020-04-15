#pragma once

#include "grpc_adapter.hpp"

namespace rpc::grpc
{


/// Template server was implemented logic to start listening messages by gRPC
class NodeServer : public BaseRpcServer
{
  public:
    /// Constructor that initialize instance of LogicService
    /// \param server_address listening ip:port
    NodeServer(const std::string& server_address, std::shared_ptr<BaseRpc> service);

    /// plain destructor that call GrpcNodeServer::stop()
    ~NodeServer() override;

    /// Register LogicService and start listening port defined in constructor
    void run() override;

    /// stop listening port defined in constructor and started by GrpcNodeServer::run()
    void stop() override;

  private:
    Adapter _service;
    const std::string _server_address;
    std::unique_ptr<::grpc::Server> _server;
};


} // namespace rpc
