#include "estructuras.h"

// En este archivo creamos y finalizamos estructuras



void crear_proceso(char *archivo_pseudocodigo, int tamanio_proceso, int prioridad)
{
   // ultimo_tamanio_proceso = tamanio_proceso; // Guardamos el tamaño del proceso de forma GLOBAL

    char *script = string_duplicate(archivo_pseudocodigo);

    t_PCB *pcb = crear_pcb(script, tamanio_proceso, prioridad);
    queue_push(cola_procesos_NEW, pcb);
    
    log_info(logger, "## (%i : 0) Se crea el proceso - Estado: NEW", pcb->PID);

   if (queue_size(cola_procesos_NEW) == 1)
   {                                // Si es el unico proceso esperando, se envía la solicitud a MEMORIA. Sino, se encola (eso es hizo en la linea de arriba).
        revisar_cola_procesos_new(); // Pedirle a memoria que inicialice el proceso
  }
}

void crear_hilo(char *archivo_pseudocodigo, int prioridad)
{                                                                      // Suponemos que pertenece al PCB del proceso en estado EXECUTE
    t_PCB *proc_EXEC = proceso_EXEC();
    agregar_nuevo_tcb(proc_EXEC, archivo_pseudocodigo, prioridad); // Crea el hilo y lo pasa a al cola NEW
    // t_TCB *tcb = queue_peek(cola_hilos_NEW);                         

    t_TCB* tcb = encontrar_hilo_asociado(proc_EXEC -> PID);      // Encuentra y devuelve hilo asociado al proceso (quitandolo de NEW)

    bool rta_mem = crear_tcb_memoria(proc_EXEC, tcb);
    if (!rta_mem)
    {
        //printf("Memoria no puede darle el OK al hilo de PID:%i y TID:%i.\n", proc_EXEC->PID, tcb->TID);
        return;
    }

    agregar_hilo_a_ready(tcb); // Agrega el hilo a la cola READY correspondiente contemplando el algoritmo de panificacion
    list_add(lista_hilos, tcb);

    log_info(logger, "## (%i : %i) Se crea el hilo - Estado: READY", proc_EXEC->PID, tcb->TID);
}

t_PCB *crear_pcb(char *pseudocodigo, int tam, int prioridad)
{
    t_PCB *pcb = (t_PCB *)malloc(sizeof(*pcb));
    pcb->PID = proximo_pid;
    pcb->prox_tid = 0;
    pcb->TIDS = list_create();
    pcb->mutexs = list_create();
    pcb->tamanio = tam;
    proximo_pid++;


    agregar_nuevo_tcb(pcb, pseudocodigo, prioridad); // El primer TID tendrá prioridad 0
    return pcb;
}

void agregar_nuevo_tcb(t_PCB *pcb, char *pseudocodigo, int prioridad)
{ // Crea un hilo para un proceso ya creado

    t_TCB *nuevo_tcb = crear_tcb(pcb, pseudocodigo, prioridad);

    int *tid_ptr = malloc(sizeof(int));
    *tid_ptr = pcb->prox_tid;
    list_add(pcb->TIDS, tid_ptr);

    queue_push(cola_hilos_NEW, nuevo_tcb); // Se encola el hilo en NEW (COLA GLOBAL)

    pcb->prox_tid++;
}

t_TCB *crear_tcb(t_PCB *pcb, char *pseudocodigo, int prioridad)
{
    t_TCB *tcb = malloc(sizeof(t_TCB));
    tcb->TID = pcb->prox_tid;
    tcb->PID = pcb->PID;
    tcb->prioridad = prioridad;
    tcb->pseudocodigo = pseudocodigo;
    tcb->hilos_bloqueados = list_create();
    return tcb;
}

