#include "economy.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// Item Definition Management
// ============================================================================

ItemDefinition* item_definition_create(
    const char* name,
    ItemType type,
    int base_value,
    bool stackable,
    int max_stack
) {
    if (!name) return NULL;

    ItemDefinition* def = calloc(1, sizeof(ItemDefinition));
    if (!def) return NULL;

    strncpy(def->name, name, MAX_ECONOMY_ITEM_NAME - 1);
    def->type = type;
    def->base_value = base_value;
    def->stackable = stackable;
    def->max_stack = max_stack;
    def->tradeable = true;  // Default tradeable
    def->consumable = false;
    def->weight = 100;  // Default 100g

    return def;
}

void item_definition_destroy(ItemDefinition* def) {
    free(def);
}

// ============================================================================
// Item Management
// ============================================================================

Item* item_create(
    int id,
    const char* item_name,
    int quantity,
    ItemQuality quality
) {
    if (!item_name || quantity <= 0) return NULL;

    Item* item = calloc(1, sizeof(Item));
    if (!item) return NULL;

    item->id = id;
    strncpy(item->item_name, item_name, MAX_ECONOMY_ITEM_NAME - 1);
    item->quantity = quantity;
    item->quality = quality;
    item->condition = 100;  // Perfect condition
    item->entity_id = 0;

    return item;
}

void item_destroy(Item* item) {
    free(item);
}

int item_get_value(const Item* item, const ItemDefinition* def) {
    if (!item || !def) return 0;

    float value = def->base_value;

    // Quality modifier
    float quality_mod = 1.0f;
    switch (item->quality) {
        case ITEM_QUALITY_POOR: quality_mod = 0.5f; break;
        case ITEM_QUALITY_NORMAL: quality_mod = 1.0f; break;
        case ITEM_QUALITY_GOOD: quality_mod = 1.5f; break;
        case ITEM_QUALITY_EXCELLENT: quality_mod = 2.0f; break;
        case ITEM_QUALITY_MASTERWORK: quality_mod = 3.0f; break;
    }

    // Condition modifier (0-100%)
    float condition_mod = item->condition / 100.0f;

    return (int)(value * quality_mod * condition_mod * item->quantity);
}

bool item_stack(Item* item_a, Item* item_b, const ItemDefinition* def) {
    if (!item_a || !item_b || !def) return false;
    if (!def->stackable) return false;
    if (strcmp(item_a->item_name, item_b->item_name) != 0) return false;
    if (item_a->quality != item_b->quality) return false;

    int total = item_a->quantity + item_b->quantity;
    if (total > def->max_stack) {
        // Partial stack
        item_a->quantity = def->max_stack;
        item_b->quantity = total - def->max_stack;
        return false;  // Couldn't fully stack
    }

    item_a->quantity = total;
    item_b->quantity = 0;
    return true;
}

Item* item_split(Item* item, int split_quantity, int new_id) {
    if (!item || split_quantity <= 0 || split_quantity >= item->quantity) {
        return NULL;
    }

    Item* new_item = item_create(new_id, item->item_name, split_quantity, item->quality);
    if (!new_item) return NULL;

    new_item->condition = item->condition;
    new_item->entity_id = item->entity_id;
    item->quantity -= split_quantity;

    return new_item;
}

// ============================================================================
// Inventory Management
// ============================================================================

Inventory* inventory_create(int entity_id, int max_slots, int max_weight) {
    Inventory* inv = calloc(1, sizeof(Inventory));
    if (!inv) return NULL;

    inv->entity_id = entity_id;
    inv->item_count = 0;
    inv->max_slots = max_slots > MAX_INVENTORY_SLOTS ? MAX_INVENTORY_SLOTS : max_slots;
    inv->total_weight = 0;
    inv->max_weight = max_weight;
    inv->currency = 0;

    return inv;
}

void inventory_destroy(Inventory* inventory) {
    if (!inventory) return;

    for (int i = 0; i < inventory->item_count; i++) {
        item_destroy(inventory->items[i]);
    }
    free(inventory);
}

