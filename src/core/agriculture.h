/**
 * agriculture.h
 * Agriculture and Farming System
 *
 * Manages crops, planting, growth, and harvesting.
 */

#ifndef AGRICULTURE_H
#define AGRICULTURE_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "game_state.h"
#include "entity.h"
#include "world.h"

// Forward declarations
typedef struct cJSON cJSON;

#define MAX_CROP_NAME 32
#define MAX_CROP_TYPES 20
#define MAX_CROPS_PER_FIELD 100
#define MAX_GROWTH_STAGES 5

// ============================================================================
// Crop Growth Stages
// ============================================================================

typedef enum {
    CROP_STAGE_SEED,        // Just planted
    CROP_STAGE_SPROUT,      // Starting to grow
    CROP_STAGE_GROWING,     // Actively growing
    CROP_STAGE_MATURE,      // Ready to harvest
    CROP_STAGE_WITHERED,    // Dead/failed
} CropGrowthStage;

const char* crop_growth_stage_to_string(CropGrowthStage stage);

// ============================================================================
// Crop Type Definition
// ============================================================================

typedef struct {
    char name[MAX_CROP_NAME];

    // Growth parameters
    int days_to_mature;         // Days from seed to mature
    int days_sprout;            // Days to sprout stage
    int days_growing;           // Days in growing stage

    // Season requirements
    Season preferred_season;    // Best season to grow
    bool can_grow_any_season;   // Can grow in any season?

    // Weather requirements
    int water_requirement;      // 0-100 (how much water needed)
    bool needs_sun;             // Requires sunny weather

    // Yield
    int base_yield;             // Base harvest amount
    int min_yield;              // Minimum yield
    int max_yield;              // Maximum yield

    // Value
    int sell_price;             // Price per unit
    int seed_cost;              // Cost to buy seeds

} CropType;

// Create crop type
CropType* crop_type_create(const char* name, int days_to_mature, Season preferred_season);

// Destroy crop type
void crop_type_destroy(CropType* type);

// ============================================================================
// Crop Instance (planted crop)
// ============================================================================

typedef struct {
    int id;
    char crop_type_name[MAX_CROP_NAME];

    // Location
    int field_location_id;
    int plot_x, plot_y;         // Position within field

    // Growth state
    CropGrowthStage stage;
    int days_planted;           // How many days since planted
    int days_in_current_stage;  // Days in current stage

    // Health
    int health;                 // 0-100 (affects yield)
    int water_level;            // 0-100
    bool watered_today;

    // Ownership
    int planted_by_entity_id;   // Who planted this

    // Harvest prediction
    int predicted_yield;        // Expected harvest amount

} Crop;

// Create crop
Crop* crop_create(int id, const char* crop_type_name, int field_location_id, int plot_x, int plot_y, int planted_by);

// Destroy crop
void crop_destroy(Crop* crop);

// Update crop (call each day)
void crop_update(Crop* crop, const CropType* type, const GameState* game_state);

// Water crop
void crop_water(Crop* crop);

// Check if ready to harvest
bool crop_is_ready_to_harvest(const Crop* crop);

// Check if crop is dead
bool crop_is_withered(const Crop* crop);

// Get crop stage progress (0.0 to 1.0)
float crop_get_stage_progress(const Crop* crop, const CropType* type);

// Serialization
cJSON* crop_to_json(const Crop* crop);
Crop* crop_from_json(cJSON* json);

// ============================================================================
// Field Manager (manages crops in a field location)
// ============================================================================

typedef struct {
    int field_location_id;

    Crop* crops[MAX_CROPS_PER_FIELD];
    int crop_count;
    int next_crop_id;

    // Field properties
    int max_plots;              // How many crops can be planted
    int field_width;            // Grid width
    int field_height;           // Grid height

    // Field state
    int total_planted;
    int total_harvested;

} FieldManager;

// Create field manager
FieldManager* field_manager_create(int location_id, int width, int height);

// Destroy field manager
void field_manager_destroy(FieldManager* manager);

// Plant crop
int field_manager_plant_crop(FieldManager* manager, const char* crop_type_name,
                             int plot_x, int plot_y, int planted_by);

// Remove crop (after harvest or death)
bool field_manager_remove_crop(FieldManager* manager, int crop_id);

// Get crop at position
Crop* field_manager_get_crop_at(FieldManager* manager, int plot_x, int plot_y);

// Get crop by ID
Crop* field_manager_get_crop(FieldManager* manager, int crop_id);

// Update all crops in field
void field_manager_update_crops(FieldManager* manager, const CropType* types[], int type_count, const GameState* game_state);

// Water all crops in field
int field_manager_water_all(FieldManager* manager);

// Get ready crops
int field_manager_get_ready_crops(const FieldManager* manager, Crop** out_crops, int max_crops);

// Count crops by stage
int field_manager_count_by_stage(const FieldManager* manager, CropGrowthStage stage);

// Check if plot is occupied
bool field_manager_is_plot_occupied(const FieldManager* manager, int plot_x, int plot_y);

// ============================================================================
// Agriculture Manager (manages all fields and crop types)
// ============================================================================

typedef struct {
    // Crop type definitions
    CropType* crop_types[MAX_CROP_TYPES];
    int crop_type_count;

    // Field managers (one per field location)
    FieldManager* fields[MAX_LOCATIONS];
    int field_count;

} AgricultureManager;

// Create agriculture manager
AgricultureManager* agriculture_manager_create(void);

// Destroy agriculture manager
void agriculture_manager_destroy(AgricultureManager* manager);

// Register crop type
bool agriculture_manager_register_crop_type(AgricultureManager* manager, CropType* type);

// Get crop type
CropType* agriculture_manager_get_crop_type(AgricultureManager* manager, const char* name);

// Register field
bool agriculture_manager_register_field(AgricultureManager* manager, int location_id, int width, int height);

// Get field manager
FieldManager* agriculture_manager_get_field(AgricultureManager* manager, int location_id);

// Plant crop in field
int agriculture_manager_plant_crop(AgricultureManager* manager, int field_location_id,
                                   const char* crop_type_name, int plot_x, int plot_y, int planted_by);

// Update all crops (call each day)
void agriculture_manager_update_all(AgricultureManager* manager, const GameState* game_state);

// Harvest crop
int agriculture_manager_harvest_crop(AgricultureManager* manager, int field_location_id, int crop_id);

// Get total crops across all fields
int agriculture_manager_get_total_crop_count(const AgricultureManager* manager);

// ============================================================================
// Time Progression System
// ============================================================================

// Advance time by one time period (morning -> afternoon -> evening -> night -> next day)
void time_advance_period(GameState* game_state, AgricultureManager* ag_manager);

// Advance to next day
void time_advance_day(GameState* game_state, AgricultureManager* ag_manager);

// Advance to next season
void time_advance_season(GameState* game_state);

// Get current time as string
const char* time_get_current_string(const GameState* game_state);

// Check if it's a good time to plant (season)
bool time_is_good_for_planting(const GameState* game_state, const CropType* crop_type);

// ============================================================================
// Common Crop Types
// ============================================================================

// Create standard crop types
CropType* create_wheat_crop_type(void);
CropType* create_corn_crop_type(void);
CropType* create_tomato_crop_type(void);
CropType* create_potato_crop_type(void);
CropType* create_carrot_crop_type(void);

// Load default crop types into agriculture manager
void load_default_crop_types(AgricultureManager* manager);

#endif // AGRICULTURE_H
