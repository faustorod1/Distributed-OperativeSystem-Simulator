#include "config_log.h"

valores_config *configuracion;
t_log* logger;

void iniciar_config(){
    t_config* config;

	config = config_create("cfg/cpu.config");
    if (config == NULL){
	    abort();
    }

    configuracion = malloc(sizeof(valores_config));
    configuracion->IP_MEMORIA = string_duplicate(config_get_string_value(config, "IP_MEMORIA"));
    configuracion->PUERTO_MEMORIA = config_get_int_value(config, "PUERTO_MEMORIA");
    configuracion->PUERTO_ESCUCHA_DISPATCH = config_get_int_value(config, "PUERTO_ESCUCHA_DISPATCH");
    configuracion->PUERTO_ESCUCHA_INTERRUPT = config_get_int_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    configuracion->LOG_LEVEL = log_level_from_string(config_get_string_value(config, "LOG_LEVEL")); 

    config_destroy(config);

}

void iniciar_logger(){
    logger = log_create("cpu.log", "Cpu", false, configuracion -> LOG_LEVEL);
    if (logger == NULL){
        free(configuracion);
		abort();
    }
}

void loguear_valores_config(){
    log_info(logger,"IP_MEMORIA: %s", configuracion->IP_MEMORIA);
    log_info(logger,"PUERTO_MEMORIA: %i", configuracion->PUERTO_MEMORIA);
    log_info(logger,"PUERTO_ESCUCHA_DISPATCH: %i", configuracion->PUERTO_ESCUCHA_DISPATCH);
    log_info(logger,"PUERTO_ESCUCHA_INTERRUPT: %i", configuracion->PUERTO_ESCUCHA_INTERRUPT);
    log_info(logger,"LOG_LEVEL: %s", log_level_as_string(configuracion->LOG_LEVEL));
}