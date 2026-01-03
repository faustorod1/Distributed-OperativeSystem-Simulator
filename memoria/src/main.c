#include "main.h"

void retardo();

int main(int argc, char* argv[]) {
    //Segun el numero de prueba se carga el config completo
    if(!strcmp(argv[1], "-1") || !strcmp(argv[1], "-1.1" ) || !strcmp(argv[1], "-1.2") || !strcmp(argv[1], "-1.3")){
        archivo_configuracion = "plani";
    }else if(!strcmp(argv[1], "-2") || !strcmp(argv[1], "-2.1") || !strcmp(argv[1], "-2.2")){
        archivo_configuracion = "race";
    }else if(!strcmp(argv[1], "-3.1") ){
        archivo_configuracion = "fijas-first";
    }else if(!strcmp(argv[1], "-3.2")){
        archivo_configuracion = "fijas-best";
    }else if(!strcmp(argv[1], "-3.3")){
        archivo_configuracion = "fijas-worst";
    }else if(!strcmp(argv[1], "-4")){
        archivo_configuracion = "part-dinam";
    }else if(!strcmp(argv[1], "-5")){
        archivo_configuracion = "fs";
    }else if(!strcmp(argv[1], "-6")){
        archivo_configuracion = "stress";
    }else if(argv[1] == NULL){//si no pasamos parametros
        archivo_configuracion = "memoria";
    }
    iniciar_config();
    iniciar_logger();

    iniciar_estructuras();

    //Para testear
    //crear_proceso(1, 10);
    //crear_hilo(1, 0, "Prueba.txt");

    //inicia memoria como servidor de kernel y cpu y como cliente de filesystem
    iniciar_conexion();
    

    
    terminar_programa();
    return 0;
}

void terminar_programa(){
    list_destroy_and_destroy_elements(configuracion->PARTICIONES, free);
    free(configuracion);
    log_destroy(logger);
}

void gestionar_peticiones_cpu(void *dir_socket){
    int socket_cpu = *((int*) dir_socket);
    free(dir_socket);
    while (true) {
        enum op_code op = recibir_operacion(socket_cpu);
        switch (op) {
        case OP_MEMCONTEXT:
            {
                t_list *datos = recibir_paquete(socket_cpu);
                int pid = *((int*) list_get(datos, 0));
                int tid = *((int*) list_get(datos, 1));
                list_destroy_and_destroy_elements(datos, free);

                t_cde cde = obtenerCDE(pid, tid);

                t_paquete *paquete = crear_paquete(OP_MEMCONTEXT);
                agregar_a_paquete(paquete, &(cde.base), sizeof(cde.base));
                agregar_a_paquete(paquete, &(cde.limite), sizeof(cde.limite));
                empaquetar_registros(paquete, cde.registros);

                retardo();
                log_info(logger, "## Contexto Solicitado - (PID:TID) - (%d:%d)", pid, tid);
                enviar_paquete(paquete, socket_cpu);
                eliminar_paquete(paquete);
            }
            break;
        case OP_MEMCONTEXT_UPDATE:
            {
                t_list *datos = recibir_paquete(socket_cpu);
                int pid = *((int*) list_get(datos, 0));
                int tid = *((int*) list_get(datos, 1));
                uint32_t registros[9];
                desempaquetar_registros(datos, registros, 2);
                list_destroy_and_destroy_elements(datos, free);

                bool rta = actualizarCDE(pid, tid, registros);
                retardo();
                log_info(logger, "## Contexto Actualizado - (PID:TID) - (%d:%d)", pid, tid);
                enviar_int(rta, socket_cpu, OP_MEMCONTEXT_UPDATE);
            }
            break;
        case OP_MEMFETCH:
            {
                t_list *datos = recibir_paquete(socket_cpu);
                int pid = *((int*) list_get(datos, 0));
                int tid = *((int*) list_get(datos, 1));
                int pc = *((int*) list_get(datos, 2));
                list_destroy_and_destroy_elements(datos, free);

                char *instr = obtenerInstruccion(pid, tid, pc);

                retardo();
                log_info(logger, "## Obtener instrucción - (PID:TID) - (%d:%d) - Instrucción: %s", pid, tid, instr);
                enviar_string(instr, socket_cpu, OP_MEMFETCH);
            }
            break;
        case OP_READ_MEM:
            {
                t_list *datos = recibir_paquete(socket_cpu);
                int pid = *((int*) list_get(datos, 0));
                int tid = *((int*) list_get(datos, 1));
                int direccion = *((int*) list_get(datos, 2));
                list_destroy_and_destroy_elements(datos, free);

                uint32_t valor = 0;
                valor = read_mem(direccion);

                retardo();
                log_info(logger, "## Lectura - (PID:TID) - (%d:%d) - Dir. Física: %d - Tamaño: 4", pid, tid, direccion);
                enviar_int(valor, socket_cpu, OP_READ_MEM);
            }
            break;
        case OP_WRITE_MEM:
            {
                t_list *datos = recibir_paquete(socket_cpu);
                int pid = *((int*) list_get(datos, 0));
                int tid = *((int*) list_get(datos, 1));
                int direccion = *((int*) list_get(datos, 2));
                int valor = *((int*) list_get(datos, 3));
                list_destroy_and_destroy_elements(datos, free);


                write_mem(direccion, valor);
                //printf("%d\n", valor);

                retardo();
                log_info(logger, "## Escritura - (PID:TID) - (%d:%d) - Dir. Física: %d - Tamaño: 4", pid, tid, direccion);
                enviar_int(1, socket_cpu, OP_WRITE_MEM);
            }
            break;
        default:
            break;
        }
    }
}

