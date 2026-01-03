#include "config_log.h"

valores_config *configuracion;
valores_config_prueba *configuracion_prueba;

t_log *logger;

void iniciar_config(){
    t_config* config;
    config = config_create("cfg/memoria.config");
    if (config == NULL){
		abort();
    }
    configuracion = malloc(sizeof(valores_config));

    t_config* config_prueba;

    char *path = string_from_format("cfg/%s.config", archivo_configuracion);
    config_prueba = config_create(path);
    free(path);

    if (config_prueba == NULL){
		abort();
    }

    configuracion->IP_FILESYSTEM = string_duplicate(config_get_string_value(config, "IP_FILESYSTEM"));
    configuracion->PATH_INSTRUCCIONES = string_duplicate(config_get_string_value(config, "PATH_INSTRUCCIONES"));
    configuracion->ESQUEMA = string_duplicate(config_get_string_value(config_prueba, "ESQUEMA"));
    configuracion->ALGORITMO_BUSQUEDA = string_duplicate(config_get_string_value(config_prueba, "ALGORITMO_BUSQUEDA"));
    configuracion->TAM_MEMORIA = config_get_int_value(config_prueba, "TAM_MEMORIA");
    configuracion->PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");
    configuracion->PUERTO_FILESYSTEM = config_get_int_value(config, "PUERTO_FILESYSTEM");
    configuracion->RETARDO_RESPUESTA = config_get_int_value(config_prueba, "RETARDO_RESPUESTA");

    configuracion->PARTICIONES = list_create(); // lista de strings, luego hay que pasarlo a int
    char** lista_config = config_get_array_value(config_prueba, "PARTICIONES");
    int i = 0;
    while (lista_config[i] != NULL) {
        int *particion = malloc(sizeof(*particion));
        *particion = atoi(lista_config[i]);
        list_add(configuracion->PARTICIONES, particion);
        i++;
    }
    string_array_destroy(lista_config);

    configuracion->LOG_LEVEL = log_level_from_string(config_get_string_value(config, "LOG_LEVEL"));
    config_destroy(config);
    config_destroy(config_prueba);
}
void iniciar_logger(){
    logger = log_create("memoria.log","Memoria",false, configuracion -> LOG_LEVEL);
    if (logger == NULL){
        list_destroy_and_destroy_elements(configuracion->PARTICIONES, free);
        free(configuracion);
        abort();
    }
}