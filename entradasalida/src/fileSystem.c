#include "../include/fileSystem.h"

uint32_t bitarray_pointer;
int conexion_kernel, conexion_memoria, estoy_libre;
void *bloquesMapeado;
t_list *archivos_fs;
char *nombre_archivo_buscado;
int bloque_inicial_archivo;

// INICIAR BITMAP FS EN MEMORIA
void levantar_bitmap()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    strcat(cwd, "/dialfs/bitmap.dat");

    int bitmap = open(cwd, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    struct stat mystat;

    if (fstat(bitmap, &mystat) < 0)
    {
        close(bitmap);
        return;
    }

    size_t tamanio_bitmap = config_interfaz->block_count / 8;

    if (ftruncate(bitmap, tamanio_bitmap) != 0)
    {
        log_error(logger, "Error al truncar el archivo bitmap.dat");
        close(bitmap);
        return;
    }

    void *bmap = mmap(NULL, tamanio_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, bitmap, 0);

    if (bmap == MAP_FAILED)
    {
        log_error(logger, "Error al mapear a memoria del bitmap");
    }
    else
    {
        bitarray = bitarray_create_with_mode(bmap, config_interfaz->block_count, LSB_FIRST);
    }
}

// INICIAR ARCHIVO DE BLOQUES EN MEMORIA
void levantar_archivo_bloques()
{
    path_arch_bloques = obtener_ruta_archivo("bloques.dat");

    FILE *fbloques = fopen(path_arch_bloques, "rb+");

    int tamArchivoBloques = config_interfaz->block_size * config_interfaz->block_count;

    archivos_fs = list_create();
    int file_descriptor;
    if (fbloques == NULL)
    {
        fbloques = fopen(path_arch_bloques, "ab+");
        file_descriptor = fileno(fbloques);
        ftruncate(file_descriptor, tamArchivoBloques);
    }
    else
    {
        file_descriptor = fileno(fbloques);
        ftruncate(file_descriptor, tamArchivoBloques);
    }

    file_descriptor = fileno(fbloques);
    ftruncate(file_descriptor, tamArchivoBloques);

    bloquesMapeado = mmap(NULL, tamArchivoBloques, PROT_WRITE, MAP_SHARED, file_descriptor, 0);

    if (bloquesMapeado == MAP_FAILED)
    {
        log_error(logger, "Error al mapear a memoria el archivo de bloques");
    }
}

// INSTRUCCIONES DEL FILE SYSTEM
void instrucciones_dialfs()
{
    tipo_buffer *buffer_dialfs = recibir_buffer(conexion_kernel);
    t_tipoDeInstruccion codigo = leer_buffer_enteroUint32(buffer_dialfs);
    uint32_t pid = leer_buffer_enteroUint32(buffer_dialfs);
    char *nombre_archivo;
    uint32_t nuevo_tamanio;
    uint32_t tamanio;
    uint32_t tamanio_marco;
    uint32_t direccion_fisica;
    uint32_t puntero_archivo;
    switch (codigo)
    {
    case IO_FS_CREATE:
        nombre_archivo = leer_buffer_string(buffer_dialfs);
        log_info(logger, "PID: <%d> - Operacion: <IO_FS_CREATE>", pid); // obligatorio
        crear_archivo(nombre_archivo, pid);
        break;
    case IO_FS_TRUNCATE:
        nuevo_tamanio = leer_buffer_enteroUint32(buffer_dialfs);
        nombre_archivo = leer_buffer_string(buffer_dialfs);
        log_info(logger, "PID: <%d> - Operacion: <IO_FS_TRUNCATE>", pid); // obligatorio
        truncar_archivo(nombre_archivo, nuevo_tamanio, pid);
        break;
    case IO_FS_READ:
        tamanio = leer_buffer_enteroUint32(buffer_dialfs);
        direccion_fisica = leer_buffer_enteroUint32(buffer_dialfs);
        puntero_archivo = leer_buffer_enteroUint32(buffer_dialfs);
        tamanio_marco = leer_buffer_enteroUint32(buffer_dialfs);
        nombre_archivo = leer_buffer_string(buffer_dialfs);
        log_info(logger, "PID: <%d> - Operacion: <IO_FS_READ>", pid); // obligatorio
        leer_archivo(nombre_archivo, tamanio, tamanio_marco, direccion_fisica, puntero_archivo, pid);
        break;
    case IO_FS_WRITE:
        tamanio = leer_buffer_enteroUint32(buffer_dialfs);
        direccion_fisica = leer_buffer_enteroUint32(buffer_dialfs);
        puntero_archivo = leer_buffer_enteroUint32(buffer_dialfs);
        nombre_archivo = leer_buffer_string(buffer_dialfs);
        log_info(logger, "PID: <%d> - Operacion: <IO_FS_WRITE>", pid); // obligatorio
        escribir_archivo(nombre_archivo, tamanio, direccion_fisica, puntero_archivo, pid);
        break;
    case IO_FS_DELETE:
        nombre_archivo = leer_buffer_string(buffer_dialfs);
        log_info(logger, "PID: <%d> - Operacion: <IO_FS_DELETE>", pid); // obligatorio
        eliminar_archivo(nombre_archivo, pid);
        break;
    default:
        log_error(logger, "Error, la operacion no existe");
        break;
    }
    destruir_buffer(buffer_dialfs);
}

