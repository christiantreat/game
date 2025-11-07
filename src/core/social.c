#include "social.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// Relationship Management
// ============================================================================

Relationship* relationship_create(int entity_a_id, int entity_b_id) {
    Relationship* rel = calloc(1, sizeof(Relationship));
    if (!rel) return NULL;

    rel->entity_a_id = entity_a_id;
    rel->entity_b_id = entity_b_id;
    rel->type = RELATIONSHIP_STRANGER;
    rel->affection = 0;
    rel->trust = 50;
    rel->respect = 50;
    rel->times_talked = 0;
    rel->times_gifted = 0;
    rel->days_since_interaction = 0;
    rel->first_met = time(NULL);
    rel->last_interaction = time(NULL);
    rel->is_locked = false;

    return rel;
}

void relationship_destroy(Relationship* rel) {
    free(rel);
}

void relationship_modify_affection(Relationship* rel, int change) {
    if (!rel || rel->is_locked) return;

    rel->affection += change;

    // Clamp to -100 to 100
    if (rel->affection > 100) rel->affection = 100;
    if (rel->affection < -100) rel->affection = -100;

    relationship_update_type(rel);
}

void relationship_modify_trust(Relationship* rel, int change) {
    if (!rel || rel->is_locked) return;

    rel->trust += change;

    // Clamp to 0 to 100
    if (rel->trust > 100) rel->trust = 100;
    if (rel->trust < 0) rel->trust = 0;
}

void relationship_modify_respect(Relationship* rel, int change) {
    if (!rel || rel->is_locked) return;

    rel->respect += change;

    // Clamp to 0 to 100
    if (rel->respect > 100) rel->respect = 100;
    if (rel->respect < 0) rel->respect = 0;
}

RelationshipType relationship_get_type(const Relationship* rel) {
    if (!rel) return RELATIONSHIP_STRANGER;
    return rel->type;
}

void relationship_update_type(Relationship* rel) {
    if (!rel || rel->is_locked) return;

    // Determine type based on affection level
    if (rel->affection >= 80) {
        rel->type = RELATIONSHIP_CLOSE_FRIEND;
    } else if (rel->affection >= 50) {
        rel->type = RELATIONSHIP_FRIEND;
    } else if (rel->affection >= 20) {
        rel->type = RELATIONSHIP_ACQUAINTANCE;
    } else if (rel->affection <= -80) {
        rel->type = RELATIONSHIP_ENEMY;
    } else if (rel->affection <= -50) {
        rel->type = RELATIONSHIP_RIVAL;
    } else {
        rel->type = RELATIONSHIP_STRANGER;
    }
}

bool relationship_meets_requirements(const Relationship* rel, int min_affection, int min_trust) {
    if (!rel) return false;
    return rel->affection >= min_affection && rel->trust >= min_trust;
}

void relationship_record_talk(Relationship* rel) {
    if (!rel) return;

    rel->times_talked++;
    rel->last_interaction = time(NULL);
    rel->days_since_interaction = 0;
}

void relationship_record_gift(Relationship* rel) {
    if (!rel) return;

    rel->times_gifted++;
    rel->last_interaction = time(NULL);
    rel->days_since_interaction = 0;
}

void relationship_apply_decay(Relationship* rel, int days_passed) {
    if (!rel || rel->is_locked) return;

    rel->days_since_interaction += days_passed;

    // Decay affection if no interaction for a while
    if (rel->days_since_interaction > 7) {
        int decay = (rel->days_since_interaction - 7) / 7;  // -1 per week
        relationship_modify_affection(rel, -decay);
    }
}

// ============================================================================
// Personality System
// ============================================================================

Personality* personality_create(int entity_id) {
    Personality* personality = calloc(1, sizeof(Personality));
    if (!personality) return NULL;

    personality->entity_id = entity_id;
    personality->trait_count = 0;
    personality->friendliness = 50;
    personality->generosity = 50;
    personality->chattiness = 50;
    personality->trustworthiness = 50;

    return personality;
}

void personality_destroy(Personality* personality) {
    free(personality);
}

