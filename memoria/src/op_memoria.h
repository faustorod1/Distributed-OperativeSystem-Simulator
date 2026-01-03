#ifndef OP_MEMORIA_H
#define OP_MEMORIA_H

#include <stdlib.h>
#include <commons/string.h>
#include <readline/readline.h>
#include "estructuras.h"
#include "config_log.h"

/**
 * @brief Crea las estructuras para el CDE de un proceso y lo agrega a la lista de procesos.
 * @param PID PID del proceso.
 * @param tam Tamaño mínimo que ocupará el proceso en memoria de usuario.
 * @return Un puntero a la estructura generada, o `NULL` en caso de error.
 */
t_proceso *crear_proceso(int PID, int tam);

/**
 * @brief Crea las estructuras para el CDE de un hilo y lo agrega a la lista de hilos de su proceso.
 * @param PID PID del proceso al que pertenece el hilo.
 * @param TID TID del hilo.
 * @param script_filename Nombre del archivo de pseudocódigo correspondiente al hilo.
 * @return Un puntero a la estructura generada, o `NULL` en caso de error.
 */
t_hilo *crear_hilo(int PID, int TID, char *script_filename);

/**
 * @brief Libera la memoria ocupada por un proceso, destruye su estructura y lo remueve de la lista de procesos.
 * Si todavía tenía hilos sin eliminar, los destruye.
 * @param PID PID del proceso.
 */
void finalizar_proceso(int PID);

/**
 * @brief Elimina la estructura de un hilo, incluyendo sus instrucciones, y lo remueve de la lista de hilos de su proceso.
 * @param PID PID del proceso al que pertenece el hilo.
 * @param TID TID del hilo.
 */
void finalizar_hilo(int PID, int TID);

/**
 * @brief Lee un archivo de pseudocódigo y almacena cada línea en una lista.
 * @param filename Nombre del archivo para leer. Solo debe ser el nombre, ya que se
 * buscará el archivo dentro de la carpeta especificada en la configuración.
 * @return Una lista con cada línea del archivo, o `NULL` en caso de error.
 */
t_list *cargar_script(char *filename);

/**
 * @brief Devuelve la instrucción nº `PC` del pseudocódigo correspondiente a un hilo.
 * @param PID PID del proceso al que pertenece el hilo.
 * @param TID TID del hilo.
 * @param PC Número de instrucción para devolver (empezando por 0).
 * @return Un `char*` con la instrucción solicitada, o `NULL` en caso de error.
 */
char *obtenerInstruccion(int PID, int TID, int PC);

/**
 * @brief Elimina la estructura de un hilo, incluyendo sus instrucciones, y lo remueve de la lista de hilos de su proceso.
 * Al ya recibir como parámetro la referencia al proceso e hilo, no debe buscarlos en sus listas correspondientes.
 * @param proceso Puntero al proceso al que pertenece el hilo.
 * @param hilo puntero al hilo.
 */
void finalizar_hilo_directo(t_proceso *proceso, t_hilo *hilo);



uint32_t read_mem(uint32_t direccion);

void write_mem(uint32_t direccion, uint32_t valor);

#endif // OP_MEMORIA_H