// CREAR ARCHIVO
void crear_archivo(char *nombre_archivo, uint32_t pid)
{
    t_config *meta_data_archivo = crear_meta_data_archivo(nombre_archivo);

    if (meta_data_archivo)
    {
        // obligatorio
        log_info(logger, "PID: <%d> - Crear Archivo: <%s>", pid, nombre_archivo);
    }
    else
    {
        enviar_op_code(conexion_kernel, ERROR_CREAR_ARCHIVO_OK);
    }
}

t_config *crear_meta_data_archivo(char *nombre_archivo)
{
    char *rutaArchivo = obtener_ruta_archivo(nombre_archivo);

    FILE *meta_data_archivo = fopen(rutaArchivo, "w");

    int prim_bloq_libre = bloque_libre();

    char *num_bloque_inicial = string_itoa(prim_bloq_libre);
    int tamanio = 0;
    txt_write_in_file(meta_data_archivo, "BLOQUE_INICIAL="); // escribimos en el archivo el bloque
    txt_write_in_file(meta_data_archivo, num_bloque_inicial);
    txt_write_in_file(meta_data_archivo, "\n");
    txt_write_in_file(meta_data_archivo, "TAMANIO_ARCHIVO=");
    txt_write_in_file(meta_data_archivo, string_itoa(tamanio));

    free(num_bloque_inicial);

    t_config *meta_data_archivo_config = config_create(rutaArchivo);

    free(rutaArchivo);

    t_archivo_data *archivo = malloc(sizeof(t_archivo_data));
    archivo->bloque_inicial = bloque_inicial;
    archivo->nombre_archivo = nombre_archivo;
    archivo->tamanio = tamanio;
    list_add(archivos_fs, archivo);
    fclose(meta_data_archivo);
    return meta_data_archivo_config;
}

char *obtener_ruta_archivo(char *nombre_archivo)
{
    char *ruta_fcb_buscado = string_new();
    string_append(&ruta_fcb_buscado, config_interfaz->path_base_dialfs);
    string_append(&ruta_fcb_buscado, "/");
    string_append(&ruta_fcb_buscado, nombre_archivo);

    return ruta_fcb_buscado;
}

// TRUNCAR ARCHIVO
void truncar_archivo(char *nombre_archivo, int nuevo_tamanio, uint32_t pid)
{
    cambiar_tamanio_archivo(nombre_archivo, nuevo_tamanio, pid);
    // obligtorio
    log_info(logger, "PID: <%d> - Truncar Archivo: <%s> - Tamaño: <%d>", pid, nombre_archivo, nuevo_tamanio);
}

void eliminar_arch(void *data)
{
    t_archivo_data *archivo = (t_archivo_data *)data;
    free(archivo->nombre_archivo);
}

