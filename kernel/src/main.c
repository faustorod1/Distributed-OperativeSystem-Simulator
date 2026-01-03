#include "main.h"

argumentos_volcado_t *hilo_DUMP_MEMORY;

int main(int argc, char *argv[])
{
    // Están hardcodeados los valores, si quieren los pueden devolver a como estaban
    proceso_inicial_t *argumentos = malloc(sizeof(proceso_inicial_t));
    argumentos->archivo_pseudocodigo = argv[1];
    argumentos->tamanio_proceso = atoi(argv[2]);

    //PRUEBAS OFICIALES
    if(!strcmp(argv[3],"-1.1")){
        archivo_configuracion = "plani-fifo";
    }else if(!strcmp(argv[3],"-1.2")){
        archivo_configuracion = "plani-prior";
    }else if(!strcmp(argv[3],"-1.3")){
        archivo_configuracion = "plani-cmn";
    }else if(!strcmp(argv[3],"-2.1")){
        archivo_configuracion = "race-cond-750";
    }else if(!strcmp(argv[3],"-2.2")){
        archivo_configuracion = "race-cond-150";
    }else if(!strcmp(argv[3],"-3") || !strcmp(argv[3],"-3.1") || !strcmp(argv[3],"-3.2") || !strcmp(argv[3],"-3.3")){
        archivo_configuracion = "part-fijas";
    }else if(!strcmp(argv[3],"-4")){
        archivo_configuracion = "part-dinam";
    }else if (!strcmp(argv[3],"-5")){
        archivo_configuracion = "fs";
    }else if(!strcmp(argv[3],"-6")){
        archivo_configuracion = "stress";
    }else if(argv[3] == NULL){//si no pasamos parametros
        archivo_configuracion = "kernel";
    }

    //printf("Archivo de pseudocódigo: %s\n", argumentos->archivo_pseudocodigo);
    //printf("Tamaño del proceso: %d\n", argumentos->tamanio_proceso);

    iniciar_config();
    iniciar_logger();

    // Inicia Kernel conexion con CPU como cliente dos veces y como cliente de memoria
    iniciar_conexion();

    iniciar_colas();
    iniciar_contador_global();

    crear_proceso(argumentos->archivo_pseudocodigo, argumentos->tamanio_proceso, 0); // Creación del proceso inicial

    pthread_t hilo_peticiones_cpu_dispatch;
    // // Hilo para desalojar hilos por fin de quantum
    // // if (!strcmp(configuracion->ALGORITMO_PLANIFICACION, "CMN"))
    // // { // para no crear este hilo al pedo si usamos otro algoritmo
    // //     pthread_t hilo_desalojar_hilo_fin_Q;
    // //     pthread_create(&hilo_desalojar_hilo_fin_Q, NULL, (void *)desalojar_hilo_fin_Q, NULL);
    // //     pthread_detach(hilo_desalojar_hilo_fin_Q);
    // // }

    elegir_hilo(); // Se elige el hilo a ejecutar

    // Hilo para gestionar peticiones de CPU dispatch
    pthread_create(&hilo_peticiones_cpu_dispatch, NULL, (void *)gestionar_peticiones_cpu_dispatch, NULL);

    pthread_join(hilo_peticiones_cpu_dispatch, NULL); // revisar si esta bien el join

    free(argumentos);
    terminar_programa();
    return 0;
}

void terminar_programa()
{
    log_destroy(logger);
    free(configuracion);
}

void iniciar_conexion()
{
    // Inicio Kernel como cliente de CPU (a puerto dispatch)
    char *puerto_str_dispatch = string_itoa(configuracion->PUERTO_CPU_DISPATCH); // Cambio de nombre al puerto
    socket_cpu_dispatch = crear_conexion(configuracion->IP_CPU, puerto_str_dispatch);
    free(puerto_str_dispatch);

    enviar_int(MOD_KERNEL, socket_cpu_dispatch, OP_HANDSHAKE);
    enum op_code op_cpu_dispatch = recibir_operacion(socket_cpu_dispatch); // Variable op cambiada a op_cpu_dispatch
    if (op_cpu_dispatch != OP_HANDSHAKE)
    {
        //printf("ERROR al conectar kernel con CPU_dispatch!\n");
        terminar_programa();
        // return;
        abort();
    }
    //printf("Se conectó KERNEL a CPU_dispatch!\n");

    // Inicio Kernel como cliente de CPU (a puerto interrupt)
    char *puerto_str_interrupt = string_itoa(configuracion->PUERTO_CPU_INTERRUPT); // Cambio de nombre al puerto
    socket_cpu_interrupt = crear_conexion(configuracion->IP_CPU, puerto_str_interrupt);
    free(puerto_str_interrupt);

    sleep(1); 

    enviar_int(MOD_KERNEL, socket_cpu_interrupt, OP_HANDSHAKE);
    enum op_code op_cpu_interrupt = recibir_operacion(socket_cpu_interrupt); // Variable op cambiada a op_cpu_interrupt
    if (op_cpu_interrupt != OP_HANDSHAKE)
    {
        //printf("ERROR al conectar kernel con CPU_interrupt!\n");
        terminar_programa();
        // return;
        abort();
    }
    //printf("Se conectó KERNEL a CPU_interrupt!\n");
}



