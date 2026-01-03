#include "config_log.h"

valores_config *configuracion;
t_log *logger;


void iniciar_config(){
    t_config* config;
    config = config_create("cfg/filesystem.config");
    if(config == NULL){
        abort();
    }

    t_config* config_prueba;
    size_t path_len = strlen("cfg/") + strlen(archivo_configuracion) + strlen(".config") + 1;
    char* path = malloc(path_len);
    if (path == NULL) {
        abort();
    }
    strcpy(path, "cfg/");
    strcat(path, archivo_configuracion);
    strcat(path, ".config");
    config_prueba = config_create(path);
    free(path);

    if (config_prueba == NULL){
		abort();
    }

    configuracion = malloc(sizeof(valores_config));

    configuracion->PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");
    configuracion->MOUNT_DIR = string_duplicate(config_get_string_value(config, "MOUNT_DIR"));
    configuracion->BLOCK_SIZE = config_get_int_value(config_prueba, "BLOCK_SIZE");
    configuracion->BLOCK_COUNT = config_get_int_value(config_prueba, "BLOCK_COUNT");
    configuracion->RETARDO_ACCESO_BLOQUE = config_get_int_value(config_prueba, "RETARDO_ACCESO_BLOQUE");

    configuracion->LOG_LEVEL = log_level_from_string(config_get_string_value(config,"LOG_LEVEL"));

    config_destroy(config);
    config_destroy(config_prueba);
}


void iniciar_logger(){
    logger = log_create("filesystem.log","FileSystem",false,configuracion -> LOG_LEVEL);
    if(logger == NULL){
        free(configuracion);
        abort();
    }
}


void loguear_valores_config(){
    log_info(logger,"PUERTO_ESCUCHA : %i", configuracion->PUERTO_ESCUCHA);
    log_info(logger,"MOUNT_DIR : %s", configuracion->MOUNT_DIR);
    log_info(logger,"BLOCK_SIZE : %i", configuracion->BLOCK_SIZE);
    log_info(logger,"BLOCK_COUNT : %i", configuracion->BLOCK_COUNT);
    log_info(logger,"RETARDO_ACCESO_BLOQUE : %i", configuracion->RETARDO_ACCESO_BLOQUE);
    log_info(logger, "LOG_LEVEL: %s", log_level_as_string(configuracion->LOG_LEVEL));
}
