#include "server.h"
#include <iostream>
#include "events.h"
#include "gt.hpp"
#include "proton/hash.hpp"
#include "proton/rtparam.hpp"
#include "utils/utils.h"
Server* server = new Server();

void Server::handle_outgoing() {
    ENetEvent evt;
    while (enet_host_service(proxy_server, &evt, 0) > 0) {
        gt_peer = evt.peer;

        switch (evt.type) {
        case ENET_EVENT_TYPE_CONNECT: {
            if (!this->connect())
                return;
        } break;
        case ENET_EVENT_TYPE_RECEIVE: {
            int packet_type = get_packet_type(evt.packet);
            switch (packet_type) {
            case NET_MESSAGE_GENERIC_TEXT:
                if (events::out::generictext(utils::get_text(evt.packet))) {
                    enet_packet_destroy(evt.packet);
                    return;
                }
                break;
            case NET_MESSAGE_GAME_MESSAGE:
                if (events::out::gamemessage(utils::get_text(evt.packet))) {
                    enet_packet_destroy(evt.packet);
                    return;
                }
                break;
            case NET_MESSAGE_GAME_PACKET: {
                auto packet = utils::get_struct(evt.packet);
                if (!packet)
                    break;

                switch (packet->m_type) {
                case PACKET_STATE:
                    if (events::out::state(packet)) {
                        enet_packet_destroy(evt.packet);
                        return;
                    }
                    break;
                case PACKET_CALL_FUNCTION:
                    if (events::out::variantlist(packet)) {
                        enet_packet_destroy(evt.packet);
                        return;
                    }
                    break;

                case PACKET_PING_REPLY:
                    if (events::out::pingreply(packet)) {
                        enet_packet_destroy(evt.packet);
                        return;
                    }
                    break;
                case PACKET_DISCONNECT:
                case PACKET_APP_INTEGRITY_FAIL:
                    if (gt::in_game)
                        return;
                    break;

                default: PRINTS("gamepacket type: %d\n", packet->m_type);
                }
            } break;
            case NET_MESSAGE_TRACK: 
            case NET_MESSAGE_CLIENT_LOG_RESPONSE: return;

            default: PRINTS("Got unknown packet of type %d.\n", packet_type); break;
            }

            if (!server_peer || !real_server)
                return;
            enet_peer_send(server_peer, 0, evt.packet);
            enet_host_flush(real_server);
        } break;
        case ENET_EVENT_TYPE_DISCONNECT: {
            if (gt::in_game)
                return;
            if (gt::connecting) {
                this->disconnect(false);
                gt::connecting = false;
                return;
            }

        } break;
        default: PRINTS("UNHANDLED\n"); break;
        }
    }
}
void Server::handle_incoming() {
    ENetEvent event;

    while (enet_host_service(real_server, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT: PRINTC("connection event\n"); break;
        case ENET_EVENT_TYPE_DISCONNECT: this->disconnect(true); return;
        case ENET_EVENT_TYPE_RECEIVE: {
            if (event.packet->data) {
                int packet_type = get_packet_type(event.packet);
                switch (packet_type) {
                case NET_MESSAGE_GENERIC_TEXT:
                    if (events::in::generictext(utils::get_text(event.packet))) {
                        enet_packet_destroy(event.packet);
                        return;
                    }
                    break;
                case NET_MESSAGE_GAME_MESSAGE:
                    if (events::in::gamemessage(utils::get_text(event.packet))) {
                        enet_packet_destroy(event.packet);
                        return;
                    }
                    break;
                case NET_MESSAGE_GAME_PACKET: {
                    auto packet = utils::get_struct(event.packet);
                    if (!packet)
                        break;

                    switch (packet->m_type) {
                    case PACKET_CALL_FUNCTION:
                        if (events::in::variantlist(packet)) {
                            enet_packet_destroy(event.packet);
                            return;
                        }
                        break;

                    case PACKET_SEND_MAP_DATA:
                        if (events::in::sendmapdata(packet)) {
                            enet_packet_destroy(event.packet);
                            return;
                        }
                        break;

                    case PACKET_STATE:
                        if (events::in::state(packet)) {
                            enet_packet_destroy(event.packet);
                            return;
                        }
                        break;
                    default: PRINTC("gamepacket type: %d\n", packet->m_type); break;
                    }
                } break;
                case NET_MESSAGE_TRACK:
                    if (events::in::tracking(utils::get_text(event.packet))) {
                        enet_packet_destroy(event.packet);
                        return;
                    }
                    break;
                case NET_MESSAGE_CLIENT_LOG_REQUEST: return;

                default: PRINTS("Got unknown packet of type %d.\n", packet_type); break;
                }
            }

            if (!gt_peer || !proxy_server)
                return;
            enet_peer_send(gt_peer, 0, event.packet);
            enet_host_flush(proxy_server);

        } break;

        default: PRINTC("UNKNOWN event: %d\n", event.type); break;
        }
    }
}

void Server::poll() {
    //outgoing packets going to real server that are intercepted by our proxy server
    this->handle_outgoing();

    if (!real_server)
        return;

    //ingoing packets coming to gt client intercepted by our proxy client
    this->handle_incoming();
}

bool Server::start() {
    ENetAddress address;
    enet_address_set_host(&address, "0.0.0.0");
    address.port = proxyport;
    proxy_server = enet_host_create(&address, 1024, 10, 0, 0);
    proxy_server->usingNewPacket = false;

    if (!proxy_server) {
        PRINTS("failed to start the proxy server!\n");
        return false;
    }
    proxy_server->checksum = enet_crc32;
    auto code = enet_host_compress_with_range_coder(proxy_server);
    if (code != 0)
        PRINTS("enet host compressing failed\n");
    PRINTS("started the enet server.\n");
    return setup_client();
}

