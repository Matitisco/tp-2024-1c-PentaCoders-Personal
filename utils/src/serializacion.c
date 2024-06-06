#include "../include/serializacion.h"
t_log *logger;

/*----------------------- MENSAJE ------------------*/
// ENVIAR MENSAJE
void enviar_mensaje(char *mensaje, int socket_cliente)
{
    /* t_paquete *paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = MENSAJE;
    paquete->buffer = malloc(sizeof(tipo_buffer));
    paquete->buffer->size = strlen(mensaje) + 1;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

    int bytes = paquete->buffer->size + 2 * sizeof(int);

    void *a_enviar = serializar_paquete(paquete, bytes);

    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar);
    eliminar_paquete(paquete); */
}
// RECIBIR MENSAJE
void recibir_mensaje(int socket_cliente)
{
    tipo_buffer *buffer = recibir_buffer(socket_cliente);
    // log_info(logger, "Me llego el mensaje %s", buffer->stream);
    free(buffer);
}

/*----------------------- PAQUETE ------------------*/

// CREAR PAQUETE
t_paquete *crear_paquete(void)
{
/*     t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PAQUETE;
    paquete->buffer = crear_buffer();
    return paquete; */
}

t_paquete *crear_paquete_cde(void)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = SOLICITUD_INICIAR_PROCESO;
    paquete->buffer = crear_buffer();
    return paquete;
}

// AGREGAR A PAQUETE
void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio)
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

    memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

    paquete->buffer->size += tamanio + sizeof(int);
}

t_list *recibir_paquete_cde(int socket_cliente)
{
    // int size = 0;
    // int desplazamiento = 0;
    tipo_buffer *buffer;
    t_list *valores = list_create();

    buffer = recibir_buffer(socket_cliente);

    // Recibir pid
    uint32_t pid = 0;
    pid = leer_buffer_enteroUint32(buffer);
    list_add(valores, (uint32_t *)pid);

    // recibir tamanio del path

    /* int tamanio = 0;
    tamanio = leer_buffer_enteroUint32(buffer);
    list_add(valores,tamanio); */

    // recibir path
    // char *path = malloc(tamanio_path);
    char *path = leer_buffer_string(buffer); // recibe el tamanio adentro de la funcion.
    list_add(valores, path);

    // recibir pid
    /* int pid = 0;
    memcpy(&pid, buffer + desplazamiento, sizeof(int));
    list_add(valores, pid); */
    /*
        // recibir tamanio del path
        int tamanio_path = 0;
        memcpy(&tamanio_path, buffer + desplazamiento, sizeof(int)); */

    /* // recibimos y copiamos el contenido del path
    char *path = malloc(tamanio_path);
    memcpy(path, buffer + desplazamiento, tamanio_path);
    list_add(valores, path);
 */
    free(buffer);
    return valores;
}

// ENVIAR PAQUETE
void enviar_paquete(t_paquete *paquete, int socket_cliente)
{
    int bytes = paquete->buffer->size + 2 * sizeof(int);
    void *a_enviar = serializar_paquete(paquete, bytes);

    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar);
}
// ELMININAR PAQUETE
void eliminar_paquete(t_paquete *paquete)
{
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}
// RECIBIR PAQUETE
t_list *recibir_paquete(int socket_cliente)
{
    int size = 0;
    int desplazamiento = 0;
    tipo_buffer *buffer;
    t_list *valores = list_create();
    int tamanio = 0;

    buffer = recibir_buffer(socket_cliente);
    while (desplazamiento <= size)
    {
        memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
        desplazamiento += sizeof(int);
        char *valor = malloc(tamanio);
        memcpy(valor, buffer + desplazamiento, tamanio);
        desplazamiento += tamanio;
        list_add(valores, valor);
    }
    free(buffer);
    return valores;
}
// SERIALIZAR PAQUETE
void *serializar_paquete(t_paquete *paquete, int bytes)
{
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

// PAQUETE
void paquete(int conexion)
{
    char *leido = NULL;
    t_paquete *paquete = crear_paquete();

    leido = readline(">");
    while (strcmp(leido, "") != 0)
    {
        agregar_a_paquete(paquete, leido, strlen(leido) - 1);
        free(leido);
        leido = readline("> ");
    }
    free(leido);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}
/*----------------------------------- CODIGO -----------------------------------*/
// ENVIAR CODIGO
void enviar_cod_enum(int socket_servidor, uint32_t cod)
{
    send(socket_servidor, &cod, sizeof(uint32_t), 0);
    // log_info(logger, "Se envio el codigo %d al servidor %d", cod, socket_servidor);
}
// RECIBIR OPERACION PARECIDO A recibi_cod(int socket_cliente)
op_code recibir_operacion(int socket_cliente)
{
    op_code cod_op;
    while (1)
    {
        if (recv(socket_cliente, &cod_op, sizeof(uint32_t), MSG_WAITALL) > 0)
            return cod_op;
        else
        {
            close(socket_cliente);
            return -1;
        }
    }
}

/* op_code recibir_operacion2(int socket_cliente)
{
    op_code cod_op;
    while (1)
    {
        if (recv(socket_cliente, &cod_op, sizeof(uint32_t), MSG_WAITALL) > 0)
            return cod_op;
        else
        {
            close(socket_cliente);
            return -1;
        }
    }
} */
/*----------------------------------- BUFFER -----------------------------------*/
// CREAR BUFFER
tipo_buffer *crear_buffer()
{
    tipo_buffer *buffer = malloc(sizeof(tipo_buffer));
    buffer->offset = 0;
    buffer->size = 0;
    buffer->stream = NULL;
    return buffer;
}
// DESTRUIR BUFFER
void destruir_buffer(tipo_buffer *buffer)
{
    free(buffer->stream);
    free(buffer);
} 
// AGREGAR EN EL BUFFER UN ENTERO UINT32
void agregar_buffer_para_enterosUint32(tipo_buffer *buffer, uint32_t entero)
{

    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint32_t));
    buffer->size += sizeof(uint32_t);

    // el offset ya esta en 0 al crearlo en crear_buffer
    memcpy(buffer->stream + buffer->offset, &entero, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
}

