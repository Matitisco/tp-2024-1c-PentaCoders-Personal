
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
        log_info(logger, "Error al establecer fstat");
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
        log_info(logger, "Bitmap creado con direccion de memoria en %p", bmap);
        bitarray = bitarray_create_with_mode(bmap, config_interfaz->block_count, LSB_FIRST);
    }
}

// INICIAR ARCHIVO DE BLOQUES EN MEMORIA
void levantar_archivo_bloques()
{

    path_arch_bloques = obtener_ruta_archivo("bloques.dat");

    FILE *fbloques = fopen(path_arch_bloques, "wb+");

    int tamArchivoBloques = config_interfaz->block_size * config_interfaz->block_count;

    archivos_fs = list_create();
    int file_descriptor;
    if (fbloques == NULL)
    {
        fbloques = fopen(path_arch_bloques, "rb+");
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

void instrucciones_dialfs()
{
    tipo_buffer *buffer_dialfs = recibir_buffer(conexion_kernel);
    t_tipoDeInstruccion codigo = leer_buffer_enteroUint32(buffer_dialfs);
    uint32_t pid = leer_buffer_enteroUint32(buffer_dialfs);
    char *nombre_archivo;
    uint32_t nuevo_tamanio;
    uint32_t tamanio;
    uint32_t direccion_fisica;
    uint32_t puntero_archivo;
    switch (codigo)
    {
    case IO_FS_CREATE:
        nombre_archivo = leer_buffer_string(buffer_dialfs);
        log_info(logger, " PID: %d- OPERACION A REALIZAR : %d", pid, F_CREATE);
        crear_archivo(nombre_archivo, pid);
        break;
    case IO_FS_TRUNCATE:
        nuevo_tamanio = leer_buffer_enteroUint32(buffer_dialfs);
        nombre_archivo = leer_buffer_string(buffer_dialfs);
        log_info(logger, " PID: %d- OPERACION A REALIZAR : %d", pid, F_TRUNCATE);
        truncar_archivo(nombre_archivo, nuevo_tamanio, pid);
        break;
    case IO_FS_READ:
        tamanio = leer_buffer_enteroUint32(buffer_dialfs);
        direccion_fisica = leer_buffer_enteroUint32(buffer_dialfs);
        puntero_archivo = leer_buffer_enteroUint32(buffer_dialfs);
        nombre_archivo = leer_buffer_string(buffer_dialfs);
        log_info(logger, " PID: %d- OPERACION A REALIZAR : %d", pid, F_READ);
        leer_archivo(nombre_archivo, tamanio, direccion_fisica, puntero_archivo, pid);
        break;
    case IO_FS_WRITE:
        tamanio = leer_buffer_enteroUint32(buffer_dialfs);
        direccion_fisica = leer_buffer_enteroUint32(buffer_dialfs);
        puntero_archivo = leer_buffer_enteroUint32(buffer_dialfs);
        nombre_archivo = leer_buffer_string(buffer_dialfs);
        log_info(logger, " PID: %d- OPERACION A REALIZAR : %d", pid, F_WRITE);
        escribir_archivo(nombre_archivo, tamanio, direccion_fisica, puntero_archivo, pid);
        break;
    case IO_FS_DELETE:
        nombre_archivo = leer_buffer_string(buffer_dialfs);
        log_info(logger, " PID: %d- OPERACION A REALIZAR : %d", pid, F_DELETE);
        eliminar_archivo(nombre_archivo, pid);
        break;
    default:
        log_error(logger, "Error, la operacion no existe");
        break;
    }
}

// CREAR ARCHIVO
void crear_archivo(char *nombre_archivo, uint32_t pid)
{
    t_config *meta_data_archivo = crear_meta_data_archivo(nombre_archivo);

    if (meta_data_archivo)
    {
        log_info(logger, "PID: %d - Crear Archivo: %s", pid, nombre_archivo);
    }
    else
    {
        log_error(logger, "ERROR AL CREAR EL ARCHIVO");
        enviar_op_code(conexion_kernel, ERROR_CREAR_ARCHIVO_OK);
    }
}

t_config *crear_meta_data_archivo(char *nombre_archivo)
{
    char *rutaArchivo = obtener_ruta_archivo(nombre_archivo);

    FILE *meta_data_archivo = fopen(rutaArchivo, "w");

    uint32_t bloqueInicial = bloque_libre();

    char *num_bloque_inicial = string_itoa(bloqueInicial);
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
    cambiar_tamanio_archivo(nombre_archivo, nuevo_tamanio);

    log_info(logger, "PID: %d - Truncar Archivo: %s> -Tamaño: %d", pid, nombre_archivo, nuevo_tamanio);
}
// ELIMINAR ARCHIVO
void eliminar_archivo(char *nombre_archivo, uint32_t pid)
{
    t_config *metadata_buscado = buscar_meta_data(nombre_archivo);
    int tamanio_archivo = config_get_int_value(metadata_buscado, "TAMANIO_ARCHIVO");
    int tamanio_bloque = config_interfaz->block_size;
    int cant_bloques_ocupados;

    if (tamanio_archivo == 0)
        cant_bloques_ocupados = 1;
    else
        cant_bloques_ocupados = tamanio_archivo / tamanio_bloque;

    int posicion_inicial = config_get_int_value(metadata_buscado, "BLOQUE_INICIAL");

    log_info(logger, "ARCHIVO: %s - BLOQUES OCUPADOS: %d", nombre_archivo, cant_bloques_ocupados);

    for (int i = posicion_inicial; i < cant_bloques_ocupados + posicion_inicial; i++)
    {
        liberarBloque(posicion_inicial + i); // marcamos los bloques libres en el bitarray
    }

    // ahora liberamos los bloques como tal

    char *ruta_archivo = obtener_ruta_archivo(nombre_archivo);
    remove(ruta_archivo); // eliminamos archivo
    free(ruta_archivo);
    log_info(logger, "PID: %d - Eliminar archivo: %s", pid, nombre_archivo);
}
// ESCRIBIR ARCHIVO
void escribir_archivo(char *nombre_archivo, uint32_t tamanio, uint32_t direccion_fisica, uint32_t puntero_archivo, uint32_t pid)
{
    enviar_op_code(conexion_memoria, ACCESO_ESPACIO_USUARIO);
    enviar_op_code(conexion_memoria, PEDIDO_LECTURA);

    tipo_buffer *buffer_memoria = crear_buffer();

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
        char *texto_leido = leer_buffer_string(buffer_recibido);
        log_info(logger, "Texto a Escribir en el Archivo: %s", texto_leido);

        t_config *metadata = buscar_meta_data(nombre_archivo);
        int bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
        int tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
        int tamanio_bloque = config_interfaz->block_size;
        int cantidad_bloques = tamanio_archivo / config_interfaz->block_size;
        long bloques_a_desplazarse = puntero_archivo / tamanio_bloque;
        long offset = (bloque_inicial + bloques_a_desplazarse) + puntero_archivo;
        FILE *fbloques = fopen(path_arch_bloques, "w+");
        fseek(fbloques, offset, SEEK_SET);          // nos posicionamos al puntero de1
        fwrite(&texto_leido, tamanio, 1, fbloques); // escribimoos la cadena
        sleep_ms(config_interfaz->tiempo_unidad_trabajo);
        log_info(logger, "PID: <%d> - Operacion: <IO_DIALFS_WRITE>", pid);
    }
    else if (codigo_memoria == ERROR_PEDIDO_LECTURA)
    {
        log_error(logger, "PID: <%d> - ERROR Operacion: <IO_DIALFS>", pid);
    }

    log_info(logger, "PID: %d - Escribir:  %s - Tamanio a Leer: %d - Puntero Archivo: %d", pid, nombre_archivo, tamanio, puntero_archivo);
}

void leer_archivo(char *nombre_archivo, uint32_t tamanio, uint32_t direccion_fisica, uint32_t puntero_archivo, uint32_t pid)
{
    void *valor_a_leer;
    t_config *metadata = buscar_meta_data(nombre_archivo);
    int bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
    int tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
    int tamanio_bloque = config_interfaz->block_size;
    int cantidad_bloques = tamanio_archivo / config_interfaz->block_size;
    long bloques_a_desplazarse = puntero_archivo / tamanio_bloque;
    long offset = (bloque_inicial + bloques_a_desplazarse) + puntero_archivo;
    FILE *fbloques = fopen(path_arch_bloques, "r+");
    fseek(fbloques, offset, SEEK_SET);
    fread(valor_a_leer, tamanio, 1, fbloques);
    tipo_buffer *buffer_memoria = crear_buffer();

    enviar_op_code(conexion_memoria, ACCESO_ESPACIO_USUARIO);
    enviar_op_code(conexion_memoria, PEDIDO_ESCRITURA);

    agregar_buffer_para_enterosUint32(buffer_memoria, direccion_fisica);
    agregar_buffer_para_enterosUint32(buffer_memoria, pid);
    agregar_buffer_para_enterosUint32(buffer_memoria, tamanio);
    agregar_buffer_para_enterosUint32(buffer_memoria, STRING);
    agregar_buffer_para_string(buffer_memoria, valor_a_leer);
    enviar_buffer(buffer_memoria, conexion_memoria);
    destruir_buffer(buffer_memoria);
    op_code codigo_memoria = recibir_op_code(conexion_memoria);
    if (codigo_memoria == OK)
    {
        sleep_ms(config_interfaz->tiempo_unidad_trabajo);
        log_info(logger, "PID: %d - Leer Archivo:  %s - Tamanio a Leer: %u - Puntero Archivo: %s", pid, nombre_archivo, tamanio, puntero_archivo);
    }
    else if (codigo_memoria == ERROR_PEDIDO_ESCRITURA)
    {
        log_error(logger, "PID: <%d> - ERROR Operacion: <lEER ARCHIVO>", pid);
    }
}

void cambiar_tamanio_archivo(char *nombre_archivo, uint32_t nuevo_tamanio)
{
    char *ruta_fcb_buscado = obtener_ruta_archivo(nombre_archivo);
    t_config *archivo_meta_data_buscado = config_create(ruta_fcb_buscado); // archivo metadata
    char *tamanio_a_aplicar;
    tamanio_a_aplicar = string_itoa(nuevo_tamanio);

    uint32_t tamanio_archivo_anterior = config_get_int_value(archivo_meta_data_buscado, "TAMANIO_ARCHIVO");
    uint32_t bloque_inicial = config_get_int_value(archivo_meta_data_buscado, "BLOQUE_INICIAL");

    config_save_in_file(archivo_meta_data_buscado, ruta_fcb_buscado);
    uint32_t cantidad_bloques_agregar = (nuevo_tamanio - tamanio_archivo_anterior) / config_interfaz->block_size;

    if (nuevo_tamanio > tamanio_archivo_anterior) // hay que amplar archiv
    {
        ampliar_archivo(nombre_archivo, archivo_meta_data_buscado, tamanio_a_aplicar, tamanio_archivo_anterior, cantidad_bloques_agregar, config_interfaz->block_size, bloque_inicial); // TODO FALTAN TODOS LOS PARAMETROS OJO
    }
    else
    {
        reducir_archivo(archivo_meta_data_buscado, tamanio_archivo_anterior, tamanio_a_aplicar, config_interfaz->block_size);
    }

    config_destroy(archivo_meta_data_buscado);
    free(ruta_fcb_buscado);
    free(tamanio_a_aplicar);
}

void ampliar_archivo(char *nombre_archivo, t_config *archivo_meta_data_buscado, char *tamanio_a_aplicar, uint32_t tamanio_archivo_anterior, uint32_t cantidad_bloques_agregar, uint32_t tamanio_bloque, uint32_t bloque_inicial)
{
    if (hay_espacio_disponible(cantidad_bloques_agregar))
    {
        if (espacio_disponible_es_contiguo(cantidad_bloques_agregar, tamanio_bloque, bloque_inicial, tamanio_archivo_anterior))
        {
            int bloque_final_sin_ampliar = bloque_inicial + tamanio_archivo_anterior / tamanio_bloque;
            int bloque_final_ampiado = bloque_final_sin_ampliar + cantidad_bloques_agregar;
            for (int i = bloque_final_sin_ampliar; i < bloque_final_ampiado; i++)
            {
                log_info(logger, "VALOR DEL BITARRAY %d", bitarray_test_bit(bitarray, i));
                (bitarray_set_bit(bitarray, i));
            }
            msync(bitarray->bitarray, bitarray->size, MS_SYNC);
        }
        else
        {
            compactar(nombre_archivo, cantidad_bloques_agregar, archivo_meta_data_buscado);
            msync(bitarray->bitarray, bitarray->size, MS_SYNC);
        }
        config_set_value(archivo_meta_data_buscado, "TAMANIO_ARCHIVO", tamanio_a_aplicar);
        config_save(archivo_meta_data_buscado);
    }
    else
    {
        log_error(logger, "No hay mas espacio para que el archivo se amplie");
    }
}

void reducir_archivo(t_config *archivo_metadata_buscado, uint32_t tamanio_archivo_anterior, uint32_t tamanio_a_aplicar, uint32_t tamanio_bloque)
{
    uint32_t bloques_a_eliminar = tamanio_archivo_anterior - tamanio_a_aplicar / config_interfaz->block_size;
    uint32_t bloque_final_sin_ampliar = bloque_inicial + tamanio_archivo_anterior / config_interfaz->block_size;
    uint32_t bloque_final_reducido = bloque_final_sin_ampliar - bloques_a_eliminar;

    for (int i = bloque_final_sin_ampliar; i < bloque_final_reducido; i--)
    {
        bitarray_clean_bit(bitarray, i); // va seteando el bloque indicado en 0
    }
    msync(bitarray->bitarray, bitarray->size, MS_SYNC);
    config_set_value(archivo_metadata_buscado, "TAMANIO ARCHIVO", tamanio_a_aplicar);
    config_save(archivo_metadata_buscado);
}

_Bool hay_espacio_disponible(uint32_t cant_bloques_agregar)
{
    int total_bloques = config_interfaz->block_count;
    int bloques_ocupados = contar_bloques_ocupados_bitarray();
    int bloques_disponibles = total_bloques - bloques_ocupados;
    return cant_bloques_agregar <= bloques_disponibles;
}

_Bool espacio_disponible_es_contiguo(uint32_t cantidad_bloques_agregar, uint32_t tamanio_bloque, uint32_t bloque_inicial, uint32_t tamanio_archivo_anterior)
{
    uint32_t bloque_final_sin_ampliar = bloque_inicial + (tamanio_archivo_anterior / tamanio_bloque);
    uint32_t bloque_final_ampiado = bloque_final_sin_ampliar + cantidad_bloques_agregar;
    uint32_t contador_libres = 1;
    int i;

    for (i = bloque_final_sin_ampliar; i < bloque_final_ampiado; i++) //
    {
        // Verifica si el bloque actual está libre
        log_info(logger, "VALOR DEL BITARRAY %d", bitarray_test_bit(bitarray, i));
        if (bitarray_test_bit(bitarray, i) == 0)
        {                      // libre
            contador_libres++; // Incrementa el contador de bloques libres
        }
    }
    // Si la cantidad de bloques libres es igual a la cantidad de bloques a agregar
    if (cantidad_bloques_agregar == contador_libres)
    {
        return true; // Los bloques necesarios están libres y son contiguos
    }
    // Si no se encuentra el espacio necesario, retorna false
    return false;
}

void sacar_bloque(t_config *archivo_meta_data_buscado, uint32_t bloques_a_eliminar, uint32_t tamanio_a_aplicar)
{
    config_set_value(archivo_meta_data_buscado, "TAMANIO_ARCHIVO", tamanio_a_aplicar);
}

int contar_bloques_ocupados_bitarray()
{
    int contador_libres;
    for (int i = 0; i < bitarray->size; i++)
    {
        if (bitarray_test_bit(bitarray, i) == 0) // libre
        {
            contador_libres++;
        }
    }
    return contador_libres;
}

uint32_t bloque_libre() // esto lo usamos cuando debamos inicializar el meta data
{
    uint32_t bloque_libre = -1;
    for (uint32_t i = 0; i < bitarray->size; i++)
    {
        if (bitarray_test_bit(bitarray, i) == 0) // libre
        {
            bitarray_set_bit(bitarray, i);
            bloque_libre = i;
            return bloque_libre;
        }
    }
    int sync = msync(bitarray->bitarray, bitarray->size, MS_SYNC);
    if (sync == -1)
    {
        log_error(logger, "Error syncing bitarray");
        return -1;
    }
    log_info(logger, "Acceso a Bitmap - Bloque: %d - Estado: %d", bloque_libre, 1);
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

    if (bitarray_test_bit(bitarray, bit_bloque) == 1) // si esta ocupado
    {
        bitarray_clean_bit(bitarray, bit_bloque);
    }
    int sync = msync(bitarray->bitarray, bitarray->size, MS_SYNC);
    if (sync == -1)
    {
        log_error(logger, "Error al sincronizar los datos al liberar el bloque: <%d>", bit_bloque);
    }

    return 0;
}

void compactar(char *nombre_archivo, int cantidad_bloques_agregar, t_config *metadata)
{
    desplazar_archivos_y_eliminar_bloques_libres();
    int primer_bloque_libre = bloque_libre();
    mover_archivo_al_primer_bloque_libre(primer_bloque_libre, nombre_archivo, cantidad_bloques_agregar, metadata);
}

t_archivo_data *obtener_primer_archivo()
{
    int cant_archivos = list_size(archivos_fs);
    t_archivo_data *archivo_buscado = list_get(archivos_fs, 0); // el primero
    int bloque_mayor = archivo_buscado->bloque_inicial;
    for (int i = 0; i < cant_archivos; i++)
    {
        t_archivo_data *archivo_victima = list_get(archivos_fs, i);
        if (archivo_victima->bloque_inicial < archivo_buscado->bloque_inicial)
        {
            archivo_buscado = archivo_victima;
        }
    }
    return archivo_buscado;
}

t_archivo_data *obtener_ultimo_archivo()
{
    int cant_archivos = list_size(archivos_fs);
    t_archivo_data *archivo_buscado = list_get(archivos_fs, 0); // el primero
    int bloque_mayor = archivo_buscado->bloque_inicial;
    for (int i = 0; i < cant_archivos; i++)
    {
        t_archivo_data *archivo_victima = list_get(archivos_fs, i);
        if (archivo_victima->bloque_inicial > archivo_buscado->bloque_inicial)
        {
            archivo_buscado = archivo_victima;
        }
    }
    return archivo_buscado;
}

_Bool buscar_arch_por_nombre(t_archivo_data *archivo)
{
    return strcmp(archivo->nombre_archivo, nombre_archivo_buscado) == 0;
}

void mover_archivo_al_primer_bloque_libre(int primer_bloque_libre, char *nombre_archivo, int cantidad_bloques_agregar, t_config *metadata)
{
    int total_bloques = config_interfaz->block_count;
    int tam_bloque = config_interfaz->block_size;
    int cantidad_bloques_archivo;
    cantidad_bloques_archivo = obtener_cantidad_bloques_archivo(nombre_archivo);

    log_info(logger, "ARCHIVO: <%s> - Cantidad de bloques a mover: <%d>", nombre_archivo, cantidad_bloques_archivo);
    nombre_archivo_buscado = nombre_archivo;
    int primer_bloque_para_config = primer_bloque_libre;
    log_info(logger, "PRIMER BLOQUE LIBRE %d", primer_bloque_para_config);
    t_archivo_data *info_archivo = list_find(archivos_fs, buscar_arch_por_nombre);
    int primer_bloque = info_archivo->bloque_inicial;
    info_archivo->bloque_inicial = primer_bloque_libre;
    for (int i = 0; i < cantidad_bloques_archivo; i++)
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

    log_info(logger, "Archivo %s - Los bloques ya fueron movidos ahora asignamos los extra que pidio", nombre_archivo);
    for (int i = primer_bloque_libre; i < primer_bloque_libre + cantidad_bloques_agregar - 1; i++)
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
    int tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
    if (tamanio_archivo == 0) // ya que si o si un archivo ocupa un bloque
    {
        return 1;
    }
    else
    {
        int cantidad_bloques = tamanio_archivo / config_interfaz->block_size;
        return cantidad_bloques;
    }
}

void desplazar_archivos_y_eliminar_bloques_libres()
{
    int total_bloques = config_interfaz->block_count;
    t_queue *lista_bloques_libres = queue_create();
    for (int i = 0; i < total_bloques; i++)
    {
        log_info(logger, "BLOQUE EN DONDE ESTOY PARADO ACTUALMENTE %d", i);
        if (!bitarray_test_bit(bitarray, i)) // si estas libre
        {
            queue_push(lista_bloques_libres, i); // meto mi bloque libre
        }
        else // si estas ocupado, significa que una tanda de lugares libres se terminaron
        {
            if (!queue_is_empty(lista_bloques_libres)) // si no esta vacia
            {                                          // el primero del bitarray esta libre
                int bloque_a_ser_ocupado = queue_pop(lista_bloques_libres);
                log_info(logger, "BLOQUE A SER OCUPADO %d", bloque_a_ser_ocupado);
                mover_bloque(i, bloque_a_ser_ocupado);            // mueve el bloque ocupado a un antecesor libre
                bitarray_set_bit(bitarray, bloque_a_ser_ocupado); // actualiza el bitarrray
                bitarray_clean_bit(bitarray, i);                  // volvemos a actaulizar el bitarray pero marcando como libres los espacios en donde estaban
                bloque_inicial_archivo = i;                       // VAR GLOBAL
                actualizar_metadata_archivo(bloque_a_ser_ocupado);
                i = -1;
            }
        }
    }
}

void mover_bloque(int origen, int destino)
{
    int tamano_bloque = config_interfaz->block_size;
    char *contenido_bloque = malloc(tamano_bloque);
    memcpy(contenido_bloque, bloquesMapeado + (origen * tamano_bloque), tamano_bloque);
    memcpy(bloquesMapeado + (destino * tamano_bloque), contenido_bloque, tamano_bloque);
    free(contenido_bloque);
}

void actualizar_metadata_archivo(int nuevo_bloque_inicial) // de los archivos, NO EL QUE QUEREMOS TRUNCAR
{
    char *nuevo_bloque_string = string_itoa(nuevo_bloque_inicial);
    t_archivo_data *archivo = list_find(archivos_fs, buscar_por_bloque);
    if (archivo == NULL)
    {

        log_info(logger, "No se encontro un archivo con que inicie con el bloque %d", bloque_inicial_archivo);
    }
    else
    {
        log_info(logger, "Se encontro un archivo que arranca en el bloque %d", bloque_inicial_archivo);
        archivo->bloque_inicial = nuevo_bloque_inicial;
        t_config *metadata = buscar_meta_data(archivo->nombre_archivo);
        int tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
        int cantidad_bloques = tamanio_archivo / config_interfaz->block_size;
        config_set_value(metadata, "BLOQUE_INICIAL", nuevo_bloque_string); // actualizar el de los otros archivos
        config_save(metadata);
    }
}

_Bool buscar_por_bloque(t_archivo_data *data)
{
    return data->bloque_inicial == bloque_inicial_archivo;
}