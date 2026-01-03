#include "estructuras.h"

t_list *lista_procesos;
void *memoria_usuario;
t_list *lista_particiones;


void iniciar_estructuras() {
    lista_procesos = list_create();
    lista_particiones = list_create();

    memoria_usuario = malloc(configuracion->TAM_MEMORIA); //memoria q pueden ver y utilizar los procesos

    if (strcmp(configuracion->ESQUEMA, "FIJAS") == 0){ // Particiones FIJAS
        int base_anterior = 0;
        int limite_anterior = 0;

        for (int i = 0; i < list_size(configuracion->PARTICIONES); i++) {
            t_particion *particion;
            particion = crear_particion(-1,base_anterior + limite_anterior,*((int*) list_get(configuracion->PARTICIONES, i)) - 1);

            list_add(lista_particiones, particion);
            
            base_anterior = particion->base;
            limite_anterior = particion->limite + 1;
        }
    }else{ // Particiones DINAMICAS
        t_particion *particion;
        particion = crear_particion(-1,0,configuracion->TAM_MEMORIA - 1);
        list_add(lista_particiones, particion);
    }
}

//ALGORITMOS
void asignar_particion(int PID){
    if (strcmp(configuracion->ALGORITMO_BUSQUEDA, "FIRST") == 0){
        first_fit(PID);
    }else if (strcmp(configuracion->ALGORITMO_BUSQUEDA, "BEST") == 0){
        best_fit(PID);
    }else if (strcmp(configuracion->ALGORITMO_BUSQUEDA, "WORST") == 0){
        worst_fit(PID);
    }
}

t_particion *crear_particion(int PID, int base, int limite){
    t_particion *particion = malloc(sizeof(t_particion));
    particion->base = base;
    particion->limite = limite;
    particion->tamanio = limite + 1;
    particion->esta_ocupado = false;
    particion->PID = PID;
    return particion;
}


void first_fit(int PID){
    t_proceso *proceso = buscar_proceso(PID);
    int tamanio_proceso = proceso->tamanio;
    
    for (int i = 0; i < list_size(lista_particiones); i++) {
        t_particion *particion = list_get(lista_particiones, i);
        
        if (!particion->esta_ocupado){
            if (tamanio_proceso <= particion->tamanio){
                if (strcmp(configuracion->ESQUEMA, "FIJAS") == 0){
                    actualizar_estatica(PID,i);
                    break;
                }else{ // DINAMICAS
                    t_particion *particion_nueva;
                    particion_nueva = crear_particion(PID,0,proceso->limite);
                    particion_nueva->esta_ocupado = true;
                    actualizar_dinamica(particion_nueva,i);
                    break;
            }
            }
        }
    }
}
/*
int buscar_el_mejor_indice(int PID){
    int indice_del_min = -1;
    for (int i = 0; i < list_size(lista_particiones); i++) {
        t_particion *particion = list_get(lista_particiones, i);
        int tam_particion = particion->tamanio;
        if(!particion->esta_ocupado){
            bool flag_de_salida_temprana = true;
            for(int j = i+1; j < list_size(lista_particiones); j++){
                t_particion *particion_siguiente = list_get(lista_particiones, j); 
                int tam_particion_siguiente = particion_siguiente->tamanio;                
                if(!particion_siguiente->esta_ocupado){
                    t_proceso *proceso = buscar_proceso(PID);
                    int tamanio_proceso = proceso->tamanio;
                    if(tam_particion - tamanio_proceso <= tam_particion_siguiente - tamanio_proceso) {
                        indice_del_min = i;
                    }else{
                        indice_del_min = j;
                        flag_de_salida_temprana = false;
                    }
                }else{
                    indice_del_min = i;
                }
            }
            
                para evitar iteraciones al pedo, cuando se hizo un for de j completo sin que ninguna de sus iteraciones haya
                modificado el indice_del_min deberiamos hacer un brake. Porque esto significaria que el indice de la 
                particion "i" es el mejor de todos los anteriores y de todos los posteriores.
                propongo usar el valor_de_salida_temprana
            
            if(flag_de_salida_temprana){
                break;
            }
        }
    }
    return indice_del_min;
}
 */


int buscar_el_mejor_indice(int PID) {
    int indice_del_min = -1;
    int fragmentacion_minima = -1;

    t_proceso *proceso = buscar_proceso(PID);
    int tamanio_proceso = proceso->tamanio;

    for (int i = 0; i < list_size(lista_particiones); i++) {
        t_particion *particion = list_get(lista_particiones, i);

        if (!particion->esta_ocupado) {
            int fragmentacion_actual = particion->tamanio - tamanio_proceso;

            if (fragmentacion_actual >= 0 && 
               (indice_del_min == -1 || fragmentacion_actual < fragmentacion_minima)) {
                indice_del_min = i;
                fragmentacion_minima = fragmentacion_actual;

                // Salida temprana si la fragmentación es perfecta (0)
                if (fragmentacion_minima == 0) {
                    break;
                }
            }
        }
    }

    return indice_del_min;
}