void revisar_cola_procesos_new()
{
    while (!queue_is_empty(cola_procesos_NEW))
    {

        t_PCB *pcb = queue_peek(cola_procesos_NEW);
        bool memoria_disponible = intentar_crear_pcb_memoria(pcb, pcb ->tamanio); // Preguntamos si memoria lo puede inicializar
        
        
        if (!memoria_disponible)
        {
            //printf("Memoria no puede admitir al proceso de PID: %i.\n", pcb->PID);
            return;   // No hay espacio en memoria. Esperar que otro proceso finalice. Esto me lo tiro una IA
        }

        // Memoria disponible

        //printf("Memoria admitió al proceso de PID: %i.\n", pcb->PID);
        
        t_TCB *tcb = queue_peek(cola_hilos_NEW);
        bool rta_mem = crear_tcb_memoria(pcb, tcb); // Le decimos a memoria que inicie el hilo 0
        if (!rta_mem)
        {
            //printf("Memoria no puede darle el OK al hilo inicial del proceso de PID: %i .\n", pcb->PID);
            return; 
        }

        tcb = queue_pop(cola_hilos_NEW);
        agregar_hilo_a_ready(tcb); // Agrega el hilo a la cola READY correspondiente contemplando el algoritmo de planificacion

        pcb = queue_pop(cola_procesos_NEW);
        queue_push(cola_procesos_READY, pcb);
    }
}

/*
void revisar_cola_procesos_new() {
    if (queue_is_empty(cola_procesos_NEW)) {
        return; // Nada que procesar
    }

    t_PCB *pcb = queue_peek(cola_procesos_NEW);
    log_info(logger, "Intentando inicializar proceso PID %i", pcb->PID);

    bool memoria_disponible = intentar_crear_pcb_memoria(pcb, ultimo_tamanio_proceso);
    if (!memoria_disponible) {
        log_info(logger, "Memoria no puede admitir al proceso de PID: %i.\n", pcb->PID);
        return; // No hay espacio en memoria
    }

    log_info(logger, "Memoria admitió al proceso de PID: %i.\n", pcb->PID);

    t_TCB *tcb = queue_peek(cola_hilos_NEW);
    bool rta_mem = crear_tcb_memoria(pcb, tcb);
    if (!rta_mem) {
        log_info(logger, "Memoria no puede darle el OK al hilo inicial del proceso de PID: %i.\n", pcb->PID);
        return; 
    }

    tcb = queue_pop(cola_hilos_NEW);
    agregar_hilo_a_ready(tcb);

    queue_pop(cola_procesos_NEW);
    queue_push(cola_procesos_READY, pcb);

    log_info(logger, "Proceso PID %i inicializado exitosamente.", pcb->PID);
}
*/


bool intentar_crear_pcb_memoria(t_PCB *pcb, int tamanio_proceso)
{
    if (pcb != NULL)
    {
        char *puerto_str = string_itoa(configuracion->PUERTO_MEMORIA);
        int socket_mem = crear_conexion(configuracion->IP_MEMORIA, puerto_str);
        free(puerto_str);

        t_paquete *paquete = crear_paquete(OP_CREAR_PROCESO); // Nombre de OPCODE cambiado
        agregar_a_paquete(paquete, &(pcb->PID), sizeof(int));
        agregar_a_paquete(paquete, &tamanio_proceso, sizeof(int));

        int bytes = enviar_paquete(paquete, socket_mem);
        if (bytes == 0)
        {
            //printf("Error al enviar paquete a MEMORIA!\n");
            eliminar_paquete(paquete);
            abort();
        }
        eliminar_paquete(paquete);

        recibir_operacion(socket_mem);
        int rta_mem = recibir_int(socket_mem);

        //return (rta_mem == 1);
        return rta_mem;
    }
    else
    {
        return false; // Si el PCB es NULL, devolvemos false
    }
}

bool crear_tcb_memoria(t_PCB *pcb, t_TCB *tcb)
{
    char *puerto_str = string_itoa(configuracion->PUERTO_MEMORIA);
    int socket_mem = crear_conexion(configuracion->IP_MEMORIA, puerto_str);
    free(puerto_str);

    t_paquete *paquete = crear_paquete(OP_INICIAR_HILO);
    agregar_a_paquete(paquete, &(pcb->PID), sizeof(int));
    agregar_a_paquete(paquete, &(tcb->TID), sizeof(int));
    agregar_a_paquete(paquete, tcb->pseudocodigo, strlen(tcb->pseudocodigo) + 1);

    int bytes = enviar_paquete(paquete, socket_mem); // Se envia el PID, TID, y archivo_pseudocodigo
    if (bytes == 0)
    {
        //printf("Error al enviar paquete a MEMORIA!\n");
        eliminar_paquete(paquete);
        abort();
    }
    eliminar_paquete(paquete);

    recibir_operacion(socket_mem);
    int rta_mem = recibir_int(socket_mem);
    
    return rta_mem;
}

