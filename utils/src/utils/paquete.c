#include "paquete.h"

void crear_buffer(t_paquete *paquete);
//void* serializar_paquete(t_paquete *paquete, int bytes);
void *recibir_buffer(int *size, int socket);


void crear_buffer(t_paquete *paquete) {
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(enum op_code op_code) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = op_code;
	crear_buffer(paquete);
	return paquete;
}
/*
void agregar_a_paquete(t_paquete *paquete, void *valor, int tam) {
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(tam) + tam);
	memcpy(paquete->buffer->stream + paquete->buffer->size, &tam, sizeof(tam));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(tam),valor, tam);
	paquete->buffer->size += sizeof(int) + tam;
}*/
void agregar_a_paquete(t_paquete *paquete, void *valor, int tam) { 
	size_t nuevo_tamano = paquete->buffer->size + sizeof(int) + tam; 
	paquete->buffer->stream = realloc(paquete->buffer->stream, nuevo_tamano); 
	memcpy(paquete->buffer->stream + paquete->buffer->size, &tam, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tam);
	paquete->buffer->size = nuevo_tamano;
}

void* serializar_paquete(t_paquete *paquete, int bytes) {
	void *magic = malloc(bytes);
	int desplazamiento = 0;
    
	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}



int enviar_paquete(t_paquete *paquete, int socket) {
    int bytes = paquete->buffer->size + 2 * sizeof(int); // Los 2 int del final son del op_code y buffer->size
	void *serializado = serializar_paquete(paquete, bytes);
	int bytes_sent = send(socket, serializado, bytes, MSG_NOSIGNAL);
	free(serializado);

	if (bytes_sent == -1) exit(EXIT_FAILURE);

    return bytes_sent;
}



void eliminar_paquete(t_paquete *paquete) {
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}



void enviar_int(int valor, int socket, enum op_code op_code) {
    t_paquete *paquete = crear_paquete(op_code);

    paquete->buffer->stream = realloc(paquete->buffer->stream, sizeof(valor));
	*((int*) paquete->buffer->stream) = valor;
    paquete->buffer->size = sizeof(valor);

    enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}



void enviar_string(char *valor, int socket, enum op_code op_code) {
    t_paquete *paquete = crear_paquete(op_code);

    int str_len = strlen(valor) + 1;
    paquete->buffer->stream = realloc(paquete->buffer->stream, str_len);
	memcpy(paquete->buffer->stream, valor, str_len);
    paquete->buffer->size = str_len;

    enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}



void enviar_operacion(int socket, enum op_code op_code) {
	int bytes_sent = send(socket, &op_code, sizeof(op_code), MSG_NOSIGNAL);
	if (bytes_sent == -1) exit(EXIT_FAILURE);
}



enum op_code recibir_operacion(int socket) {
    enum op_code codigo_operacion;
    if (recv(socket, &codigo_operacion, sizeof(int), MSG_WAITALL) > 0) {
        return codigo_operacion;
    }
    else {
        close(socket);
        return -1;
    }
}



void *recibir_buffer(int *size, int socket) {
    void *buffer;
    recv(socket, size, sizeof(int), MSG_WAITALL);
    buffer = malloc(*size);
    int bytes_received = recv(socket, buffer, *size, MSG_WAITALL);
	if (bytes_received == -1) exit(EXIT_FAILURE);
    return buffer;
}



t_list *recibir_paquete(int socket) {
	int size, desplazamiento = 0, tam;
	t_list *valores = list_create();
	void *buffer = recibir_buffer(&size, socket);

	while(desplazamiento < size) {
		memcpy(&tam, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		char *valor = malloc(tam);
		memcpy(valor, buffer + desplazamiento, tam);
		desplazamiento += tam;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}


char *recibir_string(int socket) {
	int size;
	char *buffer = recibir_buffer(&size, socket);
    return buffer;
}



int recibir_int(int socket) {
	int size;
	int *buffer = recibir_buffer(&size, socket);
	int valor = *buffer;
	free(buffer);
	return valor;
}


void empaquetar_registros(t_paquete *paquete, uint32_t registros[9]) {
	for(int i=0 ; i < 9 ; i++){
		agregar_a_paquete(paquete, &registros[i], sizeof(registros[i]));
	}
}

void desempaquetar_registros(t_list *lista, uint32_t registros[9], int pos_inicial) {
	for(int i=0 ; i < 9 ; i++){
		int *registro = list_get(lista, i + pos_inicial);
		registros[i] = *registro;
	}
}
