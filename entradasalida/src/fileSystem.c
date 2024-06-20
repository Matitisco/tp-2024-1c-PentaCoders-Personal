
#include "../include/fileSystem.h"
config_io *valores_config;
t_list *tabla_archivos_abierto;
void arrancar_interfaz_dialfs(t_interfaz *interfaz_io) // IMPLEMENTAR
{
}

// de metaData con BLOQUE_INICIAL=25
// TAMANIO_ARCHIVO=1024

void levantar_meta_data(char *path) // uno por cada archivo
{
    t_config *config_meta_data;
    config_meta_data = iniciar_config(path);
    bloque_inicial = config_get_int_value(config_meta_data, "BLOQUE_INICIAL");
    tamanio_meta_data = config_get_int_value(config_meta_data, "TAMANIO_META_DATA");

    config_destroy(config_meta_data);
}

void levantar_bitmap()
{
    int bitmap = open("/~3/src/bitmap.dat", O_RDWR);

    struct stat mystat;

    if (fstat(bitmap, &mystat) < 0)
    {
        printf("Error al establecer fstat\n");
        close(bitmap);
    }

    bmap = mmap(NULL, mystat.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, bitmap, 0);

    if (bmap == MAP_FAILED)
    {
        printf("Error al mapear a memoria: %s\n", strerror(errno));
    }

    bitarray = bitarray_create_with_mode(bmap, valores_config., MSB_FIRST);
}

