/**
 * game_state.c
 * Game State System Implementation
 */

#include "game_state.h"
#include "../../lib/cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// ============================================================================
// Helper Functions
// ============================================================================

const char* time_of_day_to_string(TimeOfDay time) {
    switch (time) {
        case TIME_MORNING: return "morning";
        case TIME_AFTERNOON: return "afternoon";
        case TIME_EVENING: return "evening";
        case TIME_NIGHT: return "night";
        default: return "unknown";
    }
}

const char* season_to_string(Season season) {
    switch (season) {
        case SEASON_SPRING: return "spring";
        case SEASON_SUMMER: return "summer";
        case SEASON_FALL: return "fall";
        case SEASON_WINTER: return "winter";
        default: return "unknown";
    }
}

const char* weather_to_string(Weather weather) {
    switch (weather) {
        case WEATHER_SUNNY: return "sunny";
        case WEATHER_RAINY: return "rainy";
        case WEATHER_CLOUDY: return "cloudy";
        case WEATHER_STORMY: return "stormy";
        case WEATHER_DROUGHT: return "drought";
        default: return "unknown";
    }
}

TimeOfDay time_of_day_get_next(TimeOfDay current) {
    return (TimeOfDay)((current + 1) % 4);
}

Season season_get_next(Season current) {
    return (Season)((current + 1) % 4);
}

static TimeOfDay string_to_time_of_day(const char* str) {
    if (strcmp(str, "morning") == 0) return TIME_MORNING;
    if (strcmp(str, "afternoon") == 0) return TIME_AFTERNOON;
    if (strcmp(str, "evening") == 0) return TIME_EVENING;
    if (strcmp(str, "night") == 0) return TIME_NIGHT;
    return TIME_MORNING;
}

static Season string_to_season(const char* str) {
    if (strcmp(str, "spring") == 0) return SEASON_SPRING;
    if (strcmp(str, "summer") == 0) return SEASON_SUMMER;
    if (strcmp(str, "fall") == 0) return SEASON_FALL;
    if (strcmp(str, "winter") == 0) return SEASON_WINTER;
    return SEASON_SPRING;
}

static Weather string_to_weather(const char* str) {
    if (strcmp(str, "sunny") == 0) return WEATHER_SUNNY;
    if (strcmp(str, "rainy") == 0) return WEATHER_RAINY;
    if (strcmp(str, "cloudy") == 0) return WEATHER_CLOUDY;
    if (strcmp(str, "stormy") == 0) return WEATHER_STORMY;
    if (strcmp(str, "drought") == 0) return WEATHER_DROUGHT;
    return WEATHER_SUNNY;
}

static void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

// ============================================================================
// GameState Functions
// ============================================================================

GameState* game_state_create(void) {
    GameState* state = (GameState*)malloc(sizeof(GameState));
    if (!state) return NULL;

    // Create entity manager
    state->entity_manager = entity_manager_create();
    if (!state->entity_manager) {
        free(state);
        return NULL;
    }

    // Initialize time
    state->day_count = 1;
    state->time_of_day = TIME_MORNING;
    state->season = SEASON_SPRING;
    state->year = 1;

    // Initialize weather
    state->current_weather = WEATHER_SUNNY;

    // Initialize player
    state->player_id = -1;

    // Initialize metadata
    strncpy(state->game_name, "Farming Village", sizeof(state->game_name) - 1);
    get_timestamp(state->created_at, sizeof(state->created_at));
    state->last_saved[0] = '\0';

    return state;
}

void game_state_destroy(GameState* state) {
    if (!state) return;

    if (state->entity_manager) {
        entity_manager_destroy(state->entity_manager);
    }

    free(state);
}

void game_state_advance_time(GameState* state) {
    if (!state) return;

    state->time_of_day = time_of_day_get_next(state->time_of_day);

    // If we wrapped to morning, advance day
    if (state->time_of_day == TIME_MORNING) {
        state->day_count++;

        // Every 28 days, advance season
        if (state->day_count % 28 == 0) {
            state->season = season_get_next(state->season);

            // If we wrapped to spring, advance year
            if (state->season == SEASON_SPRING) {
                state->year++;
            }
        }
    }
}

const char* game_state_get_time_description(const GameState* state) {
    static char buffer[256];

    if (!state) return "Unknown";

    int day_of_season = ((state->day_count - 1) % 28) + 1;

    snprintf(buffer, sizeof(buffer), "Year %d, %s, Day %d, %s",
        state->year,
        season_to_string(state->season),
        day_of_season,
        time_of_day_to_string(state->time_of_day)
    );

    return buffer;
}

void game_state_set_player(GameState* state, int entity_id) {
    if (!state) return;

    Entity* entity = entity_manager_get_entity(state->entity_manager, entity_id);
    if (entity) {
        state->player_id = entity_id;
    }
}

Entity* game_state_get_player(const GameState* state) {
    if (!state || state->player_id < 0) return NULL;

    return entity_manager_get_entity(state->entity_manager, state->player_id);
}