void finalizar_hilo(int pid, int tid){
    t_TCB* tcb = NULL;
    //printf("entro a finalizar_hilo()");
    if(hilo_EXEC->TID == tid && hilo_EXEC->PID == pid){
        tcb = hilo_EXEC;
    }
    if (tcb == NULL){
        tcb = buscar_y_eliminar_tcb_en_ready(pid, tid);      // Si no lo encuentra, devuelve NULL
    }
    if (tcb == NULL){
        tcb = buscar_y_eliminar_tcb_en_block(pid, tid);
    }
    if (tcb == NULL){
        //printf("El hilo con TID %i que se desea finalizar no existe o ya finalizo.\n", tid);
        enviar_a_ejecucion(hilo_EXEC);
        return;
    }
   
    int socket = enviar_a_memoria(OP_FINALIZAR_HILO, tcb);

    recibir_operacion(socket);
    int rta_mem = recibir_int(socket);
    if (rta_mem != 1){ 
        //printf("Memoria no puede finalizar al hilo con TID %i.\n", tid);
        return;
    }

    desbloquear_hilos_bloqueados_por_otro(tcb);
    //deberiamos poner en NULL al campo "propietario" de los recursos que tenga el hilo? Deberiamos hacer un signal de los recursos que tenga asignados?
    //porque la consigna dice q desbloquiemos a los huilos que estan bloqueados por este proceso pero no aclara mucho mas...

    queue_push(cola_hilos_EXIT, tcb);
    log_info(logger, "## (%i : %i) Finaliza el hilo",tcb -> PID,tcb -> TID);
    list_remove_element(lista_hilos, tcb);

    if(hilo_EXEC->TID == tid){ //si fue thread exit
        elegir_hilo();
        return;
    }

    if(hilo_EXEC != NULL){
        enviar_a_ejecucion(hilo_EXEC);//si fue thread cancel
        return;
    }

    return;
}

void eliminar_hilo_de_un_proceso_a_finalizar(int pid, int tid){
    t_TCB* tcb = NULL;

    if(hilo_EXEC != NULL && hilo_EXEC->TID == tid && hilo_EXEC->PID == pid){
        tcb = hilo_EXEC;
    }
    if (tcb == NULL){
        tcb = buscar_y_eliminar_tcb_en_ready(pid, tid);      // Si no lo encuentra, devuelve NULL
    }
    if (tcb == NULL){
        tcb = buscar_y_eliminar_tcb_en_block(pid, tid);
    }
   
    if (tcb == NULL){
        return;
    }

    desbloquear_hilos_bloqueados_por_otro(tcb);
    //deberiamos poner en NULL al campo "propietario" de los recursos que tenga el hilo? Deberiamos hacer un signal de los recursos que tenga asignados?
    //porque la consigna dice q desbloquiemos a los huilos que estan bloqueados por este proceso pero no aclara mucho mas...

    log_info(logger, "## (%i : %i) Finaliza el hilo",tcb -> PID,tcb -> TID);
    queue_push(cola_hilos_EXIT, tcb);
    list_remove_element(lista_hilos, tcb);



}


t_TCB* buscar_y_eliminar_tcb_en_ready(int pid, int tid){ // Retorna null si no lo encuentra
    if(!strcmp(configuracion->ALGORITMO_PLANIFICACION, "CMN")){
            if (!list_is_empty(colas_multi_level)){
                for (int i = 0; i < list_size(colas_multi_level); i++) {
                t_queue* queue = list_get(colas_multi_level, i);
                if (queue != NULL && !queue_is_empty(queue)) {
                    for (int j = 0; j < queue_size(queue); j++) {
                    t_TCB* tcb = list_get(queue->elements, j);
                    if (tcb != NULL && tcb->TID == tid && tcb->PID == pid) {
                        list_remove(queue->elements, j);
                        return tcb;
                        }
                    }
                }
            }
        }
            return NULL;
    }else{
        if(queue_size(cola_hilos_READY) > 0){
        for(int i = 0; i < queue_size(cola_hilos_READY); i++){
            t_TCB* tcb = list_get(cola_hilos_READY->elements, i);
            if(tcb != NULL && (tcb -> TID == tid) && tcb->PID == pid){
               tcb = list_remove(cola_hilos_READY->elements, i);       //no libera al tcb que remueve
               return tcb;
            }
        }
        return NULL;
        }

        return NULL;
        
    }
}