bool personality_add_trait(Personality* personality, PersonalityTrait trait) {
    if (!personality || personality->trait_count >= MAX_PERSONALITY_TRAITS) {
        return false;
    }

    // Check for duplicates
    for (int i = 0; i < personality->trait_count; i++) {
        if (personality->traits[i] == trait) {
            return false;
        }
    }

    personality->traits[personality->trait_count++] = trait;

    // Adjust stats based on trait
    switch (trait) {
        case TRAIT_FRIENDLY:
            personality->friendliness += 20;
            personality->chattiness += 15;
            break;
        case TRAIT_SHY:
            personality->friendliness -= 20;
            personality->chattiness -= 20;
            break;
        case TRAIT_GENEROUS:
            personality->generosity += 30;
            break;
        case TRAIT_GREEDY:
            personality->generosity -= 30;
            break;
        case TRAIT_HONEST:
            personality->trustworthiness += 25;
            break;
        case TRAIT_DECEITFUL:
            personality->trustworthiness -= 25;
            break;
        case TRAIT_OPTIMISTIC:
            personality->friendliness += 10;
            break;
        case TRAIT_PESSIMISTIC:
            personality->friendliness -= 10;
            break;
        default:
            break;
    }

    // Clamp stats
    if (personality->friendliness > 100) personality->friendliness = 100;
    if (personality->friendliness < 0) personality->friendliness = 0;
    if (personality->generosity > 100) personality->generosity = 100;
    if (personality->generosity < 0) personality->generosity = 0;
    if (personality->chattiness > 100) personality->chattiness = 100;
    if (personality->chattiness < 0) personality->chattiness = 0;
    if (personality->trustworthiness > 100) personality->trustworthiness = 100;
    if (personality->trustworthiness < 0) personality->trustworthiness = 0;

    return true;
}

bool personality_has_trait(const Personality* personality, PersonalityTrait trait) {
    if (!personality) return false;

    for (int i = 0; i < personality->trait_count; i++) {
        if (personality->traits[i] == trait) {
            return true;
        }
    }

    return false;
}

float personality_get_friendliness_modifier(const Personality* personality) {
    if (!personality) return 1.0f;
    return personality->friendliness / 50.0f;  // 0.0 to 2.0
}

float personality_get_generosity_modifier(const Personality* personality) {
    if (!personality) return 1.0f;
    return personality->generosity / 50.0f;  // 0.0 to 2.0
}

float personality_get_trust_modifier(const Personality* personality) {
    if (!personality) return 1.0f;
    return personality->trustworthiness / 50.0f;  // 0.0 to 2.0
}

// ============================================================================
// Conversation System
// ============================================================================

Conversation* conversation_create(int id, int initiator_id, int recipient_id) {
    Conversation* conv = calloc(1, sizeof(Conversation));
    if (!conv) return NULL;

    conv->id = id;
    conv->initiator_id = initiator_id;
    conv->recipient_id = recipient_id;
    conv->option_count = 0;
    conv->selected_option_id = -1;
    conv->started_at = time(NULL);
    conv->ended_at = 0;
    conv->completed = false;

    return conv;
}

void conversation_destroy(Conversation* conversation) {
    if (!conversation) return;

    for (int i = 0; i < conversation->option_count; i++) {
        free(conversation->options[i]);
    }
    free(conversation);
}

bool conversation_add_option(
    Conversation* conv,
    const char* text,
    ConversationTopic topic,
    int affection_change,
    int trust_change,
    int respect_change
) {
    if (!conv || !text || conv->option_count >= MAX_DIALOGUE_OPTIONS) {
        return false;
    }

    DialogueOption* option = calloc(1, sizeof(DialogueOption));
    if (!option) return false;

    option->id = conv->option_count;
    strncpy(option->text, text, MAX_DIALOGUE_TEXT - 1);
    option->topic = topic;
    option->affection_change = affection_change;
    option->trust_change = trust_change;
    option->respect_change = respect_change;
    option->requires_min_affection = false;
    option->min_affection = 0;

    conv->options[conv->option_count++] = option;

    return true;
}

bool conversation_select_option(Conversation* conv, int option_id) {
    if (!conv || option_id < 0 || option_id >= conv->option_count) {
        return false;
    }

    conv->selected_option_id = option_id;
    return true;
}

int conversation_get_available_options(
    const Conversation* conv,
    const Relationship* rel,
    DialogueOption** out_options,
    int max_options
) {
    if (!conv || !out_options) return 0;

    int count = 0;
    for (int i = 0; i < conv->option_count && count < max_options; i++) {
        DialogueOption* option = conv->options[i];

        // Check if requirements are met
        if (option->requires_min_affection && rel) {
            if (rel->affection < option->min_affection) {
                continue;  // Skip this option
            }
        }

        out_options[count++] = option;
    }

    return count;
}