// ELIMINAR ARCHIVO
void eliminar_archivo(char *nombre_archivo, uint32_t pid)
{
    nombre_archivo_buscado = nombre_archivo;
    list_remove_and_destroy_by_condition(archivos_fs, buscar_arch_por_nombre, eliminar_arch);
    t_config *metadata_buscado = buscar_meta_data(nombre_archivo);
    double tamanio_archivo = config_get_int_value(metadata_buscado, "TAMANIO_ARCHIVO");
    double tamanio_bloque = config_interfaz->block_size;
    double cant_bloques_ocupados;

    if (tamanio_archivo == 0)
        cant_bloques_ocupados = 1;
    else
        cant_bloques_ocupados = ceil(tamanio_archivo / tamanio_bloque);

    int posicion_inicial = config_get_int_value(metadata_buscado, "BLOQUE_INICIAL");
    for (int i = 0; i < cant_bloques_ocupados; i++)
    {
        liberarBloque(posicion_inicial + i);
    }
    char *ruta_archivo = obtener_ruta_archivo(nombre_archivo);
    remove(ruta_archivo);
    free(ruta_archivo);
    // log obligatorio
    log_info(logger, "PID: <%d> - Eliminar archivo: <%s>", pid, nombre_archivo);
}

// ESCRIBIR ARCHIVO
void escribir_archivo(char *nombre_archivo, uint32_t tamanio, uint32_t direccion_fisica, uint32_t puntero_archivo, uint32_t pid)
{
    enviar_op_code(conexion_memoria, ACCESO_ESPACIO_USUARIO);
    enviar_op_code(conexion_memoria, PEDIDO_LECTURA);

    tipo_buffer *buffer_memoria = crear_buffer();
    int total_bloques;
    int tamanio_bloque;
    agregar_buffer_para_enterosUint32(buffer_memoria, direccion_fisica);
    agregar_buffer_para_enterosUint32(buffer_memoria, pid);
    agregar_buffer_para_enterosUint32(buffer_memoria, tamanio);
    agregar_buffer_para_enterosUint32(buffer_memoria, STRING);
    enviar_buffer(buffer_memoria, conexion_memoria);
    // LEER EN MEMORIA EL VALOR
    op_code codigo_memoria = recibir_op_code(conexion_memoria);
    if (codigo_memoria == OK)
    {
        tipo_buffer *buffer_recibido = recibir_buffer(conexion_memoria);
        void *texto_leido = calloc(1, tamanio);
        texto_leido = leer_buffer_string(buffer_recibido);

        t_config *metadata = buscar_meta_data(nombre_archivo);
        int bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
        int tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
        tamanio_bloque = config_interfaz->block_size;

        long offset = bloque_inicial * config_interfaz->block_size + puntero_archivo;
        memcpy(bloquesMapeado + offset, texto_leido, tamanio);
        total_bloques = config_interfaz->block_count;
        msync(bloquesMapeado, total_bloques * tamanio_bloque, MS_SYNC);
        sleep_ms(config_interfaz->tiempo_unidad_trabajo);
        log_info(logger, "PID: <%d> - Operacion: <IO_DIALFS_WRITE>", pid);
    }
    else if (codigo_memoria == ERROR_PEDIDO_LECTURA)
    {
        log_error(logger, "PID: <%d> - ERROR Operacion: <IO_DIALFS>", pid);
    }

    log_info(logger, "PID: <%d> - Escribir Archivo: <%s> - Tamaño a Escribir: <%d> - Puntero Archivo: <%d>", pid, nombre_archivo, tamanio, puntero_archivo);
}