t_TCB* buscar_y_eliminar_tcb_en_block(int pid, int tid){     // Retorna null si no lo encuentra
    // Buscamos en la cola block general
    for(int i = 0; i < queue_size(cola_hilos_BLOCK); i++){
        t_TCB* tcb = list_get(cola_hilos_BLOCK->elements, i);
        if(tcb->TID == tid && tcb->PID == pid){
            list_remove(cola_hilos_BLOCK->elements, i);
            return tcb;
        }
    }

    // Buscamos en las colas de bloqueados de cada mutex de cada proceso
    // Primero revisamos el proceso EXEC
    t_PCB *proc_EXEC = proceso_EXEC();
    for(int i = 0; i < list_size(proc_EXEC->mutexs); i++){
        recurso* mutex = list_get(proc_EXEC->mutexs, i);
        for(int j = 0; j < list_size((mutex->hilos_bloqueados)->elements); j++){
            t_TCB* tcb = list_get(mutex->hilos_bloqueados->elements, j);
            if(tcb->TID == tid && tcb->PID == pid){
                list_remove(mutex->hilos_bloqueados->elements, j);
                return tcb;
            }
        }
    }
    // Luego revisamos los procesos en ready
    for(int i = 0; i < queue_size(cola_procesos_READY); i++){
        t_PCB* pcb = list_get(cola_procesos_READY->elements, i);
        for(int j = 0; j < list_size(pcb->mutexs); j++){
            recurso* mutex = list_get(pcb->mutexs, j);
            for(int k = 0; k < list_size((mutex->hilos_bloqueados)->elements); k++){
                t_TCB* tcb = list_get(mutex->hilos_bloqueados->elements, k);
                if(tcb->TID == tid && tcb->PID == pid){
                    list_remove(mutex->hilos_bloqueados->elements, k);
                    return tcb;
                }
            }
        }
    }

    return NULL; //si no encontro al tcb
}

t_TCB* buscar_tcb_en_block(int pid, int tid){     // Retorna null si no lo encuentra
    // Buscamos en la cola block general
    for(int i = 0; i < queue_size(cola_hilos_BLOCK); i++){
        t_TCB* tcb = list_get(cola_hilos_BLOCK->elements, i);
        if(tcb->TID == tid && tcb->PID == pid){
            return tcb;
        }
    }

    // Buscamos en las colas de bloqueados de cada mutex de cada proceso
    // Primero revisamos el proceso EXEC
    t_PCB *proc_EXEC = proceso_EXEC();
    for(int i = 0; i < list_size(proc_EXEC->mutexs); i++){
        recurso* mutex = list_get(proc_EXEC->mutexs, i);
        for(int j = 0; j < list_size((mutex->hilos_bloqueados)->elements); j++){
            t_TCB* tcb = list_get(mutex->hilos_bloqueados->elements, j);
            if(tcb->TID == tid && tcb->PID == pid){
                return tcb;
            }
        }
    }
    // Luego revisamos los procesos en ready
    for(int i = 0; i < queue_size(cola_procesos_READY); i++){
        t_PCB* pcb = list_get(cola_procesos_READY->elements, i);
        for(int j = 0; j < list_size(pcb->mutexs); j++){
            recurso* mutex = list_get(pcb->mutexs, j);
            for(int k = 0; k < list_size((mutex->hilos_bloqueados)->elements); k++){
                t_TCB* tcb = list_get(mutex->hilos_bloqueados->elements, k);
                if(tcb->TID == tid && tcb->PID == pid){
                    return tcb;
                }
            }
        }
    }

    return NULL; //si no encontro al tcb
}

t_TCB* encontrar_hilo_asociado(int pid) {
    // Iterar sobre la cola de hilos_NEW para encontrar el hilo asociado al PID
    for (int i = 0; i < queue_size(cola_hilos_NEW); i++) {
        t_TCB* tcb_actual = list_get(cola_hilos_NEW->elements, i); // Recupera el TCB en la posición i
        if (tcb_actual->PID == pid) {
           return list_remove(cola_hilos_NEW->elements, i);
        }
    }
    return NULL; // Retorna NULL si no encuentra ningún TCB asociado al PID
}