void conversation_end(Conversation* conv) {
    if (!conv) return;

    conv->ended_at = time(NULL);
    conv->completed = true;
}

// ============================================================================
// Gift System
// ============================================================================

Gift* gift_create(int giver_id, int receiver_id, const char* item_name, int item_value) {
    if (!item_name) return NULL;

    Gift* gift = calloc(1, sizeof(Gift));
    if (!gift) return NULL;

    gift->giver_id = giver_id;
    gift->receiver_id = receiver_id;
    strncpy(gift->item_name, item_name, 63);
    gift->item_value = item_value;
    gift->affection_gained = 0;
    gift->given_at = time(NULL);
    gift->was_loved = false;
    gift->was_liked = false;
    gift->was_neutral = false;
    gift->was_disliked = false;

    return gift;
}

void gift_destroy(Gift* gift) {
    free(gift);
}

int gift_calculate_affection(
    const Gift* gift,
    const GiftPreferences* prefs,
    const Personality* receiver_personality
) {
    if (!gift) return 0;

    int base_affection = 5;  // Default for any gift

    // Check preferences
    if (prefs) {
        if (gift_preferences_is_loved(prefs, gift->item_name)) {
            base_affection = 15;
        } else if (gift_preferences_is_liked(prefs, gift->item_name)) {
            base_affection = 10;
        } else if (gift_preferences_is_disliked(prefs, gift->item_name)) {
            base_affection = -5;
        }
    }

    // Factor in item value
    if (gift->item_value > 50) {
        base_affection += 3;  // Expensive gifts add bonus
    } else if (gift->item_value < 10) {
        base_affection -= 1;  // Cheap gifts lose a point
    }

    // Apply personality modifier
    if (receiver_personality) {
        float generosity_mod = personality_get_generosity_modifier(receiver_personality);
        // Generous people appreciate gifts more
        base_affection = (int)(base_affection * (0.5f + generosity_mod * 0.5f));
    }

    return base_affection;
}

void gift_apply_to_relationship(
    const Gift* gift,
    Relationship* rel,
    const GiftPreferences* prefs,
    const Personality* receiver_personality
) {
    if (!gift || !rel) return;

    int affection_gain = gift_calculate_affection(gift, prefs, receiver_personality);

    relationship_modify_affection(rel, affection_gain);
    relationship_record_gift(rel);
}

// Gift preferences
GiftPreferences* gift_preferences_create(int entity_id) {
    GiftPreferences* prefs = calloc(1, sizeof(GiftPreferences));
    if (!prefs) return NULL;

    prefs->entity_id = entity_id;
    prefs->loved_count = 0;
    prefs->liked_count = 0;
    prefs->disliked_count = 0;

    return prefs;
}

void gift_preferences_destroy(GiftPreferences* prefs) {
    free(prefs);
}

bool gift_preferences_add_loved(GiftPreferences* prefs, const char* item_name) {
    if (!prefs || !item_name || prefs->loved_count >= 10) return false;

    strncpy(prefs->loved_items[prefs->loved_count], item_name, 63);
    prefs->loved_count++;
    return true;
}

bool gift_preferences_add_liked(GiftPreferences* prefs, const char* item_name) {
    if (!prefs || !item_name || prefs->liked_count >= 10) return false;

    strncpy(prefs->liked_items[prefs->liked_count], item_name, 63);
    prefs->liked_count++;
    return true;
}

bool gift_preferences_add_disliked(GiftPreferences* prefs, const char* item_name) {
    if (!prefs || !item_name || prefs->disliked_count >= 10) return false;

    strncpy(prefs->disliked_items[prefs->disliked_count], item_name, 63);
    prefs->disliked_count++;
    return true;
}

bool gift_preferences_is_loved(const GiftPreferences* prefs, const char* item_name) {
    if (!prefs || !item_name) return false;

    for (int i = 0; i < prefs->loved_count; i++) {
        if (strcmp(prefs->loved_items[i], item_name) == 0) {
            return true;
        }
    }

    return false;
}

bool gift_preferences_is_liked(const GiftPreferences* prefs, const char* item_name) {
    if (!prefs || !item_name) return false;

    for (int i = 0; i < prefs->liked_count; i++) {
        if (strcmp(prefs->liked_items[i], item_name) == 0) {
            return true;
        }
    }

    return false;
}

