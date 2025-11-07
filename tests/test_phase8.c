#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../src/core/social.h"
#include "../lib/cJSON.h"

int tests_passed = 0;
int tests_failed = 0;

void test_relationships() {
    printf("\nTest: Relationship System\n");

    // Create relationship
    Relationship* rel = relationship_create(1, 2);
    assert(rel != NULL);
    assert(rel->entity_a_id == 1);
    assert(rel->entity_b_id == 2);
    assert(rel->affection == 0);
    assert(rel->trust == 50);
    assert(rel->type == RELATIONSHIP_STRANGER);
    printf("  ✓ Relationship creation works\n");

    // Modify affection
    relationship_modify_affection(rel, 30);
    assert(rel->affection == 30);
    assert(rel->type == RELATIONSHIP_ACQUAINTANCE);
    printf("  ✓ Affection modification works\n");

    // Reach friend status
    relationship_modify_affection(rel, 25);
    assert(rel->affection == 55);
    assert(rel->type == RELATIONSHIP_FRIEND);
    printf("  ✓ Friend relationship type works\n");

    // Reach close friend status
    relationship_modify_affection(rel, 30);
    assert(rel->affection == 85);
    assert(rel->type == RELATIONSHIP_CLOSE_FRIEND);
    printf("  ✓ Close friend relationship type works\n");

    // Test negative affection
    relationship_modify_affection(rel, -140);  // Brings to -55
    assert(rel->affection == -55);
    assert(rel->type == RELATIONSHIP_RIVAL);
    printf("  ✓ Rival relationship type works\n");

    // Test clamping
    relationship_modify_affection(rel, -100);
    assert(rel->affection == -100);  // Clamped at -100
    printf("  ✓ Affection clamping works\n");

    // Test interaction recording
    relationship_record_talk(rel);
    assert(rel->times_talked == 1);
    relationship_record_gift(rel);
    assert(rel->times_gifted == 1);
    printf("  ✓ Interaction recording works\n");

    relationship_destroy(rel);
    printf("  ✓ All relationship tests passed\n");
    tests_passed++;
}

void test_personality() {
    printf("\nTest: Personality System\n");

    // Create personality
    Personality* personality = personality_create(1);
    assert(personality != NULL);
    assert(personality->entity_id == 1);
    assert(personality->friendliness == 50);
    assert(personality->trait_count == 0);
    printf("  ✓ Personality creation works\n");

    // Add friendly trait
    assert(personality_add_trait(personality, TRAIT_FRIENDLY) == true);
    assert(personality->trait_count == 1);
    assert(personality->friendliness == 70);  // 50 + 20
    assert(personality->chattiness == 65);    // 50 + 15
    printf("  ✓ Adding friendly trait works\n");

    // Add generous trait
    assert(personality_add_trait(personality, TRAIT_GENEROUS) == true);
    assert(personality->generosity == 80);  // 50 + 30
    printf("  ✓ Adding generous trait works\n");

    // Check has trait
    assert(personality_has_trait(personality, TRAIT_FRIENDLY) == true);
    assert(personality_has_trait(personality, TRAIT_GENEROUS) == true);
    assert(personality_has_trait(personality, TRAIT_GREEDY) == false);
    printf("  ✓ Trait checking works\n");

    // Test modifiers
    float friend_mod = personality_get_friendliness_modifier(personality);
    assert(friend_mod > 1.0f);  // Above 50 means > 1.0
    printf("  ✓ Personality modifiers work\n");

    // Test duplicate prevention
    assert(personality_add_trait(personality, TRAIT_FRIENDLY) == false);
    assert(personality->trait_count == 2);  // Still 2
    printf("  ✓ Duplicate trait prevention works\n");

    personality_destroy(personality);
    printf("  ✓ All personality tests passed\n");
    tests_passed++;
}

