/**
 * game_state.h
 * Game State System for Transparent Game Engine
 *
 * Central state object holding all game data with save/load capability.
 */

#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "entity.h"
#include <stdbool.h>

// Time of Day
typedef enum {
    TIME_MORNING,
    TIME_AFTERNOON,
    TIME_EVENING,
    TIME_NIGHT
} TimeOfDay;

// Season
typedef enum {
    SEASON_SPRING,
    SEASON_SUMMER,
    SEASON_FALL,
    SEASON_WINTER
} Season;

// Weather
typedef enum {
    WEATHER_SUNNY,
    WEATHER_RAINY,
    WEATHER_CLOUDY,
    WEATHER_STORMY,
    WEATHER_DROUGHT
} Weather;

// Game State structure
typedef struct {
    EntityManager* entity_manager;

    // Time management
    int day_count;
    TimeOfDay time_of_day;
    Season season;
    int year;

    // Weather
    Weather current_weather;

    // Player reference
    int player_id;

    // Game metadata
    char game_name[64];
    char created_at[32];
    char last_saved[32];
} GameState;

// ============================================================================
// GameState Functions
// ============================================================================

GameState* game_state_create(void);
void game_state_destroy(GameState* state);

void game_state_advance_time(GameState* state);
const char* game_state_get_time_description(const GameState* state);

void game_state_set_player(GameState* state, int entity_id);
Entity* game_state_get_player(const GameState* state);

bool game_state_save_to_file(const GameState* state, const char* filepath);
GameState* game_state_load_from_file(const char* filepath);

cJSON* game_state_to_json(const GameState* state);
GameState* game_state_from_json(cJSON* json);

// Helper functions
const char* time_of_day_to_string(TimeOfDay time);
const char* season_to_string(Season season);
const char* weather_to_string(Weather weather);

TimeOfDay time_of_day_get_next(TimeOfDay current);
Season season_get_next(Season current);

#endif // GAME_STATE_H
