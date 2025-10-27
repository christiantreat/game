#ifndef ECONOMY_H
#define ECONOMY_H

#include <stdbool.h>
#include <stdint.h>
#include "../lib/cJSON.h"

/*
 * Phase 7: Economy & Trading System
 *
 * Implements currency, inventory, and trading mechanics.
 * All economic decisions will be transparent and logged.
 */

// Constants
#define MAX_ECONOMY_ITEM_NAME 64
#define MAX_ITEM_DESCRIPTION 256
#define MAX_INVENTORY_SLOTS 50
#define MAX_SHOP_INVENTORY 100
#define MAX_TRADE_ITEMS 10
#define MAX_SHOPS 20
#define MAX_ITEM_TYPES 100

// Item categories
typedef enum {
    ITEM_TYPE_CROP,         // Harvested crops
    ITEM_TYPE_SEED,         // Seeds for planting
    ITEM_TYPE_TOOL,         // Farming tools
    ITEM_TYPE_PRODUCT,      // Crafted products
    ITEM_TYPE_MATERIAL,     // Raw materials
    ITEM_TYPE_FOOD,         // Prepared food
    ITEM_TYPE_GIFT,         // Gift items
    ITEM_TYPE_MISC          // Miscellaneous
} ItemType;

// Item quality levels
typedef enum {
    ITEM_QUALITY_POOR = 0,
    ITEM_QUALITY_NORMAL = 1,
    ITEM_QUALITY_GOOD = 2,
    ITEM_QUALITY_EXCELLENT = 3,
    ITEM_QUALITY_MASTERWORK = 4
} ItemQuality;

// Item definition
typedef struct {
    char name[MAX_ECONOMY_ITEM_NAME];
    char description[MAX_ITEM_DESCRIPTION];
    ItemType type;
    int base_value;          // Base price in gold
    bool stackable;          // Can multiple items stack in one slot?
    int max_stack;           // Maximum stack size
    bool tradeable;          // Can be bought/sold?
    bool consumable;         // Gets used up?
    int weight;              // Weight in grams
} ItemDefinition;

// Item instance
typedef struct {
    int id;
    char item_name[MAX_ECONOMY_ITEM_NAME];  // References ItemDefinition
    int quantity;
    ItemQuality quality;
    int condition;           // 0-100, affects value
    int entity_id;           // Owner entity ID (0 if in shop)
} Item;

// Inventory for an entity
typedef struct {
    int entity_id;
    Item* items[MAX_INVENTORY_SLOTS];
    int item_count;
    int max_slots;
    int total_weight;
    int max_weight;          // Weight capacity
    int currency;            // Gold coins
} Inventory;

// Shop pricing strategy
typedef enum {
    PRICING_FIXED,           // Fixed prices
    PRICING_SUPPLY_DEMAND,   // Dynamic based on stock
    PRICING_HAGGLE,          // Can negotiate
    PRICING_BARTER           // Trade items only
} PricingStrategy;

// Shop/Vendor
typedef struct {
    int id;
    char name[MAX_ECONOMY_ITEM_NAME];
    int location_id;         // World location
    int owner_entity_id;     // Shopkeeper entity ID
    Item* stock[MAX_SHOP_INVENTORY];
    int stock_count;
    PricingStrategy pricing;
    float buy_price_modifier;   // Multiplier for buying (e.g., 0.5 = buys at 50%)
    float sell_price_modifier;  // Multiplier for selling (e.g., 1.2 = sells at 120%)
    int currency;            // Shop's money
    bool infinite_currency;  // Shop never runs out of money
    bool auto_restock;       // Automatically restocks items
} Shop;

// Trade offer between two entities
typedef struct {
    int id;
    int from_entity_id;
    int to_entity_id;
    Item* offered_items[MAX_TRADE_ITEMS];
    int offered_item_count;
    int offered_currency;
    Item* requested_items[MAX_TRADE_ITEMS];
    int requested_item_count;
    int requested_currency;
    bool accepted;
    bool completed;
    bool cancelled;
} TradeOffer;

// Economy manager
typedef struct {
    ItemDefinition* item_definitions[MAX_ITEM_TYPES];
    int item_definition_count;
    Shop* shops[MAX_SHOPS];
    int shop_count;
    int next_item_id;
    int next_shop_id;
    int next_trade_id;
    float global_price_modifier;  // Global economy state
} EconomyManager;

// ============================================================================
// Item Definition Management
// ============================================================================

ItemDefinition* item_definition_create(
    const char* name,
    ItemType type,
    int base_value,
    bool stackable,
    int max_stack
);

void item_definition_destroy(ItemDefinition* def);

// ============================================================================
// Item Management
// ============================================================================

Item* item_create(
    int id,
    const char* item_name,
    int quantity,
    ItemQuality quality
);

