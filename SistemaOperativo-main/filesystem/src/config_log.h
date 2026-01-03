
#ifndef FILESYS_CONFIG_LOG_H
#define FILESYS_CONFIG_LOG_H

#include <stdint.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <string.h>
#include "global.h"

typedef struct {
		int PUERTO_ESCUCHA;
        char *MOUNT_DIR;
        int BLOCK_SIZE;
        int BLOCK_COUNT;
        int RETARDO_ACCESO_BLOQUE;
        
        t_log_level LOG_LEVEL;
} valores_config;

typedef struct {
        int BLOCK_SIZE;
        int BLOCK_COUNT;
        int RETARDO_ACCESO_BLOQUE;
} valores_config_prueba;

extern valores_config *configuracion;
extern t_log *logger;

void iniciar_config();
void iniciar_logger();
void loguear_valores_config();

#endif