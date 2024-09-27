#include "world.h"

void World::load_from_mem(gameupdatepacket_t* packet)
{
    auto extended = utils::get_extended(packet);
    extended += 4;
    auto data = extended + 6;
    auto name_length = *(short*)data;

    char* str_len = new char[name_length + 1];
    memcpy(str_len, data + sizeof(short), name_length);
    char none = '\0';
    memcpy(str_len + name_length, &none, 1);

    name = std::string(str_len);
    connected = true;
    delete[] str_len;
    PRINTC("world name is %s\n", name.c_str());
}