// LEER ARCHIVO
void leer_archivo(char *nombre_archivo, uint32_t tamanio, uint32_t tamanio_marco, uint32_t direccion_fisica, uint32_t puntero_archivo, uint32_t pid)
{
    void *valor_a_leer = calloc(1, tamanio);
    t_config *metadata = buscar_meta_data(nombre_archivo);
    int bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
    int tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");

    long offset = (bloque_inicial * config_interfaz->block_size) + puntero_archivo;

    memcpy(valor_a_leer, bloquesMapeado + offset, tamanio);
    escribir_dato_memoria(direccion_fisica, tamanio_marco, valor_a_leer, tamanio, pid);
    log_info(logger, "PID: <%d> - Leer Archivo: <%s> - Tamaño a Leer: <%d> - Puntero Archivo: <%d>", pid, nombre_archivo, tamanio, puntero_archivo);
}

t_archivo_data *agregar_archivo(char *nombre_archivo)
{
    char *ruta_archivo = obtener_ruta_archivo(nombre_archivo);
    t_config *metadata = config_create(ruta_archivo);
    int tamanio_archivo = config_get_int_value(ruta_archivo, "TAMANIO_ARCHIVO");
    int bloque_inicial = config_get_int_value(ruta_archivo, "BLOQUE_INICIAL");
    t_archivo_data *archivo_a_agregar = malloc(sizeof(t_archivo_data));
    archivo_a_agregar->nombre_archivo = nombre_archivo;
    archivo_a_agregar->tamanio = tamanio_archivo;
    archivo_a_agregar->bloque_inicial = bloque_inicial;
    list_add(archivos_fs, archivo_a_agregar);
    free(metadata);
    free(ruta_archivo);
    return archivo_a_agregar;
}

// CAMBIAR TAMAÑO
void cambiar_tamanio_archivo(char *nombre_archivo, int tamanio_nuevo, int pid)
{
    nombre_archivo_buscado = nombre_archivo;
    t_archivo_data *archivo = list_find(archivos_fs, buscar_arch_por_nombre);
    if (archivo == NULL)
    {
        archivo = agregar_archivo(nombre_archivo);
    }

    int tamanio_anterior = archivo->tamanio;
    int bloques_usados = ceil(tamanio_anterior / config_interfaz->block_size);
    int blqoues_totales = ceil(tamanio_nuevo / config_interfaz->block_size);
    int cantidad_bloques_agregar = blqoues_totales - bloques_usados;

    if (tamanio_nuevo > tamanio_anterior)
    {
        ampliar_archivo(archivo, tamanio_nuevo, cantidad_bloques_agregar, pid);
    }
    else
    {
        reducir_archivo(archivo, tamanio_nuevo, cantidad_bloques_agregar, pid);
    }
    // ahora aca actualizamos el metadata

    char *ruta_archivo = obtener_ruta_archivo(nombre_archivo);
    t_config *metadata = config_create(ruta_archivo);
    nombre_archivo_buscado = nombre_archivo;
    t_archivo_data *archivo_compactado = list_find(archivos_fs, buscar_arch_por_nombre);
    int tamanio_real = archivo_compactado->tamanio;
    config_set_value(metadata, "TAMANIO_ARCHIVO", string_itoa(tamanio_real));
    config_set_value(metadata, "BLOQUE_INICIAL", string_itoa(archivo_compactado->bloque_inicial));
    config_save(metadata);
    // 11111110 00000001 00000000 00000000
    // 00000000 00000000 00000001 11111110
}

void ampliar_archivo(t_archivo_data *archivo, int tamanio, int cantidad_bloques_agregar, int pid)
{
    if (hay_espacio_disponible(cantidad_bloques_agregar))
    {
        if (espacio_disponible_es_contiguo(cantidad_bloques_agregar, archivo))
        {
            int bloque_final = bloque_inicial + ceil(archivo->tamanio / config_interfaz->block_size) - 1;
            int bloque_final_ampiado = bloque_final + cantidad_bloques_agregar;
            for (int i = bloque_final; i <= bloque_final_ampiado; i++)
            {
                (bitarray_set_bit(bitarray, i));
            }
            msync(bitarray->bitarray, bitarray->size, MS_SYNC);
        }
        else
        {
            log_info(logger, "PID: <%d> - Inicio Compactacion", pid); // obligatorio
            compactar(tamanio, archivo);
            log_info(logger, "PID: <%d> - Fin Compactacion", pid); // obligatorio
        }
    }
    else
    {
        log_error(logger, "FS - No hay mas espacio disponible :(");
    }
}

