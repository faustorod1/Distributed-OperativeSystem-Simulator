#include "op_memoria.h"

// TODO Inicializar base según la partición en memoria
t_proceso *crear_proceso(int PID, int tam) {
    t_proceso *proceso = malloc(sizeof(*proceso));
    proceso->PID = PID;
    proceso->base = 0; // Cambiar por lugar libre en memoria
    proceso->limite = tam - 1;
    proceso->tamanio = tam;


    if(!hay_espacio_para_el_proceso(proceso)){
        free(proceso);
        return NULL;
    }
    
    proceso->hilos = list_create();
    
    list_add(lista_procesos, proceso);
    
    asignar_particion(proceso->PID);

    for (int i=0; i<list_size(lista_particiones); i++) {
        t_particion *particion = list_get(lista_particiones, i);
        if(particion->PID == proceso->PID && particion->esta_ocupado){
            proceso->base = particion->base;
            proceso->limite = particion->limite;
            break;
        }
    }
    
    return proceso;
}


t_hilo *crear_hilo(int PID, int TID, char *script_filename) {
    t_proceso *proceso = buscar_proceso(PID);

    if (proceso == NULL) {
        return NULL;
    }
    t_list *instrucciones = cargar_script(script_filename);
    if (instrucciones == NULL) {
        return NULL;
    }
    
    t_hilo *hilo = malloc(sizeof(*hilo));
    //memset(hilo, 0, sizeof(*hilo));
    hilo->TID = TID;
    hilo->instrucciones = instrucciones;
    for (int i=0; i<9; i++) {
        hilo->registros[i] = 0;
    }
    
    list_add(proceso->hilos, hilo);
    return hilo;
}


// TODO Liberar memoria usada (y ver si se pueden "juntar" las particiones, en caso de ser dinámicas).
void finalizar_proceso(int PID) {
    t_proceso *proceso = buscar_proceso(PID);
    
    liberar_particion(proceso);

    list_remove_element(lista_procesos, proceso);

    while (!list_is_empty(proceso->hilos)) {
        t_hilo *hilo = list_get(proceso->hilos, 0);
        finalizar_hilo_directo(proceso, hilo);
    }

    list_destroy(proceso->hilos);
    free(proceso);
}


void finalizar_hilo(int PID, int TID) {
    t_proceso *proceso = buscar_proceso(PID);
    t_hilo *hilo = buscar_hilo_en_proceso(proceso, TID);

    finalizar_hilo_directo(proceso, hilo);
}


t_list *cargar_script(char *filename) {
    char *path = string_from_format("%s/%s", configuracion->PATH_INSTRUCCIONES, filename);
    FILE *stream = fopen(path, "r");
    free(path);
    if (stream == NULL) {
        return NULL;
    }

    t_list *lista = list_create();

    char *line = NULL;
    size_t len = 0;

    while(getline(&line, &len, stream) != -1) {
        char *instr;
        instr = string_replace(line, "\n", "");
        list_add(lista, instr);
    }

    fclose(stream);
    if (line) {
        free(line);
    }
    
    return lista;
}


char *obtenerInstruccion(int PID, int TID, int PC) {
    t_hilo *hilo = buscar_hilo(PID, TID);
    return list_get(hilo->instrucciones, PC);
}


void finalizar_hilo_directo(t_proceso *proceso, t_hilo *hilo) {
    list_remove_element(proceso->hilos, hilo);
    
    list_iterate(hilo->instrucciones, free);
    list_destroy(hilo->instrucciones);

    free(hilo);
}