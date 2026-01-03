#ifndef CPU_CONFIG_LOG_H
#define CPU_CONFIG_LOG_H

#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h> 

typedef struct {
		char* IP_MEMORIA;
		int PUERTO_MEMORIA;
        int PUERTO_ESCUCHA_DISPATCH;
        int PUERTO_ESCUCHA_INTERRUPT;
        t_log_level LOG_LEVEL;
} valores_config;

extern valores_config *configuracion;
extern t_log* logger;

void iniciar_config();
void iniciar_logger();
void loguear_valores_config();
void terminar_programa();

#endif