// AGREGAR EN EL BUFFER UN ENTERO UINT8
void agregar_buffer_para_enterosUint8(tipo_buffer *buffer, uint8_t entero)
{
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint8_t));
    buffer->size += sizeof(uint8_t);

    // el offset ya esta en 0 al crearlo en crear_buffer
    memcpy(buffer->stream + buffer->offset, &entero, sizeof(uint8_t));
    buffer->offset += sizeof(uint8_t);
}
// AGREGAR EN EL BUFFER UN STRING
void agregar_buffer_para_string(tipo_buffer *buffer, char *args)
{
    uint32_t tamanio = 0;

    char *string = (char *)args;

    if(args == NULL){
        return;
    }
    
    while (string[tamanio] != NULL)
    {
        tamanio++; // sumo el tamanio
    }
    agregar_buffer_para_enterosUint32(buffer, tamanio);

    buffer->stream = realloc(buffer->stream, buffer->size + tamanio); // Aumenta la memoria, suma el int tamanio con el tamanio del char*
    memcpy(buffer->stream + buffer->size, string, tamanio);
    buffer->size += tamanio; // tamanio total
}
//

/*
void agregar_buffer_string_null(tipo_buffer *buffer,char *args)
{
     uint32_t tamanio = sizeof(char*);
    char * string = NULL;
    //size es el tamaño total del buffer -> hay que aumentarle 
    //offset es el desplazamiento dentro del payload

    agregar_buffer_para_enterosUint32(buffer, tamanio);//subo el tamanio del char* al buffer
    
    memcpy(buffer->stream + buffer->size, string, tamanio);//subo el contenido del string
    buffer->size += tamanio;

}


*/
void agregar_buffer_para_registros(tipo_buffer *buffer, t_registros *registros)
{
    agregar_buffer_para_enterosUint8(buffer, registros->AX);
    agregar_buffer_para_enterosUint8(buffer, registros->BX);
    agregar_buffer_para_enterosUint8(buffer, registros->CX);
    agregar_buffer_para_enterosUint8(buffer, registros->DX);
    agregar_buffer_para_enterosUint32(buffer, registros->EAX);
    agregar_buffer_para_enterosUint32(buffer, registros->EBX);
    agregar_buffer_para_enterosUint32(buffer, registros->ECX);
    agregar_buffer_para_enterosUint32(buffer, registros->EDX);
    agregar_buffer_para_enterosUint32(buffer, registros->SI);
    agregar_buffer_para_enterosUint32(buffer, registros->DI);
}
// ENVIAR BUFFER
void enviar_buffer(tipo_buffer *buffer, int socket)
{
    send(socket, &(buffer->size), sizeof(uint32_t), 0); // enviar size

    if (buffer->size != 0)
    {
        // Enviamos el stream del buffer
        send(socket, buffer->stream, buffer->size, 0); // enviar buffer
    }
    else
    {
        printf("El buffer tiene size 0");
    }
}
// RECIBIR BUFFER
tipo_buffer *recibir_buffer(int socket)
{
    tipo_buffer *buffer = crear_buffer();
    // Recibo el tamanio del buffer y reservo espacio en memoria
    recv(socket, &(buffer->size), sizeof(uint32_t), MSG_WAITALL);
    if (buffer->size != 0)
    {
        buffer->stream = malloc(buffer->size);
        // Recibo stream del buffer
        recv(socket, buffer->stream, buffer->size, MSG_WAITALL);
    }
    return buffer;
}

