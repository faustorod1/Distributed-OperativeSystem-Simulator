#ifndef MEM_CONFIG_LOG_H
#define MEM_CONFIG_LOG_H

#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "global.h"
#include <string.h>

typedef struct {
        char* IP_FILESYSTEM;
        char* PATH_INSTRUCCIONES;
        char* ESQUEMA;
        char* ALGORITMO_BUSQUEDA;
		int TAM_MEMORIA;
        int PUERTO_ESCUCHA;
        int PUERTO_FILESYSTEM;
        int RETARDO_RESPUESTA;

        t_list* PARTICIONES;
        
        t_log_level LOG_LEVEL;
} valores_config;

typedef struct {
        char* ESQUEMA;
        char* ALGORITMO_BUSQUEDA;
		int TAM_MEMORIA;
        int RETARDO_RESPUESTA;
        t_list* PARTICIONES;
} valores_config_prueba;

extern valores_config *configuracion;
extern t_log *logger;

void iniciar_config();
void iniciar_logger();


#endif