#include "main.h"

t_bitarray *bloques_libres;

int main(int argc, char* argv[]) {

    if(!strcmp(argv[1],"-1") || !strcmp(argv[1],"-2") || !strcmp(argv[1],"-3") || !strcmp(argv[1],"-1.1") || !strcmp(argv[1],"-1.2") || !strcmp(argv[1],"-1.3") || !strcmp(argv[1],"-2.1") || !strcmp(argv[1],"-2.2") || !strcmp(argv[1],"-3.1") || !strcmp(argv[1],"-3.2") || !strcmp(argv[1],"-3.3")){
        archivo_configuracion = "123";
    }else if(!strcmp(argv[1],"-4")){
        archivo_configuracion = "part-dinam";
    }else if(!strcmp(argv[1],"-5")){
        archivo_configuracion = "fs";
    }else if(!strcmp(argv[1],"-6")){
        archivo_configuracion = "stress";
    }else if(argv[1] == NULL){
        archivo_configuracion = "filesystem";
    }

    iniciar_config();
    iniciar_logger();        

    iniciar_estructuras();

    // Inicia server para MEMORIA
    iniciar_conexion();

    terminar_programa();

    return 0;
}


void terminar_programa(){
    free(configuracion);
    log_destroy(logger);
}

void iniciar_conexion(){
    char *puerto_str = string_itoa(configuracion->PUERTO_ESCUCHA);
    int socket_escucha = iniciar_servidor(puerto_str);
    free(puerto_str);

    while (true) {
        int socket_cliente = esperar_cliente(socket_escucha);
        enum op_code op = recibir_operacion(socket_cliente);
        int modulo = recibir_int(socket_cliente);
        if (op == OP_HANDSHAKE && modulo == MOD_MEMORIA){
            //printf("Se conectó Memoria a filesystem!\n");
            enviar_operacion(socket_cliente, OP_HANDSHAKE);

            pthread_t hilo_server_memoria; // uso hilo para memoria
            pthread_create(&hilo_server_memoria, NULL, (void*)gestionar_peticion_memoria, (void*) &socket_cliente);
            pthread_detach(hilo_server_memoria);

        }else{
            //printf("Error al conectar Memoria con Filesystem!\n");
            enviar_operacion(socket_cliente, OP_ERROR);
            return;
        }
    }
}

void gestionar_peticion_memoria(void *dir_socket){
    int socket_memoria = *((int*) dir_socket);
    enum op_code op = recibir_operacion(socket_memoria);
    switch (op) {
        case OP_DUMP_MEM:
            t_list *datos = recibir_paquete(socket_memoria);
            int rta_mem;

            char *nombre_archivo = list_get(datos,0);
            int tamanio_archivo = *(int*) list_get(datos,1);
            void *contenido_archivo = list_get(datos,2);

            int bloques_a_usar = (int)ceil(tamanio_archivo / configuracion->BLOCK_SIZE);//round creo q redondeaba para arriba y para abajo asi q usamos ceil q redondea siempre para arriba

            if(cant_bloques_libres() < bloques_a_usar + 1){ // +1 porque necesitamos un bloque para la lista de indices
                rta_mem = 0;
                //enviar_int(0, socket_memoria, OP_DUMP_MEM);
                //return;
            }else{
            
                t_list *lista_de_indices = asignar_bloques(bloques_a_usar + 1, nombre_archivo);

                escribir_bloques(lista_de_indices, contenido_archivo, nombre_archivo, tamanio_archivo);
                crear_metadata(nombre_archivo, tamanio_archivo, *(int*) list_get(lista_de_indices,0));
                rta_mem = 1;
                list_destroy_and_destroy_elements(lista_de_indices, free);
            }

            log_info(logger, "## Fin de solicitud - Archivo: %s", nombre_archivo);
            enviar_int(rta_mem, socket_memoria, OP_DUMP_MEM);

           list_destroy_and_destroy_elements(datos, free);
            break;
        default:
            break;

    } 
}


void iniciar_estructuras(){
    char *path_bitmap = string_from_format("%s/bitmap.dat", configuracion->MOUNT_DIR);
    char *path_bloques = string_from_format("%s/bloques.dat", configuracion->MOUNT_DIR);
    char *path_files = string_from_format("%s/files", configuracion->MOUNT_DIR);

    int tam_bitmap = ceil(configuracion->BLOCK_COUNT / 8);
    if (configuracion->BLOCK_COUNT % 8 != 0)
        tam_bitmap++;

    int tam_bloques = configuracion->BLOCK_SIZE * configuracion->BLOCK_COUNT;
    
    iniciar_archivo(path_bitmap, tam_bitmap, true);
    
    iniciar_archivo(path_bloques, tam_bloques, false);
    free(path_bloques);
    
    // Crea la carpeta files (de los metadata) si no existe
    struct stat st = {0};
    if (stat(path_files, &st) == -1) {
        mkdir(path_files, 0700);
    }
    free(path_files);

    // Levanta el bitarray de bloques libres
    void *bitmap = mapear_archivo(path_bitmap, tam_bitmap);
    bloques_libres = bitarray_create_with_mode(bitmap, tam_bitmap, MSB_FIRST);
    free(path_bitmap);
}

