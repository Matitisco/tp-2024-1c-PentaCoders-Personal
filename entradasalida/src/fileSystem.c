
#include "../include/fileSystem.h"
config_io *valores_config;
t_list *tabla_archivos_abierto;

void levantar_bitmap()
{
    int bitmap = open("/src/bitmap.dat", O_RDWR);

    struct stat mystat;

    if (fstat(bitmap, &mystat) < 0)
    {
        log_info(logger, "Error al establecer fstat");
        close(bitmap);
    }

    bmap = mmap(NULL, mystat.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, bitmap, 0);

    if (bmap == MAP_FAILED)
    {
        log_error(logger, "Error al mapear a memoria: %s\n", strerror(errno));
    }
    else
    {
        bitarray = bitarray_create_with_mode(bmap, valores_config->block_count, MSB_FIRST);
    }
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

    uint32_t bloqueInicial = bloque_libre();

    char *num_bloque_inicial_ = string_itoa(bloqueInicial);

    txt_write_in_file(meta_data_archivo, "BLOQUE_INICIAL="); // escribimos en el archivo el bloque
    txt_write_in_file(meta_data_archivo, num_bloque_inicial);
    txt_write_in_file(meta_data_archivo, "TAMANIO_ARCHIVO=0\n");
    free(num_bloque_inicial);

    fclose(meta_data_archivo);

    t_config *meta_data_archivo = config_create(rutaArchivo);

    free(rutaArchivo);

    return meta_data_archivo;
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

void escribir_en_bitmap(uint32_t nroBloque, uint8_t valor) // VER SI LAS NECESITAMOS
{
    memcpy(bitmapBloquesSwap + nroBloque, &valor, sizeof(uint8_t));
}

uint8_t leer_de_bitmap(uint32_t nroBloque) // VER SI LAS NECESITAMOS
{
    uint8_t estadoBloque;
    memcpy(&estadoBloque, bitmapBloquesSwap + nroBloque, sizeof(uint8_t));
    return estadoBloque;
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
            eliminar_archivo(nombre_archivo, pid);
            break;

        default:
            log_error(logger, "Error, la operacion no existe");
            break;
        }
    }
}

void crear_archivo(char *nombre_archivo) // HECHO
{
    t_config *meta_data_archivo = crear_meta_data_archivo(nombre_archivo);

    enviar_codigo(socket_kernel, CREAR_ARCHIVO_OK);
    free(nombreArchivo);

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
    // no hay que destruir el config
    // hay que sobreescribir lo que hay en el meta data
    t_config *metadata_buscado = buscar_meta_data(nombre_archivo);
    int tamanio_archivo = config_get_int_value(metadata_buscado, "TAMANIO_ARCHIVO");
    int tamanio_bloque = config_get_int_value(valores_config->tama, "BLOCK_SIZE");
    int cant_bloques_ocupados = tamanio_archivo / tamanio_bloque;
    log_info(logger, "ARCHIVO: %s - BLOQUES OCUPADOS: %d", nombre_archivo, cant_bloques_ocupados);
    int posicion_inicial = config_get_int_value(metadata_buscado, "BLOQUE_INICIAL");
    for (int i = posicion_inicial; i < cant_bloques_ocupados + posicion_inicial, i++)
    {
        liberarBloque(i); // marcamos los bloques libres en el bitarray
    }
    remove(nombre_archivo); // eliminamos archivo
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
    t_config *archivo_meta_data_buscado = config_create(ruta_fcb_buscado); // archivo metadata
    char *tamanio_a_aplicar;
    tamanio_a_aplicar = string_itoa(nuevo_tamanio);

    uint32_t tamanio_archivo_anterior = config_get_int_value(archivo_meta_data_buscado, "TAMANIO_ARCHIVO");
    uint32_t bloque_inicial = config_get_int_value(archivo_meta_data_buscado, "BLOQUE_INICIAL");

    config_save_in_file(archivo_meta_data_buscado, ruta_fcb_buscado);
    uint32_t cantidad_bloques_agregar = (nuevo_tamanio - tamanio_archivo_anterior) / valores_config->tam_bloque;

    if (nuevo_tamanio > tamanio_archivo_anterior) // hay que amplar archiv
    {
        ampliar_archivo(); // TODO FALTAN TODOS LOS PARAMETROS OJO
    }
    else
    {
        reducir_archivo(tamanio_archivo_anterior, tamanio_a_aplicar) // TODO
            for (int i = 0; cantidad_bloques_agregar; i++)
                sacar_bloque_();
    }

    config_destroy(fcb_buscado);
    free(ruta_fcb_buscado);
    free(tamanio_a_aplicar);
}