void reducir_archivo(t_archivo_data *archivo, int tamanio_nuevo, int cantidad_bloques_agregar, int pid)
{
    uint32_t bloques_a_eliminar = ceil(archivo->tamanio - tamanio_nuevo / config_interfaz->block_size) - 1;
    uint32_t bloque_final_sin_ampliar = ceil(archivo->bloque_inicial + archivo->tamanio / config_interfaz->block_size) - 1;
    uint32_t bloque_final_reducido = bloque_final_sin_ampliar - bloques_a_eliminar;

    for (int i = bloque_final_sin_ampliar; i < bloque_final_reducido; i--)
    {
        bitarray_clean_bit(bitarray, i); // va seteando el bloque indicado en 0
    }
    msync(bitarray->bitarray, bitarray->size, MS_SYNC);
}

bool hay_espacio_disponible(uint32_t cant_bloques_agregar)
{
    int bloques_ocupados = contar_bloques_ocupados_bitarray();
    int bloques_disponibles = config_interfaz->block_count - bloques_ocupados;
    return cant_bloques_agregar <= bloques_disponibles;
}

bool espacio_disponible_es_contiguo(int cantidad_bloques_agregar, t_archivo_data *archivo)
{
    int bloque_final = archivo->bloque_inicial + ceil(archivo->tamanio / config_interfaz->block_size) ;
    if (bloque_final < 0)
    {
        bloque_final = 0;
    }
    log_info(logger, "CONTIGUO - Bloque final del archivo : %d", bloque_final);
    int bloque_final_ampiado = bloque_final + cantidad_bloques_agregar - 1;
    log_info(logger, "CONTIGUO - En teoria, nuevo Bloque final del archivo : %d", bloque_final_ampiado);
    int contador_libres = 0;
    for (int i = bloque_final; i <= bloque_final_ampiado; i++)
    {
        if (bitarray_test_bit(bitarray, i) == 0)
        {
            contador_libres++;
        }
    }
    if (cantidad_bloques_agregar == contador_libres)
    {
        return true;
    }
    return false;
}

int contar_bloques_ocupados_bitarray()
{
    int contar_ocupados = 0;
    for (int i = 0; i < bitarray->size; i++)
    {
        if (bitarray_test_bit(bitarray, i) == 1)
        {
            contar_ocupados++;
        }
    }
    return contar_ocupados;
}

uint32_t bloque_libre()
{
    uint32_t bloque_libre = bitarray->size;

    for (int i = 0; i < config_interfaz->block_count; i++)
    {
        if (bitarray_test_bit(bitarray, i) == 0)
        {
            bitarray_set_bit(bitarray, i);
            bloque_libre = i;
            if (bloque_libre == bitarray->size) // No se encontró ningún bloque libre
            {
                return -1;
            }

            int sync = msync(bitarray->bitarray, bitarray->size, MS_SYNC);
            if (sync == -1)
            {
                return -1;
            }
            return bloque_libre;
        }
    }
    return -1;
}

t_config *buscar_meta_data(char *nombre_archivo)
{
    char *ruta_metadata = obtener_ruta_archivo(nombre_archivo);
    t_config *metadata = config_create(ruta_metadata);

    if (metadata == NULL || !config_has_property(metadata, "TAMANIO_ARCHIVO") || !config_has_property(metadata, "BLOQUE_INICIAL"))
    {
        if (metadata != NULL)
        {
            config_destroy(metadata);
        }
        free(ruta_metadata);
        return NULL;
    }

    free(ruta_metadata);
    return metadata;
}

