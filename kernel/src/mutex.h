#ifndef KERNEL_MUTEX_H
#define KERNEL_MUTEX_H

#include <commons/collections/queue.h>
#include "utils/utils.h"
#include <stdlib.h>
#include "global.h"
#include "planificacion.h"
#include <commons/log.h>
#include "config_log.h"


typedef struct {
    char* nombre;
    int valor;
    t_queue* hilos_bloqueados;
    t_TCB* hilo_propietario;//quien tiene el recurso asignado
}recurso;

void crear_mutex(char*);
void bloquear_mutex(char*);
void desbloquear_mutex(char*);
recurso* buscar_recurso(char*);
void liberar_recurso(recurso *recurso);

#endif