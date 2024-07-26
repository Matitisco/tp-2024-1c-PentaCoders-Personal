#include "../include/memoria.h"

pthread_t hiloCpu;
pthread_t hiloKernel;
pthread_t hiloIO;

config_memoria *valores_config;
void *espacio_usuario;
int socket_io;
int server_memoria;
int PID_buscado;
int cliente_cpu;
int cliente_kernel;
int dispositivo_io;
int CLIENTE_ESPACIO_USUARIO;
int pid_a_buscar_o_eliminar;
int cant_marcos;
t_list *lista_contextos;
t_list *lista_instrucciones;
t_list *lista_global_tablas;
t_tabla_paginas *tabla_actual;
t_bit_map *array_bitmap;

int main(int argc, char *argv[])
{

    logger = iniciar_logger("memoria.log", "MEMORIA");
    valores_config = configuracion_memoria();

    iniciar_memoria();

    pthread_join(hiloCpu, NULL);
    pthread_join(hiloKernel, NULL);
    pthread_join(hiloIO, NULL);
    finalizar_memoria();
}

void finalizar_memoria()
{
    liberar_conexion(&cliente_cpu);
    liberar_conexion(&cliente_kernel);
    config_destroy(valores_config->config);
    free(valores_config->ip_memoria);
    free(valores_config->path_instrucciones);
    free(valores_config->puerto_memoria);
    log_destroy(logger);

    // estas list destroy estaban comentadas, ahora las descomente
    list_destroy_and_destroy_elements(lista_instrucciones, destruir_lista_instrucciones);
    list_destroy_and_destroy_elements(lista_global_tablas, destruir_tabla);
    list_destroy_and_destroy_elements(lista_contextos, destruir_cde);

    destruir_tabla(tabla_actual);
    free(espacio_usuario);
    free(array_bitmap);
}

void *destruir_tabla(void *tabla)
{
    t_tabla_paginas *tabla_a_eliminar = (t_tabla_paginas *)tabla;
    list_destroy_and_destroy_elements(tabla_a_eliminar->paginas_proceso, page_destroyer);
    free(tabla_a_eliminar);
}

void *page_destroyer(void *pagina)
{
    free(pagina);
}

void *destruir_lista_instrucciones(void *instrucciones)
{
    list_destroy_and_destroy_elements(instrucciones, free);
}

void *destruir_cde(void *elemento)
{
    t_cde *cde = (t_cde *)elemento;
    list_destroy_and_destroy_elements(cde->lista_instrucciones, free);
    free(cde->path);
    free(cde->registros);
    free(cde);
}

void iniciar_memoria()
{
    lista_global_tablas = list_create();
    lista_contextos = list_create();
    lista_instrucciones = list_create();

    cant_marcos = valores_config->tam_memoria / valores_config->tam_pagina; // va a ser la cantida de amrcos

    if (valores_config->tam_memoria % valores_config->tam_pagina != 0)
    {
        log_error(logger, "Memoria no puede iniciar, el tamaño de página no es múltiplo del tamaño de memoria");
        exit(EXIT_FAILURE);
    }
    crear_espacio_usuario(valores_config->tam_memoria, logger);

    inicializar_bitmap(cant_marcos);
    crearHilos();
}

void inicializar_bitmap(int cant_marcos)
{
    array_bitmap = malloc(sizeof(t_bit_map) * cant_marcos);
    for (int i = 0; i < cant_marcos; i++)
    {
        array_bitmap[i].numero_marco = i;
        array_bitmap[i].bit_ocupado = 0;
    }
}

void crearHilos()
{
    server_memoria = iniciar_servidor(logger, "Memoria", valores_config->ip_memoria, valores_config->puerto_memoria);
    cliente_cpu = esperar_cliente(logger, "Memoria", "CPU", server_memoria);
    cliente_kernel = esperar_cliente(logger, "Memoria", "Kernel", server_memoria);

    pthread_create(&hiloCpu, NULL, recibirCPU, NULL);
    pthread_create(&hiloKernel, NULL, recibirKernel, NULL);
    pthread_create(&hiloIO, NULL, conexiones_io, NULL);
}

// INTERFACES DE ENTRADA/SALIDA

