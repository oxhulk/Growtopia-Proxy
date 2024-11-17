#include "itemdatabase.h"
#include <fstream>
#include <algorithm>
#include <cstring>

template<typename src_type>
static void read(src_type& src, char*& data, size_t size = sizeof(src_type)) {
    if constexpr (std::is_same_v<src_type, std::string>) {
        std::string::size_type string_size{};
        read(string_size, data, 2);
        src.resize(string_size);
        std::copy_n(data, string_size, &src[0]);
        data += string_size;
    }
    else {
        std::copy_n(data, size, reinterpret_cast<char*>(&src));
        data += size;
    }
}

Item* ItemDatabase::get_item(int id) {
    if (id >= 0 && id < item_count) {
        return &items[id];
    }
    return nullptr;
}

void ItemDatabase::load_from_file(const std::string& filePath) {
    std::string secret = "PBG892FXX982ABC*";
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file) {
        return;
    }
    int size = file.tellg();
    if (size == -1) {
        return;
    }
    std::vector<char> buffer(size);
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), size);
    char* data = buffer.data();
    read(version, data);
    read(item_count, data);
    for (int i = 0; i < item_count; ++i) {
        Item item;
        read(item.itemID, data);
        read(item.editableType, data);
        read(item.itemCategory, data);
        read(item.actionType, data);
        read(item.hitSoundType, data);

        int16_t strLen;
        read(strLen, data);
        item.name.resize(strLen);
        for (int j = 0; j < strLen; ++j) {
            item.name[j] = data[j] ^ (secret[(j + item.itemID) % secret.length()]);
        }
        data += strLen;

        read(item.texture, data);
        read(item.textureHash, data);
        read(item.itemKind, data);
        read(item.val1, data);
        read(item.textureX, data);
        read(item.textureY, data);
        read(item.spreadType, data);
        read(item.isStripeyWallpaper, data);
        read(item.collisionType, data);
        read(item.breakHits, data);
        read(item.dropChance, data);
        read(item.clothingType, data);
        read(item.rarity, data);
        read(item.maxAmount, data);
        read(item.extraFile, data);
        read(item.extraFileHash, data);
        read(item.audioVolume, data);
        read(item.petName, data);
        read(item.petPrefix, data);
        read(item.petSuffix, data);
        read(item.petAbility, data);
        read(item.seedBase, data);
        read(item.seedOverlay, data);
        read(item.treeBase, data);
        read(item.treeLeaves, data);
        read(item.seedColor, data);
        read(item.seedOverlayColor, data);
        data += 4;
        read(item.growTime, data);
        read(item.val2, data);
        read(item.isRayman, data);
        read(item.extraOptions, data);
        read(item.texture2, data);
        read(item.extraOptions2, data);
        data += 80;

        if (version >= 11) {
            read(item.punchOptions, data);
        }
        if (version >= 12) data += 13;
        if (version >= 13) data += 4;
        if (version >= 14) data += 4;
        if (version >= 15) {
            data += 25;
            read(strLen, data);
            data += strLen;
        }
        if (version >= 16) {
            data += data[0] + 2;
        }
        if (version >= 17) data += 4;
        if (version >= 18) data += 4;
        if (version >= 19) data += 9;
        items[item.itemID] = item;
    }
}
