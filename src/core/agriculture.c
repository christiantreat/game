/**
 * agriculture.c
 * Agriculture and Farming System Implementation
 */

#include "agriculture.h"
#include "../lib/cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// Crop Growth Stage Helpers
// ============================================================================

const char* crop_growth_stage_to_string(CropGrowthStage stage) {
    switch (stage) {
        case CROP_STAGE_SEED: return "Seed";
        case CROP_STAGE_SPROUT: return "Sprout";
        case CROP_STAGE_GROWING: return "Growing";
        case CROP_STAGE_MATURE: return "Mature";
        case CROP_STAGE_WITHERED: return "Withered";
        default: return "Unknown";
    }
}

// ============================================================================
// Crop Type Implementation
// ============================================================================

CropType* crop_type_create(const char* name, int days_to_mature, Season preferred_season) {
    if (!name) return NULL;

    CropType* type = calloc(1, sizeof(CropType));
    if (!type) return NULL;

    strncpy(type->name, name, MAX_CROP_NAME - 1);
    type->days_to_mature = days_to_mature;
    type->preferred_season = preferred_season;

    // Default values
    type->days_sprout = days_to_mature / 4;
    type->days_growing = days_to_mature / 2;
    type->can_grow_any_season = false;
    type->water_requirement = 50;
    type->needs_sun = true;
    type->base_yield = 5;
    type->min_yield = 1;
    type->max_yield = 10;
    type->sell_price = 10;
    type->seed_cost = 5;

    return type;
}

void crop_type_destroy(CropType* type) {
    if (type) {
        free(type);
    }
}

// ============================================================================
// Crop Instance Implementation
// ============================================================================

Crop* crop_create(int id, const char* crop_type_name, int field_location_id,
                 int plot_x, int plot_y, int planted_by) {
    if (!crop_type_name) return NULL;

    Crop* crop = calloc(1, sizeof(Crop));
    if (!crop) return NULL;

    crop->id = id;
    strncpy(crop->crop_type_name, crop_type_name, MAX_CROP_NAME - 1);
    crop->field_location_id = field_location_id;
    crop->plot_x = plot_x;
    crop->plot_y = plot_y;
    crop->stage = CROP_STAGE_SEED;
    crop->days_planted = 0;
    crop->days_in_current_stage = 0;
    crop->health = 100;
    crop->water_level = 50;
    crop->watered_today = false;
    crop->planted_by_entity_id = planted_by;
    crop->predicted_yield = 0;

    return crop;
}

void crop_destroy(Crop* crop) {
    if (crop) {
        free(crop);
    }
}

void crop_update(Crop* crop, const CropType* type, const GameState* game_state) {
    if (!crop || !type || !game_state) return;

    // Don't update withered crops
    if (crop->stage == CROP_STAGE_WITHERED) return;

    crop->days_planted++;
    crop->days_in_current_stage++;

    // Water consumption
    if (!crop->watered_today) {
        crop->water_level -= 15;
        if (crop->water_level < 0) crop->water_level = 0;
    }
    crop->watered_today = false;  // Reset for next day

    // Health effects
    if (crop->water_level < 20) {
        crop->health -= 10;  // Drought damage
    }

    // Weather effects
    if (game_state->current_weather == WEATHER_RAINY) {
        crop->water_level += 20;
        if (crop->water_level > 100) crop->water_level = 100;
    } else if (game_state->current_weather == WEATHER_STORMY) {
        crop->health -= 5;  // Storm damage
    } else if (game_state->current_weather == WEATHER_DROUGHT) {
        crop->water_level -= 10;
        crop->health -= 5;
    }

    // Season effects
    if (!type->can_grow_any_season && game_state->season != type->preferred_season) {
        crop->health -= 2;  // Wrong season penalty
    }

    // Death check
    if (crop->health <= 0) {
        crop->stage = CROP_STAGE_WITHERED;
        crop->health = 0;
        return;
    }

    // Growth stage transitions
    switch (crop->stage) {
        case CROP_STAGE_SEED:
            if (crop->days_in_current_stage >= type->days_sprout) {
                crop->stage = CROP_STAGE_SPROUT;
                crop->days_in_current_stage = 0;
            }
            break;

        case CROP_STAGE_SPROUT:
            if (crop->days_in_current_stage >= type->days_sprout) {
                crop->stage = CROP_STAGE_GROWING;
                crop->days_in_current_stage = 0;
            }
            break;

        case CROP_STAGE_GROWING:
            if (crop->days_planted >= type->days_to_mature) {
                crop->stage = CROP_STAGE_MATURE;
                crop->days_in_current_stage = 0;

                // Calculate yield
                float health_factor = crop->health / 100.0f;
                crop->predicted_yield = type->min_yield +
                    (int)((type->max_yield - type->min_yield) * health_factor);
            }
            break;

        case CROP_STAGE_MATURE:
            // Mature crops can wither if left too long
            if (crop->days_in_current_stage > 7) {
                crop->health -= 5;
            }
            break;

        case CROP_STAGE_WITHERED:
            // Already dead
            break;
    }

    // Clamp health
    if (crop->health > 100) crop->health = 100;
}

