#ifndef UTILS_H
#define UTILS_H

#include <commons/collections/list.h>
#include <stdint.h>


enum registros{
    PC,
    AX,
    BX,
    CX,
    DX,
    EX,
    FX,
    GX,
    HX
};

typedef struct{
    uint32_t PC;
    uint32_t AX;
    uint32_t BX;
    uint32_t CX;
    uint32_t DX;
    uint32_t EX;
    uint32_t FX;
    uint32_t GX;
    uint32_t HX;
} t_registros;

typedef struct
{
	int PID;
	t_list* TIDS;
	t_list* mutexs;
    int prox_tid;
    int tamanio;
} t_PCB;

typedef struct
{                                                                                                                
	int TID;
    int PID;
	int prioridad;
    char *pseudocodigo;
    t_list* hilos_bloqueados;
} t_TCB;



#endif // UTILS_H