bool gift_preferences_is_disliked(const GiftPreferences* prefs, const char* item_name) {
    if (!prefs || !item_name) return false;

    for (int i = 0; i < prefs->disliked_count; i++) {
        if (strcmp(prefs->disliked_items[i], item_name) == 0) {
            return true;
        }
    }

    return false;
}

// ============================================================================
// Social Manager
// ============================================================================

SocialManager* social_manager_create(void) {
    SocialManager* manager = calloc(1, sizeof(SocialManager));
    if (!manager) return NULL;

    manager->relationship_count = 0;
    manager->personality_count = 0;
    manager->gift_pref_count = 0;
    manager->active_conversation_count = 0;
    manager->next_conversation_id = 1;

    return manager;
}

void social_manager_destroy(SocialManager* manager) {
    if (!manager) return;

    for (int i = 0; i < manager->relationship_count; i++) {
        relationship_destroy(manager->relationships[i]);
    }

    for (int i = 0; i < manager->personality_count; i++) {
        personality_destroy(manager->personalities[i]);
    }

    for (int i = 0; i < manager->gift_pref_count; i++) {
        gift_preferences_destroy(manager->gift_prefs[i]);
    }

    for (int i = 0; i < manager->active_conversation_count; i++) {
        conversation_destroy(manager->active_conversations[i]);
    }

    free(manager);
}

bool social_manager_add_relationship(SocialManager* manager, Relationship* rel) {
    if (!manager || !rel || manager->relationship_count >= MAX_RELATIONSHIPS) {
        return false;
    }

    manager->relationships[manager->relationship_count++] = rel;
    return true;
}

Relationship* social_manager_get_relationship(
    const SocialManager* manager,
    int entity_a_id,
    int entity_b_id
) {
    if (!manager) return NULL;

    for (int i = 0; i < manager->relationship_count; i++) {
        Relationship* rel = manager->relationships[i];
        if ((rel->entity_a_id == entity_a_id && rel->entity_b_id == entity_b_id) ||
            (rel->entity_a_id == entity_b_id && rel->entity_b_id == entity_a_id)) {
            return rel;
        }
    }

    return NULL;
}

Relationship* social_manager_ensure_relationship(
    SocialManager* manager,
    int entity_a_id,
    int entity_b_id
) {
    if (!manager) return NULL;

    Relationship* rel = social_manager_get_relationship(manager, entity_a_id, entity_b_id);
    if (rel) return rel;

    // Create new relationship
    rel = relationship_create(entity_a_id, entity_b_id);
    if (!rel) return NULL;

    if (!social_manager_add_relationship(manager, rel)) {
        relationship_destroy(rel);
        return NULL;
    }

    return rel;
}

bool social_manager_add_personality(SocialManager* manager, Personality* personality) {
    if (!manager || !personality || manager->personality_count >= MAX_RELATIONSHIPS) {
        return false;
    }

    manager->personalities[manager->personality_count++] = personality;
    return true;
}

Personality* social_manager_get_personality(const SocialManager* manager, int entity_id) {
    if (!manager) return NULL;

    for (int i = 0; i < manager->personality_count; i++) {
        if (manager->personalities[i]->entity_id == entity_id) {
            return manager->personalities[i];
        }
    }

    return NULL;
}

bool social_manager_add_gift_preferences(SocialManager* manager, GiftPreferences* prefs) {
    if (!manager || !prefs || manager->gift_pref_count >= MAX_RELATIONSHIPS) {
        return false;
    }

    manager->gift_prefs[manager->gift_pref_count++] = prefs;
    return true;
}

GiftPreferences* social_manager_get_gift_preferences(
    const SocialManager* manager,
    int entity_id
) {
    if (!manager) return NULL;

    for (int i = 0; i < manager->gift_pref_count; i++) {
        if (manager->gift_prefs[i]->entity_id == entity_id) {
            return manager->gift_prefs[i];
        }
    }

    return NULL;
}

bool social_manager_start_conversation(
    SocialManager* manager,
    int initiator_id,
    int recipient_id,
    Conversation** out_conversation
) {
    if (!manager || !out_conversation) return false;
    if (manager->active_conversation_count >= MAX_RELATIONSHIPS) return false;

    Conversation* conv = conversation_create(
        manager->next_conversation_id++,
        initiator_id,
        recipient_id
    );

    if (!conv) return false;

    manager->active_conversations[manager->active_conversation_count++] = conv;
    *out_conversation = conv;

    return true;
}