void crop_water(Crop* crop) {
    if (!crop || crop->stage == CROP_STAGE_WITHERED) return;

    crop->water_level += 40;
    if (crop->water_level > 100) crop->water_level = 100;
    crop->watered_today = true;

    // Watering provides small health boost
    if (crop->health < 100) {
        crop->health += 5;
        if (crop->health > 100) crop->health = 100;
    }
}

bool crop_is_ready_to_harvest(const Crop* crop) {
    return crop && crop->stage == CROP_STAGE_MATURE;
}

bool crop_is_withered(const Crop* crop) {
    return crop && crop->stage == CROP_STAGE_WITHERED;
}

float crop_get_stage_progress(const Crop* crop, const CropType* type) {
    if (!crop || !type) return 0.0f;

    return (float)crop->days_planted / (float)type->days_to_mature;
}

cJSON* crop_to_json(const Crop* crop) {
    if (!crop) return NULL;

    cJSON* json = cJSON_CreateObject();

    cJSON_AddNumberToObject(json, "id", crop->id);
    cJSON_AddStringToObject(json, "crop_type_name", crop->crop_type_name);
    cJSON_AddNumberToObject(json, "field_location_id", crop->field_location_id);
    cJSON_AddNumberToObject(json, "plot_x", crop->plot_x);
    cJSON_AddNumberToObject(json, "plot_y", crop->plot_y);
    cJSON_AddNumberToObject(json, "stage", crop->stage);
    cJSON_AddNumberToObject(json, "days_planted", crop->days_planted);
    cJSON_AddNumberToObject(json, "days_in_current_stage", crop->days_in_current_stage);
    cJSON_AddNumberToObject(json, "health", crop->health);
    cJSON_AddNumberToObject(json, "water_level", crop->water_level);
    cJSON_AddBoolToObject(json, "watered_today", crop->watered_today);
    cJSON_AddNumberToObject(json, "planted_by_entity_id", crop->planted_by_entity_id);
    cJSON_AddNumberToObject(json, "predicted_yield", crop->predicted_yield);

    return json;
}

Crop* crop_from_json(cJSON* json) {
    if (!json) return NULL;

    int id = cJSON_GetObjectItem(json, "id")->valueint;
    const char* type_name = cJSON_GetObjectItem(json, "crop_type_name")->valuestring;
    int field_id = cJSON_GetObjectItem(json, "field_location_id")->valueint;
    int plot_x = cJSON_GetObjectItem(json, "plot_x")->valueint;
    int plot_y = cJSON_GetObjectItem(json, "plot_y")->valueint;
    int planted_by = cJSON_GetObjectItem(json, "planted_by_entity_id")->valueint;

    Crop* crop = crop_create(id, type_name, field_id, plot_x, plot_y, planted_by);
    if (!crop) return NULL;

    crop->stage = cJSON_GetObjectItem(json, "stage")->valueint;
    crop->days_planted = cJSON_GetObjectItem(json, "days_planted")->valueint;
    crop->days_in_current_stage = cJSON_GetObjectItem(json, "days_in_current_stage")->valueint;
    crop->health = cJSON_GetObjectItem(json, "health")->valueint;
    crop->water_level = cJSON_GetObjectItem(json, "water_level")->valueint;
    crop->watered_today = cJSON_GetObjectItem(json, "watered_today")->valueint;
    crop->predicted_yield = cJSON_GetObjectItem(json, "predicted_yield")->valueint;

    return crop;
}

// ============================================================================
// Field Manager Implementation
// ============================================================================