void levantar_archivo_bloques() // CREAMOS EL ARCHIVO DE BLOQUES MAPEADO EN MEMORIA
{
    char *path_archivo_bloques = obtener_ruta_archivo("bloques");
    int archivo_bloque = open(path_archivo_bloques, O_RDWR, S_IRUSR | S_IWUSR); // bloques file descriptor tamArchivoBloques

    int tamArchivoBloques = valores_config->block_size * valores_config->block_count;

    if (archivo_bloque == -1)
    {
        archivo_bloque = open(config_file_system.path_bloques, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        ftruncate(archivo_bloque, tamArchivoBloques);
    }
    else
    {
        ftruncate(archivo_bloque, tamArchivoBloques);
    }

    ftruncate(archivo_bloque, tamArchivoBloques);

    bloquesMapeado = mmap(NULL, tamArchivoBloques, PROT_WRITE, MAP_SHARED, archivo_bloque, 0);
}

t_config *crear_meta_data_archivo(char *nombre_archivo)
{
    char *rutaArchivo = obtener_ruta_archivo(nombre_archivo);

    FILE *meta_data_archivo = fopen(rutaArchivo, "w");

    uint32_t bloqueInicial = obtener_primer_bloque_libre(); // TODO

    char *num_bloque_inicial_ = string_itoa(bloqueInicial);

    txt_write_in_file(meta_data_archivo, "BLOQUE_INICIAL="); // escribo en el archivo el bloque
    txt_write_in_file(meta_data_archivo, num_bloque_inicial);
    escribir_en_archivo_bloques(); // TODO
    txt_write_in_file(meta_data_archivo, "TAMANIO_ARCHIVO=0\n");
    free(num_bloque_inicial);

    fclose(meta_data_archivo);

    t_config *meta_data_archivo = config_create(rutaArchivo);

    free(rutaArchivo);

    return arch_meta_data;
}

uint32_t obtener_nro_bloque_libre()
{
    for (int nroBloque = 0; nroBloque < config_file_system.cantidad_bloques_swap; nroBloque++)
    {
        uint8_t bloqueOcupado = leer_de_bitmap(nroBloque);
        if (!bloqueOcupado)
            return nroBloque;
    }
}

void escribir_en_bitmap(uint32_t nroBloque, uint8_t valor)
{
    memcpy(bitmapBloquesSwap + nroBloque, &valor, sizeof(uint8_t));
}

uint8_t leer_de_bitmap(uint32_t nroBloque)
{
    uint8_t estadoBloque;
    memcpy(&estadoBloque, bitmapBloquesSwap + nroBloque, sizeof(uint8_t));
    return estadoBloque;
}

void levantar_arch_bloques() // La cantida dde bloques que tiene el fileSystem
{
    FILE *arch_bloque = fopen(path_arch_bloques, "r+"); // escritura y lectura de archivos ya creados
    if (arch_bloque == NULL)
    {
        arch_bloque = fopen(path_arch_bloques, "w+"); // si no lo encuntra lo crea
        if (arch_bloque != NULL)
        {
            log_info(logger, "El archivo de Bloques %s se creo", path_arch_bloques);
        }
        else
        {
            log_info(logger, "No se pudo crear el archivo");
        }
    }
    uint32_t tamanio_arch_bloque = valores_config->block_size * valores_config->block_count;
    ftruncate(fileno(arch_bloque), tamanio_arch_bloque); // le asigno el tamanio al archivo de bloques
}

void crear_arch(const char *nombre_archivo)
{
    FILE *archivo = fopen(nombre_archivo, "r");

    if (archivo == NULL)
    {
        // El archivo no existe, lo creamos en modo escritura
        archivo = fopen(nombre_archivo, "w");

        if (archivo != NULL)
        {
            log_info(logger, "El archivo %s se creo", nombre_archivo);
            // fclose(archivo); tengo que cerrarlo?
        }
        else
        {
            log_info(logger, "No se pudo crear el archivo");
        }
    }
    else
    {
        log_info(logger, "El archivo %s existe", nombre_archivo);

        // fclose(archivo);
    }

    // t_archivo *archivo = malloc(sizeof(t_archivo));
    // archivo->tamanio = 0;
    //  agregar una lista de pids
    // archivo->puntero_archivo = archivo;
    // strcpy(archivo->nombre_archivo, nombre_archivo);
    // list_add(tabla_archivos_abierto, archivo); // la lista de paginas seria la tabla
    return archivo;
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
            log_info(logger, " PID: %d- OPERACION A REALIZAR : %d", pid, F_CREATE);
            crear_archivo(nombre_archivo, pid);
            break;
        case F_TRUNCATE:
            log_info(logger, " PID: %d- OPERACION A REALIZAR : %d", pid, F_TRUNCATE);
            truncar_archivo(nombre_archivo, buffer_dialfs, pid);
            break;
        case F_READ:
            log_info(logger, " PID: %d- OPERACION A REALIZAR : %d", pid, F_READ);
            leer_archivo(nombre_archivo, buffer_dialfs, pid);
            break;
        case F_WRITE:
            log_info(logger, " PID: %d- OPERACION A REALIZAR : %d", pid, F_WRITE);
            escribir_archivo(nombre_archivo, buffer_dialfs, pid);
            break;
        case F_DELETE:
            log_info(logger, " PID: %d- OPERACION A REALIZAR : %d", pid, F_DELETE);
            eliminar_archiv0(nombre_archivo, pid);
            break;

        default:
            log_error(logger, "Error, la operacion no existe");
            break;
        }
    }
}

void actualizar_bit_map()
{
}

void crear_archivo(char *nombre_archivo, uint32_t pid)
{
    crear_arch(nombre_archivo);                          // archivo creado asoc a la interfaz
    ocupar_bloque(path_arch_bloques, nombre_archivo, 1); // asignamos un bloque del fs
    levantar_meta_data();
    actualizar_bit_map();
    // en algun momento le avisamos al kernel, que agregue el archivo asociado a la interfaz a na lista
    //

    log_info(logger, "PID: %d - Crear Archivo: %s", pid, nombre_archivo);
}
char *obtener_ruta_archivo(char *nombre_archivo)
{
    char *ruta_fcb_buscado = string_new();
    string_append(&ruta_fcb_buscado, valores_config->path_base_dialfs);
    string_append(&ruta_fcb_buscado, nombre_archivo);
    string_append(&ruta_fcb_buscado, ".dat");

    return ruta_fcb_buscado;
}

