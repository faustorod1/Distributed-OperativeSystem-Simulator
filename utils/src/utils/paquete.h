#ifndef PAQUETE_UTILS_H
#define PAQUETE_UTILS_H

#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <commons/collections/list.h>
#include "utils.h"


enum op_code {
    OP_MENSAJE,
    OP_HANDSHAKE,		// TODOS
	OP_ERROR,
	OP_MEMCONTEXT, // cpu memoria
	OP_CREAR_PROCESO,// memoria, kernel
	OP_INICIAR_HILO,// memoria
	OP_RECHAZO_ADMISION,
	OP_INTERRUPT, // cpu
	OP_SEGMENTATION_FAULT, // cpu
	OP_MEMFETCH, //cpu memoria
	OP_MEMCONTEXT_UPDATE, //memoria
	OP_WRITE_MEM, //cpu memoria
	OP_READ_MEM, //cpu memoria
	OP_FINALIZAR_PROCESO, // memoria
	OP_FINALIZAR_HILO, //memoria
	OP_CANCELAR_HILO,
	OP_DUMP_MEM,// cpu memoria
	OP_THREAD_JOIN,
	OP_EJECUTAR_HILO,
	OP_MUTEX_CREATE,	// kernel
	OP_MUTEX_LOCK,		// kernel
	OP_MUTEX_UNLOCK,	// kernel
	OP_IO,				// kernel
	OP_CREAR_HILO,		// kernel
	OP_ES_BLOQUEANTE
};


enum cod_modulo {
	MOD_KERNEL,
	MOD_CPU,
	MOD_MEMORIA,
	MOD_FILESYSTEM
};


typedef struct
{
	int size;
	void *stream;
} t_buffer;


typedef struct
{
	enum op_code codigo_operacion;
	t_buffer *buffer;
} t_paquete;


/**
 * @brief Crea un paquete para enviar datos serializados.
 * @param op_code Código de operación del paquete.
 * @return Un puntero al paquete creado.
 */
t_paquete* crear_paquete(enum op_code op_code);

/**
 * @brief Agrega lo apuntado por `valor` al final del paquete.
 * @param paquete Paquete al que se agregará el valor.
 * @param valor Puntero al dato que se quiere agregar.
 * @param tam Tamaño de lo apuntado por `valor`.
 */
void agregar_a_paquete(t_paquete *paquete, void *valor, int tam);

/**
 * @brief Envía el paquete serializado por la conexión indicada por `socket`.
 * @param paquete Paquete a enviar.
 * @param socket Socket de la conexión a través de la cual se enviará el paquete.
 * @return Retorna la cantidad de bytes enviados o -1 en caso de error.
 */
int enviar_paquete(t_paquete *paquete, int socket);

/**
 * @brief Libera la memoria ocupada por un paquete y su buffer.
 * @param paquete Paquete a eliminar. 
 */
void eliminar_paquete(t_paquete *paquete);

/**
 * @brief Envía un entero por la conexión indicada por `socket`.
 * @param valor El valor entero a enviar.
 * @param socket Socket de la conexión a través de la cual se enviará el valor.
 * @param op_code Código de operación del mensaje.
 */
void enviar_int(int valor, int socket, enum op_code op_code);

/**
 * @brief Envía un string por la conexión indicada por `socket`.
 * @param valor El string a enviar.
 * @param socket Socket de la conexión a través de la cual se enviará el valor.
 * @param op_code Código de operación del mensaje.
 */
void enviar_string(char *valor, int socket, enum op_code op_code);

/**
 * @brief Envía un paquete vacío con determinado código de operación.
 * @param socket Socket de la conexión.
 * @param op_code Código de operación para enviar.
 */
void enviar_operacion(int socket, enum op_code op_code);

/**
 * @brief Espera a recibir un código de operación de la conexión indicada por `socket`.
 * @param socket Socket de la conexión.
 * @return El código de operación recibido.
 */
enum op_code recibir_operacion(int socket);

/**
 * @brief Recibe un paquete serializado y lo devuelve como lista.
 * @param socket Socket de la conexión.
 * @return Un `t_list*` con todos los elementos del paquete.
 */
t_list *recibir_paquete(int socket);

/**
 * @brief Recibe un string de la conexión.
 * @param socket Socket de la conexión.
 * @return Un puntero al string recibido.
 */
char *recibir_string(int socket);

/**
 * @brief Recibe un entero de la conexión.
 * @param socket Socket de la conexión.
 * @return El valor entero recibido.
 */
int recibir_int(int socket);


/**
 * @brief Agrega un conjunto de registros a un paquete.
 * @param paquete Paquete en el que poner los registros.
 * @param registros Conjunto de registros para agregar.
 */
void empaquetar_registros(t_paquete *paquete, uint32_t registros[9]);

/**
 * @brief Copia un conjunto de registros de una lista (usualmente recibida como paquete).
 * @param lista Lista en la que se encuentran los registros.
 * @param registros Array donde se guardarán los valores de los registros desempaquetados.
 * @param pos_inicial Posición de la lista en la que está el primer registro del conjunto (PC).
 */
void desempaquetar_registros(t_list *lista, uint32_t registros[9], int pos_inicial);

void* serializar_paquete(t_paquete *paquete, int bytes);

#endif // PAQUETE_UTILS_H