void test_conversations() {
    printf("\nTest: Conversation System\n");

    // Create conversation
    Conversation* conv = conversation_create(1, 1, 2);
    assert(conv != NULL);
    assert(conv->id == 1);
    assert(conv->initiator_id == 1);
    assert(conv->recipient_id == 2);
    assert(conv->completed == false);
    printf("  ✓ Conversation creation works\n");

    // Add dialogue options
    assert(conversation_add_option(conv, "How's the weather?", TOPIC_WEATHER, 2, 1, 0) == true);
    assert(conversation_add_option(conv, "Tell me about your work", TOPIC_WORK, 3, 2, 1) == true);
    assert(conv->option_count == 2);
    printf("  ✓ Adding dialogue options works\n");

    // Get available options
    DialogueOption* options[10];
    Relationship* rel = relationship_create(1, 2);
    int count = conversation_get_available_options(conv, rel, options, 10);
    assert(count == 2);
    printf("  ✓ Getting available options works\n");

    // Select option
    assert(conversation_select_option(conv, 0) == true);
    assert(conv->selected_option_id == 0);
    printf("  ✓ Selecting dialogue option works\n");

    // End conversation
    conversation_end(conv);
    assert(conv->completed == true);
    printf("  ✓ Ending conversation works\n");

    relationship_destroy(rel);
    conversation_destroy(conv);
    printf("  ✓ All conversation tests passed\n");
    tests_passed++;
}

void test_gifts() {
    printf("\nTest: Gift System\n");

    // Create gift
    Gift* gift = gift_create(1, 2, "Wheat", 12);
    assert(gift != NULL);
    assert(gift->giver_id == 1);
    assert(gift->receiver_id == 2);
    assert(strcmp(gift->item_name, "Wheat") == 0);
    assert(gift->item_value == 12);
    printf("  ✓ Gift creation works\n");

    // Create gift preferences
    GiftPreferences* prefs = gift_preferences_create(2);
    assert(prefs != NULL);
    assert(prefs->entity_id == 2);
    printf("  ✓ Gift preferences creation works\n");

    // Add loved item
    assert(gift_preferences_add_loved(prefs, "Hoe") == true);
    assert(prefs->loved_count == 1);
    assert(gift_preferences_is_loved(prefs, "Hoe") == true);
    assert(gift_preferences_is_loved(prefs, "Wheat") == false);
    printf("  ✓ Adding loved items works\n");

    // Add liked and disliked items
    assert(gift_preferences_add_liked(prefs, "Wheat") == true);
    assert(gift_preferences_add_disliked(prefs, "Stone") == true);
    printf("  ✓ Adding liked/disliked items works\n");

    // Calculate affection for liked item
    int affection = gift_calculate_affection(gift, prefs, NULL);
    assert(affection == 10);  // Liked item
    printf("  ✓ Gift affection calculation works (liked item = 10)\n");

    // Test loved item
    Gift* loved_gift = gift_create(1, 2, "Hoe", 50);
    affection = gift_calculate_affection(loved_gift, prefs, NULL);
    assert(affection >= 15);  // Loved item + expensive bonus
    printf("  ✓ Loved gift affection works\n");

    // Test disliked item
    Gift* disliked_gift = gift_create(1, 2, "Stone", 3);
    affection = gift_calculate_affection(disliked_gift, prefs, NULL);
    assert(affection < 0);  // Disliked item
    printf("  ✓ Disliked gift affection works\n");

    // Test applying gift to relationship
    Relationship* rel = relationship_create(1, 2);
    int initial_affection = rel->affection;
    gift_apply_to_relationship(gift, rel, prefs, NULL);
    assert(rel->affection > initial_affection);
    assert(rel->times_gifted == 1);
    printf("  ✓ Applying gift to relationship works\n");

    gift_destroy(gift);
    gift_destroy(loved_gift);
    gift_destroy(disliked_gift);
    gift_preferences_destroy(prefs);
    relationship_destroy(rel);
    printf("  ✓ All gift tests passed\n");
    tests_passed++;
}

void test_social_manager() {
    printf("\nTest: Social Manager\n");

    // Create social manager
    SocialManager* manager = social_manager_create();
    assert(manager != NULL);
    assert(manager->relationship_count == 0);
    assert(manager->personality_count == 0);
    printf("  ✓ Social manager creation works\n");

    // Ensure relationship (auto-create)
    Relationship* rel = social_manager_ensure_relationship(manager, 1, 2);
    assert(rel != NULL);
    assert(manager->relationship_count == 1);
    printf("  ✓ Ensuring relationship works\n");

    // Get existing relationship
    Relationship* rel2 = social_manager_get_relationship(manager, 1, 2);
    assert(rel2 == rel);  // Should be same object
    printf("  ✓ Getting existing relationship works\n");

    // Bidirectional lookup
    Relationship* rel3 = social_manager_get_relationship(manager, 2, 1);
    assert(rel3 == rel);  // Should work in reverse
    printf("  ✓ Bidirectional relationship lookup works\n");

    // Add personality
    Personality* personality = personality_create(1);
    personality_add_trait(personality, TRAIT_FRIENDLY);
    assert(social_manager_add_personality(manager, personality) == true);
    assert(manager->personality_count == 1);
    printf("  ✓ Adding personality works\n");

    // Get personality
    Personality* found_personality = social_manager_get_personality(manager, 1);
    assert(found_personality == personality);
    printf("  ✓ Getting personality works\n");

    // Add gift preferences
    GiftPreferences* prefs = gift_preferences_create(1);
    gift_preferences_add_loved(prefs, "Hoe");
    assert(social_manager_add_gift_preferences(manager, prefs) == true);
    printf("  ✓ Adding gift preferences works\n");

    social_manager_destroy(manager);
    printf("  ✓ All social manager tests passed\n");
    tests_passed++;
}

