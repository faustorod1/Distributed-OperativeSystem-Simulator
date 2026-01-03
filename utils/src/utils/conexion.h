#ifndef CONEXION_UTILS_H
#define CONEXION_UTILS_H

#include <netdb.h>
#include <commons/error.h>
#include "paquete.h"


/**
 * @brief Levanta un servidor en el puerto especificado.
 * @param puerto Puerto en el que levantar el servidor.
 * @return Socket de escucha para esperar conexiones.
 */
int iniciar_servidor(char* puerto);

/**
 * @brief Crea una conexión cliente con un servidor.
 * @param ip IP del servidor.
 * @param puerto Puerto de la conexión.
 * @return Socket para comunicación con el servidor.
 */
int crear_conexion(char *ip, char* puerto);

/**
 * @brief Queda esperando para recibir a un cliente.
 * @return Socket para comunicación con el cliente que se conectó.
 */
int esperar_cliente(int socket_escucha);



#endif // CONEXION_UTILS_H