void iniciar_archivo(char *path_archivo,int tam,bool inicializar) {
    
    // Si el archivo ya existe, no hace nada
    if(access(path_archivo, F_OK) == 0) {
        return;
    }

    int fd = open(path_archivo, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        printf("1\n");
        perror("Error al abrir o crear archivo");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, tam) == -1) {
        perror("Error al establecer el tamaño del archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (!inicializar) {
        close(fd);
        return;
    }
    
    
    void *bitmap = mmap(NULL, tam, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (bitmap == MAP_FAILED) {
        perror("Error al mapear el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    memset(bitmap, 0, tam); // Inicializa el bitmap en 0

    if (msync(bitmap, tam, MS_SYNC) == -1) { //syncro
        perror("Error al sincronizar el archivo");
    }
    if (munmap(bitmap, tam) == -1) { //desmapeo
        perror("Error al desmapear el archivo");
    }

    close(fd);
}


void* mapear_archivo(char *path_archivo, int tam) {
    int fd = open(path_archivo, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        printf("2\n");
        perror("Error al abrir o crear archivo");
        return NULL;
    }

    void *map = mmap(NULL, tam, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    if (map == MAP_FAILED) {
        perror("Error al mapear el archivo");
        return NULL;
    }

    return map;
}

int guardar_archivo(void *map, int tam) {
    if (msync(map, tam, MS_SYNC) == -1) {
        perror("Error al sincronizar el archivo");
        return 0;
    }
    return 1;
}

int cant_bloques_libres() {
    int libres = 0;
    for (int i = 0; i < configuracion->BLOCK_COUNT; i++) {
        if (!bitarray_test_bit(bloques_libres, i)) libres++;
    }
    return libres;
}

t_list *asignar_bloques(int cantidad, char* archivo) {
    
    t_list *asignados = list_create();
    for (int i = 0; i < configuracion->BLOCK_COUNT && cantidad > 0; i++) {
        if (!bitarray_test_bit(bloques_libres, i)) {
            bitarray_set_bit(bloques_libres, i);
            int *bloque = malloc(sizeof(*bloque));
            *bloque = i;
            list_add(asignados, bloque);
            cantidad--;
            log_info(logger, "## Bloque asignado: %i - Archivo: %s - Bloques Libres: %i", i, archivo, cant_bloques_libres());
        }
    }
    guardar_archivo(bloques_libres->bitarray, bloques_libres->size);
    return asignados;
}


void print_bloques_libres(uint longitud_linea) {
    if (longitud_linea == 0) longitud_linea = 16;
    
    for (int i = 0; i < configuracion->BLOCK_COUNT; i++) {
        if ((i) % longitud_linea == 0) {
            printf("\n");
        }
        printf("%i ", bitarray_test_bit(bloques_libres, i));
    }
    printf("\n");
}

void crear_metadata(char* nombre,int tamanio, int bloque_indices){
    char *path_archivo = string_from_format("%s/files/%s", configuracion->MOUNT_DIR, nombre); // Path del archivo (lugar donde guardamos el archivo)

    int fd = open(path_archivo, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        free(path_archivo);
        printf("3\n");
        perror("Error al abrir o crear archivo");
        exit(EXIT_FAILURE);
    }

    char *info = string_from_format("SIZE=%i\nINDEX_BLOCK=%i\n", tamanio, bloque_indices);  // necesitamos asignar un bloque a la lista de indices
    ssize_t bytes_escritos = write(fd, info, strlen(info));
    free(info);
    free(path_archivo);

    if (bytes_escritos == -1) {
        perror("Error al escribir información adicional en el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    log_info(logger, "## Archivo Creado: %s - Tamaño: %i", nombre, tamanio);

    close(fd);
}

void escribir_bloques(t_list *lista_de_indices, void *contenido_archivo, char* nombre, int tamanio){
    char *path_bloques = string_from_format("%s/bloques.dat", configuracion->MOUNT_DIR);

    int fd = open(path_bloques, O_RDWR , S_IRUSR | S_IWUSR);
    if (fd == -1) {
        free(path_bloques);
        printf("4\n");
        perror("Error al abrir o crear archivo");
        exit(EXIT_FAILURE);
    }

    lseek(fd, *((int*) list_get(lista_de_indices, 0)) * configuracion->BLOCK_SIZE, SEEK_SET);
    for (int i = 1; i < list_size(lista_de_indices); i++){
        int numero_bloque = *((int*) list_get(lista_de_indices, i));
        ssize_t bytes_escritos = write(fd, &numero_bloque,sizeof(numero_bloque));
        if (bytes_escritos == -1) {
            perror("Error al escribir en el archivo");
            close(fd);
            exit(EXIT_FAILURE);
        } 
    }
    log_info(logger, "## Acceso Bloque - Archivo: %s - Tipo Bloque: ÍNDICE - Bloque File System %i",nombre,*((int*) list_get(lista_de_indices, 0)));
    retardo();

    int tamanio_escritura = configuracion->BLOCK_SIZE;
    for (int i = 1; i < list_size(lista_de_indices); i++) {
        int bloque = *((int*) list_get(lista_de_indices, i));
        lseek(fd, bloque * configuracion->BLOCK_SIZE, SEEK_SET);

        if(i == list_size(lista_de_indices) - 1){
            // Cuántos bytes se escriben en el último bloque
            tamanio_escritura = tamanio - (list_size(lista_de_indices) - 2) * configuracion->BLOCK_SIZE;
        }

        ssize_t bytes_escritos = write(fd, contenido_archivo, tamanio_escritura);
        contenido_archivo = (char *) contenido_archivo + configuracion->BLOCK_SIZE;
        if (bytes_escritos == -1) {
            perror("Error al escribir en el archivo");
            close(fd);
            exit(EXIT_FAILURE);
        }
        log_info(logger, "## Acceso Bloque - Archivo: %s - Tipo Bloque: DATOS - Bloque File System %i",nombre, bloque);
        retardo();
    }

    close(fd);
    free(path_bloques);
}

void retardo() {
    usleep(configuracion->RETARDO_ACCESO_BLOQUE * 1000);
}