void gestionar_peticion_kernel(void *dir_socket){
    int socket_cliente = *(int*)dir_socket;
    free(dir_socket);
    enum op_code op = recibir_operacion(socket_cliente);

    switch (op) {
    case OP_CREAR_PROCESO:
        {
            t_list *datos = recibir_paquete(socket_cliente);
            int pid = *((int*) list_get(datos, 0));
            int tam = *((int*) list_get(datos, 1));
            list_destroy_and_destroy_elements(datos, free);

            int rta = 0;
            if (crear_proceso(pid, tam) != NULL) {
                rta = 1;
                log_info(logger, "## Proceso Creado -  PID: %d - Tamaño: %d", pid, tam);
            }
            
            
            enviar_int(rta, socket_cliente, OP_CREAR_PROCESO);
        }
        break;
    case OP_INICIAR_HILO:
        {
            t_list *datos = recibir_paquete(socket_cliente);
            int pid = *((int*) list_get(datos, 0));
            int tid = *((int*) list_get(datos, 1));
            char *scriptname = list_get(datos, 2);

            int rta = 0;
            if (crear_hilo(pid, tid, scriptname) != NULL) {
                rta = 1;
                log_info(logger, "## Hilo Creado - (PID:TID) - (%d:%d)", pid, tid);
            }
            list_destroy_and_destroy_elements(datos, free);
            enviar_int(rta, socket_cliente, OP_INICIAR_HILO);
        }
        break;
    case OP_FINALIZAR_PROCESO:
        {
            t_list *datos = recibir_paquete(socket_cliente);
            int pid = *((int*) list_get(datos, 0));
            list_destroy_and_destroy_elements(datos, free);

            log_info(logger, "## Proceso Destruido -  PID: %d - Tamaño: %d", pid, buscar_proceso(pid)->tamanio);
            
            finalizar_proceso(pid);

            enviar_int(1, socket_cliente, OP_FINALIZAR_PROCESO);
        }
        break;
    case OP_FINALIZAR_HILO:
        {
            t_list *datos = recibir_paquete(socket_cliente);
            int pid = *((int*) list_get(datos, 0));
            int tid = *((int*) list_get(datos, 1));
            list_destroy_and_destroy_elements(datos, free);

            finalizar_hilo(pid, tid);

            log_info(logger, "## Hilo Destruido - (PID:TID) - (%d:%d)", pid, tid);
            enviar_int(1, socket_cliente, OP_FINALIZAR_HILO);
        }
        break;
    case OP_DUMP_MEM:
        {
            t_list *datos = recibir_paquete(socket_cliente);
            int pid = *((int*) list_get(datos, 0));
            int tid = *((int*) list_get(datos, 1));
            list_destroy_and_destroy_elements(datos, free);

            log_info(logger, "## Memory Dump solicitado - (PID:TID) - (%d:%d)", pid, tid);
            int rta = memory_dump(pid, tid);

            if (rta == 0){
                //printf("Error en File System al realizar Memory Dump del (%i,%i)\n", pid, tid);
            }

            enviar_int(rta, socket_cliente, OP_DUMP_MEM);
        }
        break;
    default:
        break;
    }
}

