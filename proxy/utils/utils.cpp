#include "utils.h"
#include <algorithm>
#include <chrono>
#include <random>
#include "proton/variant.hpp"

BYTE* utils::get_struct_pointer_from_tank_packet(ENetPacket* packet) {
    unsigned int packetLenght = packet->dataLength;
    BYTE* result = NULL;
    if (packetLenght >= 0x3C) {
        BYTE* packetData = packet->data;
        result = packetData + 4;

        if (*(BYTE*)(packetData + 16) & 8) {
            if (packetLenght < *(int*)(packetData + 56) + 60) {
                result = 0;
            }
        }
        else {
            int zero = 0;
            memcpy(packetData + 56, &zero, 4);
        }
    }
    return result;
}
char* utils::get_text(ENetPacket* packet) {
    gametankpacket_t* tank = reinterpret_cast<gametankpacket_t*>(packet->data);
    memset(packet->data + packet->dataLength - 1, 0, 1);
    return static_cast<char*>(&tank->m_data);
}
gameupdatepacket_t* utils::get_struct(ENetPacket* packet) {
    if (packet->dataLength < sizeof(gameupdatepacket_t) - 4)
        return nullptr;
    gametankpacket_t* tank = reinterpret_cast<gametankpacket_t*>(packet->data);
    gameupdatepacket_t* gamepacket = reinterpret_cast<gameupdatepacket_t*>(packet->data + 4);
    if (gamepacket->m_packet_flags & 8) {
        if (packet->dataLength < gamepacket->m_data_size + 60) {
            printf("got invalid packet. (too small)\n");
            return nullptr;
        }
        return reinterpret_cast<gameupdatepacket_t*>(&tank->m_data);
    } else
        gamepacket->m_data_size = 0;
    return gamepacket;
}
bool utils::replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}
bool utils::is_number(const std::string& s) {
    return !s.empty() && std::find_if(s.begin() + (*s.data() == '-' ? 1 : 0), s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}
uint32_t utils::to_bgra(uint8_t b, uint8_t g, uint8_t r, uint8_t a) {
    return (b << 24) | (g << 16) | (r << 8) | a;
}
void utils::hsv_to_rgb(float h, float s, float v, uint8_t& r, uint8_t& g, uint8_t& b) {
    int i = static_cast<int>(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6) {
    case 0: r = v * 255; g = t * 255; b = p * 255; break;
    case 1: r = q * 255; g = v * 255; b = p * 255; break;
    case 2: r = p * 255; g = v * 255; b = t * 255; break;
    case 3: r = p * 255; g = q * 255; b = v * 255; break;
    case 4: r = t * 255; g = p * 255; b = v * 255; break;
    case 5: r = v * 255; g = p * 255; b = q * 255; break;
    }
}
bool utils::is_inside(int circle_x, int circle_y, int rad, int x, int y) {
    // Compare radius of circle with distance
    // of its center from given point
    if ((x - circle_x) * (x - circle_x) + (y - circle_y) * (y - circle_y) <= rad * rad)
        return true;
    else
        return false;
}