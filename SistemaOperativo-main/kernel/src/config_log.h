//solamente poner includes

#ifndef KERNEL_CONFIG_LOG_H
#define KERNEL_CONFIG_LOG_H

#include <stdlib.h>
#include <stdint.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h> 
#include "global.h"

typedef struct{
    char* IP_MEMORIA;
    int PUERTO_MEMORIA;
    char* IP_CPU;
    int PUERTO_CPU_DISPATCH;
    int PUERTO_CPU_INTERRUPT;
    char* ALGORITMO_PLANIFICACION;    // REVISAR TIPO DE DATO
    int QUANTUM;
    t_log_level LOG_LEVEL;
} valores_config;

typedef struct{
    char* ALGORITMO_PLANIFICACION;
    int QUANTUM;
} valores_config_prueba;

extern valores_config *configuracion;
extern t_log *logger;

//MENCION DE FUNCIONES
void iniciar_config();
void iniciar_logger();
void loguear_valores_config();

#endif