bool inventory_add_item(Inventory* inventory, Item* item, const EconomyManager* manager) {
    if (!inventory || !item || !manager) return false;

    const ItemDefinition* def = economy_manager_get_item_def(manager, item->item_name);
    if (!def) return false;

    // Try to stack with existing items
    if (def->stackable) {
        for (int i = 0; i < inventory->item_count; i++) {
            if (strcmp(inventory->items[i]->item_name, item->item_name) == 0 &&
                inventory->items[i]->quality == item->quality) {

                int space = def->max_stack - inventory->items[i]->quantity;
                if (space > 0) {
                    int to_add = space < item->quantity ? space : item->quantity;
                    inventory->items[i]->quantity += to_add;
                    item->quantity -= to_add;

                    if (item->quantity == 0) {
                        item_destroy(item);
                        return true;
                    }
                }
            }
        }
    }

    // Add to new slot if needed
    if (item->quantity > 0) {
        if (inventory->item_count >= inventory->max_slots) {
            return false;  // No space
        }

        inventory->items[inventory->item_count++] = item;
        inventory->total_weight += def->weight * item->quantity;
        item->entity_id = inventory->entity_id;
        return true;
    }

    return false;
}

Item* inventory_remove_item(Inventory* inventory, int item_id) {
    if (!inventory) return NULL;

    for (int i = 0; i < inventory->item_count; i++) {
        if (inventory->items[i]->id == item_id) {
            Item* item = inventory->items[i];

            // Shift remaining items
            for (int j = i; j < inventory->item_count - 1; j++) {
                inventory->items[j] = inventory->items[j + 1];
            }
            inventory->item_count--;

            return item;
        }
    }

    return NULL;
}

int inventory_remove_quantity(Inventory* inventory, const char* item_name, int quantity) {
    if (!inventory || !item_name || quantity <= 0) return 0;

    int removed = 0;

    for (int i = 0; i < inventory->item_count && removed < quantity; i++) {
        if (strcmp(inventory->items[i]->item_name, item_name) == 0) {
            int to_remove = quantity - removed;
            if (to_remove >= inventory->items[i]->quantity) {
                // Remove entire stack
                removed += inventory->items[i]->quantity;
                Item* item = inventory_remove_item(inventory, inventory->items[i]->id);
                item_destroy(item);
                i--;  // Adjust index after removal
            } else {
                // Remove partial stack
                inventory->items[i]->quantity -= to_remove;
                removed += to_remove;
            }
        }
    }

    return removed;
}

Item* inventory_get_item(const Inventory* inventory, int item_id) {
    if (!inventory) return NULL;

    for (int i = 0; i < inventory->item_count; i++) {
        if (inventory->items[i]->id == item_id) {
            return inventory->items[i];
        }
    }

    return NULL;
}

Item* inventory_find_item(const Inventory* inventory, const char* item_name) {
    if (!inventory || !item_name) return NULL;

    for (int i = 0; i < inventory->item_count; i++) {
        if (strcmp(inventory->items[i]->item_name, item_name) == 0) {
            return inventory->items[i];
        }
    }

    return NULL;
}

int inventory_count_item(const Inventory* inventory, const char* item_name) {
    if (!inventory || !item_name) return 0;

    int total = 0;
    for (int i = 0; i < inventory->item_count; i++) {
        if (strcmp(inventory->items[i]->item_name, item_name) == 0) {
            total += inventory->items[i]->quantity;
        }
    }

    return total;
}

bool inventory_has_space(const Inventory* inventory) {
    if (!inventory) return false;
    return inventory->item_count < inventory->max_slots;
}

bool inventory_add_currency(Inventory* inventory, int amount) {
    if (!inventory || amount < 0) return false;
    inventory->currency += amount;
    return true;
}

bool inventory_remove_currency(Inventory* inventory, int amount) {
    if (!inventory || amount < 0) return false;
    if (inventory->currency < amount) return false;
    inventory->currency -= amount;
    return true;
}

int inventory_get_currency(const Inventory* inventory) {
    return inventory ? inventory->currency : 0;
}

// ============================================================================
// Shop Management
// ============================================================================

