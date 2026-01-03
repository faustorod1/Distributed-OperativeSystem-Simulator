#ifndef CPU_MAIN_H_
#define CPU_MAIN_H_

#include "global.h"
#include "execute.h"

void terminar_programa();
void iniciar_conexion();
void pedir_contexto();
void crear_hilo();
void recibir_interrupciones();
char* fetch(uint32_t PC);


#endif