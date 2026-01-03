#ifndef KERNEL_ESTRUCTURAS_H
#define KERNEL_ESTRUCTURAS_H

#include "utils/utils.h"
#include "global.h"
#include "utils/conexion.h"
#include "planificacion.h"
#include "mutex.h"


void crear_proceso(char *archivo_pseudocodigo, int tamanio_proceso, int prioridad);
void crear_hilo(char *archivo_pseudocodigo, int prioridad);
t_PCB *crear_pcb(char *pseudocodigo, int tam, int prioridad);
void agregar_nuevo_tcb(t_PCB *pcb, char *pseudocodigo, int prioridad);
t_TCB *crear_tcb(t_PCB *pcb, char *pseudocodigo, int prioridad);
void revisar_cola_procesos_new();
bool intentar_crear_pcb_memoria(t_PCB *pcb, int tamanio_proceso);
bool crear_tcb_memoria(t_PCB *pcb, t_TCB *tcb);
void agregar_nuevo_tcb(t_PCB*, char*, int);
void finalizar_hilo(int, int);
void eliminar_hilos_de_un_proceso(int);
void finalizar_proceso();
t_TCB* encontrar_hilo_asociado(int PID);

t_TCB* buscar_y_eliminar_tcb_en_ready(int, int);
t_TCB* buscar_y_eliminar_tcb_en_block(int, int);
t_TCB* buscar_tcb_en_block(int, int);
//t_TCB* buscar_y_eliminar_tcb_en_new(int);
void desbloquear_hilos_bloqueados_por_otro(t_TCB*);
void eliminar_hilo_de_un_proceso_a_finalizar(int, int);
int cant_hilos_ready_en_su_cola();
void liberar_tcb(t_TCB*);
void liberar_pcb(t_PCB*);

#endif