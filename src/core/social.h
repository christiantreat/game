#ifndef SOCIAL_H
#define SOCIAL_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "../lib/cJSON.h"

/*
 * Phase 8: Social Systems
 *
 * Implements relationships, conversations, and social interactions.
 * All social decisions will be transparent and logged.
 */

// Constants
#define MAX_RELATIONSHIPS 100
#define MAX_DIALOGUE_OPTIONS 10
#define MAX_DIALOGUE_TEXT 512
#define MAX_CONVERSATION_HISTORY 50
#define MAX_GIFT_HISTORY 20
#define MAX_PERSONALITY_TRAITS 10
#define MAX_SOCIAL_TOPICS 30

// Relationship types
typedef enum {
    RELATIONSHIP_STRANGER,
    RELATIONSHIP_ACQUAINTANCE,
    RELATIONSHIP_FRIEND,
    RELATIONSHIP_CLOSE_FRIEND,
    RELATIONSHIP_ROMANTIC,
    RELATIONSHIP_FAMILY,
    RELATIONSHIP_RIVAL,
    RELATIONSHIP_ENEMY
} RelationshipType;

// Relationship status
typedef struct {
    int entity_a_id;
    int entity_b_id;
    RelationshipType type;
    int affection;           // -100 to 100 (negative = dislike, positive = like)
    int trust;               // 0 to 100
    int respect;             // 0 to 100
    int times_talked;
    int times_gifted;
    int days_since_interaction;
    time_t first_met;
    time_t last_interaction;
    bool is_locked;          // Prevents relationship changes (e.g., family)
} Relationship;

// Personality traits
typedef enum {
    TRAIT_FRIENDLY,          // Likes meeting new people
    TRAIT_SHY,               // Dislikes social interaction
    TRAIT_GENEROUS,          // Gives gifts often
    TRAIT_GREEDY,            // Reluctant to give
    TRAIT_HONEST,            // High trust impact
    TRAIT_DECEITFUL,         // Low trust
    TRAIT_OPTIMISTIC,        // Positive interactions
    TRAIT_PESSIMISTIC,       // Negative bias
    TRAIT_AMBITIOUS,         // Driven by goals
    TRAIT_LAZY               // Low energy
} PersonalityTrait;

// Personality profile
typedef struct {
    int entity_id;
    PersonalityTrait traits[MAX_PERSONALITY_TRAITS];
    int trait_count;
    int friendliness;        // 0-100: How easy to befriend
    int generosity;          // 0-100: Likelihood of giving gifts
    int chattiness;          // 0-100: Likes to talk
    int trustworthiness;     // 0-100: How trustworthy
} Personality;

// Conversation topics
typedef enum {
    TOPIC_WEATHER,
    TOPIC_FARMING,
    TOPIC_FAMILY,
    TOPIC_WORK,
    TOPIC_HOBBIES,
    TOPIC_GOSSIP,
    TOPIC_DREAMS,
    TOPIC_PAST,
    TOPIC_ROMANCE,
    TOPIC_BUSINESS,
    TOPIC_FOOD,
    TOPIC_VILLAGE
} ConversationTopic;

// Dialogue option
typedef struct {
    int id;
    char text[MAX_DIALOGUE_TEXT];
    ConversationTopic topic;
    int affection_change;    // How much this affects relationship
    int trust_change;
    int respect_change;
    bool requires_min_affection;
    int min_affection;       // Minimum affection to unlock this option
} DialogueOption;

// Conversation
typedef struct {
    int id;
    int initiator_id;
    int recipient_id;
    DialogueOption* options[MAX_DIALOGUE_OPTIONS];
    int option_count;
    int selected_option_id;
    time_t started_at;
    time_t ended_at;
    bool completed;
} Conversation;

// Gift
typedef struct {
    int giver_id;
    int receiver_id;
    char item_name[64];
    int item_value;
    int affection_gained;
    time_t given_at;
    bool was_loved;          // Receiver loved this gift
    bool was_liked;          // Receiver liked this gift
    bool was_neutral;        // Receiver was neutral
    bool was_disliked;       // Receiver disliked this gift
} Gift;

// Gift preferences
typedef struct {
    int entity_id;
    char loved_items[10][64];     // Items that give +15 affection
    int loved_count;
    char liked_items[10][64];     // Items that give +10 affection
    int liked_count;
    char disliked_items[10][64];  // Items that give -5 affection
    int disliked_count;
} GiftPreferences;

// Social event (for transparency)
typedef enum {
    SOCIAL_EVENT_MET,
    SOCIAL_EVENT_TALKED,
    SOCIAL_EVENT_GIFTED,
    SOCIAL_EVENT_RELATIONSHIP_UP,
    SOCIAL_EVENT_RELATIONSHIP_DOWN,
    SOCIAL_EVENT_BECAME_FRIENDS,
    SOCIAL_EVENT_BECAME_RIVALS,
    SOCIAL_EVENT_ROMANCE_STARTED,
    SOCIAL_EVENT_ROMANCE_ENDED
} SocialEventType;

// Social manager
typedef struct {
    Relationship* relationships[MAX_RELATIONSHIPS];
    int relationship_count;
    Personality* personalities[MAX_RELATIONSHIPS];
    int personality_count;
    GiftPreferences* gift_prefs[MAX_RELATIONSHIPS];
    int gift_pref_count;
    Conversation* active_conversations[MAX_RELATIONSHIPS];
    int active_conversation_count;
    int next_conversation_id;
} SocialManager;

// ============================================================================
// Relationship Management
// ============================================================================

