#include "execute.h"

void execute(char** lista_instruccion){
    int posicion_registro = 0; 
    char* instruccion = lista_instruccion[0];

    if (strcmp(instruccion, "SET") == 0) {
        posicion_registro = decodificar_posicion(lista_instruccion[1]);
        int valor = string_a_uint32(lista_instruccion[2]);
        registros[posicion_registro] = valor;

    } else if (strcmp(instruccion, "READ_MEM") == 0) {
        int posicion_registro_datos = decodificar_posicion(lista_instruccion[1]);
        int direccion_logica = registros[decodificar_posicion(lista_instruccion[2])];

        int direccion_fisica = MMU(direccion_logica);
        if (direccion_fisica == -1) {
            return;
        }
        fRead(direccion_fisica, posicion_registro_datos);

    } else if (strcmp(instruccion, "WRITE_MEM") == 0) {
        int posicion_registro_datos = decodificar_posicion(lista_instruccion[2]);
        int direccion_logica = registros[decodificar_posicion(lista_instruccion[1])];

        int direccion_fisica = MMU(direccion_logica);
        if (direccion_fisica == -1) {
            return;
        }
        int valor = registros[posicion_registro_datos];
        fWrite(direccion_fisica, valor);

    } else if (strcmp(instruccion, "SUM") == 0) {
        int registro_destino = decodificar_posicion(lista_instruccion[1]);
        int registro_origen = decodificar_posicion(lista_instruccion[2]);
        registros[registro_destino] = registros[registro_destino] + registros[registro_origen];

    } else if (strcmp(instruccion, "SUB") == 0) {
        int registro_destino = decodificar_posicion(lista_instruccion[1]);
        int registro_origen = decodificar_posicion(lista_instruccion[2]);
        registros[registro_destino] = registros[registro_destino] - registros[registro_origen];

    } else if (strcmp(instruccion, "JNZ") == 0) {
        posicion_registro = decodificar_posicion(lista_instruccion[1]);
        if (registros[posicion_registro] != 0) {
            registros[PC] = string_a_uint32(lista_instruccion[2]);
        }

    } else if (strcmp(instruccion, "LOG") == 0) {
        posicion_registro = decodificar_posicion(lista_instruccion[1]);
        log_info(logger, "## (%i : %i) Valor de registro %s: %d", PID, TID, lista_instruccion[1], registros[posicion_registro]);

    } else if (strcmp(instruccion, "DUMP_MEMORY") == 0) {
        actualizar_contexto();
        dump_memory();

    } else if (strcmp(instruccion, "IO") == 0) {
        actualizar_contexto();

        int tiempo = string_a_uint32(lista_instruccion[1]);
        io(tiempo);
    } else if (strcmp(instruccion, "PROCESS_CREATE") == 0) {
        actualizar_contexto();

        char* archivo = lista_instruccion[1];
        uint32_t tamanio = string_a_uint32(lista_instruccion[2]);
        uint32_t prioridad = string_a_uint32(lista_instruccion[3]);
        process_create(archivo,tamanio,prioridad);
       
    } else if (strcmp(instruccion, "THREAD_CREATE") == 0) {
        actualizar_contexto();

        char* archivo = lista_instruccion[1];
        uint32_t prioridad = string_a_uint32(lista_instruccion[2]);
        thread_create(archivo,prioridad);

    } else if (strcmp(instruccion, "THREAD_JOIN") == 0) {
        actualizar_contexto();

        uint32_t i_tid = string_a_uint32(lista_instruccion[1]); //i_tid es "instruccion tid" que viene con la syscall
        thread_join(i_tid);

    } else if (strcmp(instruccion, "THREAD_CANCEL") == 0) {
        actualizar_contexto();

        uint32_t i_tid = string_a_uint32(lista_instruccion[1]); //i_tid es "instruccion tid" que viene con la syscall
        thread_cancel(i_tid);

    } else if (strcmp(instruccion, "MUTEX_CREATE") == 0) {
        actualizar_contexto();

        char* recurso = lista_instruccion[1];
        mutex_create(recurso);

    } else if (strcmp(instruccion, "MUTEX_LOCK") == 0) {
        actualizar_contexto();

        char* recurso = lista_instruccion[1];
        mutex_lock(recurso);

    } else if (strcmp(instruccion, "MUTEX_UNLOCK") == 0) {
        actualizar_contexto();

        char* recurso = lista_instruccion[1];
        mutex_unlock(recurso);

    } else if (strcmp(instruccion, "THREAD_EXIT") == 0) {
        actualizar_contexto();
        thread_exit();

    } else if (strcmp(instruccion, "PROCESS_EXIT") == 0) {
        actualizar_contexto();
        process_exit();

    }
    
}

