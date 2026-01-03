#include "global.h"

int socket_mem;
int socket_kernel_dis;
int socket_kernel_int;

int base;
int limite;
int TID;
int PID;
uint32_t registros[9];

bool ejecutando_proceso = false;
bool interrupt_flag = false;
bool recuperar_contexto_urgente = false;

void actualizar_contexto(){
    t_paquete* paquete_contexto = crear_paquete(OP_MEMCONTEXT_UPDATE);
    agregar_a_paquete(paquete_contexto, &PID, sizeof(PID));
    agregar_a_paquete(paquete_contexto, &TID, sizeof(TID));
//    agregar_a_paquete(paquete_contexto, &base, sizeof(base));
//    agregar_a_paquete(paquete_contexto, &limite, sizeof(limite));
    empaquetar_registros(paquete_contexto, registros);
    enviar_paquete(paquete_contexto, socket_mem);
    eliminar_paquete(paquete_contexto);

    __attribute_maybe_unused__ enum op_code op = recibir_operacion(socket_mem);
    __attribute_maybe_unused__ int rta = recibir_int(socket_mem);

    // ya no ejecuto ningun proceso
    ejecutando_proceso = false;
    log_info(logger,"## TID: %i - Actualizo Contexto Ejecución",TID);
}

void motivo(enum op_code op){
    enviar_int(TID, socket_kernel_dis, op);
    // t_paquete* paquete_motivo = crear_paquete(op);
    // agregar_a_paquete(paquete_motivo, &TID, sizeof(TID));
    // enviar_paquete(paquete_motivo, socket_kernel_dis);
    // eliminar_paquete(paquete_motivo);
}