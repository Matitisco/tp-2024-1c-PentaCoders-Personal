
#include "../include/fileSystem.h"
config_io *valores_config;
uint32_t bitarray_pointer;

void levantar_bitmap()
{
    int bitmap = open("/src/bitmap.dat", O_RDWR);

    struct stat mystat;

    if (fstat(bitmap, &mystat) < 0)
    {
        log_info(logger, "Error al establecer fstat");
        close(bitmap);
    }

    void* bmap = mmap(NULL, mystat.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, bitmap, 0);

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
    
        archivo_bloque = open(valores_config->path_bloques, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        ftruncate(archivo_bloque, tamArchivoBloques);
    }
    else
    {
        ftruncate(archivo_bloque, tamArchivoBloques);
    }

    ftruncate(archivo_bloque, tamArchivoBloques);

    void *bloquesMapeado = mmap(NULL, tamArchivoBloques, PROT_WRITE, MAP_SHARED, archivo_bloque, 0);
}

t_config *crear_meta_data_archivo(char *nombre_archivo)
{
    char *rutaArchivo = obtener_ruta_archivo(nombre_archivo);

    FILE *meta_data_archivo = fopen(rutaArchivo, "w");

    uint32_t bloqueInicial = bloque_libre();

    char *num_bloque_inicial = string_itoa(bloqueInicial);

    txt_write_in_file(meta_data_archivo, "BLOQUE_INICIAL="); // escribimos en el archivo el bloque
    txt_write_in_file(meta_data_archivo, num_bloque_inicial);
    txt_write_in_file(meta_data_archivo, "TAMANIO_ARCHIVO=0\n");
    free(num_bloque_inicial);

    fclose(meta_data_archivo);

    t_config *meta_data_archivo = config_create(rutaArchivo);

    free(rutaArchivo);

    return meta_data_archivo;
}

/*
uint32_t obtener_nro_bloque_libre()
{
    for (int nroBloque = 0; nroBloque < config_file_system.cantidad_bloques_swap; nroBloque++)
    {
        uint8_t bloqueOcupado = leer_de_bitmap(nroBloque);
        if (!bloqueOcupado)
            return nroBloque;
    }
}*/
/*
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
*/
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

void crear_archivo(char *nombre_archivo, uint32_t pid) // HECHO
{
    t_config *meta_data_archivo = crear_meta_data_archivo(nombre_archivo);
    
    if(meta_data_archivo){
    enviar_cod_enum(conexion_kernel,CREAR_ARCHIVO_OK);

    }else {
    enviar_cod_enum(conexion_kernel,ERROR_CREAR_ARCHIVO_OK);
    log_info(logger, "PID: %d - Crear Archivo: %s", pid, nombre_archivo);
    }
    
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
    t_config *metadata_buscado = buscar_meta_data(nombre_archivo);
    int tamanio_archivo = config_get_int_value(metadata_buscado, "TAMANIO_ARCHIVO");
    int tamanio_bloque = valores_config->block_size;
    int cant_bloques_ocupados = tamanio_archivo / tamanio_bloque;
    int posicion_inicial = config_get_int_value(metadata_buscado, "BLOQUE_INICIAL");
    
    log_info(logger, "ARCHIVO: %s - BLOQUES OCUPADOS: %d", nombre_archivo, cant_bloques_ocupados);

    for (int i = posicion_inicial; i < cant_bloques_ocupados + posicion_inicial; i++)
    {
        liberarBloque(posicion_inicial +i); // marcamos los bloques libres en el bitarray
    }

    char*ruta_archivo= obtener_ruta_archivo(nombre_archivo);
    remove(ruta_archivo); // eliminamos archivo
    free(ruta_archivo);
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
        ampliar_archivo(archivo_meta_data_buscado,tamanio_a_aplicar, tamanio_archivo_anterior, cantidad_bloques_agregar,valores_config->tam_bloque,bloque_inicial); // TODO FALTAN TODOS LOS PARAMETROS OJO
    }
    else
    {
        reducir_archivo(tamanio_archivo_anterior, tamanio_a_aplicar); // TODO
            for(int i = 0; i < cantidad_bloques_agregar; i++)
                sacar_bloque_();
    }

    config_destroy(archivo_meta_data_buscado);
    free(ruta_fcb_buscado);
    free(tamanio_a_aplicar);
}