// PLANIFICADOR A LARGO PLAZO //
void gestionar_peticiones_cpu_dispatch()
{
    // atender a cpu
    while (1)
    {
        enum op_code op = recibir_operacion(socket_cpu_dispatch);
       
        switch (op)
        {

        case OP_CREAR_PROCESO:
        {
            t_list *lista_recibida = recibir_paquete(socket_cpu_dispatch);
            log_info(logger, "## (%i : %i) - Solicitó syscall: PROCESS_CREATE", hilo_EXEC->PID, hilo_EXEC->TID);
            crear_proceso((char *)list_get(lista_recibida, 0), *(int *)list_get(lista_recibida, 1), *(int *)list_get(lista_recibida, 2)); // Le pasamos el pseudocodigo, el tamanio y prioridad del tid0
            list_destroy_and_destroy_elements(lista_recibida, free);
            enviar_a_ejecucion(hilo_EXEC);
            break;
        }
        case OP_FINALIZAR_PROCESO:
        {
            log_info(logger, "## (%i : %i) - Solicitó syscall: PROCESS_EXIT", hilo_EXEC->PID, hilo_EXEC->TID);
            finalizar_proceso();
            break;
        }
        case OP_CREAR_HILO:
        {
            t_list *lista_recibida = recibir_paquete(socket_cpu_dispatch);
            log_info(logger, "## (%i : %i) - Solicitó syscall: THREAD_CREATE", hilo_EXEC->PID, hilo_EXEC->TID);

            char *script_original = (char *)list_get(lista_recibida, 0);
            char *script_duplicado = string_duplicate(script_original);

            crear_hilo(script_duplicado, *(int *)list_get(lista_recibida, 1)); // le pasamos el pseudocodigo y la prioridad
            list_destroy_and_destroy_elements(lista_recibida, free);
            enviar_a_ejecucion(hilo_EXEC);
            break;
        }
        case OP_THREAD_JOIN:
        {
            int tid = recibir_int(socket_cpu_dispatch);
            log_info(logger, "## (%i : %i) - Solicitó syscall: THREAD_JOIN", hilo_EXEC->PID, hilo_EXEC->TID);
            bloquear_hilo_por_thread_join(tid); // Le pasamos el TID del hilo a ejecutar.
            break;
        }
        case OP_CANCELAR_HILO: // Finaliza a un hilo x, y acordarnos de desbloquear a los hilos que bloqueo
        {
            int tid = recibir_int(socket_cpu_dispatch);
            log_info(logger, "## (%i : %i) - Solicitó syscall: THREAD_CANCEL", hilo_EXEC->PID, hilo_EXEC->TID);
            finalizar_hilo(hilo_EXEC->PID, tid); // Le pasamos el TID del hilo a finalizar
            break;
        }
        case OP_FINALIZAR_HILO: // Finaliza al hilo en ejecución y acordarnos de desbloquear a los hilos que bloqueo
        {
            log_info(logger, "## (%i : %i) - Solicitó syscall: THREAD_EXIT", hilo_EXEC->PID, hilo_EXEC->TID);
            finalizar_hilo(hilo_EXEC->PID, hilo_EXEC->TID); // Le pasamos el TID del hilo en ejecucion. Aunque sea global lo hacemos por polimorfismo.
            break;
        }
        case OP_MUTEX_CREATE:
        {
            char *nombre = recibir_string(socket_cpu_dispatch);
            log_info(logger, "## (%i : %i) - Solicitó syscall: MUTEX_CREATE", hilo_EXEC->PID, hilo_EXEC->TID);
            crear_mutex(nombre); // Le pasamos el nombre del recurso
            break;
        }
        case OP_MUTEX_LOCK:
        {
            char *nombre = recibir_string(socket_cpu_dispatch);
            log_info(logger, "## (%i : %i) - Solicitó syscall: MUTEX_LOCK", hilo_EXEC->PID, hilo_EXEC->TID);
            bloquear_mutex(nombre); // Le pasamos el nombre del recurso 
            free(nombre);
            break;
        }

        case OP_MUTEX_UNLOCK:
        {
            char *nombre = recibir_string(socket_cpu_dispatch);
            log_info(logger, "## (%i : %i) - Solicitó syscall: MUTEX_UNLOCK", hilo_EXEC->PID, hilo_EXEC->TID);
            desbloquear_mutex(nombre); // Le pasamos el nombre del recurso
            free(nombre);
            break;
        }
        case OP_DUMP_MEM:
        {
            log_info(logger, "## (%i : %i) - Solicitó syscall: DUMP_MEMORY", hilo_EXEC->PID, hilo_EXEC->TID);
            volcado_memoria(); // Solo trabaja con hilo_EXEC;
            break;
        }
        case OP_IO:
        {
            int tiempo = recibir_int(socket_cpu_dispatch);
            log_info(logger, "## (%i : %i) - Solicitó syscall: IO", hilo_EXEC->PID, hilo_EXEC->TID);
            bloquear_hilo_por_io(tiempo); // Le pasamos el tiempo en milisegundos
            break;
        }
        case OP_INTERRUPT:
        {
            int tid = recibir_int(socket_cpu_dispatch);
            cambiar_hilo_exec_fin_Q(tid); // Le pasamos el tid
            break;
        }
        case OP_SEGMENTATION_FAULT:
        {
            __attribute_maybe_unused__ int entero = recibir_int(socket_cpu_dispatch);
            
            log_info(logger, "## (%i : %i) - Solicitó syscall: PROCESS_EXIT", hilo_EXEC->PID, hilo_EXEC->TID);
            finalizar_proceso();

            break;
        }
        default:
            break;
        }
    }
}