void test_social_interactions() {
    printf("\nTest: Social Interactions\n");

    SocialManager* manager = social_manager_create();

    // Create two entities with personalities
    Personality* person1 = personality_create(1);
    personality_add_trait(person1, TRAIT_FRIENDLY);
    social_manager_add_personality(manager, person1);

    Personality* person2 = personality_create(2);
    personality_add_trait(person2, TRAIT_GENEROUS);
    social_manager_add_personality(manager, person2);

    // Have conversation
    assert(social_manager_have_conversation(manager, 1, 2, TOPIC_WEATHER) == true);
    printf("  ✓ Having conversation works\n");

    // Check relationship was created and updated
    Relationship* rel = social_manager_get_relationship(manager, 1, 2);
    assert(rel != NULL);
    assert(rel->affection > 0);  // Conversation increases affection
    assert(rel->times_talked == 1);
    printf("  ✓ Conversation creates and updates relationship\n");

    // Give gift
    GiftPreferences* prefs = gift_preferences_create(2);
    gift_preferences_add_loved(prefs, "Hoe");
    social_manager_add_gift_preferences(manager, prefs);

    int affection_before = rel->affection;
    assert(social_manager_give_gift(manager, 1, 2, "Hoe", 50) == true);
    assert(rel->affection > affection_before);  // Gift increases affection
    assert(rel->times_gifted == 1);
    printf("  ✓ Giving gift works\n");

    // Multiple conversations increase friendship
    social_manager_have_conversation(manager, 1, 2, TOPIC_FARMING);
    social_manager_have_conversation(manager, 1, 2, TOPIC_WORK);
    assert(rel->times_talked == 3);
    printf("  ✓ Multiple interactions tracked\n");

    social_manager_destroy(manager);
    printf("  ✓ All social interaction tests passed\n");
    tests_passed++;
}

void test_relationship_decay() {
    printf("\nTest: Relationship Decay\n");

    Relationship* rel = relationship_create(1, 2);
    relationship_modify_affection(rel, 50);
    assert(rel->affection == 50);

    // No decay in first week
    relationship_apply_decay(rel, 3);
    assert(rel->affection == 50);
    printf("  ✓ No decay within 7 days\n");

    // Decay after two weeks (need 14+ days for -1 decay)
    relationship_apply_decay(rel, 12);  // Now 15 days total
    assert(rel->affection < 50);  // Should have decayed by 1
    printf("  ✓ Decay applies after 14 days\n");

    // Locked relationships don't decay
    Relationship* locked_rel = relationship_create(3, 4);
    relationship_modify_affection(locked_rel, 50);  // Modify before locking
    locked_rel->is_locked = true;  // Lock after setting affection
    relationship_apply_decay(locked_rel, 30);
    assert(locked_rel->affection == 50);  // No decay
    printf("  ✓ Locked relationships don't decay\n");

    relationship_destroy(rel);
    relationship_destroy(locked_rel);
    printf("  ✓ All decay tests passed\n");
    tests_passed++;
}