Shop* shop_create(
    int id,
    const char* name,
    int location_id,
    int owner_entity_id,
    PricingStrategy pricing
) {
    if (!name) return NULL;

    Shop* shop = calloc(1, sizeof(Shop));
    if (!shop) return NULL;

    shop->id = id;
    strncpy(shop->name, name, MAX_ECONOMY_ITEM_NAME - 1);
    shop->location_id = location_id;
    shop->owner_entity_id = owner_entity_id;
    shop->stock_count = 0;
    shop->pricing = pricing;
    shop->buy_price_modifier = 0.5f;   // Buys at 50% of value
    shop->sell_price_modifier = 1.2f;  // Sells at 120% of value
    shop->currency = 1000;  // Starting capital
    shop->infinite_currency = false;
    shop->auto_restock = false;

    return shop;
}

void shop_destroy(Shop* shop) {
    if (!shop) return;

    for (int i = 0; i < shop->stock_count; i++) {
        item_destroy(shop->stock[i]);
    }
    free(shop);
}

bool shop_add_stock(Shop* shop, Item* item) {
    if (!shop || !item) return false;
    if (shop->stock_count >= MAX_SHOP_INVENTORY) return false;

    shop->stock[shop->stock_count++] = item;
    item->entity_id = 0;  // Shop owns it now
    return true;
}

Item* shop_remove_stock(Shop* shop, int item_id) {
    if (!shop) return NULL;

    for (int i = 0; i < shop->stock_count; i++) {
        if (shop->stock[i]->id == item_id) {
            Item* item = shop->stock[i];

            // Shift remaining stock
            for (int j = i; j < shop->stock_count - 1; j++) {
                shop->stock[j] = shop->stock[j + 1];
            }
            shop->stock_count--;

            return item;
        }
    }

    return NULL;
}

int shop_get_buy_price(const Shop* shop, const Item* item, const EconomyManager* manager) {
    if (!shop || !item || !manager) return 0;

    const ItemDefinition* def = economy_manager_get_item_def(manager, item->item_name);
    if (!def || !def->tradeable) return 0;

    int base_value = item_get_value(item, def);
    return (int)(base_value * shop->buy_price_modifier);
}

int shop_get_sell_price(const Shop* shop, const Item* item, const EconomyManager* manager) {
    if (!shop || !item || !manager) return 0;

    const ItemDefinition* def = economy_manager_get_item_def(manager, item->item_name);
    if (!def || !def->tradeable) return 0;

    int base_value = item_get_value(item, def);
    return (int)(base_value * shop->sell_price_modifier * manager->global_price_modifier);
}

bool shop_buy_item(
    Shop* shop,
    Inventory* buyer_inventory,
    int item_id,
    const EconomyManager* manager
) {
    if (!shop || !buyer_inventory || !manager) return false;

    // Find item in shop
    Item* item = NULL;
    for (int i = 0; i < shop->stock_count; i++) {
        if (shop->stock[i]->id == item_id) {
            item = shop->stock[i];
            break;
        }
    }

    if (!item) return false;

    // Calculate price
    int price = shop_get_sell_price(shop, item, manager);

    // Check buyer has money
    if (buyer_inventory->currency < price) return false;

    // Check buyer has space
    if (!inventory_has_space(buyer_inventory)) {
        // Check if can stack with existing
        const ItemDefinition* def = economy_manager_get_item_def(manager, item->item_name);
        if (!def || !def->stackable) return false;

        Item* existing = inventory_find_item(buyer_inventory, item->item_name);
        if (!existing || existing->quantity >= def->max_stack) return false;
    }

    // Execute transaction
    Item* purchased = shop_remove_stock(shop, item_id);
    if (!inventory_add_item(buyer_inventory, purchased, manager)) {
        // Failed to add, return to shop
        shop_add_stock(shop, purchased);
        return false;
    }

    inventory_remove_currency(buyer_inventory, price);
    shop->currency += price;

    return true;
}

bool shop_sell_item(
    Shop* shop,
    Inventory* seller_inventory,
    int item_id,
    const EconomyManager* manager
) {
    if (!shop || !seller_inventory || !manager) return false;

    // Find item in seller's inventory
    Item* item = inventory_get_item(seller_inventory, item_id);
    if (!item) return false;

    // Calculate price
    int price = shop_get_buy_price(shop, item, manager);

    // Check shop has money
    if (!shop->infinite_currency && shop->currency < price) return false;

    // Check shop has space
    if (shop->stock_count >= MAX_SHOP_INVENTORY) return false;

    // Execute transaction
    Item* sold = inventory_remove_item(seller_inventory, item_id);
    if (!shop_add_stock(shop, sold)) {
        // Failed to add, return to seller
        inventory_add_item(seller_inventory, sold, manager);
        return false;
    }

    inventory_add_currency(seller_inventory, price);
    if (!shop->infinite_currency) {
        shop->currency -= price;
    }

    return true;
}

