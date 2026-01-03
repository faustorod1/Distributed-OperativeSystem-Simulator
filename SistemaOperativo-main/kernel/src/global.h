#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <commons/collections/queue.h>
#include <commons/temporal.h>
#include "utils/utils.h"
#include "utils/paquete.h"
#include "utils/conexion.h"
#include <commons/string.h>
#include <stdlib.h>
#include <stdio.h>
#include "config_log.h"
#include <pthread.h>
#include <commons/temporal.h>

enum tipo_bloqueante {
    NO_BLOQUEANTE,
    BLOQUEANTE
};

typedef struct {
    int TID_de_hilo_bloqueado;
    int socket;
    int PID_de_hilo_bloqueado;
} argumentos_volcado_t;

extern pthread_mutex_t mutex;

extern t_temporal *contador_global;

// extern int socket_mem;
extern int socket_cpu_dispatch;
extern int socket_cpu_interrupt;
extern int proximo_pid;
extern int ultimo_tamanio_proceso;
extern char* archivo_configuracion;
//extern t_PCB* proceso_EXEC;
extern t_TCB* hilo_EXEC;
extern t_temporal* clock_RR;
extern t_temporal* clock_IO;

extern t_queue* cola_procesos_NEW;
extern t_queue* cola_procesos_READY;
extern t_queue* cola_procesos_EXIT;

extern t_queue* cola_hilos_NEW;
extern t_queue* cola_hilos_READY;
extern t_list* colas_multi_level;
extern t_queue* cola_hilos_BLOCK;
extern t_queue* cola_hilos_EXIT;

extern t_queue *cola_hilos_BLOCK_IO;
extern t_queue *cola_hilos_BLOCK_DUMP_MEMORY;

extern t_list *lista_hilos;


void iniciar_contador_global();
void iniciar_colas();
int enviar_a_memoria(enum op_code, t_TCB *);
t_PCB*  buscar_y_eliminar_pcb_en_ready(int);
int enviar_pid_a_memoria(enum op_code, t_TCB*);
t_PCB *buscar_proceso_READY(int PID);
t_PCB *proceso_EXEC();
void enviar_bloqueante_y_esperar(int valor);

#endif