Conversation* social_manager_get_active_conversation(
    const SocialManager* manager,
    int entity_id
) {
    if (!manager) return NULL;

    for (int i = 0; i < manager->active_conversation_count; i++) {
        Conversation* conv = manager->active_conversations[i];
        if ((conv->initiator_id == entity_id || conv->recipient_id == entity_id) &&
            !conv->completed) {
            return conv;
        }
    }

    return NULL;
}

bool social_manager_end_conversation(SocialManager* manager, int conversation_id) {
    if (!manager) return false;

    for (int i = 0; i < manager->active_conversation_count; i++) {
        if (manager->active_conversations[i]->id == conversation_id) {
            conversation_end(manager->active_conversations[i]);
            return true;
        }
    }

    return false;
}

bool social_manager_have_conversation(
    SocialManager* manager,
    int entity_a_id,
    int entity_b_id,
    ConversationTopic topic
) {
    if (!manager) return false;

    (void)topic;  // Topic could be used for topic-specific affection modifiers in future

    // Get or create relationship
    Relationship* rel = social_manager_ensure_relationship(manager, entity_a_id, entity_b_id);
    if (!rel) return false;

    // Get personalities
    Personality* person_a = social_manager_get_personality(manager, entity_a_id);
    Personality* person_b = social_manager_get_personality(manager, entity_b_id);

    // Calculate affection gain based on topic and personalities
    int affection_gain = 3;  // Base gain from conversation

    // Chattiness affects how much they enjoy talking
    if (person_b && person_b->chattiness > 70) {
        affection_gain += 2;  // Chatty people enjoy conversations more
    }

    // Friendly people gain more affection from social interaction
    if (person_a && personality_has_trait(person_a, TRAIT_FRIENDLY)) {
        affection_gain += 1;
    }
    if (person_b && personality_has_trait(person_b, TRAIT_FRIENDLY)) {
        affection_gain += 1;
    }

    // Apply to relationship
    relationship_modify_affection(rel, affection_gain);
    relationship_modify_trust(rel, 1);  // Small trust gain
    relationship_record_talk(rel);

    return true;
}

bool social_manager_give_gift(
    SocialManager* manager,
    int giver_id,
    int receiver_id,
    const char* item_name,
    int item_value
) {
    if (!manager || !item_name) return false;

    // Get or create relationship
    Relationship* rel = social_manager_ensure_relationship(manager, giver_id, receiver_id);
    if (!rel) return false;

    // Get receiver's preferences and personality
    GiftPreferences* prefs = social_manager_get_gift_preferences(manager, receiver_id);
    Personality* personality = social_manager_get_personality(manager, receiver_id);

    // Create gift
    Gift* gift = gift_create(giver_id, receiver_id, item_name, item_value);
    if (!gift) return false;

    // Apply to relationship
    gift_apply_to_relationship(gift, rel, prefs, personality);

    gift_destroy(gift);
    return true;
}

void social_manager_update_all(SocialManager* manager, int days_passed) {
    if (!manager) return;

    for (int i = 0; i < manager->relationship_count; i++) {
        relationship_apply_decay(manager->relationships[i], days_passed);
    }
}

// ============================================================================
// Default Content
// ============================================================================

void create_default_personalities(SocialManager* manager) {
    if (!manager) return;

    // Farmer (friendly, hardworking)
    Personality* farmer = personality_create(1);
    if (farmer) {
        personality_add_trait(farmer, TRAIT_FRIENDLY);
        personality_add_trait(farmer, TRAIT_HONEST);
        personality_add_trait(farmer, TRAIT_GENEROUS);
        social_manager_add_personality(manager, farmer);
    }

    // Merchant (greedy but honest)
    Personality* merchant = personality_create(2);
    if (merchant) {
        personality_add_trait(merchant, TRAIT_GREEDY);
        personality_add_trait(merchant, TRAIT_HONEST);
        personality_add_trait(merchant, TRAIT_AMBITIOUS);
        social_manager_add_personality(manager, merchant);
    }

    // Shy villager
    Personality* shy_villager = personality_create(3);
    if (shy_villager) {
        personality_add_trait(shy_villager, TRAIT_SHY);
        personality_add_trait(shy_villager, TRAIT_HONEST);
        social_manager_add_personality(manager, shy_villager);
    }
}

