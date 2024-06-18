
#include "../include/fileSystem.h"

void arrancar_interfaz_dialfs(t_interfaz *interfaz_io) // IMPLEMENTAR
{
}
void instrucciones_dialfs()

{
    while (1)
    {
        operacion_dialfs codigo = recibir_operacion(conexion_kernel);
        tipo_buffer *buffer_dialfs = recibir_buffer(conexion_kernel);
        char *nombre_archivo = leer_buffer_string(buffer_dialfs);
        uint32_t pid = leer_buffer_enteroUint32(buffer_dialfs);

        switch (codigo)
        {
        case F_CREATE:
            log_info(logger," PID: %d- OPERACION A REALIZAR : %d",pid, F_CREATE);
            crear_archivo(nombre_archivo, pid);
            break;
        case F_TRUNCATE:
            log_info(logger," PID: %d- OPERACION A REALIZAR : %d",pid, F_TRUNCATE);
            truncar_archivo(nombre_archivo, buffer_dialfs, pid);
            break;
        case F_READ:
            log_info(logger," PID: %d- OPERACION A REALIZAR : %d",pid, F_READ);
            leer_archivo(nombre_archivo, buffer_dialfs, pid);
            break;
        case F_WRITE:
            log_info(logger," PID: %d- OPERACION A REALIZAR : %d",pid, F_WRITE);
            escribir_archivo(nombre_archivo, buffer_dialfs, pid);
            break;
        case F_DELETE:
            log_info(logger," PID: %d- OPERACION A REALIZAR : %d",pid, F_DELETE);
            eliminar_archiv0(nombre_archivo, pid);
            break;

        default:
            log_error("Error, la operacion no existe");
            break;
        }
    }
}

void crear_archivo(char *nombre_archivo, uint32_t pid)
{

    t_config *fcb_buscado = crear_fcb_archivo(nombre_archivo);
    config_destroy(fcb_buscado);

    // enviar_codigo(conexion_kernel, CREAR_ARCHIVO_OK);
    log_info(logger, "PID: %d - Crear Archivo: %s", pid, nombre_archivo);
}

void truncar_archivo(char *nombre_archivo, tipo_buffer *buffer, uint32_t pid)
{
    uint32_t nuevo_tamanio = leer_buffer_enteroUint32(buffer);
    cambiar_tamanio_archivo(nombre_archivo, nuevo_tamanio);

    //enviar_codigo(socket_kernel, CAMBIAR_TAMANIO_OK);
    log_info(logger, "PID: %d - Truncar Archivo: %s> -Tamaño: %d", pid, nombre_archivo, nuevo_tamanio);
}
void eliminar_archivo(char *nombre_archivo, uint32_t pid)
{
    /*Eliminar el archivo de metadatos:
     Esto implica eliminar el archivo .meta asociado al archivo que deseas eliminar.
     Actualizar el bitmap: Marcar los bloques que eran usados por el archivo como libres en el bitmap.*/
    log_info(logger, "PID: %d - Eliminar archivo: %s", pid, nombre_archivo);
}

void escribir_archivo(char *nombre_archivo, tipo_buffer *buffer, uint32_t pid)
{
    char *direccion_fisica = leer_buffer_string(buffer);
    char *tamanio = leer_buffer_string(buffer);
    char *reg_direccion = leer_buffer_string(buffer);
    char *puntero_archivo = leer_buffer_string(buffer);
    enviar_cod_enum(conexion_memoria, PEDIDO_ESCRITURA);
    enviar_cod_enum(conexion_memoria, SOLICITUD_DIALFS);
    tipo_buffer *buffer_memoria = crear_buffer();
    agregar_buffer_para_string(buffer_memoria, nombre_archivo);
    agregar_buffer_para_string(buffer_memoria, direccion_fisica);
    agregar_buffer_para_string(buffer_memoria, tamanio);
    agregar_buffer_para_string(buffer_memoria, puntero_archivo);
    agregar_buffer_para_string(buffer_memoria, reg_direccion);
    destruir_buffer(buffer);
    // free(nombre_archivo); este no estoy segura
    // tendria que enviarle el pid de alguna forma
    log_info(logger, "PID: %d - Escribir:  %s - Tamanio a Leer: %d - Puntero Archivo: %d", pid, nombre_archivo,tamanio, puntero_archivo);
}
void leer_archivo(char *nombre_archivo, tipo_buffer *buffer, uint32_t pid)
{
    char *direccion_fisica = leer_buffer_string(buffer);
    char *tamanio = leer_buffer_string(buffer);
    char *reg_direccion = leer_buffer_string(buffer);
    char *puntero_archivo = leer_buffer_string(buffer);
    

    char* rutaArchivo = obtener_ruta_archivo(nombre_archivo);
    t_config* fcb = config_create(rutaArchivo);

    tipo_buffer *buffer_memoria = crear_buffer();
    enviar_cod_enum(conexion_memoria, PEDIDO_LECTURA);
    enviar_cod_enum(conexion_memoria, SOLICITUD_DIALFS);
    

    void* bloqueLeido = leer_en_archivo_bloques(fcb, puntero);
    agregar_buffer_para_string(buffer_memoria, nombre_archivo);
    agregar_buffer_para_string(buffer_memoria, direccion_fisica);
    agregar_buffer_para_string(buffer_memoria, tamanio);
    agregar_buffer_para_string(buffer_memoria, puntero_archivo);
    agregar_buffer_para_string(buffer_memoria, reg_direccion);
    destruir_buffer(buffer);
    log_info(logger, "PID: %d - Leer Archivo:  %s - Tamanio a Leer: %d - Puntero Archivo: %d", pid, nombre_archivo,tamanio, puntero_archivo);
    // free(nombre_archivo); este no estoy segura
    // tendria que enviarle el pid de alguna forma
    
}