void test_serialization() {
    printf("\nTest: Serialization\n");

    // Relationship serialization
    Relationship* rel = relationship_create(1, 2);
    relationship_modify_affection(rel, 60);
    relationship_modify_trust(rel, 10);

    cJSON* rel_json = relationship_to_json(rel);
    assert(rel_json != NULL);
    printf("  ✓ Relationship to JSON works\n");

    Relationship* rel2 = relationship_from_json(rel_json);
    assert(rel2 != NULL);
    assert(rel2->entity_a_id == 1);
    assert(rel2->entity_b_id == 2);
    assert(rel2->affection == 60);
    assert(rel2->trust == 60);
    printf("  ✓ Relationship from JSON works\n");

    cJSON_Delete(rel_json);
    relationship_destroy(rel);
    relationship_destroy(rel2);

    // Personality serialization
    Personality* personality = personality_create(1);
    personality_add_trait(personality, TRAIT_FRIENDLY);
    personality_add_trait(personality, TRAIT_HONEST);

    cJSON* pers_json = personality_to_json(personality);
    assert(pers_json != NULL);
    printf("  ✓ Personality to JSON works\n");

    Personality* personality2 = personality_from_json(pers_json);
    assert(personality2 != NULL);
    assert(personality2->entity_id == 1);
    assert(personality2->trait_count == 2);
    assert(personality_has_trait(personality2, TRAIT_FRIENDLY) == true);
    printf("  ✓ Personality from JSON works\n");

    cJSON_Delete(pers_json);
    personality_destroy(personality);
    personality_destroy(personality2);

    // Gift preferences serialization
    GiftPreferences* prefs = gift_preferences_create(1);
    gift_preferences_add_loved(prefs, "Hoe");
    gift_preferences_add_liked(prefs, "Wheat");
    gift_preferences_add_disliked(prefs, "Stone");

    cJSON* prefs_json = gift_preferences_to_json(prefs);
    assert(prefs_json != NULL);
    printf("  ✓ Gift preferences to JSON works\n");

    GiftPreferences* prefs2 = gift_preferences_from_json(prefs_json);
    assert(prefs2 != NULL);
    assert(prefs2->entity_id == 1);
    assert(prefs2->loved_count == 1);
    assert(gift_preferences_is_loved(prefs2, "Hoe") == true);
    printf("  ✓ Gift preferences from JSON works\n");

    cJSON_Delete(prefs_json);
    gift_preferences_destroy(prefs);
    gift_preferences_destroy(prefs2);

    printf("  ✓ All serialization tests passed\n");
    tests_passed++;
}

void test_default_content() {
    printf("\nTest: Default Content\n");

    SocialManager* manager = social_manager_create();

    // Create default personalities
    create_default_personalities(manager);
    assert(manager->personality_count == 3);
    printf("  ✓ Default personalities created (3 NPCs)\n");

    // Check farmer personality
    Personality* farmer = social_manager_get_personality(manager, 1);
    assert(farmer != NULL);
    assert(personality_has_trait(farmer, TRAIT_FRIENDLY) == true);
    assert(personality_has_trait(farmer, TRAIT_HONEST) == true);
    printf("  ✓ Farmer personality has correct traits\n");

    // Create default gift preferences
    create_default_gift_preferences(manager);
    assert(manager->gift_pref_count == 3);
    printf("  ✓ Default gift preferences created (3 NPCs)\n");

    // Check farmer preferences
    GiftPreferences* farmer_prefs = social_manager_get_gift_preferences(manager, 1);
    assert(farmer_prefs != NULL);
    assert(gift_preferences_is_loved(farmer_prefs, "Hoe") == true);
    assert(gift_preferences_is_loved(farmer_prefs, "Watering Can") == true);
    printf("  ✓ Farmer loves farming tools\n");

    social_manager_destroy(manager);
    printf("  ✓ All default content tests passed\n");
    tests_passed++;
}

int main() {
    printf("============================================================\n");
    printf("PHASE 8: Social Systems Tests\n");
    printf("============================================================\n");

    test_relationships();
    test_personality();
    test_conversations();
    test_gifts();
    test_social_manager();
    test_social_interactions();
    test_relationship_decay();
    test_serialization();
    test_default_content();

    printf("\n============================================================\n");
    printf("RESULTS: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("============================================================\n");

    if (tests_failed == 0) {
        printf("\n🎉 Phase 8: Social Systems - COMPLETE! 🎉\n\n");
        printf("Success Criteria Met:\n");
        printf("✓ Relationships track affection, trust, and respect\n");
        printf("✓ Relationship types evolve based on affection\n");
        printf("✓ Personality traits affect social interactions\n");
        printf("✓ Conversation system with dialogue options\n");
        printf("✓ Gift system with preferences and reactions\n");
        printf("✓ Social manager coordinates all interactions\n");
        printf("✓ Relationships decay without maintenance\n");
        printf("✓ Serialization saves/loads social state\n");
        printf("✓ Default NPCs have unique personalities\n");
    }

    return tests_failed > 0 ? 1 : 0;
}