Relationship* relationship_create(int entity_a_id, int entity_b_id);
void relationship_destroy(Relationship* rel);

// Update affection, trust, respect
void relationship_modify_affection(Relationship* rel, int change);
void relationship_modify_trust(Relationship* rel, int change);
void relationship_modify_respect(Relationship* rel, int change);

// Get relationship level based on affection
RelationshipType relationship_get_type(const Relationship* rel);

// Update relationship type based on current stats
void relationship_update_type(Relationship* rel);

// Check if relationship meets requirements
bool relationship_meets_requirements(const Relationship* rel, int min_affection, int min_trust);

// Record interaction
void relationship_record_talk(Relationship* rel);
void relationship_record_gift(Relationship* rel);

// Decay over time (relationships fade if not maintained)
void relationship_apply_decay(Relationship* rel, int days_passed);

// ============================================================================
// Personality System
// ============================================================================

Personality* personality_create(int entity_id);
void personality_destroy(Personality* personality);

// Add personality trait
bool personality_add_trait(Personality* personality, PersonalityTrait trait);

// Check if entity has trait
bool personality_has_trait(const Personality* personality, PersonalityTrait trait);

// Get modifier for social interaction based on personality
float personality_get_friendliness_modifier(const Personality* personality);
float personality_get_generosity_modifier(const Personality* personality);
float personality_get_trust_modifier(const Personality* personality);

// ============================================================================
// Conversation System
// ============================================================================

Conversation* conversation_create(int id, int initiator_id, int recipient_id);
void conversation_destroy(Conversation* conversation);

// Add dialogue option
bool conversation_add_option(
    Conversation* conv,
    const char* text,
    ConversationTopic topic,
    int affection_change,
    int trust_change,
    int respect_change
);

// Select a dialogue option
bool conversation_select_option(Conversation* conv, int option_id);

// Get available options (filtered by requirements)
int conversation_get_available_options(
    const Conversation* conv,
    const Relationship* rel,
    DialogueOption** out_options,
    int max_options
);

// End conversation
void conversation_end(Conversation* conv);

// ============================================================================
// Gift System
// ============================================================================

Gift* gift_create(int giver_id, int receiver_id, const char* item_name, int item_value);
void gift_destroy(Gift* gift);

// Calculate affection gained from gift
int gift_calculate_affection(
    const Gift* gift,
    const GiftPreferences* prefs,
    const Personality* receiver_personality
);

// Apply gift effects to relationship
void gift_apply_to_relationship(
    const Gift* gift,
    Relationship* rel,
    const GiftPreferences* prefs,
    const Personality* receiver_personality
);

// Gift preferences
GiftPreferences* gift_preferences_create(int entity_id);
void gift_preferences_destroy(GiftPreferences* prefs);

bool gift_preferences_add_loved(GiftPreferences* prefs, const char* item_name);
bool gift_preferences_add_liked(GiftPreferences* prefs, const char* item_name);
bool gift_preferences_add_disliked(GiftPreferences* prefs, const char* item_name);

bool gift_preferences_is_loved(const GiftPreferences* prefs, const char* item_name);
bool gift_preferences_is_liked(const GiftPreferences* prefs, const char* item_name);
bool gift_preferences_is_disliked(const GiftPreferences* prefs, const char* item_name);

// ============================================================================
// Social Manager
// ============================================================================

SocialManager* social_manager_create(void);
void social_manager_destroy(SocialManager* manager);

// Register relationship
bool social_manager_add_relationship(SocialManager* manager, Relationship* rel);

// Find relationship between two entities
Relationship* social_manager_get_relationship(
    const SocialManager* manager,
    int entity_a_id,
    int entity_b_id
);

// Create relationship if it doesn't exist
Relationship* social_manager_ensure_relationship(
    SocialManager* manager,
    int entity_a_id,
    int entity_b_id
);

// Personality management
bool social_manager_add_personality(SocialManager* manager, Personality* personality);
Personality* social_manager_get_personality(const SocialManager* manager, int entity_id);

// Gift preferences management
bool social_manager_add_gift_preferences(SocialManager* manager, GiftPreferences* prefs);
GiftPreferences* social_manager_get_gift_preferences(
    const SocialManager* manager,
    int entity_id
);

// Conversation management
bool social_manager_start_conversation(
    SocialManager* manager,
    int initiator_id,
    int recipient_id,
    Conversation** out_conversation
);

Conversation* social_manager_get_active_conversation(
    const SocialManager* manager,
    int entity_id
);

bool social_manager_end_conversation(SocialManager* manager, int conversation_id);

// High-level social actions
bool social_manager_have_conversation(
    SocialManager* manager,
    int entity_a_id,
    int entity_b_id,
    ConversationTopic topic
);

bool social_manager_give_gift(
    SocialManager* manager,
    int giver_id,
    int receiver_id,
    const char* item_name,
    int item_value
);

// Update all relationships (decay)
void social_manager_update_all(SocialManager* manager, int days_passed);

// Create default personalities for NPCs
void create_default_personalities(SocialManager* manager);
void create_default_gift_preferences(SocialManager* manager);

// ============================================================================
// Serialization
// ============================================================================

cJSON* relationship_to_json(const Relationship* rel);
Relationship* relationship_from_json(const cJSON* json);

cJSON* personality_to_json(const Personality* personality);
Personality* personality_from_json(const cJSON* json);

cJSON* gift_preferences_to_json(const GiftPreferences* prefs);
GiftPreferences* gift_preferences_from_json(const cJSON* json);

#endif // SOCIAL_H
