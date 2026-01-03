#include "global.h"

pthread_mutex_t mutex;

t_temporal *contador_global;

t_log* logger;
valores_config* configuracion;
char* archivo_configuracion;
valores_config_prueba* configuracion_prueba;

int proximo_pid = 1;
//t_PCB* proceso_EXEC = NULL;
t_TCB* hilo_EXEC = NULL;

// int socket_mem;
int socket_cpu_dispatch;
int socket_cpu_interrupt;
int ultimo_tamanio_proceso;
t_temporal* clock_RR;
t_temporal* clock_IO;

t_queue* cola_procesos_NEW;
t_queue* cola_procesos_READY;
t_queue* cola_procesos_EXIT;

t_queue* cola_hilos_NEW;
t_queue* cola_hilos_READY;
t_list* colas_multi_level;
t_queue* cola_hilos_BLOCK;
t_queue* cola_hilos_EXIT;

t_queue *cola_hilos_BLOCK_IO;
t_queue *cola_hilos_BLOCK_DUMP_MEMORY;

t_list *lista_hilos;

void iniciar_contador_global() {
    contador_global = temporal_create();
}

void iniciar_colas() {
    cola_procesos_NEW = queue_create();
    cola_procesos_READY = queue_create();
    cola_procesos_EXIT = queue_create();

    cola_hilos_NEW = queue_create();
    cola_hilos_READY = queue_create();
    colas_multi_level = list_create();
    cola_hilos_BLOCK = queue_create();
    cola_hilos_EXIT = queue_create();
    lista_hilos = list_create();

    cola_hilos_BLOCK_IO = queue_create();
    cola_hilos_BLOCK_DUMP_MEMORY = queue_create();
}

int enviar_a_memoria(enum op_code codigo, t_TCB *tcb) //envia op_code, pid y tid
{
   char *puerto_str = string_itoa(configuracion->PUERTO_MEMORIA);
    int socket_mem = crear_conexion(configuracion->IP_MEMORIA, puerto_str);
    free(puerto_str);

    t_paquete *paquete = crear_paquete(codigo);
    agregar_a_paquete(paquete, &(tcb->PID), sizeof(int));
    agregar_a_paquete(paquete, &(tcb->TID), sizeof(int));

    int bytes = enviar_paquete(paquete, socket_mem);
    if (bytes == 0){
        //printf("Error al enviar paquete a MEMORIA!\n");
        eliminar_paquete(paquete);
        abort();
    }
    eliminar_paquete(paquete);
    return socket_mem;
}

int enviar_pid_a_memoria(enum op_code codigo, t_TCB *tcb) //envia op_code y pid
{
    char *puerto_str = string_itoa(configuracion->PUERTO_MEMORIA);
    int socket_mem = crear_conexion(configuracion->IP_MEMORIA, puerto_str);
    free(puerto_str);

    t_paquete *paquete = crear_paquete(codigo);
    agregar_a_paquete(paquete, &(tcb->PID), sizeof(int));

    int bytes = enviar_paquete(paquete, socket_mem);
    if (bytes == 0){
        //printf("Error al enviar paquete a MEMORIA!\n");
        eliminar_paquete(paquete);
        abort();
    }
    eliminar_paquete(paquete);
    return socket_mem;
}

t_PCB* buscar_y_eliminar_pcb_en_ready(int pid){
        t_PCB* proceso_actual;
        int cantidad_de_procesos_ready = queue_size(cola_procesos_READY);

        for (int q = 0; q < cantidad_de_procesos_ready; q++){
            proceso_actual = list_get(cola_procesos_READY->elements, q);

            if (proceso_actual -> PID == pid){ 
                proceso_actual = list_remove(cola_procesos_READY -> elements, q);
                return proceso_actual;
        }
    }
    return NULL;
}

t_PCB *buscar_proceso_READY(int PID) {
    bool es_el_buscado(t_PCB *proceso) {
        return proceso->PID == PID;
    }
    return list_find(cola_procesos_READY->elements, (void*) es_el_buscado);
}




t_PCB *proceso_EXEC() {
    return buscar_proceso_READY(hilo_EXEC->PID);
}


void enviar_bloqueante_y_esperar(int valor) {
    enviar_int(valor, socket_cpu_dispatch, OP_ES_BLOQUEANTE);
    recibir_operacion(socket_cpu_dispatch);
}