void ampliar_archivo()
{
    if (hay_espacio_disponible(cantidad_bloques_agregar))
    {
        if (espacio_disponible_es_contiguo(cantidad_bloques_agregar, valores_config->tam_bloque, bloque_inicial, tamanio_archivo_anterior))
        {
            for (int i = 0; i < (nuevo_tamanio - tamanio_archivo_anterior) / valores_config->tam_bloque; i++)
            {
                int bloque_final_sin_ampliar = bloque_inicial + tamanio_archivo_anterior / tamanio_bloque;
                int bloque_final_ampiado = bloque_final_sin_ampliar + cantidad_bloques_agregar;
                for (int i = bloque_final_sin_ampliar, i < bloque_final_ampiado; i++)
                { // |1|1|1|3|2|2|2||||||||||
                        (bitarray_set_bit(bitarray, 1)
                }
                config_set_value(archivo_meta_data_buscado, "TAMANIO_ARCHIVO", tamanio_a_aplicar);
            }
        }
        else
        {
            compactar(); // TODO
            for (int i = 0; i < cantidad_bloques_agregar; i++)
            {
                // nos posicionamos al final y asignamos los blqoues al arcgivo TODO
            }
            config_set_value(archivo_meta_data_buscado, "TAMANIO_ARCHIVO", tamanio_a_aplicar);
        }
    }
    else
    {
        log_error(logger, "NO hay mas espacio para que el archivo se amplie")
    }
}
void reducir_archivo(uint32_t tamaio_archivo_anterior, uint32_t tamanio_a_aplicar)
{
    uint32_t bloques_a_eliminar = tamanio_archivo_anterior - tamanio_a_aplicar / valores_config->tam_bloque;
    | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 bloque_final_sin_ampliar = bloque_inicial + tamanio_archivo_anterior / tamanio_bloque;
    bloque_final_reducido = bloque_final_sin_ampliar - bloques_a_eliminar;

    for (int i = bloque_final_sin_ampliar i < bloque_final_reducido, i--)
    {

        bitarray_set_bit(bitarray, 0);
        bloque_libre = i;
        i = bitarray_size;
    }
}

// necesitamos saber cuales son los bloques que si o si va a usar
// de ahi obtenemos cuantos vamos a sacar y liberar
// luego iteramos la cantidad de bloques a liberar
// y limpiamos el bloque
// y luego en  el bit array lo marcamos como libre
// y listo :D
_Bool hay_espacio_disponible(uint32_t cant_bloques_agregar)
{
    int total_bloques = valores_config->count_bloque;
    int bloques_ocupados = contar_bloques_ocupados_bitarray();
    int bloques_disponibles = total_bloques - bloques_ocupados;
    return cant_bloques_agregar <= bloques_disponibles;
}

_Bool espacio_disponible_es_contiguo(uint32_t cantidad_bloques_agregar, uint32_t tamanio_bloque, uint32_t bloque_inicial, uint32_t tamanio_archivo_anterior)
{
    bloque_final_sin_ampliar = bloque_inicial + tamanio_archivo_anterior / tamanio_bloque;
    bloque_final_ampiado = bloque_final_sin_ampliar + cantidad_bloques_agregar;

    for (i = bloque_final_sin_ampliar, i < bloque_final_ampiado; i++)
    {                                            // |1|1|1|3|2|2|2||||||||||
        if (bitarray_test_bit(bitarray, i) == 0) // libre
        {
            contador_libres++; // cantidad de bloques libres
        }
        else
        {
            return false;
        }
    }
    if (cantidad_bloques_agregar == contador_libres)
    {
        return true; // esto simbolizaria que los bloque que neceiesta el archivo estan libres y son contiguos
    }

    // me posicion al final de archivo
    // cuento si desde el final del archivo hasta la cantidad de bloques que quieor agregar si estan libres
    // si todos estan libres  turue
}

void agregar_bloque(uint32_t bloques_agregar, uint32_t tamanio_a_aplicar)
{
    // Cambiar el config del tamanio del meta data
    // mrcar los bitocupados del bitarray
    config_set_value(archivo_meta_data_buscado, "TAMANIO_ARCHIVO", tamanio_a_aplicar);
}

void sacar_bloque(uint32_t bloques_a_eliminar, uint32_t tamanio_a_aplicar)
{
    config_set_value(archivo_meta_data_buscado, "TAMANIO_ARCHIVO", tamanio_a_aplicar);
}
int contar_bloques_ocupados_bitarray()
{
    int contador_libres;
    for (int i = 0; i < bitarray_size; i++)
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
    for (uint32_t i = 0; i < bitarray_size; i++)
    {
        if (bitarray_test_bit(bitarray, i) == 0)
        {
            bitarray_set_bit(bitarray, i);
            bloque_libre = i;
            i = bitarray_size;
        }
    }
    int sync = msync(bitarray_pointer, bitarray_size, MS_SYNC);
    if (sync == -1)
    {
        {
            printf("Error syncing bitarray");
        }
        log_info(logger, "Acceso a Bitmap - Bloque: %d - Estado: %d", bloque_libre, 1);
        return bloque_libre;
    }
}
t_config *buscar_meta_data(char *nombre_archivo)
{
    char *ruta_metadata = obtener_ruta_archivo(nombre_archivo); // Function to get the file path
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

// libera bloque del bitarray
int liberarBloque(uint32_t bit)
{
    if (bitarray_test_bit(bitarray, bit) == 0)
    {
        bitarray_clean_bit(bitarray, bit);
    }
    int sync = msync(bitarray_pointer, bitarray_size, MS_SYNC);
    if (sync == -1)
    {
        log_error(logger, "");
    }
    log_info(logger, "Acceso a Bitmap - Bloque: %d - Estado: %d", bit, 0);

    return 0;
}
// Forma de resolver compactacion lista de nombre sde archivos, el ultimo dejarlo al final(el que se quiere compactar
// y nada exitos
/*
arch2 15
arcg2 18
[ Arh1|Arch2|||Arch3|||||||||||||||| ||2|||| ||2||||||2||||
||||||||||||||2|||| |||||||||2||||||2|||| ||]
||||||||2||||||2|||| |||||||||2||||||2|||| |
//Tamnio archivo y tamanio a ampliar
//Caso a analizar->Ver que hay algun bloqe libre
//Si hay bloques:
    el tamaño a ampliar es menor a al tamaño disponible
                  Si hay espacio disponible:
                    ES espacio contiguo ?                        S
                    Si los bloques siguientes de nuestro archivo estan libre, entonces BIEN
                    No es contiguo, compactamos,
*/
//[]