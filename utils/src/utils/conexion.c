#include "conexion.h"


int iniciar_servidor(char* puerto) {
    int socket_escucha;
    struct addrinfo hints, *serverinfo;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, puerto, &hints, &serverinfo);

    socket_escucha = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
    if (socket_escucha == -1) {
        error_show("Error al crear socket");
        freeaddrinfo(serverinfo);
        exit(EXIT_FAILURE);
    }

    setsockopt(socket_escucha, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
    bind(socket_escucha, serverinfo->ai_addr, serverinfo->ai_addrlen);
    listen(socket_escucha, SOMAXCONN);

    freeaddrinfo(serverinfo);
    return socket_escucha;
}



int crear_conexion(char *ip, char* puerto) {
	struct addrinfo hints;
	struct addrinfo *server_info;
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_conexion = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (socket_conexion == -1) {
        error_show("Error al crear socket");
        freeaddrinfo(server_info);
        exit(EXIT_FAILURE);
    }

	connect(socket_conexion, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);
	return socket_conexion;
}



int esperar_cliente(int socket_escucha) {
    int socket_conexion = accept(socket_escucha, NULL, NULL);
    return socket_conexion;
}