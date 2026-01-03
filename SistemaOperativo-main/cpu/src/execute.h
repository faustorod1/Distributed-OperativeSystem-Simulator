#ifndef CPU_EXECUTE_H_
#define CPU_EXECUTE_H_

#include "global.h"

void execute(char** lista_instruccion);
int decodificar_posicion(char *registro);
uint32_t string_a_uint32(const char *str);
void fRead(int direccion_fisica,int posicion_registro);
void fWrite(int direccion_fisica,int valor);
void dump_memory();
void io(int tiempo);
void process_create(char* archivo,uint32_t tamanio,uint32_t prioridad);
void thread_create(char* archivo, uint32_t prioridad);
void thread_join(uint32_t i_tid);
void thread_cancel(uint32_t i_tid);
void mutex_create(char* recurso);
void mutex_lock(char* recurso);
void mutex_unlock(char* recurso);
void thread_exit();
void process_exit();
int MMU(int desplazamiento);
void revisar_bloqueante();


#endif