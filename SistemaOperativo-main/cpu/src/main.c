#include "main.h"

int main(int argc, char* argv[]) {
    iniciar_config();
    iniciar_logger();

    //inicia CPU conexion con MEMORIA como cliente y se levanta como servidor para kernel dos veces
    iniciar_conexion();

    //creo hilo para recibir interrupciones del kernel
    crear_hilo();

    while(true){

        if (!ejecutando_proceso) {
            interrupt_flag = false;
            
            /*
            t_list* lista_recibida = recibir_paquete(socket_cpu_dispatch);
            enum op_code op = (enum op_code)(intptr_t)list_get(lista_recibida, 0);
            */
           
            enum op_code op = recibir_operacion(socket_kernel_dis);
            t_list* proceso_kernel = recibir_paquete(socket_kernel_dis);
            if (op == OP_EJECUTAR_HILO) {
                PID = *(int*) list_get(proceso_kernel, 0);
                TID = *(int*) list_get(proceso_kernel, 1);

                pedir_contexto();

                list_destroy_and_destroy_elements(proceso_kernel, free);

                ejecutando_proceso = true;
            }
        }

        //fetch
        char* proxima_instruccion = fetch(registros[PC]);
        log_info(logger,"## TID: %i - FETCH - Program Counter: %i",TID,registros[PC]);
        registros[PC]++;

        //decode
        char** lista_instruccion = string_split(proxima_instruccion, " ");

        //execute
        execute(lista_instruccion);

        // Log de execute
        char **parametros_aux = string_n_split(proxima_instruccion, 2, " ");
        if (parametros_aux[1] != NULL) {
            log_info(logger, "## TID: %d - Ejecutando: %s - %s ", TID, parametros_aux[0], parametros_aux[1]);
        }
        else {
            log_info(logger, "## TID: %d - Ejecutando: %s ", TID, parametros_aux[0]);
        }
        string_array_destroy(parametros_aux);

        if (recuperar_contexto_urgente) {
            recuperar_contexto_urgente = false;
            enum op_code op = recibir_operacion(socket_kernel_dis);
            t_list* proceso_kernel = recibir_paquete(socket_kernel_dis);
            if (op == OP_EJECUTAR_HILO) {
                PID = *(int*) list_get(proceso_kernel, 0);
                TID = *(int*) list_get(proceso_kernel, 1);

                pedir_contexto();

                list_destroy_and_destroy_elements(proceso_kernel, free);

                ejecutando_proceso = true;
            }
        }

        //check interrupt
        if(interrupt_flag && ejecutando_proceso){
            actualizar_contexto();
            motivo(OP_INTERRUPT);
            interrupt_flag = false;
        }

        free(proxima_instruccion);
        string_array_destroy(lista_instruccion);

        // for (int i = 0; i < 9; i++) {
        //     printf("%d  ", registros[i]);
        // }
        // printf("\n");
    }
    terminar_programa();
    return 0;
}

void terminar_programa(){
    log_destroy(logger);
    free(configuracion);
}