cJSON* game_state_to_json(const GameState* state) {
    if (!state) return NULL;

    cJSON* json = cJSON_CreateObject();

    // Metadata
    cJSON* metadata = cJSON_CreateObject();
    cJSON_AddStringToObject(metadata, "game_name", state->game_name);
    cJSON_AddStringToObject(metadata, "created_at", state->created_at);

    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    cJSON_AddStringToObject(metadata, "last_saved", timestamp);
    cJSON_AddStringToObject(metadata, "version", "1.0");
    cJSON_AddItemToObject(json, "metadata", metadata);

    // Time
    cJSON* time = cJSON_CreateObject();
    cJSON_AddNumberToObject(time, "day_count", state->day_count);
    cJSON_AddStringToObject(time, "time_of_day", time_of_day_to_string(state->time_of_day));
    cJSON_AddStringToObject(time, "season", season_to_string(state->season));
    cJSON_AddNumberToObject(time, "year", state->year);
    cJSON_AddItemToObject(json, "time", time);

    // Weather
    cJSON* weather = cJSON_CreateObject();
    cJSON_AddStringToObject(weather, "current_weather", weather_to_string(state->current_weather));
    cJSON_AddItemToObject(json, "weather", weather);

    // Player
    cJSON* player = cJSON_CreateObject();
    cJSON_AddNumberToObject(player, "player_id", state->player_id);
    cJSON_AddItemToObject(json, "player", player);

    // Entities
    cJSON* entities = entity_manager_to_json(state->entity_manager);
    if (entities) {
        cJSON_AddItemToObject(json, "entities", entities);
    }

    return json;
}

GameState* game_state_from_json(cJSON* json) {
    if (!json) return NULL;

    GameState* state = game_state_create();
    if (!state) return NULL;

    // Restore metadata
    cJSON* metadata = cJSON_GetObjectItem(json, "metadata");
    if (metadata) {
        cJSON* game_name = cJSON_GetObjectItem(metadata, "game_name");
        if (game_name && cJSON_IsString(game_name)) {
            strncpy(state->game_name, game_name->valuestring, sizeof(state->game_name) - 1);
        }

        cJSON* created_at = cJSON_GetObjectItem(metadata, "created_at");
        if (created_at && cJSON_IsString(created_at)) {
            strncpy(state->created_at, created_at->valuestring, sizeof(state->created_at) - 1);
        }

        cJSON* last_saved = cJSON_GetObjectItem(metadata, "last_saved");
        if (last_saved && cJSON_IsString(last_saved)) {
            strncpy(state->last_saved, last_saved->valuestring, sizeof(state->last_saved) - 1);
        }
    }

    // Restore time
    cJSON* time = cJSON_GetObjectItem(json, "time");
    if (time) {
        cJSON* day_count = cJSON_GetObjectItem(time, "day_count");
        if (day_count) state->day_count = day_count->valueint;

        cJSON* time_of_day = cJSON_GetObjectItem(time, "time_of_day");
        if (time_of_day && cJSON_IsString(time_of_day)) {
            state->time_of_day = string_to_time_of_day(time_of_day->valuestring);
        }

        cJSON* season = cJSON_GetObjectItem(time, "season");
        if (season && cJSON_IsString(season)) {
            state->season = string_to_season(season->valuestring);
        }

        cJSON* year = cJSON_GetObjectItem(time, "year");
        if (year) state->year = year->valueint;
    }

    // Restore weather
    cJSON* weather = cJSON_GetObjectItem(json, "weather");
    if (weather) {
        cJSON* current_weather = cJSON_GetObjectItem(weather, "current_weather");
        if (current_weather && cJSON_IsString(current_weather)) {
            state->current_weather = string_to_weather(current_weather->valuestring);
        }
    }

    // Restore player
    cJSON* player = cJSON_GetObjectItem(json, "player");
    if (player) {
        cJSON* player_id = cJSON_GetObjectItem(player, "player_id");
        if (player_id) state->player_id = player_id->valueint;
    }

    // Restore entities
    cJSON* entities = cJSON_GetObjectItem(json, "entities");
    if (entities) {
        entity_manager_destroy(state->entity_manager);
        state->entity_manager = entity_manager_from_json(entities);
    }

    return state;
}

bool game_state_save_to_file(const GameState* state, const char* filepath) {
    if (!state || !filepath) return false;

    cJSON* json = game_state_to_json(state);
    if (!json) return false;

    char* json_string = cJSON_Print(json);
    cJSON_Delete(json);

    if (!json_string) return false;

    FILE* file = fopen(filepath, "w");
    if (!file) {
        free(json_string);
        return false;
    }

    fprintf(file, "%s", json_string);
    fclose(file);
    free(json_string);

    printf("Game saved to %s\n", filepath);

    return true;
}

GameState* game_state_load_from_file(const char* filepath) {
    if (!filepath) return NULL;

    FILE* file = fopen(filepath, "r");
    if (!file) {
        printf("Failed to open file: %s\n", filepath);
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read file
    char* json_string = (char*)malloc(file_size + 1);
    if (!json_string) {
        fclose(file);
        return NULL;
    }

    fread(json_string, 1, file_size, file);
    json_string[file_size] = '\0';
    fclose(file);

    // Parse JSON
    cJSON* json = cJSON_Parse(json_string);
    free(json_string);

    if (!json) {
        printf("Failed to parse JSON from file: %s\n", filepath);
        return NULL;
    }

    GameState* state = game_state_from_json(json);
    cJSON_Delete(json);

    if (state) {
        printf("Game loaded from %s\n", filepath);
    }

    return state;
}