void shop_restock(Shop* shop, const EconomyManager* manager) {
    if (!shop || !manager || !shop->auto_restock) return;

    // Simple restock: add one of each registered item type
    // In a real game, this would be more sophisticated
    for (int i = 0; i < manager->item_definition_count && shop->stock_count < MAX_SHOP_INVENTORY; i++) {
        const ItemDefinition* def = manager->item_definitions[i];
        if (def && def->tradeable) {
            // Check if shop already has this item
            bool has_item = false;
            for (int j = 0; j < shop->stock_count; j++) {
                if (strcmp(shop->stock[j]->item_name, def->name) == 0) {
                    has_item = true;
                    break;
                }
            }

            if (!has_item) {
                Item* item = item_create(
                    manager->next_item_id,
                    def->name,
                    def->stackable ? 10 : 1,
                    ITEM_QUALITY_NORMAL
                );
                if (item) {
                    shop_add_stock(shop, item);
                }
            }
        }
    }
}

// ============================================================================
// Trading Between Entities
// ============================================================================

TradeOffer* trade_offer_create(int id, int from_entity_id, int to_entity_id) {
    TradeOffer* offer = calloc(1, sizeof(TradeOffer));
    if (!offer) return NULL;

    offer->id = id;
    offer->from_entity_id = from_entity_id;
    offer->to_entity_id = to_entity_id;
    offer->offered_item_count = 0;
    offer->offered_currency = 0;
    offer->requested_item_count = 0;
    offer->requested_currency = 0;
    offer->accepted = false;
    offer->completed = false;
    offer->cancelled = false;

    return offer;
}

void trade_offer_destroy(TradeOffer* offer) {
    if (!offer) return;

    for (int i = 0; i < offer->offered_item_count; i++) {
        item_destroy(offer->offered_items[i]);
    }
    for (int i = 0; i < offer->requested_item_count; i++) {
        item_destroy(offer->requested_items[i]);
    }
    free(offer);
}

bool trade_offer_add_offered_item(TradeOffer* offer, Item* item) {
    if (!offer || !item) return false;
    if (offer->offered_item_count >= MAX_TRADE_ITEMS) return false;

    offer->offered_items[offer->offered_item_count++] = item;
    return true;
}

bool trade_offer_set_offered_currency(TradeOffer* offer, int amount) {
    if (!offer || amount < 0) return false;
    offer->offered_currency = amount;
    return true;
}

bool trade_offer_add_requested_item(TradeOffer* offer, Item* item) {
    if (!offer || !item) return false;
    if (offer->requested_item_count >= MAX_TRADE_ITEMS) return false;

    offer->requested_items[offer->requested_item_count++] = item;
    return true;
}

bool trade_offer_set_requested_currency(TradeOffer* offer, int amount) {
    if (!offer || amount < 0) return false;
    offer->requested_currency = amount;
    return true;
}

bool trade_offer_execute(
    TradeOffer* offer,
    Inventory* from_inventory,
    Inventory* to_inventory
) {
    if (!offer || !from_inventory || !to_inventory) return false;
    if (offer->completed || offer->cancelled) return false;

    // Validate from_inventory has everything offered
    if (from_inventory->currency < offer->offered_currency) return false;

    // Validate to_inventory has everything requested
    if (to_inventory->currency < offer->requested_currency) return false;

    // Execute currency exchange
    if (offer->offered_currency > 0) {
        if (!inventory_remove_currency(from_inventory, offer->offered_currency)) return false;
        inventory_add_currency(to_inventory, offer->offered_currency);
    }

    if (offer->requested_currency > 0) {
        if (!inventory_remove_currency(to_inventory, offer->requested_currency)) return false;
        inventory_add_currency(from_inventory, offer->requested_currency);
    }

    // Note: Item transfer would require economy manager for proper handling
    // This is a simplified version

    offer->completed = true;
    return true;
}