void ampliar_archivo(t_config* archivo_meta_data_buscado,uint32_t tamanio_a_aplicar ,uint32_t tamanio_archivo_anterior,uint32_t cantidad_bloques_agregar,uint32_t tamanio_bloque, uint32_t bloque_inicial)
{
    if (hay_espacio_disponible(cantidad_bloques_agregar))
    {
        if (espacio_disponible_es_contiguo(cantidad_bloques_agregar, tamanio_bloque, bloque_inicial, tamanio_archivo_anterior))
        {
            for (int i = 0; i < (tamanio_a_aplicar- tamanio_archivo_anterior) / valores_config->tam_bloque; i++)
            {
                int bloque_final_sin_ampliar = bloque_inicial + tamanio_archivo_anterior / tamanio_bloque;
                int bloque_final_ampiado = bloque_final_sin_ampliar + cantidad_bloques_agregar;
                for (int i = bloque_final_sin_ampliar; i < bloque_final_ampiado; i++)
                { // |1|1|1|3|2|2|2||||||||||
                        (bitarray_set_bit(bitarray, i));
                }
                msync(bitarray->bitarray, bitarray->size, MS_SYNC);
                config_set_value(archivo_meta_data_buscado, "TAMANIO_ARCHIVO", tamanio_a_aplicar);
                config_save(archivo_meta_data_buscado);
            
            }
        }
        else
        {
            compactar(); // TODO
            for (int i = 0; i < cantidad_bloques_agregar; i++)
            {
                uint32_t nuevo_blpque = obtener_nro_bloque_libre();
                bitarray_set_bit(bitarray, nuevo_blpque);// nos posicionamos al final y asignamos los blqoues al archivo TODO
            }
            msync(bitarray->bitarray, bitarray->size, MS_SYNC);
            config_set_value(archivo_meta_data_buscado, "TAMANIO_ARCHIVO", tamanio_a_aplicar);
            config_save(archivo_meta_data_buscado);
           
        }
    }
    else
    {
        log_error(logger, "NO hay mas espacio para que el archivo se amplie");
    }
}
void reducir_archivo(t_config* archivo_metadata_buscado, uint32_t tamanio_archivo_anterior, uint32_t tamanio_a_aplicar, uint32_t tamanio_bloque)
{
    uint32_t bloques_a_eliminar = tamanio_archivo_anterior - tamanio_a_aplicar / valores_config->tam_bloque;
    uint32_t bloque_final_sin_ampliar = bloque_inicial + tamanio_archivo_anterior / tamanio_bloque;
    uint32_t bloque_final_reducido = bloque_final_sin_ampliar - bloques_a_eliminar;

    for (int i = bloque_final_sin_ampliar ;i < bloque_final_reducido; i--)
    {
        //va liberndo los bits
        bitarray_clean_bit(bitarray, i);//va seteando el bloque indicado en 0
    }
    msync(bitarray->bitarray,bitarray->size,MS_SYNC);
    config_set_value(archivo_metadata_buscado,"TAMANIIO ARCHIVO", tamanio_a_aplicar);
    config_save(archivo_metadata_buscado);
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

_Bool espacio_disponible_es_contiguo(uint32_t cantidad_bloques_agregar, uint32_t tamanio_bloque, uint32_t bloque_inicial, uint32_t tamanio_archivo_anterior) {
    // Calcula el bloque final actual del archivo
    uint32_t bloque_final_sin_ampliar = bloque_inicial + (tamanio_archivo_anterior / tamanio_bloque);
    uint32_t bloque_final_ampiado = bloque_final_sin_ampliar + cantidad_bloques_agregar;
    uint32_t contador_libres = 0;
    int i;

    // Itera desde el bloque final actual hasta el bloque final ampliado
    for (i = bloque_final_sin_ampliar; i < bloque_final_ampiado; i++) {
        // Verifica si el bloque actual está libre
        if (bitarray_test_bit(bitarray, i) == 0) { // libre
            contador_libres++; // Incrementa el contador de bloques libres
        } else {
            return false; // Si algún bloque no está libre, retorna false
        }
    }

    // Si la cantidad de bloques libres es igual a la cantidad de bloques a agregar
    if (cantidad_bloques_agregar == contador_libres) {
        return true; // Los bloques necesarios están libres y son contiguos
    }

    // Si no se encuentra el espacio necesario, retorna false
    return false;
}


void sacar_bloque(t_config *archivo_meta_data_buscado,uint32_t bloques_a_eliminar, uint32_t tamanio_a_aplicar)
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
    for (uint32_t i = 0; i <bitarray->size; i++)
    {
        if (bitarray_test_bit(bitarray, i) == 0)
        {
            bitarray_set_bit(bitarray, i);
            bloque_libre = i;
            i = bitarray->size;
        }
    }
    int sync = msync(bitarray_pointer,bitarray->size, MS_SYNC);
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
    int sync = msync(bitarray_pointer, bitarray->size, MS_SYNC);
    if (sync == -1)
    {
        log_error(logger, "");
    }
    log_info(logger, "Acceso a Bitmap - Bloque: %d - Estado: %d", bit, 0);

    return 0;
}
_Bool espacio_disponible_es_contiguo(uint32_t cantidad_bloques_agregar, uint32_t tamanio_bloque, uint32_t bloque_inicial, uint32_t tamanio_archivo_anterior) {
    uint32_t bloque_final_sin_ampliar = bloque_inicial + (tamanio_archivo_anterior / tamanio_bloque);
    uint32_t bloque_final_ampiado = bloque_final_sin_ampliar + cantidad_bloques_agregar;
    uint32_t contador_libres = 0;
    int i;
    for (i = bloque_final_sin_ampliar; i < bloque_final_ampiado; i++) {
        if (bitarray_test_bit(bitarray, i) == 0) { // libre
            contador_libres++;
        } else {
            return false; 
        }
    }
    if (cantidad_bloques_agregar == contador_libres) {
        return true;
    }
    return false;
}
int* obtener_bloques_archivo(char* nombre_archivo)
{
    // Esta función debe devolver la lista de bloques ocupados por el archivo específico
    // Supongamos que los bloques están almacenados en la metadata del archivo
    // Aquí hay un ejemplo de cómo podrías implementarla

    t_config* metadata = obtener_metadata_archivo(nombre_archivo);
    int tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
    int cantidad_bloques = tamanio_archivo / valores_config->block_size;

    int* bloques = malloc(cantidad_bloques * sizeof(int));
    for (int i = 0; i < cantidad_bloques; i++)
    {
        bloques[i] = config_get_int_value(metadata, "BLOQUE_" + i);
    }

    return bloques;
}