FieldManager* field_manager_create(int location_id, int width, int height) {
    FieldManager* manager = calloc(1, sizeof(FieldManager));
    if (!manager) return NULL;

    manager->field_location_id = location_id;
    manager->field_width = width;
    manager->field_height = height;
    manager->max_plots = width * height;
    manager->crop_count = 0;
    manager->next_crop_id = 1;
    manager->total_planted = 0;
    manager->total_harvested = 0;

    return manager;
}

void field_manager_destroy(FieldManager* manager) {
    if (!manager) return;

    for (int i = 0; i < manager->crop_count; i++) {
        crop_destroy(manager->crops[i]);
    }

    free(manager);
}

int field_manager_plant_crop(FieldManager* manager, const char* crop_type_name,
                             int plot_x, int plot_y, int planted_by) {
    if (!manager || !crop_type_name) return -1;

    // Check if plot is occupied
    if (field_manager_is_plot_occupied(manager, plot_x, plot_y)) {
        return -1;
    }

    // Check capacity
    if (manager->crop_count >= MAX_CROPS_PER_FIELD ||
        manager->crop_count >= manager->max_plots) {
        return -1;
    }

    int id = manager->next_crop_id++;
    Crop* crop = crop_create(id, crop_type_name, manager->field_location_id,
                            plot_x, plot_y, planted_by);
    if (!crop) return -1;

    manager->crops[manager->crop_count++] = crop;
    manager->total_planted++;

    return id;
}

bool field_manager_remove_crop(FieldManager* manager, int crop_id) {
    if (!manager) return false;

    for (int i = 0; i < manager->crop_count; i++) {
        if (manager->crops[i]->id == crop_id) {
            crop_destroy(manager->crops[i]);

            // Shift remaining crops
            for (int j = i; j < manager->crop_count - 1; j++) {
                manager->crops[j] = manager->crops[j + 1];
            }

            manager->crop_count--;
            return true;
        }
    }

    return false;
}

Crop* field_manager_get_crop_at(FieldManager* manager, int plot_x, int plot_y) {
    if (!manager) return NULL;

    for (int i = 0; i < manager->crop_count; i++) {
        if (manager->crops[i]->plot_x == plot_x &&
            manager->crops[i]->plot_y == plot_y) {
            return manager->crops[i];
        }
    }

    return NULL;
}

Crop* field_manager_get_crop(FieldManager* manager, int crop_id) {
    if (!manager) return NULL;

    for (int i = 0; i < manager->crop_count; i++) {
        if (manager->crops[i]->id == crop_id) {
            return manager->crops[i];
        }
    }

    return NULL;
}

void field_manager_update_crops(FieldManager* manager, const CropType* types[],
                                int type_count, const GameState* game_state) {
    if (!manager || !types || !game_state) return;

    for (int i = 0; i < manager->crop_count; i++) {
        Crop* crop = manager->crops[i];

        // Find crop type
        const CropType* type = NULL;
        for (int j = 0; j < type_count; j++) {
            if (strcmp(types[j]->name, crop->crop_type_name) == 0) {
                type = types[j];
                break;
            }
        }

        if (type) {
            crop_update(crop, type, game_state);
        }
    }
}

int field_manager_water_all(FieldManager* manager) {
    if (!manager) return 0;

    int watered = 0;
    for (int i = 0; i < manager->crop_count; i++) {
        if (!crop_is_withered(manager->crops[i])) {
            crop_water(manager->crops[i]);
            watered++;
        }
    }

    return watered;
}

int field_manager_get_ready_crops(const FieldManager* manager, Crop** out_crops, int max_crops) {
    if (!manager || !out_crops) return 0;

    int count = 0;
    for (int i = 0; i < manager->crop_count && count < max_crops; i++) {
        if (crop_is_ready_to_harvest(manager->crops[i])) {
            out_crops[count++] = manager->crops[i];
        }
    }

    return count;
}

int field_manager_count_by_stage(const FieldManager* manager, CropGrowthStage stage) {
    if (!manager) return 0;

    int count = 0;
    for (int i = 0; i < manager->crop_count; i++) {
        if (manager->crops[i]->stage == stage) {
            count++;
        }
    }

    return count;
}

bool field_manager_is_plot_occupied(const FieldManager* manager, int plot_x, int plot_y) {
    return field_manager_get_crop_at((FieldManager*)manager, plot_x, plot_y) != NULL;
}