// ============================================================================
// Economy Manager
// ============================================================================

EconomyManager* economy_manager_create(void) {
    EconomyManager* manager = calloc(1, sizeof(EconomyManager));
    if (!manager) return NULL;

    manager->item_definition_count = 0;
    manager->shop_count = 0;
    manager->next_item_id = 1;
    manager->next_shop_id = 1;
    manager->next_trade_id = 1;
    manager->global_price_modifier = 1.0f;

    return manager;
}

void economy_manager_destroy(EconomyManager* manager) {
    if (!manager) return;

    for (int i = 0; i < manager->item_definition_count; i++) {
        item_definition_destroy(manager->item_definitions[i]);
    }

    for (int i = 0; i < manager->shop_count; i++) {
        shop_destroy(manager->shops[i]);
    }

    free(manager);
}

bool economy_manager_register_item(EconomyManager* manager, ItemDefinition* def) {
    if (!manager || !def) return false;
    if (manager->item_definition_count >= MAX_ITEM_TYPES) return false;

    manager->item_definitions[manager->item_definition_count++] = def;
    return true;
}

const ItemDefinition* economy_manager_get_item_def(
    const EconomyManager* manager,
    const char* name
) {
    if (!manager || !name) return NULL;

    for (int i = 0; i < manager->item_definition_count; i++) {
        if (strcmp(manager->item_definitions[i]->name, name) == 0) {
            return manager->item_definitions[i];
        }
    }

    return NULL;
}

bool economy_manager_register_shop(EconomyManager* manager, Shop* shop) {
    if (!manager || !shop) return false;
    if (manager->shop_count >= MAX_SHOPS) return false;

    manager->shops[manager->shop_count++] = shop;
    return true;
}

Shop* economy_manager_get_shop(const EconomyManager* manager, int shop_id) {
    if (!manager) return NULL;

    for (int i = 0; i < manager->shop_count; i++) {
        if (manager->shops[i]->id == shop_id) {
            return manager->shops[i];
        }
    }

    return NULL;
}

Shop* economy_manager_find_shop_at_location(
    const EconomyManager* manager,
    int location_id
) {
    if (!manager) return NULL;

    for (int i = 0; i < manager->shop_count; i++) {
        if (manager->shops[i]->location_id == location_id) {
            return manager->shops[i];
        }
    }

    return NULL;
}

Item* economy_manager_create_item(
    EconomyManager* manager,
    const char* item_name,
    int quantity,
    ItemQuality quality
) {
    if (!manager || !item_name) return NULL;

    const ItemDefinition* def = economy_manager_get_item_def(manager, item_name);
    if (!def) return NULL;

    Item* item = item_create(manager->next_item_id++, item_name, quantity, quality);
    return item;
}

// ============================================================================
// Default Content
// ============================================================================

