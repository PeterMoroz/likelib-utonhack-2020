#include "protocol.hpp"

#include "base/log.hpp"
#include "lk/core.hpp"


namespace
{

DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(
        MessageType, unsigned char, (NOT_AVAILABLE)(HANDSHAKE)(PING)(PONG)(TRANSACTION)(GET_BLOCK)(BLOCK)(BLOCK_NOT_FOUND)(GET_INFO)(INFO))

template<typename... Args>
base::Bytes createMessage(MessageType type, Args&&... args)
{
    base::SerializationOArchive ret;
    ret << type;
    (ret << ... << std::forward<Args>(args));
    return ret.getBytes();
}

}


namespace lk
{

//=====================================

Peer::Handler::Handler(Peer& owning_peer, Network& owning_network_object, net::Session& handled_session, Core& core)
    : _owning_peer{owning_peer}, _owning_network_object{owning_network_object}, _session{handled_session}, _core{core}
{}


void Peer::Handler::onReceive(const base::Bytes& data)
{
    if(_session.isClosed()) {
        return;
    }

    base::SerializationIArchive ia(data);
    MessageType type;
    ia >> type;
    base::Bytes data_without_type;
    ia >> data_without_type;

    if(_owning_peer._state != State::SYNCHRONISED) {
        if(type == MessageType::HANDSHAKE) {
            base::Bytes top_block_hash;
            ia >> top_block_hash;
            onHandshake(base::Sha256(top_block_hash));
        }
        else if(type == MessageType::BLOCK) {
            bc::Block block = bc::Block::deserialize(ia);
            onBlock(std::move(block));
        }
        else {
            LOG_DEBUG << "Received on non-handshaked connection with peer " << _session.getId();
        }
    }
    else {
        switch(type) {
            case MessageType::PING: {
                onPing();
                break;
            }
            case MessageType::PONG: {
                onPong();
                break;
            }
            case MessageType::TRANSACTION: {
                bc::Transaction tx;
                ia >> tx;
                onTransaction(std::move(tx));
                break;
            }
            case MessageType::BLOCK: {
                auto block = bc::Block::deserialize(ia);
                onBlock(std::move(block));
                break;
            }
            case MessageType::GET_BLOCK: {
                base::Bytes block_hash;
                ia >> block_hash;
                onGetBlock(base::Sha256(std::move(block_hash)));
                break;
            }
            case MessageType::INFO: {
                base::Bytes top_block_hash;
                std::vector<std::string> available_peers_strings;
                ia >> top_block_hash >> available_peers_strings;
                std::vector<net::Endpoint> available_peers;
                for(auto&& s : available_peers_strings) {
                    available_peers.emplace_back(std::move(s));
                }
                onInfo(std::move(top_block_hash), std::move(available_peers));
                break;
            }
            case MessageType::GET_INFO: {
                onGetInfo();
                break;
            }
            default: {
                LOG_DEBUG << "Received an invalid block from peer " << _session.getId();
                break;
            }
        }
    }
}


void Peer::Handler::onClose()
{
    LOG_DEBUG << "Closing peer";
    _owning_network_object.removePeer(_owning_peer);
}


void Peer::Handler::onHandshake(base::Sha256&& top_block_hash)
{

}


void Peer::Handler::onPing()
{
    _session.send(createMessage(MessageType::PONG));
}


void Peer::Handler::onPong()
{}


void Peer::Handler::onTransaction(bc::Transaction&& tx)
{
    _core.performTransaction(tx);
}


void Peer::Handler::onBlock(bc::Block&& block)
{
    _core.tryAddBlock(block);
}


void Peer::Handler::onGetBlock(base::Sha256&& block_hash)
{
    auto block = _core.findBlock(block_hash);
    if(block) {
        _session.send(createMessage(MessageType::BLOCK, *block));
    }
    else {
        _session.send(createMessage(MessageType::BLOCK_NOT_FOUND, block_hash.getBytes()));
    }
}


void Peer::Handler::onGetInfo()
{
    _session.send(createMessage(MessageType::INFO, _core.getTopBlock(), _owning_network_object.allPeersAddresses()));
}


void Peer::Handler::onInfo(base::Sha256&& top_block_hash, std::vector<net::Endpoint>&& available_peers)
{

}


Peer::Peer(Network& owning_network_object, net::Session& session, Core& core)
    : _owning_network_object{owning_network_object}, _session{session}, _core{core}
{}


std::unique_ptr<Peer::Handler> Peer::createHandler()
{
    return std::make_unique<Peer::Handler>(*this, _owning_network_object, _session, _core);
}


std::optional<net::Endpoint> Peer::getServerEndpoint() const
{

}

//=====================================

Network::HandlerFactory::HandlerFactory(Network& owning_network_object) : _owning_network_object{owning_network_object}
{}


std::unique_ptr<net::Handler> Network::HandlerFactory::create(net::Session& session)
{
    auto& peer = _owning_network_object.createPeer(session);
    return peer.createHandler();
}


void Network::HandlerFactory::destroy()
{}

//=====================================

Network::Network(const base::PropertyTree& config, Core& core) : _host{config}, _core{core}
{}


Peer& Network::createPeer(net::Session& session)
{
    auto& peer = _peers.emplace_back(*this, session, _core);
    return peer;
}


void Network::removePeer(const Peer& peer)
{}


void Network::broadcastBlock(const bc::Block& block)
{
    base::SerializationOArchive oa;
    oa << MessageType::BLOCK << block;
    _host.broadcast(std::move(oa).getBytes());
}


void Network::broadcastTransaction(const bc::Transaction& tx)
{
    base::SerializationOArchive oa;
    oa << MessageType::TRANSACTION << tx;
    _host.broadcast(std::move(oa).getBytes());
}


std::vector<net::Endpoint> Network::allPeersAddresses() const
{
    std::vector<net::Endpoint> ret;
    for(const auto& peer : _peers) {
        if(auto server_endpoint = peer.getServerEndpoint()) {
            ret.push_back(*std::move(server_endpoint));
        }
    }
}


void Network::run()
{
    _host.run(std::make_unique<HandlerFactory>(*this));
}

//=====================================

// void Peer::onReceive(const base::Bytes& bytes)
//{
//    LOG_DEBUG << "onReceive " << bytes.toString();
//}
//
//
// void Peer::onClose()
//{
//    LOG_DEBUG << "onClose";
//}
//
//
// std::unique_ptr<net::Handler> HandlerFactory::create()
//{
//    auto ret = std::make_unique<Handler>();
//    return std::move(ret);
//}
//
//
//
// Network::Network(const base::PropertyTree& config, Core& core) : _config(config), _core{core}, _host{_config}
//{}
//
//
// void Network::broadcastBlock(const bc::Block& block)
//{
//    base::SerializationOArchive oa;
//    oa << MessageType::BLOCK << block;
//    _host.broadcast(std::move(oa).getBytes());
//}
//
//
// void Network::broadcastTransaction(const bc::Transaction& tx)
//{
//    base::SerializationOArchive oa;
//    oa << MessageType::TRANSACTION << tx;
//    _host.broadcast(std::move(oa).getBytes());
//}
//
//
// void Network::run()
//{
//    auto on_accept = [this](net::Session& session) {
//        auto it = _peers.find(session.getId());
//        if(it != _peers.end()) {
//            LOG_DEBUG << "SessionManager is already present for node " << session.getId();
//            return;
//        }
//        else {
//            auto [inserted_it, is_inserted] =
//                _peers.insert({session.getId(), SessionManager::accepted(session, _core, _host)});
//            ASSERT(is_inserted);
//            it = inserted_it;
//        }
//    };
//
//    auto on_connect = [this](net::Session& session) {
//        auto it = _peers.find(session.getId());
//        if(it != _peers.end()) {
//            LOG_DEBUG << "SessionManager is already present for node " << session.getId();
//            return;
//        }
//        else {
//            auto [inserted_it, is_inserted] =
//                _peers.insert({session.getId(), SessionManager::connected(session, _core, _host)});
//            ASSERT(is_inserted);
//            it = inserted_it;
//        }
//    };
//
//    auto on_receive = [this](net::Session& session, const base::Bytes& received_data) {
//        auto it = _peers.find(session.getId());
//        ASSERT(it != _peers.end());
//        auto& session_manager = *it->second;
//        session_manager.handle(session, received_data);
//    };
//
//    _host.run(on_accept, on_connect, on_receive);
//}
//
//
// std::unique_ptr<SessionManager> SessionManager::accepted(net::Session& session, Core& core, net::Host& host)
//{
//    std::unique_ptr<SessionManager> ret{new SessionManager(core, host)};
//    ret->_state = State::ACCEPTED;
//    ret->handshakeRoutine(session);
//    return ret;
//}
//
//
// std::unique_ptr<SessionManager> SessionManager::connected(net::Session& session, Core& core, net::Host& host)
//{
//    std::unique_ptr<SessionManager> ret{new SessionManager(core, host)};
//    ret->_state = State::CONNECTED;
//    ret->handshakeRoutine(session);
//    return ret;
//}
//
//
// SessionManager::SessionManager(Core& core, net::Host& host) : _core{core}, _host{host}
//{}
//
//
// void SessionManager::handle(net::Session& session, const base::Bytes& data)
//{
//    if(session.isClosed()) {
//        return;
//    }
//
//    base::SerializationIArchive ia(data);
//    MessageType type;
//    ia >> type;
//
//    if(_state != State::SYNCHRONISED) {
//        if(type == MessageType::HANDSHAKE) {
//            base::Bytes top_block_hash;
//            ia >> top_block_hash;
//            onHandshake(session, base::Sha256(top_block_hash));
//        }
//        else if(type == MessageType::BLOCK) {
//            bc::Block block = bc::Block::deserialize(ia);
//            onSyncBlock(session, std::move(block));
//        }
//        else {
//            LOG_DEBUG << "Received on non-handshaked connection with peer " << session.getId();
//        }
//    }
//    else {
//        switch(type) {
//            case MessageType::PING: {
//                onPing(session);
//                break;
//            }
//            case MessageType::PONG: {
//                break;
//            }
//            case MessageType::TRANSACTION: {
//                bc::Transaction tx;
//                ia >> tx;
//                onTransaction(session, std::move(tx));
//                break;
//            }
//            case MessageType::BLOCK: {
//                auto block = bc::Block::deserialize(ia);
//                onBlock(session, std::move(block));
//                break;
//            }
//            case MessageType::GET_BLOCK: {
//                base::Bytes block_hash;
//                ia >> block_hash;
//                onGetBlock(session, base::Sha256(block_hash));
//                break;
//            }
//            case MessageType::INFO: {
//                break;
//            }
//            default: {
//                LOG_DEBUG << "Received an invalid block from peer " << session.getId();
//                break;
//            }
//        }
//    }
//}
//
//
// void SessionManager::onPing(net::Session& session)
//{
//    base::SerializationOArchive msg;
//    msg << MessageType::PONG;
//    session.send(std::move(msg).getBytes());
//}
//
//
// void SessionManager::onPong(net::Session& session)
//{}
//
//
// void SessionManager::onTransaction(net::Session& session, bc::Transaction&& tx)
//{
//    LOG_TRACE << "Session::onTransaction peer " << session.getId();
//    _core.performTransaction(tx);
//}
//
//
// void SessionManager::onBlock(net::Session& session, bc::Block&& block)
//{
//    LOG_TRACE << "Session::onBlock peer " << session.getId();
//    _core.tryAddBlock(std::move(block));
//}
//
//
// void SessionManager::onGetBlock(net::Session& session, base::Sha256&& block_hash)
//{
//    LOG_TRACE << "Session::onGetBlock peer " << session.getId();
//    auto block = _core.findBlock(block_hash);
//    if(block) {
//        base::SerializationOArchive msg;
//        msg << MessageType::BLOCK << *block;
//        session.send(std::move(msg).getBytes());
//    }
//}
//
//
// void SessionManager::onInfo(net::Session& session)
//{
//    LOG_TRACE << "Session::onInfo peer " << session.getId();
//    base::SerializationOArchive oa;
//    oa << MessageType::INFO << base::Sha256::compute(base::toBytes(_core.getTopBlock())).getBytes();
//    session.send(std::move(oa).getBytes());
//}
//
//
// void SessionManager::handshakeRoutine(net::Session& session)
//{
//    LOG_TRACE << "Session::handshakeRoutine peer " << session.getId();
//    base::SerializationOArchive oa;
//    oa << MessageType::HANDSHAKE << base::Sha256::compute(base::toBytes(_core.getTopBlock())).getBytes();
//    session.send(std::move(oa).getBytes());
//}
//
//
// void SessionManager::onHandshake(net::Session& session, base::Sha256&& top_block_hash)
//{
//    LOG_DEBUG << "SessionManager::onHandshake with node " << session.getId();
//    if(top_block_hash != base::Sha256::compute(base::toBytes(_core.getTopBlock()))) {
//        // someone needs to synchronise
//        if(_core.findBlock(top_block_hash)) {
//            LOG_INFO << "Synchronized with node " << session.getId();
//            _state = State::SYNCHRONISED;
//            return; // we have the top block of other node, so we does't sync
//        }
//        else {
//            // we need to synchronise
//            LOG_INFO << "Will synchronize with node " << session.getId();
//            base::SerializationOArchive oa;
//            oa << MessageType::GET_BLOCK << top_block_hash.getBytes();
//            session.send(std::move(oa).getBytes());
//        }
//    }
//    else {
//        LOG_DEBUG << "We doesn't need to synchronize with node = " << session.getId();
//        _state = State::SYNCHRONISED;
//    }
//}
//
//
// void SessionManager::onSyncBlock(net::Session& session, bc::Block&& block)
//{
//    LOG_DEBUG << "SessionManager::onSyncBlock with node " << session.getId();
//    if(_core.findBlock(base::Sha256::compute(base::toBytes(block)))) {
//        // we reached the block we have, so right now we can add received top-blocks
//        LOG_DEBUG << "Applying received blocks from node " << session.getId();
//        while(!_sync_blocks_stack.empty()) {
//            _core.tryAddBlock(_sync_blocks_stack.top());
//            _sync_blocks_stack.pop();
//        }
//        _state = State::SYNCHRONISED;
//    }
//    else {
//        LOG_DEBUG << "Received sync block from node " << session.getId();
//        base::SerializationOArchive oa;
//        oa << MessageType::GET_BLOCK << block.getPrevBlockHash().getBytes();
//        session.send(std::move(oa).getBytes());
//
//        _sync_blocks_stack.push(std::move(block));
//    }
//}


} // namespace lk