int decodificar_posicion(char *registro) {
    if (strcmp(registro, "PC") == 0) {
        return PC;
    } else if (strcmp(registro, "AX") == 0) {
        return AX;
    } else if (strcmp(registro, "BX") == 0) {
        return BX;
    } else if (strcmp(registro, "CX") == 0) { 
        return CX;
    } else if (strcmp(registro, "DX") == 0) {
        return DX;
    } else if (strcmp(registro, "EX") == 0) {
        return EX;
    } else if (strcmp(registro, "FX") == 0) {
        return FX;
    } else if (strcmp(registro, "GX") == 0) {
        return GX;
    } else if (strcmp(registro, "HX") == 0) {
        return HX;
    } else { 
        //printf("Registro desconocido: %s\n", registro);
        return -1;
    }
}

uint32_t string_a_uint32(const char *str) {
    return (uint32_t) strtoul(str, NULL, 10);
}

void fRead(int direccion_fisica,int posicion_registro){

    t_paquete* paquete = crear_paquete(OP_READ_MEM);
    agregar_a_paquete(paquete, &PID, sizeof(PID));
    agregar_a_paquete(paquete, &TID, sizeof(TID));
    agregar_a_paquete(paquete, &direccion_fisica, sizeof(direccion_fisica));
    enviar_paquete(paquete, socket_mem);
    eliminar_paquete(paquete);

    recibir_operacion(socket_mem);
    registros[posicion_registro] = recibir_int(socket_mem);

    log_info(logger, "## TID: %d - Acción: LEER - Dirección Física: %d ", TID,direccion_fisica);
}
void fWrite(int direccion_fisica,int valor){

    t_paquete* paquete = crear_paquete(OP_WRITE_MEM);
    agregar_a_paquete(paquete, &PID, sizeof(PID));
    agregar_a_paquete(paquete, &TID, sizeof(TID));
    agregar_a_paquete(paquete, &direccion_fisica, sizeof(direccion_fisica));
    agregar_a_paquete(paquete, &valor, sizeof(valor));
    enviar_paquete(paquete, socket_mem);
    eliminar_paquete(paquete);

    enum op_code op = recibir_operacion(socket_mem);
    int ok = recibir_int(socket_mem);

    if (op == OP_WRITE_MEM && ok == 1){
        log_info(logger, "## TID: %d - Acción: ESCRIBIR - Dirección Física: %d ",TID,direccion_fisica);
    } 
    return;
}
void dump_memory(){
    enviar_operacion(socket_kernel_dis, OP_DUMP_MEM);
    // t_paquete* paquete = crear_paquete(OP_DUMP_MEM);
    // enviar_paquete(paquete, socket_kernel_dis);
    // eliminar_paquete(paquete);
}

void io(int tiempo){
    enviar_int(tiempo, socket_kernel_dis, OP_IO);
    // t_paquete* paquete = crear_paquete(OP_IO);
    // agregar_a_paquete(paquete, &tiempo, sizeof(tiempo));
    // enviar_paquete(paquete, socket_kernel_dis);
    // eliminar_paquete(paquete);
}

void process_create(char* archivo,uint32_t tamanio,uint32_t prioridad){
    t_paquete* paquete = crear_paquete(OP_CREAR_PROCESO);
    agregar_a_paquete(paquete, archivo, strlen(archivo)+1);
    agregar_a_paquete(paquete, &tamanio, sizeof(tamanio));
    agregar_a_paquete(paquete, &prioridad, sizeof(prioridad));
    enviar_paquete(paquete, socket_kernel_dis);
    eliminar_paquete(paquete);
    recuperar_contexto_urgente = true;
}

