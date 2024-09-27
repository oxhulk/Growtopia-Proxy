#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include "enet/include/enet.h"
#include "packet.h"
#include "proton/variant.hpp"

#define PRINTS(msg, ...) printf("[SERVER]: " msg, ##__VA_ARGS__);
#define PRINTC(msg, ...) printf("[CLIENT]: " msg, ##__VA_ARGS__);
#define MALLOC(type, ...) (type*)(malloc(sizeof(type) __VA_ARGS__))
#define get_packet_type(packet) (packet->dataLength > 3 ? *packet->data : 0)
#define DO_ONCE            \
    ([]() {                \
        static char o = 0; \
        return !o && ++o;  \
    }())
#ifdef _WIN32
#define INLINE __forceinline
#else //for gcc/clang
#define INLINE inline
#endif

namespace utils {
    BYTE* get_struct_pointer_from_tank_packet(ENetPacket* packet);
    char* get_text(ENetPacket* packet);
    gameupdatepacket_t* get_struct(ENetPacket* packet);
    bool replace(std::string& str, const std::string& from, const std::string& to);
    INLINE uint8_t* get_extended(gameupdatepacket_t* packet) {
        return reinterpret_cast<uint8_t*>(&packet->m_data_size);
    }
    bool is_number(const std::string& s);
    uint32_t to_bgra(uint8_t b, uint8_t g, uint8_t r, uint8_t a);
    void hsv_to_rgb(float h, float s, float v, uint8_t& r, uint8_t& g, uint8_t& b);
    bool is_inside(int circle_x, int circle_y, int rad, int x, int y);
} // namespace utils