void create_default_gift_preferences(SocialManager* manager) {
    if (!manager) return;

    // Farmer loves farming items
    GiftPreferences* farmer_prefs = gift_preferences_create(1);
    if (farmer_prefs) {
        gift_preferences_add_loved(farmer_prefs, "Hoe");
        gift_preferences_add_loved(farmer_prefs, "Watering Can");
        gift_preferences_add_loved(farmer_prefs, "Wheat Seeds");
        gift_preferences_add_liked(farmer_prefs, "Wheat");
        gift_preferences_add_liked(farmer_prefs, "Corn");
        gift_preferences_add_disliked(farmer_prefs, "Stone");
        social_manager_add_gift_preferences(manager, farmer_prefs);
    }

    // Merchant loves valuables
    GiftPreferences* merchant_prefs = gift_preferences_create(2);
    if (merchant_prefs) {
        gift_preferences_add_loved(merchant_prefs, "Iron Ore");
        gift_preferences_add_loved(merchant_prefs, "Bread");
        gift_preferences_add_liked(merchant_prefs, "Wheat");
        gift_preferences_add_liked(merchant_prefs, "Corn");
        social_manager_add_gift_preferences(manager, merchant_prefs);
    }

    // Shy villager loves simple gifts
    GiftPreferences* shy_prefs = gift_preferences_create(3);
    if (shy_prefs) {
        gift_preferences_add_loved(shy_prefs, "Carrot");
        gift_preferences_add_loved(shy_prefs, "Tomato");
        gift_preferences_add_liked(shy_prefs, "Bread");
        gift_preferences_add_liked(shy_prefs, "Vegetable Soup");
        social_manager_add_gift_preferences(manager, shy_prefs);
    }
}

// ============================================================================
// Serialization
// ============================================================================

cJSON* relationship_to_json(const Relationship* rel) {
    if (!rel) return NULL;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "entity_a_id", rel->entity_a_id);
    cJSON_AddNumberToObject(json, "entity_b_id", rel->entity_b_id);
    cJSON_AddNumberToObject(json, "type", rel->type);
    cJSON_AddNumberToObject(json, "affection", rel->affection);
    cJSON_AddNumberToObject(json, "trust", rel->trust);
    cJSON_AddNumberToObject(json, "respect", rel->respect);
    cJSON_AddNumberToObject(json, "times_talked", rel->times_talked);
    cJSON_AddNumberToObject(json, "times_gifted", rel->times_gifted);
    cJSON_AddNumberToObject(json, "days_since_interaction", rel->days_since_interaction);
    cJSON_AddBoolToObject(json, "is_locked", rel->is_locked);

    return json;
}

Relationship* relationship_from_json(const cJSON* json) {
    if (!json) return NULL;

    const cJSON* entity_a = cJSON_GetObjectItem(json, "entity_a_id");
    const cJSON* entity_b = cJSON_GetObjectItem(json, "entity_b_id");

    if (!entity_a || !entity_b) return NULL;

    Relationship* rel = relationship_create(entity_a->valueint, entity_b->valueint);
    if (!rel) return NULL;

    const cJSON* type = cJSON_GetObjectItem(json, "type");
    const cJSON* affection = cJSON_GetObjectItem(json, "affection");
    const cJSON* trust = cJSON_GetObjectItem(json, "trust");
    const cJSON* respect = cJSON_GetObjectItem(json, "respect");
    const cJSON* times_talked = cJSON_GetObjectItem(json, "times_talked");
    const cJSON* times_gifted = cJSON_GetObjectItem(json, "times_gifted");
    const cJSON* days_since = cJSON_GetObjectItem(json, "days_since_interaction");
    const cJSON* is_locked = cJSON_GetObjectItem(json, "is_locked");

    if (type) rel->type = type->valueint;
    if (affection) rel->affection = affection->valueint;
    if (trust) rel->trust = trust->valueint;
    if (respect) rel->respect = respect->valueint;
    if (times_talked) rel->times_talked = times_talked->valueint;
    if (times_gifted) rel->times_gifted = times_gifted->valueint;
    if (days_since) rel->days_since_interaction = days_since->valueint;
    if (is_locked) rel->is_locked = cJSON_IsTrue(is_locked);

    return rel;
}

