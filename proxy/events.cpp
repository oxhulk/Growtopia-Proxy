#include "events.h"
#include "gt.hpp"
#include "proton/hash.hpp"
#include "proton/rtparam.hpp"
#include "proton/variant.hpp"
#include "server.h"
#include "utils/utils.h"
#include <thread>
#include <limits.h>

bool events::out::variantlist(gameupdatepacket_t* packet) {
    variantlist_t varlist{};
    varlist.serialize_from_mem(utils::get_extended(packet));
    PRINTS("varlist: %s\n", varlist.print().c_str());
    return false;
}
bool events::out::pingreply(gameupdatepacket_t* packet) {
    packet->m_vec2_x = 1000.f;  
    packet->m_vec2_y = 250.f;  
    packet->m_vec_x = 64.f;    
    packet->m_vec_y = 64.f;  
    packet->m_jump_amount = 0;  
    packet->m_player_flags = 0; 
    return false;
}
bool find_command(std::string chat, std::string name) {
    bool found = chat.find("/" + name) == 0;
    if (found)
        gt::send_log("`6" + chat);
    return found;
}
bool events::out::generictext(std::string packet) {
    PRINTS("Generic text: %s\n", packet.c_str());
    auto& world = server->world;
    rtvar var = rtvar::parse(packet);
    if (!var.valid())
        return false;

    size_t posTankIDPass = packet.find("tankIDPass|");
    if (posTankIDPass != std::string::npos) {
        size_t endPosTankIDPass = packet.find("\n", posTankIDPass);
        if (endPosTankIDPass != std::string::npos) {
            packet.erase(posTankIDPass, endPosTankIDPass - posTankIDPass + 1);
        }
    }
    size_t posRequestedName = packet.find("requestedName|");
    if (posRequestedName != std::string::npos) {
        size_t endPosRequestedName = packet.find("\n", posRequestedName);
        if (endPosRequestedName != std::string::npos) {
            packet.erase(posRequestedName, endPosRequestedName - posRequestedName + 1);
        }
    }
    if (var.get(0).m_key == "action" && var.get(0).m_value == "input") {
        if (var.size() < 2 || var.get(1).m_values.size() < 2 || !world.connected)
            return false;

        auto chat = var.get(1).m_values[1];
        if (find_command(chat, "name ")) {
            std::string name = "``" + chat.substr(6) + "``";
            variantlist_t va{ "OnNameChanged" };
            va[1] = name;
            server->send(true, va, world.local.netid, -1);
            gt::send_log("name set to: " + name);
            return true;
        }
        else if (find_command(chat, "flag ")) {
            int flag = atoi(chat.substr(6).c_str());
            variantlist_t va{ "OnGuildDataChanged" };
            va[1] = 1;
            va[2] = 2;
            va[3] = flag;
            va[4] = 3;
            server->send(true, va, world.local.netid, -1);
            gt::send_log("flag set to item id: " + std::to_string(flag));
            return true;
        }
        else if (find_command(chat, "proxy")) {
            gt::send_log(
                "/name [name] (sets name to name)"
                "/flag [id] (sets flag to item id)"
            );
            return true;
        }
        return false;
    }
    if (packet.find("game_version|") != std::string::npos) {
        rtvar var = rtvar::parse(packet);
        var.set("meta", server->meta);
        packet = var.serialize();
        packet += "requestedName|\n";
        packet += "tankIDPass|\n";
        gt::in_game = false;
        PRINTS("Spoofing login info\n");
        server->send(false, packet);
        return true;
    }
    return false;
}
bool events::out::gamemessage(std::string packet) {
    PRINTS("Game message: %s\n", packet.c_str());
    if (packet == "action|quit") {
        server->quit();
        return true;
    }
    return false;
}
bool events::out::state(gameupdatepacket_t* packet) {
    if (!server->world.connected)
        return false;

    server->world.local.pos = vector2_t{ packet->m_vec_x, packet->m_vec_y };
    PRINTS("local pos: %.0f %.0f\n", packet->m_vec_x, packet->m_vec_y);

    return false;
}
bool events::in::variantlist(gameupdatepacket_t* packet) {
    variantlist_t varlist{};
    auto extended = utils::get_extended(packet);
    extended += 4; 
    varlist.serialize_from_mem(extended);
    PRINTC("varlist: %s\n", varlist.print().c_str());
    auto func = varlist[0].get_string();

    if (func.find("OnSuperMainStartAcceptLogon") != -1)
        gt::in_game = true;

    switch (hs::hash32(func.c_str())) {
    case fnv32("OnRequestWorldSelectMenu"): {
        auto& world = server->world;
        world.players.clear();
        world.local = {};
        world.connected = false;
        world.name = "EXIT";
    } break;
    case fnv32("OnSendToServer"): server->redirect_server(varlist); return true;

    case fnv32("OnConsoleMessage"): {
        varlist[1] = "`4[PROXY]`` " + varlist[1].get_string();
        server->send(true, varlist);
        return true;
    } break;
    case fnv32("OnRemove"): {
        auto text = varlist.get(1).get_string();
        if (text.find("netID|") == 0) {
            auto netid = atoi(text.substr(6).c_str());

            if (netid == server->world.local.netid)
                server->world.local = {};

            auto& players = server->world.players;
            for (size_t i = 0; i < players.size(); i++) {
                auto& player = players[i];
                if (player.netid == netid) {
                    players.erase(std::remove(players.begin(), players.end(), player), players.end());
                    break;
                }
            }
        }
    } break;
    case fnv32("OnSpawn"): {
        std::string meme = varlist.get(1).get_string();
        rtvar var = rtvar::parse(meme);
        auto name = var.find("name");
        auto netid = var.find("netID");
        auto onlineid = var.find("onlineID");
        if (name && netid && onlineid) {
            Player ply{};
            ply.mod = false;
            ply.invis = false;
            ply.name = name->m_value;
            ply.country = var.get("country");
            name->m_values[0] += " `4[" + netid->m_value + "]``";
            auto pos = var.find("posXY");
            if (pos && pos->m_values.size() >= 2) {
                auto x = atoi(pos->m_values[0].c_str());
                auto y = atoi(pos->m_values[1].c_str());
                ply.pos = vector2_t{ float(x), float(y) };
            }
            ply.userid = var.get_int("userID");
            ply.netid = var.get_int("netID");
            if (meme.find("type|local") != -1) {
                var.find("mstate")->m_values[0] = "1";
                server->world.local = ply;
            }
            server->world.players.push_back(ply);
            auto str = var.serialize();
            utils::replace(str, "onlineID", "onlineID|");
            varlist[1] = str;
            PRINTC("new: %s\n", varlist.print().c_str());
            server->send(true, varlist, -1, -1);
            return true;
        }
    } break;
    }
    return false;
}
bool events::in::generictext(std::string packet) {
    PRINTC("Generic text: %s\n", packet.c_str());
    return false;
}
bool events::in::gamemessage(std::string packet) {
    PRINTC("Game message: %s\n", packet.c_str());
    return false;
}
bool events::in::sendmapdata(gameupdatepacket_t* packet) {
    server->world.load_from_mem(packet);
    return false;
}
bool events::in::state(gameupdatepacket_t* packet) {
    if (!server->world.connected)
        return false;
    if (packet->m_player_flags == -1)
        return false;

    auto& players = server->world.players;
    for (auto& player : players) {
        if (player.netid == packet->m_player_flags) {
            player.pos = vector2_t{ packet->m_vec_x, packet->m_vec_y };
            PRINTC("player %s position is %.0f %.0f\n", player.name.c_str(), player.pos.m_x, player.pos.m_y);
            break;
        }
    }
    return false;
}
bool events::in::tracking(std::string packet) {
    PRINTC("Tracking packet: %s\n", packet.c_str());
    return true;
}