// ============================================================================
// Agriculture Manager Implementation
// ============================================================================

AgricultureManager* agriculture_manager_create(void) {
    AgricultureManager* manager = calloc(1, sizeof(AgricultureManager));
    if (!manager) return NULL;

    manager->crop_type_count = 0;
    manager->field_count = 0;

    return manager;
}

void agriculture_manager_destroy(AgricultureManager* manager) {
    if (!manager) return;

    // Destroy crop types
    for (int i = 0; i < manager->crop_type_count; i++) {
        crop_type_destroy(manager->crop_types[i]);
    }

    // Destroy fields
    for (int i = 0; i < manager->field_count; i++) {
        field_manager_destroy(manager->fields[i]);
    }

    free(manager);
}

bool agriculture_manager_register_crop_type(AgricultureManager* manager, CropType* type) {
    if (!manager || !type || manager->crop_type_count >= MAX_CROP_TYPES) {
        return false;
    }

    manager->crop_types[manager->crop_type_count++] = type;
    return true;
}

CropType* agriculture_manager_get_crop_type(AgricultureManager* manager, const char* name) {
    if (!manager || !name) return NULL;

    for (int i = 0; i < manager->crop_type_count; i++) {
        if (strcmp(manager->crop_types[i]->name, name) == 0) {
            return manager->crop_types[i];
        }
    }

    return NULL;
}

bool agriculture_manager_register_field(AgricultureManager* manager, int location_id, int width, int height) {
    if (!manager || manager->field_count >= MAX_LOCATIONS) {
        return false;
    }

    FieldManager* field = field_manager_create(location_id, width, height);
    if (!field) return false;

    manager->fields[manager->field_count++] = field;
    return true;
}

FieldManager* agriculture_manager_get_field(AgricultureManager* manager, int location_id) {
    if (!manager) return NULL;

    for (int i = 0; i < manager->field_count; i++) {
        if (manager->fields[i]->field_location_id == location_id) {
            return manager->fields[i];
        }
    }

    return NULL;
}

int agriculture_manager_plant_crop(AgricultureManager* manager, int field_location_id,
                                   const char* crop_type_name, int plot_x, int plot_y, int planted_by) {
    if (!manager) return -1;

    FieldManager* field = agriculture_manager_get_field(manager, field_location_id);
    if (!field) return -1;

    // Check if crop type exists
    if (!agriculture_manager_get_crop_type(manager, crop_type_name)) {
        return -1;
    }

    return field_manager_plant_crop(field, crop_type_name, plot_x, plot_y, planted_by);
}

void agriculture_manager_update_all(AgricultureManager* manager, const GameState* game_state) {
    if (!manager || !game_state) return;

    // Update all fields
    for (int i = 0; i < manager->field_count; i++) {
        field_manager_update_crops(manager->fields[i],
                                   (const CropType**)manager->crop_types,
                                   manager->crop_type_count,
                                   game_state);
    }
}

int agriculture_manager_harvest_crop(AgricultureManager* manager, int field_location_id, int crop_id) {
    if (!manager) return 0;

    FieldManager* field = agriculture_manager_get_field(manager, field_location_id);
    if (!field) return 0;

    Crop* crop = field_manager_get_crop(field, crop_id);
    if (!crop || !crop_is_ready_to_harvest(crop)) {
        return 0;
    }

    int yield = crop->predicted_yield;
    field->total_harvested++;

    // Remove crop
    field_manager_remove_crop(field, crop_id);

    return yield;
}

int agriculture_manager_get_total_crop_count(const AgricultureManager* manager) {
    if (!manager) return 0;

    int total = 0;
    for (int i = 0; i < manager->field_count; i++) {
        total += manager->fields[i]->crop_count;
    }

    return total;
}

// ============================================================================
// Time Progression Implementation
// ============================================================================

void time_advance_period(GameState* game_state, AgricultureManager* ag_manager) {
    if (!game_state) return;

    TimeOfDay old_time = game_state->time_of_day;

    switch (game_state->time_of_day) {
        case TIME_MORNING:
            game_state->time_of_day = TIME_AFTERNOON;
            break;

        case TIME_AFTERNOON:
            game_state->time_of_day = TIME_EVENING;
            break;

        case TIME_EVENING:
            game_state->time_of_day = TIME_NIGHT;
            break;

        case TIME_NIGHT:
            // Advance to next day
            time_advance_day(game_state, ag_manager);
            break;
    }

    (void)old_time;  // Suppress unused warning
}

