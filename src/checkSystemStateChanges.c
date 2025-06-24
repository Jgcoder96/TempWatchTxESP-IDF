#include "config.h"
#include "systemStatus.h"
#include <string.h>

// Función para convertir el estado del sistema a string
static const char* systemStateToString(SystemState state) {
    switch(state) {
        case STATE_NORMAL:      return "normal";
        case STATE_PREVENTIVE:  return "preventive";
        case STATE_EMERGENCY:   return "emergency";
        default:                return "unknown";
    }
}

// Función para crear un objeto JSON de un sistema (versión extendida con estado)
static cJSON* systemToJsonWithStatus(const SystemStruct* system) {
    cJSON* json_system = cJSON_CreateObject();
    cJSON_AddStringToObject(json_system, "id_sensors", system->name);
    cJSON_AddNumberToObject(json_system, "voltage", system->voltage);
    cJSON_AddNumberToObject(json_system, "temperature", system->temperature);
    cJSON_AddStringToObject(json_system, "status", systemStateToString(system->current_state));
    return json_system;
}

// Función principal que genera JSON solo con sistemas que cambiaron de estado
char* changedSystemsToJsonString(const SystemCollectionStruct* systems) {
    cJSON* root = cJSON_CreateObject();
    cJSON* data_array = cJSON_CreateArray();
    
    // Array de punteros a los sistemas para iterar fácilmente
    const SystemStruct* systemArray[] = {
        &systems->system1,
        &systems->system2,
        &systems->system3,
        &systems->system4
    };
    
    // Verificar cada sistema
    for (int i = 0; i < 4; i++) {
        if (systemArray[i]->previous_state != systemArray[i]->current_state) {
            // Solo agregar al JSON si el estado cambió
            cJSON_AddItemToArray(data_array, systemToJsonWithStatus(systemArray[i]));
        }
    }
    
    // Si no hubo cambios, devolver NULL o un JSON vacío según prefieras
    if (cJSON_GetArraySize(data_array) == 0) {
        cJSON_Delete(root);
        return NULL;
    }
    
    cJSON_AddItemToObject(root, "data", data_array);
    
    char* json_string = cJSON_Print(root);
    cJSON_Delete(root);
    return json_string;
}