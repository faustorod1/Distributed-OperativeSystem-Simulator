#ifndef FILESYSTEM_LOG_H_
#define FILESYSTEM_LOG_H_

#include "config_log.h"
#include "utils/conexion.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <commons/bitarray.h>
#include <math.h>
#include "global.h"

void terminar_programa();
void iniciar_conexion();
void iniciar_estructuras();
void iniciar_archivo(char *path_archivo,int tam,bool inicializar);
void gestionar_peticion_memoria(void *dir_socket);
void* mapear_archivo(char *path_archivo, int tam);
int guardar_archivo(void *map, int tam);
int cant_bloques_libres();
t_list *asignar_bloques(int cantidad, char* nombre_archivo);
void escribir_bloques(t_list *lista_de_indices, void *contenido_archivo, char* nombre, int tamanio);
void print_bloques_libres(uint longitud_linea);
void crear_metadata(char* nombre,int tamanio, int bloque_indices);
void retardo();
    

#endif