void iniciar_conexion(){
    char *puerto_str;
    enum op_code op;

    // Inicia MEMORIA conexión con FILESYSTEM como cliente
    /* puerto_str = string_itoa(configuracion->PUERTO_FILESYSTEM);
    int socket_fs = crear_conexion(configuracion->IP_FILESYSTEM, puerto_str);
    free(puerto_str);

    enviar_int(MOD_MEMORIA,socket_fs,OP_HANDSHAKE);
    op = recibir_operacion(socket_fs);
    if (op != OP_HANDSHAKE) {
        //printf("ERROR al concectar Memoria con FileSystem!\n");
        terminar_programa();
        return;
    }
    //printf("Se conectó Memoria a Filesystem!\n"); */


    puerto_str = string_itoa(configuracion->PUERTO_ESCUCHA);
    int socket_escucha = iniciar_servidor(puerto_str); 
    free(puerto_str);


    //inicio server para CPU
    int socket_cpu = esperar_cliente(socket_escucha);
    op = recibir_operacion(socket_cpu);
    int modulo = recibir_int(socket_cpu);
    if (op == OP_HANDSHAKE && modulo == MOD_CPU) {
        //printf("Se conectó CPU a Memoria!\n");
        enviar_operacion(socket_cpu, OP_HANDSHAKE);

        int *ptr_socket_cpu = malloc(sizeof(*ptr_socket_cpu));
        *ptr_socket_cpu = socket_cpu;
        pthread_t hilo_server_cpu; // uso hilo para cpu
        pthread_create(&hilo_server_cpu, NULL, (void*)gestionar_peticiones_cpu, (void*) ptr_socket_cpu);
        pthread_detach(hilo_server_cpu);
    }else{
        //printf("ERROR al conectar CPU a Memoria!\n");
        enviar_operacion(socket_cpu, OP_ERROR);
        terminar_programa();
        return;
    }
    

    // Iniciamos server para KERNEL
    while (true) {
        int socket_kernel = esperar_cliente(socket_escucha);   // Socket cambiado a socket_mem
        log_info(logger, "## Kernel Conectado - FD del socket: %i", socket_kernel);

        int *ptr_socket_kernel = malloc(sizeof(*ptr_socket_kernel));
        *ptr_socket_kernel = socket_kernel;

        pthread_t hilo_server_kernel; // uso hilo para kernel
        pthread_create(&hilo_server_kernel, NULL, (void*)gestionar_peticion_kernel, (void*) ptr_socket_kernel);
        pthread_detach(hilo_server_kernel);

        // Versión del código que hace handshake en cada conexión
        // int socket_kernel = esperar_cliente(socket_escucha);   // Socket cambiado a socket_mem
        // enum op_code op = recibir_operacion(socket_kernel);
        // int modulo = recibir_int(socket_kernel);
        // if (op == OP_HANDSHAKE && modulo == MOD_KERNEL) {
        // log_info(logger, "## Kernel Conectado - FD del socket: %i", socket_kernel);
        //     enviar_operacion(socket_kernel, OP_HANDSHAKE);

        //     pthread_t hilo_server_kernel; // uso hilo para kernel
        //     pthread_create(&hilo_server_kernel, NULL, (void*)gestionar_peticion_kernel, (void*) &socket_kernel);
        //     pthread_detach(hilo_server_kernel);
        // }else{
        //     printf("ERROR al conectar Kernel con Memoria!\n");
        //     enviar_operacion(socket_kernel, OP_ERROR);
        //     terminar_programa();
        //     return;
        // }
    }
}


void retardo() {
    usleep(configuracion->RETARDO_RESPUESTA * 1000);
}