void compactar(char* nombre_archivo)
{
    desplazar_archivos_y_eliminar_bloques_libres();
    mover_archivo_al_final_del_fs(nombre_archivo);
}
void mover_archivo_al_final_del_fs(char* nombre_archivo) {
    int total_bloques = valores_config->block_count;
    int tam_bloque = valores_config->block_size;
    int* bloques_archivo;
    int cantidad_bloques_archivo;

    // Obtener los bloques del archivo y su cantidad
    bloques_archivo = obtener_bloques_archivo(nombre_archivo);
    cantidad_bloques_archivo = obtener_cantidad_bloques_archivo(nombre_archivo);

    // Encontrar el primer bloque libre al final del sistema de archivos
    int primer_bloque_libre = total_bloques - cantidad_bloques_archivo;

    // Mover cada bloque del archivo al final del sistema de archivos
    for (int i = 0; i < cantidad_bloques_archivo; i++) {
        int bloque_actual = bloques_archivo[i];
        void* src = bloquesMapeado + (bloque_actual * tam_bloque);
        void* dest = bloquesMapeado + (primer_bloque_libre * tam_bloque);
        memmove(dest, src, tam_bloque);

        // Actualizar el bitarray y la metadata del archivo
        bitarray_clean_bit(bitarray, bloque_actual);
        bitarray_set_bit(bitarray, primer_bloque_libre);
        bloques_archivo[i] = primer_bloque_libre;

        // Avanzar al siguiente bloque libre al final del FS
        primer_bloque_libre++;
    }

    // Actualizar la metadata del archivo con los nuevos bloques
    actualizar_metadata_archivo(nombre_archivo, bloques_archivo);
    // Sincronizar el bitarray y los bloques mapeados con el sistema de archivos
    msync(bitarray_pointer, bitarray->size, MS_SYNC);
    msync(bloquesMapeado, total_bloques * tam_bloque, MS_SYNC);

    // Liberar memoria utilizada para los bloques del archivo
    free(bloques_archivo);

    // Loggear la operación completada
    log_info(logger, "Archivo %s movido al final del área ocupada del sistema de archivos", nombre_archivo);
}
void desplazar_archivos_y_eliminar_bloques_libres(){
    int total_bloques = valores_config->block_count;
    int bloque_actual = 0; // posición de lectura de bloques ocupados
    int bloque_libre = 0;  // posición de escritura de bloques libres

    // Recorrer todos los bloques del bitarray
    while (bloque_actual < total_bloques) {
        // Si el bloque actual está ocupado
        if (bitarray_test_bit(bitarray, bloque_actual)) {
            // Si la posición de escritura es menor a la posición de lectura
            if (bloque_libre < bloque_actual) {
                // Copiar el contenido del bloque ocupado al bloque libre
                mover_bloque(bloque_actual, bloque_libre);
                // Marcar el bloque libre como ocupado en el bitarray
                bitarray_set_bit(bitarray, bloque_libre);
                // Limpiar el bloque ocupado en el bitarray
                bitarray_clean_bit(bitarray, bloque_actual);
                // Actualizar la metadata del archivo que usa este bloque
                actualizar_metadata_bloque(bloque_actual, bloque_libre);
                // Avanzar a la siguiente posición libre
                bloque_libre++;
            } else {
                // Si la posición de escritura es igual o mayor a la posición de lectura
                // No hay nada que mover, simplemente avanzar a la siguiente posición libre
                bloque_libre++;
            }
        }
        // Avanzar a la siguiente posición de lectura
        bloque_actual++;
    }

    // Sincronizar el bitarray con el sistema de archivos
    msync(bitarray_pointer, bitarray->size, MS_SYNC);
}
void mover_bloque(int origen, int destino) {
    int tamano_bloque = valores_config->block_size;
    char *contenido_bloque = malloc(tamano_bloque);
    
    // Leer el contenido del bloque de origen
    memcpy(contenido_bloque, bloquesMapeado + (origen * tamano_bloque), tamano_bloque);
    // Escribir el contenido en el bloque de destino
    memcpy(bloquesMapeado + (destino * tamano_bloque), contenido_bloque, tamano_bloque);
    
    free(contenido_bloque);
}
void actualizar_metadata_archivo(char* nombre_archivo, int* nuevos_bloques)
{
    t_config* metadata = obtener_metadata_archivo(nombre_archivo);
    int tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
    int cantidad_bloques = tamanio_archivo / valores_config->block_size;

    for (int i = 0; i < cantidad_bloques; i++)
    {
        char* clave = string_from_format("BLOQUE_%d", i);
        config_set_value(metadata, clave, string_itoa(nuevos_bloques[i]));
        free(clave);
    }

    config_save(metadata);
}
//[1|1|1|||2|2|2|||3|4||||||||||||| |]
//[1|1|1|2|2|2|3|4||||||||||||||||| |]
//Deberia volver a mover el archivo pero al final por ejemplo si fuese el 1
//