void truncar_archivo(char *nombre_archivo, tipo_buffer *buffer, uint32_t pid)
{
    uint32_t nuevo_tamanio = leer_buffer_enteroUint32(buffer);
    cambiar_tamanio_archivo(nombre_archivo, nuevo_tamanio);

    // enviar_codigo(socket_kernel, CAMBIAR_TAMANIO_OK);
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
    uint32_t tamanio = leer_buffer_para_enterosUint32(buffer);
    uint32_t direccion_fisica = leer_buffer_string(buffer);
    uint32_t puntero_archivo = leer_buffer_enteroUint32(buffer);
    enviar_cod_enum(conexion_memoria, PEDIDO_ESCRITURA);
    enviar_cod_enum(conexion_memoria, SOLICITUD_DIALFS);
    tipo_buffer *buffer_memoria = crear_buffer();
    agregar_buffer_para_string(buffer_memoria, nombre_archivo);
    agregar_buffer_para_string(buffer_memoria, direccion_fisica);
    agregar_buffer_para_enterosUint32(buffer_memoria, tamanio);
    agregar_buffer_para_string(buffer_memoria, puntero_archivo);
    agregar_buffer_para_enterosUint32(buffer_memoria, direccion_fisica);
    destruir_buffer(buffer);
    // free(nombre_archivo); este no estoy segura
    // tendria que enviarle el pid de alguna forma
    log_info(logger, "PID: %d - Escribir:  %s - Tamanio a Leer: %d - Puntero Archivo: %d", pid, nombre_archivo, tamanio, puntero_archivo);
}
void leer_archivo(char *nombre_archivo, tipo_buffer *buffer, uint32_t pid)
{
    char *direccion_fisica = leer_buffer_string(buffer);
    uint32_t tamanio = leer_buffer_enteroUint32(buffer);
    uint32_t direccion_fisica = leer_buffer_enteroUint32(buffer);
    char *puntero_archivo = leer_buffer_string(buffer);

    // char *rutaArchivo = obtener_ruta_archivo(nombre_archivo);
    // t_config *fcb = config_create(rutaArchivo);

    tipo_buffer *buffer_memoria = crear_buffer();
    enviar_cod_enum(conexion_memoria, PEDIDO_LECTURA);
    enviar_cod_enum(conexion_memoria, SOLICITUD_DIALFS);
    // void *bloqueLeido = leer_en_archivo_bloques(fcb, puntero_archivo);
    agregar_buffer_para_string(buffer_memoria, nombre_archivo);
    agregar_buffer_para_string(buffer_memoria, direccion_fisica);
    agregar_buffer_para_enterosUint32(buffer_memoria, tamanio);
    agregar_buffer_para_string(buffer_memoria, puntero_archivo);
    agregar_buffer_para_enterosUint32(buffer_memoria, direccion_fisica);
    destruir_buffer(buffer);
    log_info(logger, "PID: %d - Leer Archivo:  %s - Tamanio a Leer: %d - Puntero Archivo: %d", pid, nombre_archivo, tamanio, puntero_archivo);
    // free(nombre_archivo); este no estoy segura
    // tendria que enviarle el pid de alguna forma
}
void cambiar_tamanio_archivo(char *nombre_archivo, uint32_t nuevo_tamanio)
{

    char *ruta_fcb_buscado = obtener_ruta_archivo(nombre_archivo);
    t_config *fcb_buscado = config_create(ruta_fcb_buscado); // archivo metadata
    char *tamanio_a_aplicar;
    tamanio_a_aplicar = string_itoa(nuevo_tamanio);

    uint32_t tamanio_archivo_anterior = config_get_int_value(fcb_buscado, "TAMANIO_ARCHIVO");
    uint32_t bloque_inicial = config_get_int_value(fcb_buscado, "BLOQUE_INICIAL");

    config_set_value(fcb_buscado, "TAMANIO_ARCHIVO", tamanio_a_aplicar);
    config_save_in_file(fcb_buscado, ruta_fcb_buscado);

    if (nuevo_tamanio > tamanio_archivo_anterior)
    {
        for (int i = 0; i < (nuevo_tamanio - tamanio_archivo_anterior) / valores_config->tam_bloque; i++)
        {
            // agregar_bloque_();
        }
    }

    else
    {
        for (int i = 0; (tamanio_archivo_anterior - nuevo_tamanio) / valores_config->tam_bloque; i++)
        // sacar_bloque_();
    }

    config_destroy(fcb_buscado);
    free(ruta_fcb_buscado);
    free(tamanio_a_aplicar);
}
