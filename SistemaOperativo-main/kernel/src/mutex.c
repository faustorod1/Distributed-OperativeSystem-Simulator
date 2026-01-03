#include "mutex.h"

void crear_mutex(char* nombre)
{
    recurso *recurso = malloc(sizeof(*recurso));
    recurso->nombre = nombre;
    recurso->valor = 1;
    recurso->hilos_bloqueados = queue_create();
    t_PCB *proc_EXEC = proceso_EXEC();
    list_add(proc_EXEC->mutexs, recurso);

    enviar_a_ejecucion(hilo_EXEC);
}

void bloquear_mutex(char* nombre)         // wait(nombre_recurso);
{   
    
    recurso *recurso = buscar_recurso(nombre);

    // si el recurso NO existe:
    if (recurso == NULL)
    {
        enviar_bloqueante_y_esperar(BLOQUEANTE);
        queue_push(cola_hilos_EXIT, hilo_EXEC);
        elegir_hilo();
        return;
    }

    // si el recurso SI existe:
    recurso->valor--;
    if (recurso->valor == 0)
    { // recurso disponible
        enviar_bloqueante_y_esperar(NO_BLOQUEANTE);
        
        recurso->hilo_propietario = hilo_EXEC;
        
        t_paquete *paquete = crear_paquete(OP_EJECUTAR_HILO);
        agregar_a_paquete(paquete, &(hilo_EXEC->PID), sizeof(hilo_EXEC->PID));
        agregar_a_paquete(paquete, &(hilo_EXEC->TID), sizeof(hilo_EXEC->TID));

        enviar_paquete(paquete, socket_cpu_dispatch);
        eliminar_paquete(paquete);
        return;
    }
    else{ // recurso NO disponible
        enviar_bloqueante_y_esperar(BLOQUEANTE);
        queue_push(recurso->hilos_bloqueados, hilo_EXEC);
        log_info(logger, "## (%i : %i) - Bloqueado por: MUTEX", hilo_EXEC->PID, hilo_EXEC->TID);
        elegir_hilo();
    }

}

void desbloquear_mutex(char* nombre) //signal()
{

    recurso *recurso = buscar_recurso(nombre);

    // si el recurso NO existe:
    if (recurso == NULL)
    {
        enviar_bloqueante_y_esperar(BLOQUEANTE);
        queue_push(cola_hilos_EXIT, hilo_EXEC);
        elegir_hilo();
        return;
    }

    // si el recurso No esta tomado por este hilo:
    if (recurso->hilo_propietario != hilo_EXEC)
    {
        return; // no realiza ningun desbloqueo
    }

    // si el recurso SI esta tomado por este hilo:
    recurso->valor++;

    if (queue_is_empty(recurso->hilos_bloqueados))
    {
        recurso->hilo_propietario = NULL;
    }
    else
    {
        recurso->hilo_propietario = queue_pop(recurso->hilos_bloqueados);
        agregar_a_cola_ready_correspondiente(recurso->hilo_propietario);
    }

    enviar_bloqueante_y_esperar(NO_BLOQUEANTE);
    enviar_a_ejecucion(hilo_EXEC);
    
}

recurso *buscar_recurso(char* nombre)
{
    recurso *recurso;
    t_PCB *proc_EXEC = proceso_EXEC();
    for (int i = 0; i < list_size(proc_EXEC->mutexs); i++)
    {
        recurso = list_get(proc_EXEC->mutexs, i);
        if (!strcmp(recurso->nombre,nombre))
        {
            return recurso;
        }
    }
    return NULL;
}

void liberar_recurso(recurso *recurso) {
    free(recurso->nombre);
    queue_destroy(recurso->hilos_bloqueados);
    //queue_destroy_and_destroy_elements(recurso->hilos_bloqueados, free);//aca no seria liberar_tcb?
    //list_clean_and_destroy_elements(recurso->hilos_bloqueados->elements, (void (*)(void*)) liberar_tcb);
    free(recurso);
}