void *conexiones_io()
{
    while (1)
    {
        socket_io = esperar_cliente(logger, "Memoria", "Interfaz IO", server_memoria);
        pthread_create(&hiloIO, NULL, recibir_interfaz_io, NULL);
    }
}

void *recibir_interfaz_io()
{
    int dispositivo_io = socket_io;
    while (1)
    {
        op_code codigo_io = recibir_op_code(dispositivo_io);
        if (codigo_io == -1)
        {
            log_error(logger, "Interfaz Desconectada de Memoria");
            pthread_exit((void *)0);
        }
        switch (codigo_io)
        {
            sleep_ms(valores_config->retardo_respuesta);
        case ACCESO_ESPACIO_USUARIO:
            CLIENTE_ESPACIO_USUARIO = dispositivo_io;
            acceso_a_espacio_usuario();
            break;
        default:
            log_error(logger, "ERROR - ACCESO A ESPACIO DE USUARIO");
            break;
        }
    }
}

// KERNEL

void *recibirKernel()
{
    while (1)
    {
        op_code cod_op = recibir_op_code(cliente_kernel);

        if (cod_op == -1)
        {
            log_error(logger, "KERNEL se desconecto. Terminando hilo");
            pthread_cancel(hiloCpu);
            pthread_cancel(hiloIO);
            pthread_exit((void *)EXIT_FAILURE);
        }

        switch (cod_op)
        {
        case SOLICITUD_INICIAR_PROCESO:
            iniciar_proceso(cliente_kernel);
            break;
        case SOLICITUD_FINALIZAR_PROCESO:
            finalizar_proceso(cliente_kernel);
            break;
        default:
            log_warning(logger, "Operacion desconocida. No quieras meter la pata");
            return (void *)EXIT_FAILURE;
            break;
        }
    }
}

// CPU

void *recibirCPU()
{
    enviar_tamanio_pagina(cliente_cpu);
    while (1)
    {
        op_code cod_op = recibir_op_code(cliente_cpu);

        if (cod_op == -1)
        {
            log_error(logger, "CPU se desconecto. Terminando hilo");
            pthread_exit((void *)EXIT_FAILURE);
        }

        switch (cod_op)
        {
            sleep_ms(valores_config->retardo_respuesta);
        case PEDIDO_INSTRUCCION:
            pedido_instruccion_cpu_dispatch(cliente_cpu);
            break;
        case ACCESO_ESPACIO_USUARIO:
            CLIENTE_ESPACIO_USUARIO = cliente_cpu;
            acceso_a_espacio_usuario_cpu();
            break;
        case RESIZE_OP:
            tipo_buffer *buffer_cpu = recibir_buffer(cliente_cpu);
            uint32_t nuevo_tamanio = leer_buffer_enteroUint32(buffer_cpu);
            t_cde *cde = leer_cde(buffer_cpu);
            int tamanio_actual = tamanio_proceso(cde->pid);
            if (tamanio_actual > nuevo_tamanio)
            {
                // log obligatorio
                log_info(logger, "PID: %d - Tamaño Actual: <%d> - Tamaño a Reducir: <%d>", cde->pid, tamanio_actual, nuevo_tamanio);
                reducir_proceso(cde->pid, nuevo_tamanio, cliente_cpu);
            }
            else
            {
                // log obligatorio
                log_info(logger, "PID: <%d> - Tamaño Actual: <%d> - Tamaño a Ampliar: <%d>", cde->pid, tamanio_actual, nuevo_tamanio);
                ampliar_proceso(cde->pid, nuevo_tamanio, cliente_cpu);
            }
            imprimir_espacio_usuario(espacio_usuario, valores_config->tam_memoria, valores_config->tam_pagina, array_bitmap);
            destruir_buffer(buffer_cpu);
            free(cde->registros);
            free(cde);
            break;
        case PEDIDO_FRAME:
            pedido_frame_mmu(cliente_cpu);
            break;

        default:
            log_warning(logger, "Operacion desconocida. No quieras meter la pata");
            break;
        }
    }
}

// PEDIDO DE MMU