/*
void volcado_memoria()
{
  
    int socket = enviar_a_memoria(OP_DUMP_MEM, hilo_EXEC);
    t_TCB* tcb = hilo_EXEC;

    queue_push(cola_hilos_BLOCK, hilo_EXEC); // "Esta syscall bloqueará al hilo que la invocó hasta que el módulo memoria confirme la finalización de la operación"
    
    elegir_hilo();

    recibir_operacion(socket);
    int rta_mem = recibir_int(socket);      

    tcb = buscar_tcb_en_block(tcb -> PID, tcb -> TID);
    
    if (rta_mem == 1){  
        tcb = buscar_y_eliminar_tcb_en_block(tcb -> PID, tcb -> TID);        
        agregar_hilo_a_ready(tcb); // Agrega el hilo a la cola READY correspondiente contemplando el algoritmo de planificacion
        if(hilo_EXEC == NULL){
            elegir_hilo();
        }

    }else{                                      
        t_PCB *proceso_a_manejar;
        proceso_a_manejar = buscar_proceso_READY(tcb -> PID);
        int socket_2 = enviar_pid_a_memoria(OP_FINALIZAR_PROCESO, tcb);  // Debe finalizar el proceso relacionado al hilo_previamente_bloqueado
    
        recibir_operacion(socket_2);
        int rta = recibir_int(socket_2);

        if (rta != 1){
            return;
        }

        for(int i=0; i<list_size(proceso_a_manejar->TIDS); i++){
            int* tid_ptr = (int*) list_get(proceso_a_manejar->TIDS, i); // Obtén el puntero
            int tid = *tid_ptr; // Desreferencia para obtener el valor real
            eliminar_hilo_de_un_proceso_a_finalizar(proceso_a_manejar->PID, tid);

        }

        list_remove_element(cola_procesos_READY->elements, proceso_a_manejar);
        queue_push(cola_procesos_EXIT, proceso_a_manejar);
        log_info(logger, "## Finaliza el proceso %i\n", proceso_a_manejar -> PID);

        list_clean_and_destroy_elements(cola_hilos_EXIT->elements, (void (*)(void*)) liberar_tcb);

        liberar_pcb(proceso_a_manejar);

        revisar_cola_procesos_new();   
         
        if(hilo_EXEC == NULL){
            elegir_hilo();
        }
    }   
}
*/