void desbloquear_hilos_bloqueados_por_otro(t_TCB* tcb){
    for(int i = 0; i< list_size(tcb->hilos_bloqueados); i++){
        //sacamos al tcb de block
        int *puntero_a_tid = list_get(tcb -> hilos_bloqueados, i);
        int tid = *puntero_a_tid; // Aquí accedemos al valor entero
        
        t_TCB* hilo_a_desbloquear = buscar_y_eliminar_tcb_en_block(tcb->PID, tid);
        //lo agregamos a ready

        agregar_hilo_a_ready(hilo_a_desbloquear);
        //sacamos al tid de la cola de bloqueados por el otro tcb
        list_remove(tcb->hilos_bloqueados, i);

        free(puntero_a_tid);
    }
}




void finalizar_proceso(){

    int socket = enviar_pid_a_memoria(OP_FINALIZAR_PROCESO, hilo_EXEC);  //envia op code y pid
    recibir_operacion(socket);
    int rta = recibir_int(socket);

    if (rta != 1){
        //printf("Error al finalizar proceso en memoria\n");
        return;
    }

    t_PCB *proc_EXEC = proceso_EXEC();
    for(int i=0; i<list_size(proc_EXEC->TIDS); i++){
        int* tid_ptr = (int*) list_get(proc_EXEC->TIDS, i); // Obtén el puntero
        int tid = *tid_ptr; // Desreferencia para obtener el valor real
        eliminar_hilo_de_un_proceso_a_finalizar(proc_EXEC->PID, tid);
    }

    list_remove_element(cola_procesos_READY->elements, proc_EXEC);
    queue_push(cola_procesos_EXIT, proc_EXEC);
    log_info(logger, "## Finaliza el proceso %i\n", proc_EXEC -> PID);       // Podria ser hilo_EXEC -> PID

    list_clean_and_destroy_elements(cola_hilos_EXIT->elements, (void (*)(void*)) liberar_tcb);


    liberar_pcb(proc_EXEC);

    revisar_cola_procesos_new();    
    elegir_hilo();  
    
}  
 
int cant_hilos_ready_en_su_cola(){//esta funcion sirve para CMN
    int cant = 0;
    t_queue* cola = list_get(colas_multi_level, hilo_EXEC->prioridad);
    cant = queue_size(cola);
    return cant;
}

void liberar_tcb(t_TCB* tcb){

    if (tcb == NULL) return; // Evitar trabajar con un puntero nulo

    list_remove_element(cola_hilos_BLOCK->elements, tcb);


    int pos_eliminar = -1;
    for (int i = 0; i < queue_size(cola_hilos_BLOCK_IO); i++) {
        hilo_io_args_t *args = list_get(cola_hilos_BLOCK_IO->elements, i);
        if (args->hilo_bloqueado == tcb) {
            pos_eliminar = i;
            break;
        }
    }
    if (pos_eliminar != -1) {
        hilo_io_args_t *args = list_remove(cola_hilos_BLOCK_IO->elements, pos_eliminar);
        free(args);
    }
    pos_eliminar = -1;
    for (int i = 0; i < queue_size(cola_hilos_BLOCK_DUMP_MEMORY); i++) {
        argumentos_volcado_t *args = list_get(cola_hilos_BLOCK_DUMP_MEMORY->elements, i);
        if (args->PID_de_hilo_bloqueado == tcb->PID && args->TID_de_hilo_bloqueado == tcb->TID) {
            pos_eliminar = i;
            break;
        }
    }
    if (pos_eliminar != -1) {
        hilo_io_args_t *args = list_remove(cola_hilos_BLOCK_DUMP_MEMORY->elements, pos_eliminar);
        free(args);
    }

    //list_remove_element(cola_hilos_BLOCK_IO->elements, tcb);

    if (tcb->pseudocodigo != NULL) {
        free(tcb->pseudocodigo);
    }
    if (tcb->hilos_bloqueados != NULL) {
        list_destroy_and_destroy_elements(tcb->hilos_bloqueados, free);
    }

   free(tcb); 
}

void liberar_pcb(t_PCB* pcb){
    list_destroy_and_destroy_elements(pcb->TIDS, free);
    list_destroy_and_destroy_elements(pcb->mutexs, (void (*)(void*)) liberar_recurso);
    free(pcb);
}