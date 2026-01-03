#include "config_log.h"

void iniciar_config(){
    t_config* config;
    config = config_create("cfg/kernel.config"); //EL path deberia ser generico. Por ahora lo dejamos asi ///home/utnso/Desktop/tp-2024-2c-Los-Avengers/kernel/cfg/kernel.config"
    if (config == NULL){
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

    configuracion -> IP_MEMORIA = string_duplicate(config_get_string_value(config, "IP_MEMORIA"));// xq se usa string duplicate? valgrind me tira q tenemos un quilombo con eso
    configuracion -> PUERTO_MEMORIA = config_get_int_value (config, "PUERTO_MEMORIA");
    configuracion -> IP_CPU = string_duplicate(config_get_string_value(config, "IP_CPU"));//aca pasa lo mismo
    configuracion -> PUERTO_CPU_DISPATCH = config_get_int_value(config, "PUERTO_CPU_DISPATCH");
    configuracion -> PUERTO_CPU_INTERRUPT = config_get_int_value(config, "PUERTO_CPU_INTERRUPT");
    configuracion -> ALGORITMO_PLANIFICACION = string_duplicate(config_get_string_value(config_prueba, "ALGORITMO_PLANIFICACION"));//aca xq no usamos string duplicate?
    configuracion -> QUANTUM = config_get_int_value(config_prueba, "QUANTUM");
    //configuracion -> LOG_LEVEL = config_get_int_value(config, "LOG_LEVEL");
    configuracion -> LOG_LEVEL = log_level_from_string(config_get_string_value(config, "LOG_LEVEL"));//y aca xq no lo usamos?

    config_destroy(config);
    config_destroy(config_prueba);

}



void iniciar_logger(){
	logger = log_create("kernel.log", "LOGGER_KERNEL", false,configuracion -> LOG_LEVEL);
	 if (logger == NULL){
        free(configuracion);
        abort();
    }
}

void loguear_valores_config(){  // Loggeo de valores cargados en la config
    log_info(logger, "Configuración cargada:");
    log_info(logger, "IP_MEMORIA: %s", configuracion->IP_MEMORIA);
    log_info(logger, "PUERTO_MEMORIA: %i", configuracion->PUERTO_MEMORIA);
    log_info(logger, "IP_CPU: %s", configuracion->IP_CPU);
    log_info(logger, "PUERTO_CPU_DISPATCH: %i", configuracion->PUERTO_CPU_DISPATCH);
    log_info(logger, "PUERTO_CPU_INTERRUPT: %i", configuracion->PUERTO_CPU_INTERRUPT);
    log_info(logger, "ALGORITMO_PLANIFICACION: %s", configuracion->ALGORITMO_PLANIFICACION);
    log_info(logger, "QUANTUM: %i", configuracion->QUANTUM);
    log_info(logger, "LOG_LEVEL: %s", log_level_as_string(configuracion->LOG_LEVEL));
}