void load_default_item_definitions(EconomyManager* manager) {
    if (!manager) return;

    // Crop items (from agriculture system)
    economy_manager_register_item(manager,
        item_definition_create("Wheat", ITEM_TYPE_CROP, 12, true, 99));
    economy_manager_register_item(manager,
        item_definition_create("Corn", ITEM_TYPE_CROP, 15, true, 99));
    economy_manager_register_item(manager,
        item_definition_create("Tomato", ITEM_TYPE_CROP, 10, true, 99));
    economy_manager_register_item(manager,
        item_definition_create("Potato", ITEM_TYPE_CROP, 8, true, 99));
    economy_manager_register_item(manager,
        item_definition_create("Carrot", ITEM_TYPE_CROP, 6, true, 99));

    // Seeds
    economy_manager_register_item(manager,
        item_definition_create("Wheat Seeds", ITEM_TYPE_SEED, 5, true, 99));
    economy_manager_register_item(manager,
        item_definition_create("Corn Seeds", ITEM_TYPE_SEED, 8, true, 99));
    economy_manager_register_item(manager,
        item_definition_create("Tomato Seeds", ITEM_TYPE_SEED, 6, true, 99));
    economy_manager_register_item(manager,
        item_definition_create("Potato Seeds", ITEM_TYPE_SEED, 4, true, 99));
    economy_manager_register_item(manager,
        item_definition_create("Carrot Seeds", ITEM_TYPE_SEED, 3, true, 99));

    // Tools
    ItemDefinition* hoe = item_definition_create("Hoe", ITEM_TYPE_TOOL, 50, false, 1);
    if (hoe) {
        hoe->weight = 500;
        economy_manager_register_item(manager, hoe);
    }

    ItemDefinition* watering_can = item_definition_create("Watering Can", ITEM_TYPE_TOOL, 30, false, 1);
    if (watering_can) {
        watering_can->weight = 300;
        economy_manager_register_item(manager, watering_can);
    }

    ItemDefinition* sickle = item_definition_create("Sickle", ITEM_TYPE_TOOL, 40, false, 1);
    if (sickle) {
        sickle->weight = 400;
        economy_manager_register_item(manager, sickle);
    }

    // Materials
    economy_manager_register_item(manager,
        item_definition_create("Wood", ITEM_TYPE_MATERIAL, 5, true, 50));
    economy_manager_register_item(manager,
        item_definition_create("Stone", ITEM_TYPE_MATERIAL, 3, true, 50));
    economy_manager_register_item(manager,
        item_definition_create("Iron Ore", ITEM_TYPE_MATERIAL, 15, true, 50));

    // Food (prepared)
    economy_manager_register_item(manager,
        item_definition_create("Bread", ITEM_TYPE_FOOD, 10, true, 20));
    economy_manager_register_item(manager,
        item_definition_create("Vegetable Soup", ITEM_TYPE_FOOD, 15, true, 10));
}

void create_default_shops(EconomyManager* manager) {
    if (!manager) return;

    // General Store (location 2 - Village Square from world.c)
    Shop* general_store = shop_create(
        manager->next_shop_id++,
        "General Store",
        2,  // Village Square
        0,  // No specific owner yet
        PRICING_FIXED
    );

    if (general_store) {
        general_store->infinite_currency = true;
        general_store->auto_restock = true;

        // Add initial stock
        Item* item;
        item = economy_manager_create_item(manager, "Wheat Seeds", 20, ITEM_QUALITY_NORMAL);
        if (item) shop_add_stock(general_store, item);

        item = economy_manager_create_item(manager, "Corn Seeds", 20, ITEM_QUALITY_NORMAL);
        if (item) shop_add_stock(general_store, item);

        item = economy_manager_create_item(manager, "Hoe", 1, ITEM_QUALITY_NORMAL);
        if (item) shop_add_stock(general_store, item);

        item = economy_manager_create_item(manager, "Watering Can", 1, ITEM_QUALITY_NORMAL);
        if (item) shop_add_stock(general_store, item);

        economy_manager_register_shop(manager, general_store);
    }

    // Farmer's Market (location 6 - Market from world.c)
    Shop* market = shop_create(
        manager->next_shop_id++,
        "Farmer's Market",
        6,  // Market location
        0,
        PRICING_SUPPLY_DEMAND
    );

    if (market) {
        market->buy_price_modifier = 0.7f;   // Better prices for crops
        market->sell_price_modifier = 1.1f;
        market->currency = 5000;

        economy_manager_register_shop(manager, market);
    }
}

// ============================================================================
// Serialization
// ============================================================================

cJSON* item_to_json(const Item* item) {
    if (!item) return NULL;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "id", item->id);
    cJSON_AddStringToObject(json, "item_name", item->item_name);
    cJSON_AddNumberToObject(json, "quantity", item->quantity);
    cJSON_AddNumberToObject(json, "quality", item->quality);
    cJSON_AddNumberToObject(json, "condition", item->condition);
    cJSON_AddNumberToObject(json, "entity_id", item->entity_id);

    return json;
}

