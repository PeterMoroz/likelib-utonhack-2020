#pragma once

#include "base/bytes.hpp"
#include "base/hash.hpp"
#include "base/utility.hpp"
#include "bc/block.hpp"
#include "bc/blockchain.hpp"
#include "bc/transaction.hpp"
#include "net/host.hpp"

#include <forward_list>

namespace lk
{

//================

// clang-format off
    DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(MessageType, unsigned char,
                                              (NOT_AVAILABLE)
                                                      (HANDSHAKE)
                                                      (PING)
                                                      (PONG)
                                                      (TRANSACTION)
                                                      (GET_BLOCK)
                                                      (BLOCK)
                                                      (BLOCK_NOT_FOUND)
                                                      (GET_INFO)
                                                      (INFO)
    )
// clang-format on


class Core;
class Network;
class Peer;


class HandshakeMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const bc::Block& block, std::uint16_t public_port);
    void serialize(base::SerializationOArchive& oa);
    static HandshakeMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    bc::Block _theirs_top_block;
    std::uint16_t _public_port;

    HandshakeMessage(bc::Block&& top_block, std::uint16_t public_port);
};


class PingMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa);
    static PingMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    PingMessage() = default;
};

//============================================

class PongMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa);
    static PongMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    PongMessage() = default;
};

//============================================

class TransactionMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, bc::Transaction tx);
    void serialize(base::SerializationOArchive& oa);
    static TransactionMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    bc::Transaction _tx;

    TransactionMessage(bc::Transaction tx);
};

//============================================

class GetBlockMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const base::Sha256& block_hash);
    void serialize(base::SerializationOArchive& oa);
    static GetBlockMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    base::Sha256 _block_hash;

    GetBlockMessage(base::Sha256 block_hash);
};

//============================================

class BlockMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, const bc::Block& block);
    void serialize(base::SerializationOArchive& oa);
    static BlockMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    bc::Block _block;

    BlockMessage(bc::Block block);
};

//============================================

class BlockNotFoundMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa, base::Sha256& block_hash);
    void serialize(base::SerializationOArchive& oa);
    static BlockNotFoundMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    base::Sha256 _block_hash;

    BlockNotFoundMessage(base::Sha256 block_hash);
};

//============================================

class GetInfoMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(base::SerializationOArchive& oa);
    static GetInfoMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    GetInfoMessage() = default;
};

//============================================

class InfoMessage
{
  public:
    static constexpr MessageType getHandledMessageType();
    static void serialize(
        base::SerializationOArchive& oa, base::Sha256& top_block_hash, std::vector<net::Endpoint>& available_peers);
    void serialize(base::SerializationOArchive& oa);
    static InfoMessage deserialize(base::SerializationIArchive& ia);
    void handle(Peer& peer, Network& network, Core& core);

  private:
    base::Sha256 _top_block_hash;
    std::vector<net::Endpoint> _available_peers;

    InfoMessage(base::Sha256&& top_block_hash, std::vector<net::Endpoint>&& available_peers);
};

//============================================

class MessageProcessor
{
  public:
    MessageProcessor(Peer& peer, Network& network, Core& core);

    void process(const base::Bytes& raw_message);

  private:
    static const base::TypeList<HandshakeMessage, PingMessage, PongMessage, TransactionMessage, GetBlockMessage,
        BlockMessage, BlockNotFoundMessage, GetInfoMessage, InfoMessage>
        _all_message_types;

    Peer& _peer;
    Network& _network;
    Core& _core;
};


//================

class Peer
{
  public:
    //================
    enum class State
    {
        JUST_ESTABLISHED,
        REQUESTED_BLOCKS,
        SYNCHRONISED
    };
    //================
    Peer(Network& owning_network_object, net::Session& session, Core& _core);
    //================
    [[nodiscard]] std::optional<net::Endpoint> getServerEndpoint() const;
    void setServerEndpoint(net::Endpoint endpoint);
    //================
    void setState(State new_state);
    State getState() const noexcept;
    //================
    void addSyncBlock(bc::Block block);
    bool applySyncs();
    [[nodiscard]] const std::forward_list<bc::Block>& getSyncBlocks() const noexcept;
    //================
    [[nodiscard]] std::unique_ptr<net::Handler> createHandler();
    //================
    void send(const base::Bytes& data);
    void send(base::Bytes&& data);
    //================
  private:
    //================
    class Handler : public net::Handler
    {
      public:
        //===================
        Handler(Peer& owning_peer, Network& owning_network_object, net::Session& handled_session, Core& core);
        ~Handler() override = default;
        //================
        void onReceive(const base::Bytes& data) override;
        // virtual void onSend() = 0;
        void onClose() override;
        //================
      private:
        //================
        Peer& _owning_peer;
        Network& _owning_network_object;
        net::Session& _session;
        Core& _core;
        //================
        MessageProcessor _message_processor;
        //================
    };
    //================
    void doHandshake();
    //================
    Network& _owning_network_object;
    net::Session& _session;
    Core& _core;
    //================
    State _state{State::JUST_ESTABLISHED};
    std::optional<net::Endpoint> _address_for_incoming_connections;
    //================
    std::forward_list<bc::Block> _sync_blocks;
    //================
};


class Network
{
  public:
    //================
    Network(const base::PropertyTree& config, Core& core);
    //================
    void run();
    //================
    [[nodiscard]] std::vector<net::Endpoint> allPeersAddresses() const;
    //================
  private:
    //================
    friend class Peer; // in order to be able to call removePeer from Handler
    //================
    class HandlerFactory : public net::HandlerFactory
    {
      public:
        //================
        explicit HandlerFactory(Network& owning_network_object);
        ~HandlerFactory() override = default;
        //================
        std::unique_ptr<net::Handler> create(net::Session& session) override;
        void destroy() override;
        //================
      private:
        //================
        Network& _owning_network_object;
        //================
    };
    //================
    const base::PropertyTree& _config;
    net::Host _host;
    std::forward_list<Peer> _peers;
    Core& _core;
    //================
    std::optional<std::uint16_t> _public_port;
    //================
    Peer& createPeer(net::Session& session);
    void removePeer(const Peer& peer);
    //================
};

} // namespace lk