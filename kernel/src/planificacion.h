#ifndef KERNEL_PLANIFICACION_H
#define KERNEL_PLANIFICACION_H

#include <string.h>
#include "utils/utils.h"
#include "global.h"
#include "config_log.h"
#include <commons/collections/queue.h>
#include "utils/paquete.h"
#include <pthread.h>
#include "estructuras.h"

typedef struct {
    int tiempo;
    t_TCB* hilo_bloqueado;
} hilo_io_args_t;

extern hilo_io_args_t *hilo_IO;

void elegir_hilo();
void enviar_a_ejecucion(t_TCB *tcb);
void desalojar_hilo_fin_Q();
void cambiar_hilo_exec_fin_Q(int);
void bloquear_hilo_por_io(int);
void revisar_cola_IO();
void revisar_fin_io(void*);
void desbloquear_hilo(int);
void agregar_hilo_a_ready(t_TCB*);
void agregar_a_cola_ready_correspondiente(t_TCB*);
void bloquear_hilo_por_thread_join(int);
void iniciar_conteo_q();


#endif