void pedido_frame_mmu(int cliente_cpu)
{
    tipo_buffer *buffer_mmu_cpu = recibir_buffer(cliente_cpu);
    int pid = leer_buffer_enteroUint32(buffer_mmu_cpu);
    int pagina = leer_buffer_enteroUint32(buffer_mmu_cpu);
    destruir_buffer(buffer_mmu_cpu);

    t_tabla_paginas *tabla = buscar_en_lista_global(pid);
    if (tabla == NULL)
    {
        enviar_op_code(cliente_cpu, PEDIDO_FRAME_INCORRECTO);
        // log_error(logger, "PID: <%d> - NO SE ENCONTRO LA TABLA DE PAGINAS", pid);
        return;
    }

    int marco = consultar_marco_de_una_pagina(tabla, pagina);

    // log obligatorio
    log_info(logger, "PID: <%d> - Pagina: <%d> - Marco: <%d>", pid, pagina, marco);

    if (marco == -1)
    {
        enviar_op_code(cliente_cpu, PEDIDO_FRAME_INCORRECTO);
        log_error(logger, "No Se Pudo Encontrar El Frame");
    }
    else
    {
        tipo_buffer *buffer_memoria_mmu = crear_buffer();
        agregar_buffer_para_enterosUint32(buffer_memoria_mmu, marco);
        enviar_op_code(cliente_cpu, PEDIDO_FRAME_CORRECTO);
        enviar_buffer(buffer_memoria_mmu, cliente_cpu);
        destruir_buffer(buffer_memoria_mmu);
    }
}

int tamanio_proceso(int pid)
{
    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid); // devuelve mi tabla de proceso
    int cant_paginas_proceso = list_size(tabla_paginas->paginas_proceso);
    return cant_paginas_proceso * valores_config->tam_pagina;
}

void *acceso_a_espacio_usuario_cpu()
{
    op_code codigo, solicitud;
    codigo = recibir_op_code(CLIENTE_ESPACIO_USUARIO);
    switch (codigo)
    {
    case PEDIDO_ESCRITURA:
        tipo_buffer *buffer = recibir_buffer(CLIENTE_ESPACIO_USUARIO);
        t_write_memoria *escribir = leer_t_write_memoria_buffer(buffer);
        int resultado = escribir_espacio_usuario(escribir->direccion_fisica, escribir->stream, escribir->size, logger, escribir->pid);
        if (resultado != -1)
        {
            enviar_op_code(CLIENTE_ESPACIO_USUARIO, OK);
        }
        else
        {
            enviar_op_code(CLIENTE_ESPACIO_USUARIO, ERROR_PEDIDO_LECTURA);
        }
        destruir_buffer(buffer);
        free_t_write_memoria(escribir);
        break;
    case PEDIDO_LECTURA:
        tipo_buffer *buffer_lectura = recibir_buffer(CLIENTE_ESPACIO_USUARIO);
        lectura(buffer_lectura, CLIENTE_ESPACIO_USUARIO);
        destruir_buffer(buffer_lectura);
        break;
    default:
        log_error(logger, "ERROR - ACCESO ESPACIO USUARIO");
        break;
    }
    return (void *)1;
}

void *acceso_a_espacio_usuario()
{
    op_code codigo;
    codigo = recibir_op_code(CLIENTE_ESPACIO_USUARIO);
    switch (codigo)
    {
    case PEDIDO_ESCRITURA:
        tipo_buffer *buffer_escritura = recibir_buffer(CLIENTE_ESPACIO_USUARIO);
        escritura(buffer_escritura, CLIENTE_ESPACIO_USUARIO);
        destruir_buffer(buffer_escritura);
        break;
    case PEDIDO_LECTURA:
        tipo_buffer *buffer_lectura = recibir_buffer(CLIENTE_ESPACIO_USUARIO);
        lectura(buffer_lectura, CLIENTE_ESPACIO_USUARIO);
        destruir_buffer(buffer_lectura);
        break;
    default:
        log_error(logger, "ERROR - ACCESO ESPACIO USUARIO");
        return (void *)-1;
        break;
    }
    return (void *)1;
}