void item_destroy(Item* item);

// Get effective value considering quality and condition
int item_get_value(const Item* item, const ItemDefinition* def);

// Combine two items if they can stack
bool item_stack(Item* item_a, Item* item_b, const ItemDefinition* def);

// Split item into two stacks
Item* item_split(Item* item, int split_quantity, int new_id);

// ============================================================================
// Inventory Management
// ============================================================================

Inventory* inventory_create(int entity_id, int max_slots, int max_weight);
void inventory_destroy(Inventory* inventory);

// Add item to inventory
bool inventory_add_item(Inventory* inventory, Item* item, const EconomyManager* manager);

// Remove item from inventory (returns item, caller owns it)
Item* inventory_remove_item(Inventory* inventory, int item_id);

// Remove quantity from a stackable item
int inventory_remove_quantity(Inventory* inventory, const char* item_name, int quantity);

// Find item by ID
Item* inventory_get_item(const Inventory* inventory, int item_id);

// Find item by name (first match)
Item* inventory_find_item(const Inventory* inventory, const char* item_name);

// Count total quantity of an item type
int inventory_count_item(const Inventory* inventory, const char* item_name);

// Check if inventory has space
bool inventory_has_space(const Inventory* inventory);

// Currency operations
bool inventory_add_currency(Inventory* inventory, int amount);
bool inventory_remove_currency(Inventory* inventory, int amount);
int inventory_get_currency(const Inventory* inventory);

// ============================================================================
// Shop Management
// ============================================================================

Shop* shop_create(
    int id,
    const char* name,
    int location_id,
    int owner_entity_id,
    PricingStrategy pricing
);

void shop_destroy(Shop* shop);

// Add item to shop stock
bool shop_add_stock(Shop* shop, Item* item);

// Remove item from shop stock
Item* shop_remove_stock(Shop* shop, int item_id);

// Get buy price (what shop pays player)
int shop_get_buy_price(const Shop* shop, const Item* item, const EconomyManager* manager);

// Get sell price (what shop charges player)
int shop_get_sell_price(const Shop* shop, const Item* item, const EconomyManager* manager);

// Player buys from shop
bool shop_buy_item(
    Shop* shop,
    Inventory* buyer_inventory,
    int item_id,
    const EconomyManager* manager
);

// Player sells to shop
bool shop_sell_item(
    Shop* shop,
    Inventory* seller_inventory,
    int item_id,
    const EconomyManager* manager
);

// Restock shop (if auto_restock enabled)
void shop_restock(Shop* shop, const EconomyManager* manager);

// ============================================================================
// Trading Between Entities
// ============================================================================

TradeOffer* trade_offer_create(
    int id,
    int from_entity_id,
    int to_entity_id
);

void trade_offer_destroy(TradeOffer* offer);

// Add items/currency to offer
bool trade_offer_add_offered_item(TradeOffer* offer, Item* item);
bool trade_offer_set_offered_currency(TradeOffer* offer, int amount);

// Add items/currency to request
bool trade_offer_add_requested_item(TradeOffer* offer, Item* item);
bool trade_offer_set_requested_currency(TradeOffer* offer, int amount);

// Execute trade (transfers items between inventories)
bool trade_offer_execute(
    TradeOffer* offer,
    Inventory* from_inventory,
    Inventory* to_inventory
);

// ============================================================================
// Economy Manager
// ============================================================================

EconomyManager* economy_manager_create(void);
void economy_manager_destroy(EconomyManager* manager);

// Register item definition
bool economy_manager_register_item(EconomyManager* manager, ItemDefinition* def);

// Get item definition by name
const ItemDefinition* economy_manager_get_item_def(
    const EconomyManager* manager,
    const char* name
);

// Register shop
bool economy_manager_register_shop(EconomyManager* manager, Shop* shop);

// Find shop by ID
Shop* economy_manager_get_shop(const EconomyManager* manager, int shop_id);

// Find shop by location
Shop* economy_manager_find_shop_at_location(
    const EconomyManager* manager,
    int location_id
);

// Create item instance from definition
Item* economy_manager_create_item(
    EconomyManager* manager,
    const char* item_name,
    int quantity,
    ItemQuality quality
);

// Load default item definitions
void load_default_item_definitions(EconomyManager* manager);

// Create default shops
void create_default_shops(EconomyManager* manager);

// ============================================================================
// Serialization
// ============================================================================

cJSON* item_to_json(const Item* item);
Item* item_from_json(const cJSON* json);

cJSON* inventory_to_json(const Inventory* inventory);
Inventory* inventory_from_json(const cJSON* json);

cJSON* shop_to_json(const Shop* shop);
Shop* shop_from_json(const cJSON* json);

#endif // ECONOMY_H
