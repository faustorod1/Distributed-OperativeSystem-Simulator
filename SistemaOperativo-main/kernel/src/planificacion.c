#include "planificacion.h"

int64_t timestamp_q_actual = 0;
hilo_io_args_t *hilo_IO = NULL;

void elegir_hilo()
{
    hilo_EXEC = NULL;
    if (!strcmp(configuracion->ALGORITMO_PLANIFICACION, "FIFO"))
    {
        if (!queue_is_empty(cola_hilos_READY))
        {

            hilo_EXEC = queue_pop(cola_hilos_READY);

            enviar_a_ejecucion(hilo_EXEC);
        }
    }
    else if (!strcmp(configuracion->ALGORITMO_PLANIFICACION, "PRIORIDADES"))
    {
        if (!queue_is_empty(cola_hilos_READY))
        {

            int cantidad_de_hilos = queue_size(cola_hilos_READY);
            int posicion_elegida = 0;
            t_TCB *tcb_elegido = queue_peek(cola_hilos_READY);
            t_TCB *tcb_actual;

            for (int i = 1; i < cantidad_de_hilos; i++)
            { // Comenzamos desde el segundo hilo
                tcb_actual = list_get(cola_hilos_READY->elements, i);

                if (tcb_actual->prioridad < tcb_elegido->prioridad)
                { // Esto respeta FIFO
                    tcb_elegido = tcb_actual;
                    posicion_elegida = i;
                }
            }
            hilo_EXEC = list_remove(cola_hilos_READY->elements, posicion_elegida);

            // int cantidad_de_procesos = queue_size(cola_procesos_READY);
            // t_PCB *proceso_elegido;

            // for (int i = 0; i < cantidad_de_procesos; i++)
            // {
            //     proceso_elegido = list_get(cola_procesos_READY->elements, i);

            //     // if (proceso_elegido->PID == hilo_EXEC->PID)
            //     // {
            //     //     proceso_EXEC = list_remove(cola_procesos_READY->elements, i);
            //     //     break;
            //     // }
            // }

            enviar_a_ejecucion(hilo_EXEC);
        }
    }
   else if (!strcmp(configuracion->ALGORITMO_PLANIFICACION, "CMN"))
    {
        
        if (!list_is_empty(colas_multi_level))
        {
            for (int i = 0; i < list_size(colas_multi_level); i++)
            {

                if (!queue_is_empty(list_get(colas_multi_level, i)))
                { // Toma el primer hilo (FIFO) de la cola con mayor prioridad
                    hilo_EXEC = queue_pop(list_get(colas_multi_level, i));
                    enviar_a_ejecucion(hilo_EXEC);
                    iniciar_conteo_q();
                    break;
                }
            }
        }
    }
}



void enviar_a_ejecucion(t_TCB *tcb)
{
    t_paquete *paquete = crear_paquete(OP_EJECUTAR_HILO);

    agregar_a_paquete(paquete, &(tcb->PID), sizeof(int));
    agregar_a_paquete(paquete, &(tcb->TID), sizeof(int));

    enviar_paquete(paquete, socket_cpu_dispatch);

    eliminar_paquete(paquete);
}

void cambiar_hilo_exec_fin_Q(int tid)
{ // cuando cpu nos avisa que ya desalojo al hilo, les mandamos uno nuevo.
    log_info(logger, "## (%i : %i) - Desalojado por fin de Quantum", hilo_EXEC->PID, tid);

    agregar_a_cola_ready_correspondiente(hilo_EXEC);

    elegir_hilo();
}

void bloquear_hilo_por_io(int tiempo)
{
    queue_push(cola_hilos_BLOCK, hilo_EXEC);
    t_TCB *hilo_bloqueado = hilo_EXEC;
    log_info(logger, "## (%i : %i) - Bloqueado por: IO", hilo_EXEC->PID, hilo_EXEC->TID);

    hilo_io_args_t *args = malloc(sizeof(hilo_io_args_t));
    args->tiempo = tiempo;
    args->hilo_bloqueado = hilo_bloqueado;

    queue_push(cola_hilos_BLOCK_IO, args);
    revisar_cola_IO();
    
    elegir_hilo();

    
}