void escritura(tipo_buffer *buffer, int cliente_solicitante)
{
    uint32_t direccion_fisica = leer_buffer_enteroUint32(buffer);
    uint32_t pid_ejecutando = leer_buffer_enteroUint32(buffer);
    uint32_t tamanio = leer_buffer_enteroUint32(buffer);
    tipoDato tipo_dato = leer_buffer_enteroUint32(buffer);
    int resultado;
    uint32_t valor_a_escribir;
    char *valor_string = string_new();

    if (tipo_dato == INTEGER)
    {
        valor_a_escribir = leer_buffer_enteroUint32(buffer);
        resultado = escribir_espacio_usuario(direccion_fisica, &valor_a_escribir, tamanio, logger, pid_ejecutando);
    }

    else if (tipo_dato == STRING)
    {
        valor_string = leer_buffer_string(buffer);
        resultado = escribir_espacio_usuario(direccion_fisica, valor_string, tamanio, logger, pid_ejecutando);
    }
    imprimir_espacio_usuario(espacio_usuario, valores_config->tam_memoria, valores_config->tam_pagina, array_bitmap);
    if (resultado != -1)
    {
        enviar_op_code(cliente_solicitante, OK);
    }
    else
    {
        enviar_op_code(cliente_solicitante, ERROR_PEDIDO_ESCRITURA);
    }
}

void lectura(tipo_buffer *buffer_lectura, int cliente_solicitante)
{
    uint32_t direccion_fisica = leer_buffer_enteroUint32(buffer_lectura);
    uint32_t pid_ejecutando = leer_buffer_enteroUint32(buffer_lectura);
    uint32_t tamanio = leer_buffer_enteroUint32(buffer_lectura);
    tipoDato tipo_dato = leer_buffer_enteroUint32(buffer_lectura);
    void *valor_leido = calloc(1, tamanio);
    valor_leido = leer_espacio_usuario(direccion_fisica, tamanio, logger, pid_ejecutando);
    uint32_t valor32;
    uint8_t valor8;
    char *valor_char = string_new();
    imprimir_espacio_usuario(espacio_usuario, valores_config->tam_memoria, valores_config->tam_pagina, array_bitmap);
    if (valor_leido != NULL)
    {
        enviar_op_code(cliente_solicitante, OK);
        tipo_buffer *buffer = crear_buffer();
        if (tipo_dato == STRING)
        {
            char *valor_leido_string = malloc(tamanio + 1);
            valor_leido_string[tamanio] = '\0';
            memcpy(valor_leido_string, valor_leido, tamanio);
            valor_char = (char *)valor_leido;

            agregar_buffer_para_string(buffer, valor_leido);
            log_info(logger, "SE LEYO EL VALOR STRING: <%s>", valor_char);
            free(valor_leido_string);
        }
        else if (tipo_dato == INTEGER)
        {
            if (tamanio == 4)
            {
                valor32 = *(uint32_t *)valor_leido;
                agregar_buffer_para_enterosUint32(buffer, valor32);
                log_info(logger, "SE LEYO EL VALOR DE 4 bytes: %d", valor32);
            }
            else if (tamanio == 1)
            {
                valor8 = *(uint8_t *)valor_leido;
                agregar_buffer_para_enterosUint8(buffer, valor8);
                log_info(logger, "SE LEYO EL VALOR de 1 byte: %d", valor8);
            }
        }

        enviar_buffer(buffer, cliente_solicitante);
        destruir_buffer(buffer);
    }
    else
    {
        enviar_op_code(cliente_solicitante, ERROR_PEDIDO_LECTURA);
    }
    free(valor_leido);
}

// INICIO DE PROCESO

void iniciar_proceso(int cliente_fd)
{
    tipo_buffer *buffer = recibir_buffer(cliente_fd);
    t_cde *cde = armarCde(buffer);
    destruir_buffer(buffer);

    cde->lista_instrucciones = leerArchivoConInstrucciones(cde->path);
    if (cde->path == NULL || cde->lista_instrucciones == NULL)
    {
        enviar_op_code(cliente_fd, ERROR_INICIAR_PROCESO);
    }
    else
    {
        crear_y_agregar_tabla_a_lista_global(cde->pid);
        list_add(lista_contextos, cde);
        list_add(lista_instrucciones, cde->lista_instrucciones);
        enviar_op_code(cliente_fd, INICIAR_PROCESO_CORRECTO);
    }
}