cJSON* personality_to_json(const Personality* personality) {
    if (!personality) return NULL;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "entity_id", personality->entity_id);
    cJSON_AddNumberToObject(json, "friendliness", personality->friendliness);
    cJSON_AddNumberToObject(json, "generosity", personality->generosity);
    cJSON_AddNumberToObject(json, "chattiness", personality->chattiness);
    cJSON_AddNumberToObject(json, "trustworthiness", personality->trustworthiness);

    cJSON* traits = cJSON_CreateArray();
    for (int i = 0; i < personality->trait_count; i++) {
        cJSON_AddItemToArray(traits, cJSON_CreateNumber(personality->traits[i]));
    }
    cJSON_AddItemToObject(json, "traits", traits);

    return json;
}

Personality* personality_from_json(const cJSON* json) {
    if (!json) return NULL;

    const cJSON* entity_id = cJSON_GetObjectItem(json, "entity_id");
    if (!entity_id) return NULL;

    Personality* personality = personality_create(entity_id->valueint);
    if (!personality) return NULL;

    const cJSON* friendliness = cJSON_GetObjectItem(json, "friendliness");
    const cJSON* generosity = cJSON_GetObjectItem(json, "generosity");
    const cJSON* chattiness = cJSON_GetObjectItem(json, "chattiness");
    const cJSON* trustworthiness = cJSON_GetObjectItem(json, "trustworthiness");

    if (friendliness) personality->friendliness = friendliness->valueint;
    if (generosity) personality->generosity = generosity->valueint;
    if (chattiness) personality->chattiness = chattiness->valueint;
    if (trustworthiness) personality->trustworthiness = trustworthiness->valueint;

    const cJSON* traits = cJSON_GetObjectItem(json, "traits");
    if (traits) {
        const cJSON* trait = NULL;
        cJSON_ArrayForEach(trait, traits) {
            if (personality->trait_count < MAX_PERSONALITY_TRAITS) {
                personality->traits[personality->trait_count++] = trait->valueint;
            }
        }
    }

    return personality;
}

cJSON* gift_preferences_to_json(const GiftPreferences* prefs) {
    if (!prefs) return NULL;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "entity_id", prefs->entity_id);

    cJSON* loved = cJSON_CreateArray();
    for (int i = 0; i < prefs->loved_count; i++) {
        cJSON_AddItemToArray(loved, cJSON_CreateString(prefs->loved_items[i]));
    }
    cJSON_AddItemToObject(json, "loved", loved);

    cJSON* liked = cJSON_CreateArray();
    for (int i = 0; i < prefs->liked_count; i++) {
        cJSON_AddItemToArray(liked, cJSON_CreateString(prefs->liked_items[i]));
    }
    cJSON_AddItemToObject(json, "liked", liked);

    cJSON* disliked = cJSON_CreateArray();
    for (int i = 0; i < prefs->disliked_count; i++) {
        cJSON_AddItemToArray(disliked, cJSON_CreateString(prefs->disliked_items[i]));
    }
    cJSON_AddItemToObject(json, "disliked", disliked);

    return json;
}

GiftPreferences* gift_preferences_from_json(const cJSON* json) {
    if (!json) return NULL;

    const cJSON* entity_id = cJSON_GetObjectItem(json, "entity_id");
    if (!entity_id) return NULL;

    GiftPreferences* prefs = gift_preferences_create(entity_id->valueint);
    if (!prefs) return NULL;

    const cJSON* loved = cJSON_GetObjectItem(json, "loved");
    if (loved) {
        const cJSON* item = NULL;
        cJSON_ArrayForEach(item, loved) {
            if (item->valuestring) {
                gift_preferences_add_loved(prefs, item->valuestring);
            }
        }
    }

    const cJSON* liked = cJSON_GetObjectItem(json, "liked");
    if (liked) {
        const cJSON* item = NULL;
        cJSON_ArrayForEach(item, liked) {
            if (item->valuestring) {
                gift_preferences_add_liked(prefs, item->valuestring);
            }
        }
    }

    const cJSON* disliked = cJSON_GetObjectItem(json, "disliked");
    if (disliked) {
        const cJSON* item = NULL;
        cJSON_ArrayForEach(item, disliked) {
            if (item->valuestring) {
                gift_preferences_add_disliked(prefs, item->valuestring);
            }
        }
    }

    return prefs;
}