// LEER BUFFER UINT_32
uint32_t leer_buffer_enteroUint32(tipo_buffer *buffer)
{
    int entero32; // malloc(sizeof(uint32_t));
    // void *stream = buffer->stream;
    //  Deserializamos los campos que tenemos en el buffer
    memcpy(&entero32, buffer->stream + buffer->offset, sizeof(uint32_t));
    // stream += sizeof(uint32_t);
    buffer->offset += sizeof(uint32_t);

    return entero32;
}
// LEER BUFFER UINT_8
uint8_t leer_buffer_enteroUint8(tipo_buffer *buffer)
{
    int entero8;

    // void *stream = buffer->stream;
    //  Deserializamos los campos que tenemos en el buffer
    memcpy(&entero8, buffer->stream + buffer->offset, sizeof(uint8_t));
    // stream += sizeof(uint8_t);
    buffer->offset += sizeof(uint8_t);

    return entero8;
}
// LEER BUFFER
char *leer_buffer_string(tipo_buffer *buffer)
{
    char *cadena;
    uint32_t tamanio;

    // void *stream = buffer->stream;
    /* // Deserializamos los campos que tenemos en el buffer
    memcpy(tamanio, stream, sizeof(uint32_t)); // Recibe tamanio
    stream += sizeof(uint32_t); */

    tamanio = leer_buffer_enteroUint32(buffer);
    cadena = malloc((tamanio) + 1);                             // Reserva lugar para el string con el tamanio recibido
    memcpy(cadena, buffer->stream + buffer->offset, (tamanio)); // Recibe string
    buffer->offset += tamanio;

    *(cadena + tamanio) = '\0';

    return cadena;
}

t_list *leer_buffer_instrucciones(tipo_buffer *buffer)
{
    t_list *lista_instrucciones;
    uint32_t tamanio;
    tamanio = leer_buffer_enteroUint32(buffer);

    return lista_instrucciones;
}
// t_registros *registro;
t_registros *leer_buffer_registros(tipo_buffer *buffer)
{
    t_registros *registros = malloc(sizeof(t_registros));

    registros->AX = leer_buffer_enteroUint8(buffer);
    registros->BX = leer_buffer_enteroUint8(buffer);
    registros->CX = leer_buffer_enteroUint8(buffer);
    registros->DX = leer_buffer_enteroUint8(buffer);
    registros->EAX = leer_buffer_enteroUint32(buffer);
    registros->EBX = leer_buffer_enteroUint32(buffer);
    registros->ECX = leer_buffer_enteroUint32(buffer);
    registros->EDX = leer_buffer_enteroUint32(buffer);
    registros->SI = leer_buffer_enteroUint32(buffer);
    registros->DI = leer_buffer_enteroUint32(buffer);
    return registros;
}

t_cde *leer_cde(tipo_buffer *buffer)
{
    t_cde *cde = malloc(sizeof(t_cde));
    cde->pid = leer_buffer_enteroUint32(buffer);
    cde->PC = leer_buffer_enteroUint32(buffer);
    //scde->path = leer_buffer_string(buffer);
    cde->registros = malloc(sizeof(t_registros));
    cde->registros = leer_buffer_registros(buffer);
    

    return cde;
}
void agregar_cde_buffer(tipo_buffer *buffer, t_cde *cde)
{
    //t_list *lista_instrucciones;
    agregar_buffer_para_enterosUint32(buffer,cde->pid);
    agregar_buffer_para_enterosUint32(buffer,cde->PC);
    //agregar_buffer_para_string(buffer, cde->path);
    agregar_buffer_para_registros(buffer,cde->registros);
    
}

void agregar_escrituraMemoria_buffer(tipo_buffer * buffer,t_escrituraMemoria valores)
{
    agregar_buffer_para_enterosUint32(buffer,valores.valor);
    agregar_buffer_para_enterosUint32(buffer,valores.direccionFisica);
}
t_escrituraMemoria leer_escrituraMemoria(tipo_buffer *buffer)
{
    t_escrituraMemoria valores;
    valores.valor = leer_buffer_enteroUint32(buffer);
    valores.direccionFisica = leer_buffer_enteroUint32(buffer);
    return valores;
}
// ESCRIBIR EN EL BUFFER UNA LISTA
/*void escribir_buffer_para_listas(tipo_buffer *buffer, void *args)
{
    uint32_t tamanio_lista = 0;
    uint32_t posicion = 0;

    t_list *lista = *((t_list *)args);
    tamanio_lista = list_size(lista);

    while (tamanio_lista > posicion)
    {
        list_get(lista, posicion);
        posicion++;

    }
}*/