void thread_create(char* archivo, uint32_t prioridad){
    t_paquete* paquete = crear_paquete(OP_CREAR_HILO);
    agregar_a_paquete(paquete, archivo, strlen(archivo)+1);
    agregar_a_paquete(paquete, &prioridad, sizeof(prioridad));
    enviar_paquete(paquete, socket_kernel_dis);
    eliminar_paquete(paquete);
    recuperar_contexto_urgente = true;
}

void thread_join(uint32_t i_tid){
    enviar_int(i_tid, socket_kernel_dis, OP_THREAD_JOIN);
    revisar_bloqueante();
    // t_paquete* paquete = crear_paquete(OP_THREAD_JOIN);
    // agregar_a_paquete(paquete, &i_tid, sizeof(i_tid));
    // enviar_paquete(paquete, socket_kernel_dis);
    // eliminar_paquete(paquete);
}

void thread_cancel(uint32_t i_tid){
    enviar_int(i_tid, socket_kernel_dis, OP_CANCELAR_HILO);
    recuperar_contexto_urgente = true;
    // t_paquete* paquete = crear_paquete(OP_CANCELAR_HILO);
    // agregar_a_paquete(paquete, &i_tid, sizeof(i_tid));
    // enviar_paquete(paquete, socket_kernel_dis);
    // eliminar_paquete(paquete);
}

void mutex_create(char* recurso){
    enviar_string(recurso, socket_kernel_dis, OP_MUTEX_CREATE);
    recuperar_contexto_urgente = true;
    // t_paquete* paquete = crear_paquete(OP_MUTEX_CREATE);
    // agregar_a_paquete(paquete, recurso, strlen(recurso)+1);
    // enviar_paquete(paquete, socket_kernel_dis);
    // eliminar_paquete(paquete);
}

void mutex_lock(char* recurso){
    enviar_string(recurso, socket_kernel_dis, OP_MUTEX_LOCK);
    revisar_bloqueante();

    // t_paquete* paquete = crear_paquete(OP_MUTEX_LOCK);
    // agregar_a_paquete(paquete, recurso, strlen(recurso)+1);
    // enviar_paquete(paquete, socket_kernel_dis);
    // eliminar_paquete(paquete);
}

void mutex_unlock(char* recurso){
    enviar_string(recurso, socket_kernel_dis, OP_MUTEX_UNLOCK);
    revisar_bloqueante();
    // t_paquete* paquete = crear_paquete(OP_MUTEX_UNLOCK);
    // agregar_a_paquete(paquete, recurso, strlen(recurso)+1);
    // enviar_paquete(paquete, socket_kernel_dis);
    // eliminar_paquete(paquete);
}

void thread_exit(){
    enviar_operacion(socket_kernel_dis, OP_FINALIZAR_HILO);
    // t_paquete* paquete = crear_paquete(OP_FINALIZAR_HILO);
    // enviar_paquete(paquete, socket_kernel_dis);
    // eliminar_paquete(paquete);
}

void process_exit(){
    enviar_operacion(socket_kernel_dis, OP_FINALIZAR_PROCESO);
    // t_paquete* paquete = crear_paquete(OP_FINALIZAR_PROCESO);
    // enviar_paquete(paquete, socket_kernel_dis);
    // eliminar_paquete(paquete);
}

int MMU(int desplazamiento){
    int direccion_fisica = base + desplazamiento;
    if(direccion_fisica > base + limite){
        actualizar_contexto();
        motivo(OP_SEGMENTATION_FAULT);
        return -1;
    }else{
        return direccion_fisica;
    }
}

void revisar_bloqueante() {
    recibir_operacion(socket_kernel_dis);
    int rta = recibir_int(socket_kernel_dis);

    if (rta == 0) {
        recuperar_contexto_urgente = true;
    }
    enviar_operacion(socket_kernel_dis, OP_ES_BLOQUEANTE);
}