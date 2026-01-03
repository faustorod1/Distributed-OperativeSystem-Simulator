#ifndef ESTRUCTURAS_MEMORIA_H
#define ESTRUCTURAS_MEMORIA_H

#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/temporal.h>
#include "config_log.h"
#include "utils/conexion.h"
#include "utils/utils.h"

typedef struct {
    int base;
    int limite;
    int tamanio;
    bool esta_ocupado;
    int PID; // PID del proceso que la ocupa
} t_particion;

typedef struct{
    int PID;
    int base;
    int limite;
    int tamanio;
    t_list* hilos; // guardamos t_hilos de cada proceso
} t_proceso;

typedef struct{
    int TID;
    t_list* instrucciones;
    uint32_t registros[9];
} t_hilo;


typedef struct {
    int base;
    int limite;
    uint32_t registros[9];
} t_cde;

extern t_list *lista_procesos;
extern void *memoria_usuario;
extern t_list *lista_particiones;

/**
 * @brief Inicializa las estructuras globales de Memoria, tales como `lista_procesos`.
 */
void iniciar_estructuras();

/**
 * @brief Busca un proceso en `lista_procesos` según su PID.
 * @param PID PID del proceso a buscar.
 * @return Un puntero al proceso, o `NULL` si no lo encuentra.
 */
t_proceso *buscar_proceso(int PID);

/**
 * @brief Busca un hilo según su TID y el PID del proceso al que pertenece.
 * @param PID PID del proceso al que pertenece el hilo.
 * @param TID TID del hilo a buscar.
 * @return Un puntero al hilo, o `NULL` si no encuentra el hilo o el proceso.
 */
t_hilo *buscar_hilo(int PID, int TID);

/**
 * @brief Busca una partición según el PID del proceso que la ocupa.
 * @param PID PID del proceso correspondiente.
 * @return Un puntero a la partición, o `NULL` si no encuentra ninguna.
 */
t_particion *buscar_particion_por_PID(int PID);

/**
 * @brief Busca un hilo según su TID en la lista de hilos de `proceso`.
 * @param proceso Un puntero al proceso al que pertence hilo.
 * @param TID TID del hilo a buscar.
 * @return Un puntero al hilo, o `NULL` si no lo encuentra.
 */
t_hilo *buscar_hilo_en_proceso(t_proceso *proceso, int TID);

/**
 * @brief Obtiene el contexto de ejecución de un determinado proceso-hilo.
 * @param PID PID del proceso.
 * @param TID TID del hilo.
 * @return Una estructura con los datos necesarios del CDE (base, límite, registros).
 */
t_cde obtenerCDE(int PID, int TID);

/**
 * @brief Actualiza el contexto de ejecución (solo los registros) de un determinado proceso-hilo.
 * @param PID PID del proceso.
 * @param TID TID del hilo.
 * @param registros Registros actualizados.
 * @return `true` en caso de éxito, o `false` si no encuentra el proceso o hilo.
 */
bool actualizarCDE(int PID, int TID, uint32_t registros[9]);

void liberar_particion(t_proceso* proceso);

void asignar_particion(int PID);

bool hay_espacio_para_el_proceso(t_proceso* proceso);

int enviar_a_file_system(enum op_code codigo, t_proceso* proceso, t_hilo* hilo);

t_particion *crear_particion(int PID, int base, int limite);

void first_fit(int PID);

void best_fit(int PID);

void worst_fit(int PID);

void actualizar_estatica(int PID, int indice);

void actualizar_dinamica(t_particion* particion_nueva, int indice);

void liberar_particion_estatica(t_proceso* proceso);

void liberar_particion_dinamica(t_proceso* proceso);

int memory_dump(int PID, int TID);

void print_particiones();

#endif // ESTRUCTURAS_MEMORIA_H
