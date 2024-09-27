#pragma once
#include <unordered_map>
#include <string>

struct Item {
    int itemID;
    char editableType;
    char itemCategory;
    char actionType;
    char hitSoundType;
    std::string name;
    std::string texture;
    int textureHash;
    char itemKind;
    int val1;
    char textureX;
    char textureY;
    char spreadType;
    char isStripeyWallpaper;
    char collisionType;
    char aa;
    char breakHits;
    int dropChance;
    char clothingType;
    int16_t rarity;
    unsigned char maxAmount;
    std::string extraFile;
    int extraFileHash;
    int audioVolume;
    std::string petName;
    std::string petPrefix;
    std::string petSuffix;
    std::string petAbility;
    char seedBase;
    char seedOverlay;
    char treeBase;
    char treeLeaves;
    int seedColor;
    int seedOverlayColor;
    int growTime;
    short val2;
    short isRayman;
    std::string extraOptions;
    std::string texture2;
    std::string extraOptions2;
    std::string punchOptions;
};
class ItemDatabase {
public:
	Item* get_item(int id);
	void load_from_file(const std::string& filePath);
private:
	uint16_t version;
	uint32_t item_count;
	std::unordered_map<uint32_t, Item> items;
};