int liberarBloque(uint32_t bit_bloque)
{
    bitarray_clean_bit(bitarray, bit_bloque);
    int sync = msync(bitarray->bitarray, bitarray->size, MS_SYNC);
    if (sync == -1)
    {
        log_error(logger, "Error al sincronizar los datos al liberar el bloque: <%d>", bit_bloque);
    }

    return 0;
}
/*
void compactaAr(char *nombre_archivo, int cantidad_bloques_agregar)
{
    desplazar_archivos_y_eliminar_bloques_libres();
    int primer_bloque_libre = bloque_libre();
    mover_archivo_al_primer_bloque_libre(primer_bloque_libre, nombre_archivo, cantidad_bloques_agregar, metadata);
    sleep_ms(config_interfaz->retraso_compactacion);
}*/

_Bool buscar_arch_por_nombre(void *data)
{
    t_archivo_data *archivo = (t_archivo_data *)data;
    return strcmp(archivo->nombre_archivo, nombre_archivo_buscado) == 0;
}
/*
void mover_archivo_al_primer_bloque_libre(int primer_bloque_libre, char *nombre_archivo, int cantidad_bloques_agregar, t_config *metadata)
{
    int total_bloques = config_interfaz->block_count;
    int tam_bloque = config_interfaz->block_size;
    int cantidad_bloques_archivo = obtener_cantidad_bloques_archivo(nombre_archivo);
    nombre_archivo_buscado = nombre_archivo;
    int primer_bloque_para_config = primer_bloque_libre;
    int primer_bloque;

    t_archivo_data *info_archivo = list_find(archivos_fs, buscar_arch_por_nombre);
    if (info_archivo == NULL)
    {
        t_archivo_data *info_archivo = malloc(sizeof(t_archivo_data));
        info_archivo->nombre_archivo = nombre_archivo;
        t_config *metadata_fs = buscar_meta_data(nombre_archivo);
        int bloque_inicial = config_get_int_value(metadata_fs, "BLOQUE_INICIAL");
        int tamanio = config_get_int_value(metadata_fs, "TAMANIO_ARCHIVO");
        info_archivo->bloque_inicial = bloque_inicial;
        info_archivo->tamanio = tamanio;

        primer_bloque = info_archivo->bloque_inicial;
        info_archivo->bloque_inicial = primer_bloque_libre;
        list_add(archivos_fs, info_archivo);
    }
    else
    {
        primer_bloque = info_archivo->bloque_inicial;
        info_archivo->bloque_inicial = primer_bloque_libre;
    }

    for (int i = 0; i < cantidad_bloques_archivo; i++) // muevo los bloques que ya tiene
    {
        int bloque_a_mover = primer_bloque + i; // obtengo los bloques de ese archivo
        void *src = bloquesMapeado + (bloque_a_mover * tam_bloque);
        void *dest = bloquesMapeado + (primer_bloque_libre * tam_bloque);
        memmove(dest, src, tam_bloque);
        // Actualizar el bitarray y la metadata del archivo
        bitarray_clean_bit(bitarray, bloque_a_mover);
        bitarray_set_bit(bitarray, primer_bloque_libre);
        // Avanzar al siguiente bloque libre al final del FS
        primer_bloque_libre++;
    }
    for (int i = primer_bloque_libre; i < primer_bloque_libre + cantidad_bloques_agregar - 1; i++) // Agrega la cantidad de bloques a agrega
    {
        bitarray_set_bit(bitarray, i);
    }
    msync(bitarray->bitarray, bitarray->size, MS_SYNC);
    msync(bloquesMapeado, total_bloques * tam_bloque, MS_SYNC);
    config_set_value(metadata, "BLOQUE_INICIAL", string_itoa(primer_bloque_para_config));
}

int obtener_cantidad_bloques_archivo(char *nombre_archivo)
{
    t_config *metadata = buscar_meta_data(nombre_archivo);
    double tamanio_bloque = config_interfaz->block_size;
    double tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
    if (tamanio_archivo == 0)
    {
        return 1;
    }
    else
    {
        double cantidad_bloques = ceil(tamanio_archivo / tamanio_bloque);
        return cantidad_bloques;
    }
}
/*
void desplazar_archivos_y_eliminar_bloques_libres()
{
    int es_primer_bloque = 1;
    int cant_bloques;
    t_queue *lista_bloques_libres = queue_create();

    for (int i = 0; i < config_interfaz->block_count; i++)
    {
        if (bitarray_test_bit(bitarray, i) == 0)
        {
            queue_push(lista_bloques_libres, i);
        }
        else
        {
            if (!queue_is_empty(lista_bloques_libres))
            {
                int bloque_a_ser_ocupado = queue_pop(lista_bloques_libres);
                mover_bloque(i, bloque_a_ser_ocupado);
                bitarray_set_bit(bitarray, bloque_a_ser_ocupado);
                bitarray_clean_bit(bitarray, i);
                bloque_inicial_archivo = i; // el original

                if (es_primer_bloque)
                {
                    cant_bloques = actualizar_metadata_archivo(bloque_a_ser_ocupado);
                    es_primer_bloque = 0;
                }
                cant_bloques--;
                if (cant_bloques <= 0)
                {
                    es_primer_bloque = 1;
                }

                i = -1;
            }
        }
    }
}
*/
void mover_bloque(int origen, int destino)
{
    int tamano_bloque = config_interfaz->block_size;
    char *contenido_bloque = malloc(tamano_bloque);
    memcpy(contenido_bloque, bloquesMapeado + (origen * tamano_bloque), tamano_bloque);
    memcpy(bloquesMapeado + (destino * tamano_bloque), contenido_bloque, tamano_bloque);
    free(contenido_bloque);
}