void revisar_cola_IO() {
    void *args = NULL;

    if (hilo_IO == NULL && queue_size(cola_hilos_BLOCK_IO) > 0) {
        args = queue_pop(cola_hilos_BLOCK_IO);
        hilo_IO = args;
    }

    if (args != NULL) {
        pthread_t hilo_io;
        pthread_create(&hilo_io, NULL, (void *)revisar_fin_io, (void *)args);
        pthread_detach(hilo_io);
    }
}

void revisar_fin_io(void *arg)
{
    hilo_io_args_t *args = (hilo_io_args_t *)arg;
    int tiempo = args->tiempo;
    t_TCB *hilo_bloqueado = args->hilo_bloqueado;
    free(args);

    usleep(tiempo * 1000);

    desbloquear_hilo(hilo_bloqueado->TID);

    agregar_hilo_a_ready(hilo_bloqueado); // Agrega el hilo a la cola READY correspondiente contemplando el algoritmo de panificacion

    hilo_IO = NULL;
    revisar_cola_IO();
    if (hilo_EXEC == NULL){
        elegir_hilo();
    }

    log_info(logger, "## (%i : %i) finalizó IO y pasa a READY", hilo_bloqueado->PID, hilo_bloqueado->TID);
}

void desbloquear_hilo(int tid)
{
    for (int i = 0; i < queue_size(cola_hilos_BLOCK); i++)
    {
        t_TCB *tcb = list_get(cola_hilos_BLOCK->elements, i);
        if (tcb->TID == tid)
        {
            list_remove(cola_hilos_BLOCK->elements, i);
            break;
        }
    }
}

void agregar_hilo_a_ready(t_TCB *tcb)
{
    if (!strcmp(configuracion->ALGORITMO_PLANIFICACION, "CMN"))
    {
        agregar_a_cola_ready_correspondiente(tcb);
    }
    else
    {
        queue_push(cola_hilos_READY, tcb);
    }
}

void agregar_a_cola_ready_correspondiente(t_TCB *tcb)
{
    while (tcb->prioridad >= list_size(colas_multi_level))
    {
        list_add(colas_multi_level, queue_create());
    }
    queue_push(list_get(colas_multi_level, tcb->prioridad), tcb); // Se tendra una cola por cada nivel de prioridad existente entre los hilos del sistema
}

void bloquear_hilo_por_thread_join(int tid)
{
    t_TCB* tcb_actual;

    for(int i = 0; i < list_size(lista_hilos); i++){
        tcb_actual = list_get(lista_hilos, i);
        if(tcb_actual -> TID == tid && tcb_actual -> PID == hilo_EXEC -> PID){
            int *tid_ptr = malloc(sizeof(int));
            *tid_ptr = hilo_EXEC -> TID;
            list_add(tcb_actual->hilos_bloqueados, tid_ptr); // dejamos constancia a q hilos bloquea el hilo pasado por prametro para que al momento de finalizarlo los desbloquee
            queue_push(cola_hilos_BLOCK, hilo_EXEC);
            log_info(logger, "## (%i : %i) - Bloqueado por: PTHREAD_JOIN", hilo_EXEC->PID, hilo_EXEC->TID);
            enviar_bloqueante_y_esperar(BLOQUEANTE);
            elegir_hilo();
            return;
        }
    }
    enviar_bloqueante_y_esperar(NO_BLOQUEANTE);
    enviar_a_ejecucion(hilo_EXEC); //si el hilo pasado por parametro finalizo o no existe


}


void iniciar_conteo_q() {
    int64_t *timestamp = malloc(sizeof(*timestamp));
    *timestamp = temporal_gettime(contador_global);

    timestamp_q_actual = *timestamp;

    pthread_t hilo_fin_q;
    pthread_create(&hilo_fin_q, NULL, (void *)desalojar_hilo_fin_Q, (void*) timestamp);
    pthread_detach(hilo_fin_q);
}


void desalojar_hilo_fin_Q(void *timestamp_p) {
    int64_t timestamp = *(int64_t*) timestamp_p;
    free(timestamp_p);

    usleep(configuracion->QUANTUM * 1000);

    if (timestamp_q_actual == timestamp && hilo_EXEC != NULL) {
        enviar_int(hilo_EXEC->TID, socket_cpu_interrupt, OP_INTERRUPT);
    }
}