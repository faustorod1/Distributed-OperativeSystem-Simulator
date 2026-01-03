#ifndef GLOBAL_H_
#define GLOBAL_H_

#include "config_log.h"
#include "utils/conexion.h"
#include "utils/utils.h"
#include <commons/collections/list.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>


extern int socket_mem;
extern int socket_kernel_dis;
extern int socket_kernel_int;
 
extern int base;
extern int limite;
extern int TID;
extern int PID;
extern uint32_t registros[9];
 
extern bool ejecutando_proceso;
extern bool interrupt_flag;
extern bool recuperar_contexto_urgente;

void actualizar_contexto();
void motivo(enum op_code);

#endif 