int actualizar_metadata_archivo(int nuevo_bloque_inicial) // de los archivos, NO EL QUE QUEREMOS TRUNCAR
{
    char *nuevo_bloque_string = string_itoa(nuevo_bloque_inicial);
    t_archivo_data *archivo = list_find(archivos_fs, buscar_por_bloque);
    if (archivo == NULL)
    {
        // log_info(logger, "nO LO ENCONTRE DEVULEVO 0");
        return 0;
    }
    else
    {
        archivo->bloque_inicial = nuevo_bloque_inicial;
        t_config *metadata = buscar_meta_data(archivo->nombre_archivo);
        config_set_value(metadata, "BLOQUE_INICIAL", nuevo_bloque_string); // actualizar el de los otros archivos
        config_save(metadata);
    }
    return ceil(archivo->tamanio / config_interfaz->block_size);
}

_Bool buscar_por_bloque(void *data)
{
    t_archivo_data *archivo = (t_archivo_data *)data;
    return archivo->bloque_inicial == bloque_inicial_archivo;
}

int bloques_necesarios(int tamanio)
{
    return ceil(tamanio / config_interfaz->block_size);
}

char *obtener_informacion_archivo(t_archivo_data *archivo, int bloques_totales)
{
    int bloque_inicial = archivo->bloque_inicial;
    char *contenido_bloques = calloc(1, bloques_totales * config_interfaz->block_size);
    contenido_bloques = leer_bloques(bloque_inicial, bloques_totales * config_interfaz->block_size);
    return contenido_bloques; /// contneido de los blqoues
}
// nuevo compactar, no usamos mas metadata, nos manejamos directamente con listas y luego al final de compactar
// lo acutalizamos
void compactar(int nuevo_tamanio, t_archivo_data *archivo)
{
    int bloques_totales = bloques_necesarios(nuevo_tamanio); // tenemos los bloques necesarios totales para el archivo
    char *contenido_bloque = calloc(1, nuevo_tamanio);

    // obtenemos la informacion del archivo a mover
    contenido_bloque = obtener_informacion_archivo(archivo, bloques_totales);

    // liberamos los bloques que estamos usando ahora
    marcar_bloques_libres(archivo->bloque_inicial, bloques_necesarios(archivo->tamanio) + archivo->bloque_inicial - 1);

    for (int i = 0; i < list_size(archivos_fs) - 1; i++)
    {
        t_archivo_data *arch_a_liberar = list_get(archivos_fs, i);
        if (arch_a_liberar->bloque_inicial == archivo->bloque_inicial)
        {
            // continue;//se cumple la condicion  y sale la ejecuon
            int bloque_inicial = arch_a_liberar->bloque_inicial;
            int bloque_final = bloques_necesarios((arch_a_liberar->tamanio) + bloque_inicial - 1);
            int tamanio = bloques_necesarios(arch_a_liberar->tamanio) * config_interfaz->block_size;

            char *contenido_bloques = calloc(1, tamanio);
            contenido_bloques = buscar_contenido_de(arch_a_liberar);

            int nuevo_bloque_inicial = copiar_contenido_a(contenido_bloques, tamanio); // ya compactaste

            arch_a_liberar->bloque_inicial = nuevo_bloque_inicial; // nuevo bloque inicial

            /* crear_arch_list(arch_a_liberar); // creo el nuevo elemento de la lista y lo agrego a archivos:fs.
            // va a tener el nuevo bloque inicial
            // actualizar la lista */
            t_archivo_data *archivo_sacado = list_remove(archivos_fs, i);
            archivo_sacado->bloque_inicial = nuevo_bloque_inicial;
            archivo_sacado->nombre_archivo = arch_a_liberar->nombre_archivo;
            list_add(archivos_fs, archivo_sacado);
            marcar_bloques_ocupados(archivo_sacado->bloque_inicial, bloque_final);

            // free(arch_a_liberar->nombre_archivo);
            // free(arch_a_liberar);
        }
    }

    int nuevo_bloque_inicial_archivo_agrandar = copiar_contenido_a(contenido_bloque, nuevo_tamanio);
    return nuevo_bloque_inicial_archivo_agrandar;
}
void crear_arch_list(t_archivo_data *arch)
{

    list_add(arch, archivos_fs);
}
int copiar_contenido_a(char *contenido, int tamanio)
{
    int bloque_inicial = bloque_libre();
    long offset = bloque_inicial * config_interfaz->block_size; // aca punterp archivo es 0
    memcpy(bloquesMapeado + offset, contenido, tamanio);        // se supone que puntero ava  ser 0

    if (bloques_necesarios(tamanio) == 1)
    {
        marcar_bloques_ocupados(bloque_inicial, 1);
    }
    else
    {
        marcar_bloques_ocupados(bloque_inicial, bloques_necesarios(tamanio));
    }
    return bloque_inicial;
}
char *buscar_contenido_de(t_archivo_data *arch)
{
    int bloque_inicial = arch->bloque_inicial;
    int bloques_a_leer = bloques_necesarios(arch->tamanio);
    int tamanio_a_leer = bloques_a_leer * config_interfaz->block_size;

    char *contenido = malloc(tamanio_a_leer);

    contenido = leer_bloques(bloque_inicial, tamanio_a_leer);

    return contenido;
}
char *leer_bloques(int bloque_inicial, uint32_t tamanio_a_leer)
{
    int bloques = bloques_necesarios(tamanio_a_leer);
    char *contenido = calloc(1, config_interfaz->block_count * tamanio_a_leer);
    for (int i = bloque_inicial; i < bloques + bloque_inicial; i++)
    {
        long offset = bloque_inicial * config_interfaz->block_size;
        memcpy(contenido, bloquesMapeado + offset, tamanio_a_leer);
    }
    return contenido;
}
void marcar_bloques_libres(int inicio, int final)
{
    for (int i = inicio; i < final; i++)
    {
        bitarray_clean_bit(bitarray, i);
        int sync = msync(bitarray->bitarray, bitarray->size, MS_SYNC);
        if (sync == -1)
        {
            log_error(logger, "Error al sincronizar los datos al liberar el bloque: <%d>", i);
        }
    }
}
void marcar_bloques_ocupados(int inicio, int fin)
{
    for (int i = inicio; i < fin; i++)
    {
        bitarray_set_bit(bitarray, i);
        int sync = msync(bitarray->bitarray, bitarray->size, MS_SYNC);
        if (sync == -1)
        {
            log_error(logger, "Error al sincronizar los datos al liberar el bloque: <%d>", i);
        }
    }
}
int max(int a, int b)
{
    return (a > b) ? a : b;
}