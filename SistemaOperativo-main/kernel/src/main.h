//solamente poner includes

#ifndef KERNEL_MAIN_H_
#define KERNEL_MAIN_H_


#include "config_log.h"
#include "global.h"
#include "estructuras.h"
#include "utils/conexion.h"
#include "utils/utils.h"
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <pthread.h>
#include <commons/temporal.h>


typedef struct {
    int tamanio_proceso;
    char* archivo_pseudocodigo;
} proceso_inicial_t;

argumentos_volcado_t *hilo_DUMP_MEMORY = NULL;

void terminar_programa();
void iniciar_conexion();
void gestionar_peticiones_cpu_dispatch();


void volcado_memoria();
void revisar_cola_dump();
void revisar_rta_volcado(void *arg);// ¿Esta bien declarado el tipo de dato? me pa q si. Igual creo q no es necesario el "arg"



#endif