void iniciar_conexion(){
    char *puerto_str;
    enum op_code op;

    // inicio CPU como cliente de MEMORIA
    puerto_str = string_itoa(configuracion->PUERTO_MEMORIA);
    int socket_memoria = crear_conexion(configuracion->IP_MEMORIA, puerto_str);
    free(puerto_str);

    enviar_int(MOD_CPU,socket_memoria,OP_HANDSHAKE);
    op = recibir_operacion(socket_memoria);
    if (op != OP_HANDSHAKE) {
        //printf("ERROR al conectar CPU con MEMORIA!\n");
        terminar_programa();
        return;                       // La funcion está declarada como void. No puede retornar ningun valor
    }
    //printf("Se conectó CPU a MEMORIA!\n");
    socket_mem = socket_memoria;
    
    //inicia server para KERNEL - Dispatch
    int modulo;
    puerto_str = string_itoa(configuracion->PUERTO_ESCUCHA_DISPATCH); // Cambio de nombre al puerto
    int socket_escucha_dispatch = iniciar_servidor(puerto_str);
    free(puerto_str);
    
    int socket_kernel_dispatch = esperar_cliente(socket_escucha_dispatch);
    op = recibir_operacion(socket_kernel_dispatch);
    modulo = recibir_int(socket_kernel_dispatch);
    if (op == OP_HANDSHAKE && modulo == MOD_KERNEL) {
        //printf("Se conectó Kernel a CPU!\n");
        enviar_operacion(socket_kernel_dispatch, OP_HANDSHAKE);
        socket_kernel_dis = socket_kernel_dispatch;
        //kernel enviará el proceso

    }else{
        //printf("Error al conectar Kernel a CPU!\n");
        enviar_operacion(socket_kernel_dispatch, OP_ERROR);
        terminar_programa();
        return;
    }

     //inicia server para KERNEL - Interrupt
    puerto_str = string_itoa(configuracion->PUERTO_ESCUCHA_INTERRUPT); // Cambio de nombre al puerto
    int socket_escucha_interrupt = iniciar_servidor(puerto_str);
    free(puerto_str);

    int socket_kernel_interrupt = esperar_cliente(socket_escucha_interrupt); 
    op = recibir_operacion(socket_kernel_interrupt);
    modulo = recibir_int(socket_kernel_interrupt);
    if (op == OP_HANDSHAKE && modulo == MOD_KERNEL) {
        //printf("Se conectó Kernel a CPU!\n");
        enviar_operacion(socket_kernel_interrupt, OP_HANDSHAKE);
        socket_kernel_int = socket_kernel_interrupt;
    }else{
        //printf("Error al conectar Kernel a CPU!\n");
        enviar_operacion(socket_kernel_interrupt, OP_ERROR);
        terminar_programa();
        return;
    }
    
}

void pedir_contexto(){
    t_paquete* paquete = crear_paquete(OP_MEMCONTEXT);
    agregar_a_paquete(paquete, &PID, sizeof(PID));
    agregar_a_paquete(paquete, &TID, sizeof(TID));
    
    int bytes = enviar_paquete(paquete, socket_mem);
    if (bytes == 0) {
        //printf("Error al enviar paquete a MEMORIA!\n");
        eliminar_paquete(paquete);
        terminar_programa();
        return;
    }
    eliminar_paquete(paquete);

    log_info(logger,"## TID: %i - Solicito Contexto Ejecución.", TID);
    recibir_operacion(socket_mem); // op
    t_list* contexto_de_ejecucion = recibir_paquete(socket_mem);
    base = *(int*) list_get(contexto_de_ejecucion, 0);
    limite = *(int*) list_get(contexto_de_ejecucion, 1);
    desempaquetar_registros(contexto_de_ejecucion,registros,2);

    list_destroy_and_destroy_elements(contexto_de_ejecucion, free);
}

void crear_hilo(){
    pthread_t hilo;
    pthread_create(&hilo, NULL, (void*) recibir_interrupciones, NULL);
    pthread_detach(hilo);
}

void recibir_interrupciones(){//Para q precisa el tid?
    enum op_code op;
    int tid_interrupcion;
    while(true){
        op = recibir_operacion(socket_kernel_int);
        if (op == OP_INTERRUPT) {
            tid_interrupcion = recibir_int(socket_kernel_int);
            if (tid_interrupcion == TID) {
                log_info(logger,"## Llega interrupción al puerto Interrupt");
                interrupt_flag = true;
            }
        }
    }
}

char* fetch(uint32_t PC){

    t_paquete* paquete = crear_paquete(OP_MEMFETCH);

    agregar_a_paquete(paquete, &PID, sizeof(PID));
    agregar_a_paquete(paquete, &TID, sizeof(TID));
    agregar_a_paquete(paquete, &PC, sizeof(PC));
    
    int bytes = enviar_paquete(paquete, socket_mem);
    eliminar_paquete(paquete);
    if (bytes == 0) {
        //printf("Error al enviar paquete a MEMORIA!\n");
        terminar_programa();
        return NULL;
    }
    recibir_operacion(socket_mem);
    return recibir_string(socket_mem); // a verificar , pueden enviarons un paquete
}