void best_fit(int PID){
    int indice = buscar_el_mejor_indice(PID);
    t_proceso *proceso = buscar_proceso(PID);
    
    if (strcmp(configuracion->ESQUEMA, "FIJAS") == 0){
        actualizar_estatica(PID,indice);
    }else{ // DINAMICAS
        t_particion *particion_nueva;
        particion_nueva = crear_particion(PID,0,proceso->limite); 
        particion_nueva->esta_ocupado = true;
        actualizar_dinamica(particion_nueva,indice);
    }

}

/*
int buscar_el_peor_indice(int PID){
    int indice_del_may = -1;
    for (int i = 0; i < list_size(lista_particiones); i++) {
        t_particion *particion = list_get(lista_particiones, i);
        int tam_particion = particion->tamanio;
        if(!particion->esta_ocupado){
            bool flag_de_salida_temprana = true;
            for(int j = i+1; j < list_size(lista_particiones); j++){
                t_particion *particion_siguiente = list_get(lista_particiones, j);
                int tam_particion_siguiente = particion_siguiente->tamanio;
                if(!particion_siguiente->esta_ocupado){
                    t_proceso *proceso = buscar_proceso(PID);
                    int tamanio_proceso = proceso->tamanio;
                    if(tam_particion - tamanio_proceso >= tam_particion_siguiente - tamanio_proceso ){
                        indice_del_may = i;
                    }else{
                        indice_del_may = j;
                        flag_de_salida_temprana = false;
                    }
                }else{
                    indice_del_may = i;
                }
            }
            if(flag_de_salida_temprana){
                break;
            }
        }
    }
    return indice_del_may;
}
*/

int buscar_el_peor_indice(int PID) {
    int indice_del_may = -1;
    int fragmentacion_maxima = -1;

    t_proceso *proceso = buscar_proceso(PID);
    int tamanio_proceso = proceso->tamanio;

    for (int i = 0; i < list_size(lista_particiones); i++) {
        t_particion *particion = list_get(lista_particiones, i);

        if (!particion->esta_ocupado) {
            int fragmentacion_actual = particion->tamanio - tamanio_proceso;

            if (fragmentacion_actual >= 0 && (indice_del_may == -1 || fragmentacion_actual > fragmentacion_maxima)) {
                indice_del_may = i;
                fragmentacion_maxima = fragmentacion_actual;
            }
        }
    }

    return indice_del_may;
}


void worst_fit(int PID){
    int indice = buscar_el_peor_indice(PID);
    t_proceso *proceso = buscar_proceso(PID);
    
    if (strcmp(configuracion->ESQUEMA, "FIJAS") == 0){
        actualizar_estatica(PID,indice);
    }else{ // DINAMICAS
        t_particion *particion_nueva;
        particion_nueva = crear_particion(PID,0,proceso->limite);
        particion_nueva->esta_ocupado = true;
        actualizar_dinamica(particion_nueva,indice);
    }
}

void actualizar_estatica(int PID, int indice){
   t_particion *particion = list_get(lista_particiones, indice);   
   particion->esta_ocupado = true;
   particion->PID = PID;
}

void actualizar_dinamica(t_particion* particion_nueva, int indice){
    t_particion *particion = list_get(lista_particiones, indice);
    particion_nueva->base = particion->base;
    particion->base = particion_nueva->limite + particion_nueva->base + 1;
    particion->limite -= particion_nueva->limite - 1; 
    particion->tamanio -= particion_nueva->tamanio;
    list_add_in_index(lista_particiones, indice, particion_nueva);
}

void print_particiones() {
    int cant_particiones = list_size(lista_particiones);
    for (int i=0; i < cant_particiones; i++) {
        t_particion *part = list_get(lista_particiones, i);
        if (part->esta_ocupado)
            printf("x");
        else
            printf("-");
        printf("%i ", part->tamanio);
    }
    printf("\n");
}

bool hay_espacio_para_el_proceso(t_proceso* proceso){
    for (int i = 0; i < list_size(lista_particiones); i++) {
        t_particion* particion = list_get(lista_particiones, i);
        if(!particion->esta_ocupado && particion->tamanio >= proceso->tamanio){
            return true;
        }
    }
    return false;
}

void liberar_particion(t_proceso* proceso){

    if(strcmp(configuracion->ESQUEMA, "FIJAS") == 0){
        liberar_particion_estatica(proceso);
    }else{ //DINAMICAS
        liberar_particion_dinamica(proceso);
    }
}