void crear_y_agregar_tabla_a_lista_global(int pid)
{
    t_tabla_paginas *tabla_proceso = malloc(sizeof(t_tabla_paginas));
    tabla_proceso->paginas_proceso = list_create();
    tabla_proceso->pid = pid;
    list_add(lista_global_tablas, tabla_proceso);
    // log obligatorio
    log_info(logger, "PID: <%d> - Tamaño : <%d>", pid, list_size(tabla_proceso->paginas_proceso));
}

t_cde *armarCde(tipo_buffer *buffer)
{
    t_cde *cde = malloc(sizeof(t_cde));
    cde->pid = leer_buffer_enteroUint32(buffer);
    cde->path = leer_buffer_string(buffer);
    return cde;
}

t_list *leerArchivoConInstrucciones(char *nombre_archivo)
{
    t_list *list_instrucciones = list_create();
    char directorioActual[1024];
    getcwd(directorioActual, sizeof(directorioActual));
    strcat(directorioActual, valores_config->path_instrucciones);
    strcat(directorioActual, nombre_archivo);

    FILE *archivo = fopen(directorioActual, "r");

    if (archivo == NULL)
    {
        return NULL;
    }
    char linea_instruccion[1024];
    while (fgets(linea_instruccion, sizeof(linea_instruccion), archivo) != NULL)
    {
        char *token = strdup(strtok(linea_instruccion, "\n"));
        list_add(list_instrucciones, token);
    }
    fclose(archivo);
    return list_instrucciones;
}

void enviar_tamanio_pagina(int cpu)
{
    tipo_buffer *buffer_con_tam_pagina = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer_con_tam_pagina, valores_config->tam_pagina);
    enviar_buffer(buffer_con_tam_pagina, cpu);
    destruir_buffer(buffer_con_tam_pagina);
}

// EJECUCION PROCESO

void pedido_instruccion_cpu_dispatch(int cpu_dispatch)
{
    tipo_buffer *buffer = recibir_buffer(cpu_dispatch);
    uint32_t PID = leer_buffer_enteroUint32(buffer);
    uint32_t PC = leer_buffer_enteroUint32(buffer);

    t_cde *contexto = obtener_contexto_en_ejecucion(PID);

    tipo_buffer *buffer_instruccion = crear_buffer();
    char *instruccion = list_get(contexto->lista_instrucciones, PC);

    enviar_op_code(cpu_dispatch, ENVIAR_INSTRUCCION_CORRECTO);
    agregar_buffer_para_string(buffer_instruccion, instruccion);

    enviar_buffer(buffer_instruccion, cpu_dispatch);
    destruir_buffer(buffer_instruccion);
    destruir_buffer(buffer);
}

// FINALIZAR PROCESO

void finalizar_proceso(int socket_kernel)
{
    tipo_buffer *buffer = recibir_buffer(socket_kernel);
    uint32_t pid = leer_buffer_enteroUint32(buffer);

    eliminar_tabla_paginas(pid);
    eliminar_cde(pid);

    enviar_op_code(socket_kernel, FINALIZAR_PROCESO);
    destruir_buffer(buffer);
}

t_cde *obtener_contexto_en_ejecucion(int pid)
{
    pid_a_buscar_o_eliminar = pid;
    t_cde *cde_proceso = list_find(lista_contextos, estaElContextoConCiertoPID);
    return cde_proceso;
}

_Bool estaElContextoConCiertoPID(t_cde *contexto)
{
    return contexto->pid == pid_a_buscar_o_eliminar;
}

void eliminar_cde(int pid)
{
    pid_a_buscar_o_eliminar = pid;

    t_cde *cde = obtener_contexto_en_ejecucion(pid);
    list_destroy_and_destroy_elements(cde->lista_instrucciones, element_destroyer);
    list_remove_by_condition(lista_contextos, estaElContextoConCiertoPID);

    free(cde->path);
    free(cde);
}

void element_destroyer(void *element)
{
    char *valor = (char *)element;
    free(valor);
}

// CONFIGURACION DE LA MEMORIA