void volcado_memoria()
{

    queue_push(cola_hilos_BLOCK, hilo_EXEC); // "Esta syscall bloqueará al hilo que la invocó hasta que el módulo memoria confirme la finalización de la operación"
    
    int *TID_de_hilo_bloqueado = &(hilo_EXEC->TID);
    int *PID_de_hilo_bloqueado = &(hilo_EXEC->PID);

    argumentos_volcado_t *args = malloc(sizeof(argumentos_volcado_t));
    args->TID_de_hilo_bloqueado = *TID_de_hilo_bloqueado;
    args->PID_de_hilo_bloqueado = *PID_de_hilo_bloqueado;
    args->socket = -1;

    queue_push(cola_hilos_BLOCK_DUMP_MEMORY, args);
    revisar_cola_dump();

    elegir_hilo();
}


void revisar_cola_dump() {
    argumentos_volcado_t *args = NULL;

    if (hilo_DUMP_MEMORY == NULL && queue_size(cola_hilos_BLOCK_DUMP_MEMORY) > 0) {
        args = queue_pop(cola_hilos_BLOCK_DUMP_MEMORY);
        hilo_DUMP_MEMORY = args;
    }

    if (args != NULL) {
        int socket = enviar_a_memoria(OP_DUMP_MEM, hilo_EXEC);
        args->socket = socket;

        pthread_t hilo_volcado;
        pthread_create(&hilo_volcado, NULL, (void *)revisar_rta_volcado, ((void *)args));
        pthread_detach(hilo_volcado);
    }
}


void revisar_rta_volcado(void *arg){
    //printf("Entro a revisar rta volcado\n");
    argumentos_volcado_t *args = (argumentos_volcado_t *)arg;

    int TID_de_hilo_bloqueado = args->TID_de_hilo_bloqueado;
    int PID_de_hilo_bloqueado = args->PID_de_hilo_bloqueado;
    int socket = args->socket;
    free(args);
    hilo_DUMP_MEMORY = NULL;

    recibir_operacion(socket);
    int rta_mem = recibir_int(socket);      

    t_TCB *hilo_previamente_bloqueado;

    hilo_previamente_bloqueado = buscar_tcb_en_block(PID_de_hilo_bloqueado, TID_de_hilo_bloqueado);

    hilo_DUMP_MEMORY = NULL;
    revisar_cola_dump();
    if (rta_mem == 1){  

        hilo_previamente_bloqueado = buscar_y_eliminar_tcb_en_block(PID_de_hilo_bloqueado, TID_de_hilo_bloqueado);        
        agregar_hilo_a_ready(hilo_previamente_bloqueado); // Agrega el hilo a la cola READY correspondiente contemplando el algoritmo de planificacion
        if(hilo_EXEC == NULL){
            elegir_hilo();
        }

    }else{        

        t_PCB *proceso_a_manejar;

        proceso_a_manejar = buscar_proceso_READY(hilo_previamente_bloqueado -> PID);
        int socket_2 = enviar_pid_a_memoria(OP_FINALIZAR_PROCESO, hilo_previamente_bloqueado);  // Debe finalizar el proceso relacionado al hilo_previamente_bloqueado

        recibir_operacion(socket_2);
        int rta = recibir_int(socket_2);

        if (rta != 1){
            return;
        }

        for(int i=0; i<list_size(proceso_a_manejar->TIDS); i++){
            int* tid_ptr = (int*) list_get(proceso_a_manejar->TIDS, i); // Obtén el puntero
            int tid = *tid_ptr; // Desreferencia para obtener el valor real
            eliminar_hilo_de_un_proceso_a_finalizar(proceso_a_manejar->PID, tid);
        }
    
        list_remove_element(cola_procesos_READY->elements, proceso_a_manejar);
        queue_push(cola_procesos_EXIT, proceso_a_manejar);
        log_info(logger, "## Finaliza el proceso %i\n", proceso_a_manejar -> PID);

        list_clean_and_destroy_elements(cola_hilos_EXIT->elements, (void (*)(void*)) liberar_tcb);
        liberar_pcb(proceso_a_manejar);

        revisar_cola_procesos_new();   

        if(hilo_EXEC == NULL){
            elegir_hilo();
        }

    }   

}