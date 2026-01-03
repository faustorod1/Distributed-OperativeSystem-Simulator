//solamente poner includes

#ifndef MEMORIA_MAIN_H
#define MEMORIA_MAIN_H

#include"config_log.h"
#include "utils/conexion.h"
#include "utils/utils.h"
#include <pthread.h>
#include "estructuras.h"
#include "op_memoria.h"
#include "global.h"

void iniciar_conexion();
void terminar_programa();
void gestionar_peticiones_cpu(void *dir_socket);
void gestionar_peticion_kernel(void *dir_socket);

#endif