config_memoria *configuracion_memoria()
{
    config_memoria *valores_config = malloc(sizeof(config_memoria));
    char directorioActual[1024];
    getcwd(directorioActual, sizeof(directorioActual));
    char *ultimo_dir = basename(directorioActual);
    if (strcmp(ultimo_dir, "bin") == 0)
    {
        chdir("..");
        getcwd(directorioActual, sizeof(directorioActual));
    }
    strcat(directorioActual, "/memoria.config");
    valores_config->config = iniciar_config(directorioActual);
    valores_config->ip_memoria = config_get_string_value(valores_config->config, "IP");
    valores_config->puerto_memoria = config_get_string_value(valores_config->config, "PUERTO_ESCUCHA");
    valores_config->path_instrucciones = config_get_string_value(valores_config->config, "PATH_INSTRUCCIONES");
    valores_config->tam_memoria = config_get_int_value(valores_config->config, "TAM_MEMORIA");
    valores_config->tam_pagina = config_get_int_value(valores_config->config, "TAM_PAGINA");
    valores_config->retardo_respuesta = config_get_int_value(valores_config->config, "RETARDO_RESPUESTA");
    return valores_config;
}

// PAGINACION SIMPLE

t_pagina *crear_pagina(int marco)
{
    t_pagina *pagina = malloc(sizeof(t_pagina));
    pagina->marco = marco;
    return pagina;
}

t_list *agregar_pagina(t_pagina *pagina, t_list *list_paginas)
{
    list_add(list_paginas, pagina);
    return list_paginas;
}

void eliminar_tabla_paginas(uint32_t pid)
{
    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid); // busco en la lista global de tabla del proceso

    // Calcula la cantidad de páginas antes de eliminarlas
    int cant_paginas = list_size(tabla_paginas->paginas_proceso);

    // Elimina todas las páginas y libera su memoria
    list_destroy_and_destroy_elements(tabla_paginas->paginas_proceso, (void *)destruir_pagina);
    // log obligatorio
    log_info(logger, "PID: <%d> - Tamaño: <%d> ", pid, cant_paginas);
}

void destruir_pagina(void *pagina)
{
    t_pagina *pagina_a_eliminar = (t_pagina *)pagina;
    liberar_marco(pagina_a_eliminar->marco);
    free(pagina_a_eliminar);
}

t_tabla_paginas *buscar_en_lista_global(int pid)
{
    int cantidad_tablas = list_size(lista_global_tablas);

    for (int i = 0; i < cantidad_tablas; i++)
    {
        tabla_actual = list_get(lista_global_tablas, i);
        if (tabla_actual->pid == pid)
        {
            return tabla_actual;
        }
    }
    return NULL;
}

void colocar_pagina_en_marco(t_pagina *pagina)
{
    int marco_libre = obtener_posicion_marco_libre(); // consigo el primer marco libre que encuentro
                                                      /*     if (marco_libre == -1)
                                                          {
                                                              log_info(logger, "No hay ningun marco libre, OUT OF MEMORY");
                                                          } */
    pagina->marco = marco_libre;
}

int obtener_posicion_marco_libre()
{
    for (int i = 0; i < cant_marcos; i++)
    {
        if (array_bitmap[i].bit_ocupado == 0)
        {
            return i;
        }
    }
    return -1;
}

int consultar_pagina_de_un_marco(t_tabla_paginas *tabla, int marco)
{
    for (int i = 0; i < list_size(tabla->paginas_proceso); i++)
    {
        t_pagina *pagina = list_get(tabla->paginas_proceso, i);
        if (pagina->marco == marco)
        {
            return 1; // true
        }
    }
    return -1; // false
}

int consultar_marco_de_una_pagina(t_tabla_paginas *tabla, int nroPagina)
{
    int nroUltimaPagina = list_size(tabla->paginas_proceso) - 1;
    if (nroPagina > nroUltimaPagina || nroPagina < 0)
    {
        return -1;
    }
    t_pagina *pagina = list_get(tabla->paginas_proceso, nroPagina);
    return pagina->marco;
}