void time_advance_day(GameState* game_state, AgricultureManager* ag_manager) {
    if (!game_state) return;

    game_state->day_count++;
    game_state->time_of_day = TIME_MORNING;

    // Check for season change (every 30 days)
    if (game_state->day_count % 30 == 0) {
        time_advance_season(game_state);
    }

    // Update all crops
    if (ag_manager) {
        agriculture_manager_update_all(ag_manager, game_state);
    }

    // TODO: Random weather changes could happen here
}

void time_advance_season(GameState* game_state) {
    if (!game_state) return;

    switch (game_state->season) {
        case SEASON_SPRING:
            game_state->season = SEASON_SUMMER;
            break;

        case SEASON_SUMMER:
            game_state->season = SEASON_FALL;
            break;

        case SEASON_FALL:
            game_state->season = SEASON_WINTER;
            break;

        case SEASON_WINTER:
            game_state->season = SEASON_SPRING;
            game_state->year++;
            break;
    }
}

const char* time_get_current_string(const GameState* game_state) {
    if (!game_state) return "Unknown";

    switch (game_state->time_of_day) {
        case TIME_MORNING: return "Morning";
        case TIME_AFTERNOON: return "Afternoon";
        case TIME_EVENING: return "Evening";
        case TIME_NIGHT: return "Night";
        default: return "Unknown";
    }
}

bool time_is_good_for_planting(const GameState* game_state, const CropType* crop_type) {
    if (!game_state || !crop_type) return false;

    // Can always plant if crop grows in any season
    if (crop_type->can_grow_any_season) return true;

    // Check if current season matches preferred season
    return game_state->season == crop_type->preferred_season;
}

// ============================================================================
// Common Crop Types
// ============================================================================

CropType* create_wheat_crop_type(void) {
    CropType* wheat = crop_type_create("Wheat", 8, SEASON_SPRING);
    if (wheat) {
        wheat->water_requirement = 40;
        wheat->base_yield = 6;
        wheat->min_yield = 3;
        wheat->max_yield = 10;
        wheat->sell_price = 12;
        wheat->seed_cost = 5;
    }
    return wheat;
}

CropType* create_corn_crop_type(void) {
    CropType* corn = crop_type_create("Corn", 10, SEASON_SUMMER);
    if (corn) {
        corn->water_requirement = 60;
        corn->base_yield = 8;
        corn->min_yield = 4;
        corn->max_yield = 15;
        corn->sell_price = 15;
        corn->seed_cost = 8;
    }
    return corn;
}

CropType* create_tomato_crop_type(void) {
    CropType* tomato = crop_type_create("Tomato", 7, SEASON_SUMMER);
    if (tomato) {
        tomato->water_requirement = 70;
        tomato->base_yield = 10;
        tomato->min_yield = 5;
        tomato->max_yield = 20;
        tomato->sell_price = 8;
        tomato->seed_cost = 6;
    }
    return tomato;
}

CropType* create_potato_crop_type(void) {
    CropType* potato = crop_type_create("Potato", 9, SEASON_FALL);
    if (potato) {
        potato->water_requirement = 50;
        potato->base_yield = 12;
        potato->min_yield = 6;
        potato->max_yield = 20;
        potato->sell_price = 6;
        potato->seed_cost = 4;
        potato->can_grow_any_season = true;  // Hardy crop
    }
    return potato;
}

CropType* create_carrot_crop_type(void) {
    CropType* carrot = crop_type_create("Carrot", 6, SEASON_SPRING);
    if (carrot) {
        carrot->water_requirement = 45;
        carrot->base_yield = 8;
        carrot->min_yield = 4;
        carrot->max_yield = 12;
        carrot->sell_price = 7;
        carrot->seed_cost = 3;
    }
    return carrot;
}

void load_default_crop_types(AgricultureManager* manager) {
    if (!manager) return;

    agriculture_manager_register_crop_type(manager, create_wheat_crop_type());
    agriculture_manager_register_crop_type(manager, create_corn_crop_type());
    agriculture_manager_register_crop_type(manager, create_tomato_crop_type());
    agriculture_manager_register_crop_type(manager, create_potato_crop_type());
    agriculture_manager_register_crop_type(manager, create_carrot_crop_type());
}