void liberar_particion_dinamica(t_proceso *proceso){
    for (int i = 0; i < list_size(lista_particiones); i++) {
        t_particion *particion = list_get(lista_particiones, i);

        if(particion->PID == proceso->PID){
            particion->esta_ocupado = false;
            particion->PID = -1;

            //CONSOLIDACION
            if(i+1 <= list_size(lista_particiones)){//para evitar accesos invalidos a la lista
                t_particion* particion_siguiente = list_get(lista_particiones, i+1);
                
                if(!particion_siguiente->esta_ocupado){
                    particion->limite += particion_siguiente->tamanio;
                    particion->tamanio += particion_siguiente->tamanio;
                    
                    list_remove(lista_particiones, i+1);
                    free(particion_siguiente);
                }
            }

            if(i > 0){//para evitar accesos invalidos a la lista
                t_particion* particion_anterior = list_get(lista_particiones, i-1);
                
                if(!particion_anterior->esta_ocupado){
                    particion_anterior->limite += particion->tamanio;
                    particion_anterior->tamanio += particion->tamanio;

                    list_remove(lista_particiones, i);
                    free(particion);
                }
            }
        }  
    }
}

void liberar_particion_estatica(t_proceso* proceso){
    t_particion* particion = buscar_particion_por_PID(proceso->PID);
    particion->esta_ocupado = false;
    particion->PID = -1;
}

//-------------------------------------------------------------------------------------------------------------

uint32_t read_mem(uint32_t direccion) {
    return *((uint32_t*)(memoria_usuario + direccion));         
}

void write_mem(uint32_t direccion, uint32_t valor) {
    *((uint32_t*)(memoria_usuario + direccion)) = valor;
}
//------------------------------------------------------------------------------------------------------------

int memory_dump(int PID, int TID) {
    t_proceso *proceso = buscar_proceso(PID);
    t_hilo *hilo = buscar_hilo(PID, TID);

    if (proceso == NULL || hilo == NULL) {
        //printf("Error: proceso o hilo no encontrados");
        abort();
    }

    int socket = enviar_a_file_system(OP_DUMP_MEM, proceso, hilo);
    
    recibir_operacion(socket);
    int rta = recibir_int(socket);

    return rta;
}
    

int enviar_a_file_system(enum op_code codigo, t_proceso *proceso, t_hilo *hilo){
    char *puerto_str = string_itoa(configuracion->PUERTO_FILESYSTEM);
    int socket_fs = crear_conexion(configuracion->IP_FILESYSTEM, puerto_str);
    free(puerto_str);
    enviar_int(MOD_MEMORIA, socket_fs, OP_HANDSHAKE);
    recibir_operacion(socket_fs);

    t_paquete *paquete = crear_paquete(codigo);
    char *timestamp = temporal_get_string_time("%H:%M:%S:%MS");
    char* nombre_archivo = string_from_format("%i-%i-%s.dmp", proceso->PID, hilo->TID, timestamp);
    free(timestamp);
    void *inicio_particion = memoria_usuario + proceso->base;

    agregar_a_paquete(paquete, nombre_archivo, strlen(nombre_archivo) + 1);
    agregar_a_paquete(paquete, &(proceso->tamanio), sizeof(int));// El tamaño del proceso equivale a la memoria reservada para el proceso
    agregar_a_paquete(paquete, inicio_particion, proceso->tamanio); // El contenido de la partición del proceso
    free(nombre_archivo);

    int bytes = enviar_paquete(paquete, socket_fs);
    if (bytes == 0){
        //printf("Error al enviar paquete a FILE SYSTEM!\n");
        eliminar_paquete(paquete);
        abort();
    }
    eliminar_paquete(paquete);
    return socket_fs;
}





//------------------------------------------------------------------------------------------------------------






t_proceso *buscar_proceso(int PID) {
    bool es_el_buscado(t_proceso *proceso){
        return proceso->PID == PID;
    }
    return list_find(lista_procesos, (void*) es_el_buscado);
}


t_hilo *buscar_hilo(int PID, int TID) {
    t_proceso *proceso = buscar_proceso(PID);
    if (proceso == NULL)
        return NULL;
    return buscar_hilo_en_proceso(proceso, TID);
}


t_hilo *buscar_hilo_en_proceso(t_proceso *proceso, int TID) {
    bool es_el_buscado(t_hilo *hilo){
        return hilo->TID == TID;
    }
    return list_find(proceso->hilos, (void*) es_el_buscado);
}

t_particion *buscar_particion_por_PID(int PID) {
    bool es_el_buscado (t_particion *particion) {
        return particion->PID == PID;
    }
    return list_find(lista_particiones, (void*) es_el_buscado);
}


// TODO qué pasa en caso de error
t_cde obtenerCDE(int PID, int TID) {
    t_cde cde;
    t_proceso *proceso = buscar_proceso(PID);

    cde.base = proceso->base;
    cde.limite = proceso->limite;

    t_hilo *hilo = buscar_hilo_en_proceso(proceso, TID);
    memcpy(cde.registros, hilo->registros, sizeof(cde.registros));
    
    return cde;
}


bool actualizarCDE(int PID, int TID, uint32_t registros[9]) {
    t_proceso *proceso = buscar_proceso(PID);
    if (proceso == NULL)
        return false;
    t_hilo *hilo = buscar_hilo_en_proceso(proceso, TID);
    if (hilo == NULL)
        return false;
    
    memcpy(hilo->registros, registros, sizeof(*registros) * 9);

    return true;
}