Item* item_from_json(const cJSON* json) {
    if (!json) return NULL;

    const cJSON* id = cJSON_GetObjectItem(json, "id");
    const cJSON* name = cJSON_GetObjectItem(json, "item_name");
    const cJSON* quantity = cJSON_GetObjectItem(json, "quantity");
    const cJSON* quality = cJSON_GetObjectItem(json, "quality");

    if (!id || !name || !quantity) return NULL;

    Item* item = item_create(id->valueint, name->valuestring,
                            quantity->valueint, quality ? quality->valueint : ITEM_QUALITY_NORMAL);

    if (item) {
        const cJSON* condition = cJSON_GetObjectItem(json, "condition");
        const cJSON* entity_id = cJSON_GetObjectItem(json, "entity_id");

        if (condition) item->condition = condition->valueint;
        if (entity_id) item->entity_id = entity_id->valueint;
    }

    return item;
}

cJSON* inventory_to_json(const Inventory* inventory) {
    if (!inventory) return NULL;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "entity_id", inventory->entity_id);
    cJSON_AddNumberToObject(json, "max_slots", inventory->max_slots);
    cJSON_AddNumberToObject(json, "max_weight", inventory->max_weight);
    cJSON_AddNumberToObject(json, "currency", inventory->currency);

    cJSON* items = cJSON_CreateArray();
    for (int i = 0; i < inventory->item_count; i++) {
        cJSON_AddItemToArray(items, item_to_json(inventory->items[i]));
    }
    cJSON_AddItemToObject(json, "items", items);

    return json;
}

Inventory* inventory_from_json(const cJSON* json) {
    if (!json) return NULL;

    const cJSON* entity_id = cJSON_GetObjectItem(json, "entity_id");
    const cJSON* max_slots = cJSON_GetObjectItem(json, "max_slots");
    const cJSON* max_weight = cJSON_GetObjectItem(json, "max_weight");

    if (!entity_id || !max_slots) return NULL;

    Inventory* inv = inventory_create(
        entity_id->valueint,
        max_slots->valueint,
        max_weight ? max_weight->valueint : 10000
    );

    if (inv) {
        const cJSON* currency = cJSON_GetObjectItem(json, "currency");
        if (currency) inv->currency = currency->valueint;

        const cJSON* items = cJSON_GetObjectItem(json, "items");
        if (items) {
            const cJSON* item_json = NULL;
            cJSON_ArrayForEach(item_json, items) {
                Item* item = item_from_json(item_json);
                if (item) {
                    inv->items[inv->item_count++] = item;
                }
            }
        }
    }

    return inv;
}

cJSON* shop_to_json(const Shop* shop) {
    if (!shop) return NULL;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "id", shop->id);
    cJSON_AddStringToObject(json, "name", shop->name);
    cJSON_AddNumberToObject(json, "location_id", shop->location_id);
    cJSON_AddNumberToObject(json, "owner_entity_id", shop->owner_entity_id);
    cJSON_AddNumberToObject(json, "pricing", shop->pricing);
    cJSON_AddNumberToObject(json, "currency", shop->currency);

    cJSON* stock = cJSON_CreateArray();
    for (int i = 0; i < shop->stock_count; i++) {
        cJSON_AddItemToArray(stock, item_to_json(shop->stock[i]));
    }
    cJSON_AddItemToObject(json, "stock", stock);

    return json;
}

Shop* shop_from_json(const cJSON* json) {
    if (!json) return NULL;

    const cJSON* id = cJSON_GetObjectItem(json, "id");
    const cJSON* name = cJSON_GetObjectItem(json, "name");
    const cJSON* location_id = cJSON_GetObjectItem(json, "location_id");
    const cJSON* owner = cJSON_GetObjectItem(json, "owner_entity_id");
    const cJSON* pricing = cJSON_GetObjectItem(json, "pricing");

    if (!id || !name || !location_id) return NULL;

    Shop* shop = shop_create(
        id->valueint,
        name->valuestring,
        location_id->valueint,
        owner ? owner->valueint : 0,
        pricing ? pricing->valueint : PRICING_FIXED
    );

    if (shop) {
        const cJSON* currency = cJSON_GetObjectItem(json, "currency");
        if (currency) shop->currency = currency->valueint;

        const cJSON* stock = cJSON_GetObjectItem(json, "stock");
        if (stock) {
            const cJSON* item_json = NULL;
            cJSON_ArrayForEach(item_json, stock) {
                Item* item = item_from_json(item_json);
                if (item) {
                    shop_add_stock(shop, item);
                }
            }
        }
    }

    return shop;
}