void Server::quit() {
    gt::in_game = false;
    this->disconnect(true);
}

bool Server::setup_client() {
    real_server = enet_host_create(0, 1, 2, 0, 0);
    real_server->usingNewPacket = true;
    if (!real_server) {
        PRINTC("failed to start the client\n");
        return false;
    }
    real_server->checksum = enet_crc32;
    auto code = enet_host_compress_with_range_coder(real_server);
    if (code != 0)
        PRINTC("enet host compressing failed\n");
    enet_host_flush(real_server);
    PRINTC("Started enet client\n");
    return true;
}

void Server::redirect_server(variantlist_t& varlist) {
    port = varlist[1].get_uint32();
    token = varlist[2].get_uint32();
    user = varlist[3].get_uint32();
    auto str = varlist[4].get_string();

    auto doorid = str.substr(str.find("|"));
    ip = str.erase(str.find("|")); //remove | and doorid from end
    PRINTC("port: %d token %d user %d server %s doorid %s\n", port, token, user, ip.c_str(), doorid.c_str());
    varlist[1] = proxyport;
    varlist[4] = "127.0.0.1" + doorid;

    gt::connecting = true;
    send(true, varlist);
    if (real_server) {
        enet_host_destroy(real_server);
        real_server = nullptr;
    }
}

void Server::disconnect(bool reset) {
    world.connected = false;
    world.players.clear();
    if (server_peer) {
        enet_peer_disconnect(server_peer, 0);
        server_peer = nullptr;
        enet_host_destroy(real_server);
        real_server = nullptr;
    }
    if (reset) {
        user = 0;
        token = 0;
        ip = "213.179.209.168";
        port = 17179;
    }
}

bool Server::connect() {
    PRINTS("Connecting to server.\n");
    ENetAddress address;
    enet_address_set_host(&address, ip.c_str());
    address.port = port;
    PRINTS("port is %d and server is %s\n", port, ip.c_str());
    if (!this->setup_client()) {
        PRINTS("Failed to setup client when trying to connect to server!\n");
        return false;
    }
    server_peer = enet_host_connect(real_server, &address, 2, 0);
    if (!server_peer) {
        PRINTS("Failed to connect to real server.\n");
        return false;
    }
    return true;
}

//bool client: true - sends to growtopia client    false - sends to gt server
void Server::send(bool client, int32_t type, uint8_t* data, int32_t len) {
    auto peer = client ? gt_peer : server_peer;
    auto host = client ? proxy_server : real_server;

    if (!peer || !host)
        return;
    auto packet = enet_packet_create(0, len + 5, ENET_PACKET_FLAG_RELIABLE);
    auto game_packet = (gametextpacket_t*)packet->data;
    game_packet->m_type = type;
    if (data)
        memcpy(&game_packet->m_data, data, len);

    memset(&game_packet->m_data + len, 0, 1);
    int code = enet_peer_send(peer, 0, packet);
    if (code != 0)
        PRINTS("Error sending packet! code: %d\n", code);
    enet_host_flush(host);
}

//bool client: true - sends to growtopia client    false - sends to gt server
void Server::send(bool client, variantlist_t& list, int32_t netid, int32_t delay) {
    auto peer = client ? gt_peer : server_peer;
    auto host = client ? proxy_server : real_server;

    if (!peer || !host)
        return;

    uint32_t data_size = 0;
    void* data = list.serialize_to_mem(&data_size, nullptr);

    //optionally we wouldnt allocate this much but i dont want to bother looking into it
    auto update_packet = MALLOC(gameupdatepacket_t, +data_size);
    auto game_packet = MALLOC(gametextpacket_t, +sizeof(gameupdatepacket_t) + data_size);

    if (!game_packet || !update_packet)
        return;

    memset(update_packet, 0, sizeof(gameupdatepacket_t) + data_size);
    memset(game_packet, 0, sizeof(gametextpacket_t) + sizeof(gameupdatepacket_t) + data_size);
    game_packet->m_type = NET_MESSAGE_GAME_PACKET;

    update_packet->m_type = PACKET_CALL_FUNCTION;
    update_packet->m_player_flags = netid;
    update_packet->m_packet_flags |= 8;
    update_packet->m_int_data = delay;
    memcpy(&update_packet->m_data, data, data_size);
    update_packet->m_data_size = data_size;
    memcpy(&game_packet->m_data, update_packet, sizeof(gameupdatepacket_t) + data_size);
    free(update_packet);

    auto packet = enet_packet_create(game_packet, data_size + sizeof(gameupdatepacket_t), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
    enet_host_flush(host);
    free(game_packet);
}

//bool client: true - sends to growtopia client    false - sends to gt server
void Server::send(bool client, std::string text, int32_t type) {
    auto peer = client ? gt_peer : server_peer;
    auto host = client ? proxy_server : real_server;

    if (!peer || !host)
        return;
    auto packet = enet_packet_create(0, text.length() + 5, ENET_PACKET_FLAG_RELIABLE);
    auto game_packet = (gametextpacket_t*)packet->data;
    game_packet->m_type = type;
    memcpy(&game_packet->m_data, text.c_str(), text.length());

    memset(&game_packet->m_data + text.length(), 0, 1);
    int code = enet_peer_send(peer, 0, packet);
    if (code != 0)
        PRINTS("Error sending packet! code: %d\n", code);
    enet_host_flush(host);
}