void liberar_marco(int nroMarco)
{
    array_bitmap[nroMarco].bit_ocupado = 0;
}
/* void imprimir_estado_marcos()
{
    printf_yellow("ESTADO DE LOS MARCOS");
    printf_yellow("---------------------");
    printf_yellow("|MARCO   | OCUPADO |");
    for (int i = 0; i < cant_marcos; i++)
    {
        printf_yellow("|  %d      |    %d   |", i, array_bitmap[i].bit_ocupado);
        printf_yellow("---------------------");
    }
} */
void ampliar_proceso(uint32_t pid, uint32_t tamanio, int cliente_cpu)
{
    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid);
    if (!tabla_paginas)
    {
        log_error(logger, "PID: <%d> - NO SE ENCONTRO TABLA PAGINAS", pid);
        return;
    }
    float f_tamanio = tamanio;
    float tam_pagina = valores_config->tam_pagina;
    double cantidad_paginas_nuevas = ceil(f_tamanio / tam_pagina);
    int cantidad_paginas_actuales = list_size(tabla_paginas->paginas_proceso);
    int paginas_adicionales = cantidad_paginas_nuevas - cantidad_paginas_actuales;

    if (paginas_adicionales > cantidad_marcos_libres())
    {
        enviar_op_code(cliente_cpu, OUT_OF_MEMORY);
        return;
    }

    asignar_paginas_nuevas(tabla_paginas, paginas_adicionales, pid);

    enviar_op_code(cliente_cpu, RESIZE_EXITOSO);
}

void asignar_paginas_nuevas(t_tabla_paginas *tabla_paginas, int paginas_adicionales, uint32_t pid)
{
    t_list *tp_paginas_proceso = tabla_paginas->paginas_proceso;
    for (int i = 0, asignadas = 0; i < cant_marcos && asignadas < paginas_adicionales; ++i)
    {
        if (!array_bitmap[i].bit_ocupado)
        {
            agregar_pagina(crear_pagina(i), tp_paginas_proceso);
            array_bitmap[i].bit_ocupado = 1;
            ++asignadas;
        }
    }
}

int cantidad_marcos_libres()
{
    int marcos_libres = 0;
    for (int i = 0; i < cant_marcos; i++)
    {
        if (array_bitmap[i].bit_ocupado == 0)
        {
            marcos_libres++;
        }
    }
    // log_info(logger, "MARCOS DISPONIBLES: <%d>", marcos_libres);
    return marcos_libres;
}
/*void imprimir_tabla_de_paginas_proceso(t_tabla_paginas *tabla_paginas_proceso)
{ // t_tabla_paginas
    t_list *tp_paginas_proceso = tabla_paginas_proceso->paginas_proceso;
    if (!list_is_empty(tp_paginas_proceso))
    {
        t_pagina *pagina2 = list_get(tp_paginas_proceso, list_size(tp_paginas_proceso) - 1);
        printf_yellow("      TABLA PROCESO %d", tabla_paginas_proceso->pid);

        printf_yellow("--------------------");
        printf_yellow("|PAGINA   | MARCO  |");
        for (int i = 0; i < list_size(tp_paginas_proceso); ++i)
        {
            t_pagina *pagina = list_get(tp_paginas_proceso, i);
            printf_yellow("|  %d      |    %d   |", i, pagina->marco);
            printf_yellow("--------------------");
        }
    }
    else
    {
        printf_yellow("      TABLA PROCESO %d VACÍA", tabla_paginas_proceso->pid);
    }
}*/

void reducir_proceso(uint32_t pid, uint32_t tamanio, int cliente_cpu)
{
    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid); // busco en la lista de listaa
    if (tabla_paginas == NULL)
    {
        return;
    }
    int tam_pagina = valores_config->tam_pagina;
    int paginas_post_resize = tamanio / tam_pagina;
    int cantidad_paginas_actuales = list_size(tabla_paginas->paginas_proceso);
    int cantidad_paginas_a_eliminar = cantidad_paginas_actuales - paginas_post_resize;

    t_list *paginas = tabla_paginas->paginas_proceso;
    eliminar_paginas(paginas, cantidad_paginas_a_eliminar);

    enviar_op_code(cliente_cpu, RESIZE_EXITOSO);
}

void eliminar_paginas(t_list *paginas, int cantidad_a_eliminar)
{
    for (int i = 0; i < cantidad_a_eliminar; i++)
    {
        // Siempre se elimina la última página debido a que la lista se reduce en cada iteración
        t_pagina *pagina_a_eliminar = list_remove(paginas, list_size(paginas) - 1);
        liberar_marco(pagina_a_eliminar->marco);
        free(pagina_a_eliminar); // Liberar la memoria de la página
    }
}