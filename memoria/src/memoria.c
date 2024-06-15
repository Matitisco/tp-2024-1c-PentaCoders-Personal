
#include "../include/memoria.h"
// CRAER HILOS COMO VARIABLES GLOBALES

pthread_t hiloCpu;
pthread_t hiloKernel;
pthread_t hiloIO;

config_memoria *valores_config;

int server_fd;
int PID_buscado;

t_list *lista_contextos;
t_list *lista_instrucciones;
t_list *lista_global_tablas;
t_list *lista_global_marcos;
t_bit_map *array_bitmap;

int main(int argc, char *argv[])
{
    logger = iniciar_logger("memoria.log", "MEMORIA");
    valores_config = configuracion_memoria();

    lista_global_marcos = list_create(); // esta seria la tabla de marcos. Todos los marcos componen a la memoria
    lista_global_tablas = list_create();
    cant_marcos = valores_config->tam_memoria / valores_config->tam_pagina; // va a ser la cantida de amrcos
    espacio_usuario = malloc(sizeof(valores_config->tam_memoria));
    if (espacio_usuario == NULL)
    {
        log_error(logger, "ERROR ESPACIO USUARIO");
    }
    inicializar_bitmap(cant_marcos);
    int i = 0;
    while (i < cant_marcos)
    {
        list_add(lista_global_marcos, NULL); // creo mi lista de marcos-tabla. En principio todo en null
        i++;
    }

    lista_contextos = list_create();
    lista_instrucciones = list_create();

    crearHilos();

    pthread_join(hiloCpu, NULL);
    pthread_join(hiloKernel, NULL);
    pthread_join(hiloIO, NULL);
    destruirConfig(valores_config->config);
    destruirLog(logger);
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
    server_fd = iniciar_servidor(logger, "Memoria", valores_config->ip_memoria, valores_config->puerto_memoria);

    pthread_create(&hiloCpu, NULL, recibirCPU, NULL);
    pthread_create(&hiloKernel, NULL, recibirKernel, NULL);
    pthread_create(&hiloIO, NULL, recibir_interfaces_io, NULL);
}

void *recibir_interfaces_io()
{
    while (1)
    {
        int dispositivo_io = esperar_cliente(logger, "Memoria", "Interfaz IO", server_fd);
        op_code codigo_io = recibir_operacion(dispositivo_io);
        switch (codigo_io)
        {
        case ACCESO_ESPACIO_USUARIO:
            acceso_a_espacio_usuario(dispositivo_io);
            break;
        default:
            break;
        }
    }
}

void *recibirKernel()
{
    tipo_buffer *buffer = crear_buffer();
    int cliente_fd = esperar_cliente(logger, "Memoria", "Kernel", server_fd);
    while (1)
    {
        op_code cod_op = recibir_operacion(cliente_fd);

        if (cod_op == -1)
        {
            log_error(logger, " El KERNEL se desconecto. Terminando servidor");
            return (void *)EXIT_FAILURE;
        }

        switch (cod_op)
        {
        case SOLICITUD_INICIAR_PROCESO:
            iniciar_proceso(cliente_fd, buffer);
            break;
        case SOLICITUD_FINALIZAR_PROCESO:
            finalizar_proceso(cliente_fd, buffer);
            break;
        default:
            log_warning(logger, "Operacion desconocida. No quieras meter la pata");
            return (void *)EXIT_FAILURE;
            break;
        }
    }
}

void *recibirCPU()
{
    int cliente_cpu = esperar_cliente(logger, "Memoria", "CPU", server_fd);
    enviar_tamanio_pagina(cliente_cpu);
    while (1)
    {
        op_code cod_op = recibir_operacion(cliente_cpu);

        if (cod_op == -1)
        {
            log_error(logger, "El cliente se desconecto. Terminando servidor");
            exit(EXIT_FAILURE);
            return (void *)EXIT_FAILURE;
        }

        switch (cod_op)
        {
        case PEDIDO_INSTRUCCION:
            pedido_instruccion_cpu_dispatch(cliente_cpu, lista_contextos);
            break;
        case ACCESO_ESPACIO_USUARIO:
            acceso_a_espacio_usuario(cliente_cpu);
            break;
        case RESIZE_EXTEND:
            tipo_buffer *buffer_cpu = recibir_buffer(cliente_cpu);
            uint32_t nuevo_tamanio = leer_buffer_enteroUint32(buffer_cpu);
            t_cde *cde = leer_cde(buffer_cpu);
            int tamanio = tamanio_proceso(cde->pid);
            if (tamanio > nuevo_tamanio)
            {
                reducir_proceso(cde->pid, nuevo_tamanio, cliente_cpu); // entonce signfica que hay que reducir
            }
            else
            {
                ampliar_proceso(cde->pid, nuevo_tamanio, cliente_cpu); // significa que es mayor
            }
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

void pedido_frame_mmu(int cliente_cpu)
{

    tipo_buffer *buffer_mmu_cpu = recibir_buffer(cliente_cpu);
    int pagina = leer_buffer_enteroUint32(buffer_mmu_cpu);
    int pid = leer_buffer_enteroUint32(buffer_mmu_cpu);
    destruir_buffer(buffer_mmu_cpu);

    t_tabla_paginas *tabla_proceso = buscar_en_lista_global(pid);
    t_pagina *pagina_buscada = list_get(tabla_proceso->tabla_paginas_proceso, pagina);
    int marco = consultar_marco_de_una_pagina(tabla_proceso, pagina_buscada);

    tipo_buffer *buffer_memoria_mmu = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer_memoria_mmu, marco);
    enviar_buffer(buffer_memoria_mmu, cliente_cpu);
    destruir_buffer(buffer_memoria_mmu);
}

int tamanio_proceso(int pid)
{
    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid); // devuelve mi tabla de proceso
    int cant_paginas_proceso = list_size(tabla_paginas->tabla_paginas_proceso);
    return cant_paginas_proceso * valores_config->tam_pagina;
}

void *acceso_a_espacio_usuario(int cliente_solicitante)
{
    op_code codigo, solicitud;
    codigo = recibir_operacion(cliente_solicitante);
    switch (codigo)
    {
    case PEDIDO_ESCRITURA:

        solicitud = recibir_operacion(cliente_solicitante);

        if (solicitud == SOLICITUD_INTERFAZ_STDIN)
        {
            tipo_buffer *buffer_escritura = recibir_buffer(cliente_solicitante);
            escritura_interfaz(buffer_escritura, cliente_solicitante);
        }
        else if (solicitud == SOLICITUD_ESCRITURA_CPU)
        {
            tipo_buffer *buffer_escritura = recibir_buffer(cliente_solicitante);
            escritura_cpu(buffer_escritura, cliente_solicitante);
        }
        break;

    case PEDIDO_LECTURA:

        solicitud = recibir_operacion(cliente_solicitante);

        if (solicitud == SOLICITUD_INTERFAZ_STDOUT)
        {
            tipo_buffer *buffer_lectura = recibir_buffer(cliente_solicitante);
            lectura_interfaz(buffer_lectura, cliente_solicitante);
        }
        else if (solicitud == LECTURA_CPU)
        {
            tipo_buffer *buffer_lectura = recibir_buffer(cliente_solicitante);
            lectura_cpu(buffer_lectura, cliente_solicitante);
            destruir_buffer(buffer_lectura);
        }
        break;
    default:
        log_error(logger, "ERROR - ACCESO ESPACIO USUARIO");
        break;
    }
    return (void *)1;
}

void escritura_interfaz(tipo_buffer *buffer, int cliente_solicitante)
{
    int resultado;
    uint32_t direccion_fisica = leer_buffer_enteroUint32(buffer);
    uint32_t pid_ejecutando = leer_buffer_enteroUint32(buffer);
    uint32_t cant_caracteres = leer_buffer_enteroUint32(buffer);

    uint32_t numero_pagina = direccion_fisica / valores_config->tam_pagina;
    uint32_t offset = direccion_fisica % valores_config->tam_pagina;

    for (int i = 0; i < cant_caracteres; i++)
    {
        uint32_t valor = leer_buffer_enteroUint32(buffer);
        resultado = escribir_memoria(numero_pagina, offset, pid_ejecutando, valor, sizeof(valor));
        if (resultado == -1)
        {
            log_error(logger, "ERROR AL ESCRIBIR EL VALOR %d", valor);
        }
        else
        {
            log_info(logger, "Se escribio el valor: %d", valor);
            log_info(logger, "PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamaño <%d> ", pid_ejecutando, direccion_fisica, cant_caracteres);
        }
        offset += sizeof(valor);
    }
    if (resultado != -1)
    {
        enviar_cod_enum(cliente_solicitante, OK);
    }
    else
    {
        enviar_cod_enum(cliente_solicitante, ERROR_PEDIDO_ESCRITURA);
    }
    destruir_buffer(buffer);
}

void escritura_cpu(tipo_buffer *buffer, int cliente_solicitante)
{
    uint32_t direccion_fisica = leer_buffer_enteroUint32(buffer);
    uint32_t valor_a_escribir = leer_buffer_enteroUint32(buffer);
    uint32_t pid_ejecutando = leer_buffer_enteroUint32(buffer);
    uint32_t tamanio = leer_buffer_enteroUint32(buffer);

    uint32_t numero_pagina = direccion_fisica / valores_config->tam_pagina;
    uint32_t offset = direccion_fisica % valores_config->tam_pagina;

    int resultado = escribir_memoria(numero_pagina, offset, pid_ejecutando, valor_a_escribir, tamanio);

    if (resultado != -1)
    {
        enviar_cod_enum(cliente_solicitante, OK);
        log_info(logger, "PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamaño <%d> ", pid_ejecutando, direccion_fisica, tamanio);
    }
    else
    {
        enviar_cod_enum(cliente_solicitante, ERROR_PEDIDO_ESCRITURA);
    }
    destruir_buffer(buffer);
}

void lectura_interfaz(tipo_buffer *buffer_lectura, int cliente_solicitante)
{
    uint32_t direccion_fisica = leer_buffer_enteroUint32(buffer_lectura);
    uint32_t pid_ejecutando = leer_buffer_enteroUint32(buffer_lectura);
    uint32_t limite = leer_buffer_enteroUint32(buffer_lectura);

    uint32_t numero_pagina = direccion_fisica / valores_config->tam_pagina;
    uint32_t offset = direccion_fisica % valores_config->tam_pagina;

    tipo_buffer *buffer_stdout = crear_buffer();
    int valor;
    int *valor_void;

    for (int i = 0; i < limite; i++)
    {
        valor_void = (int *)leer_memoria(numero_pagina, offset, pid_ejecutando, sizeof(limite));
        valor = *valor_void;
        if (valor_void != NULL) //(&valor)
        {
            agregar_buffer_para_enterosUint32(buffer_stdout, valor);
            log_info(logger, "SE LEYO EL VALOR : %d", valor);
        }
        else
        {
            log_error(logger, "ERROR AL TRAER EL VALOR");
        }
        offset += (sizeof(limite));
    }

    if ((valor_void) != NULL) // (&valor)
    {
        enviar_cod_enum(cliente_solicitante, OK);
        log_info(logger, "PID: <%d> - Accion: LEER - Direccion fisica: <%d> - Tamaño <%d>", pid_ejecutando, direccion_fisica, limite);
        enviar_buffer(buffer_stdout, cliente_solicitante);
        destruir_buffer(buffer_stdout);
    }
    else
    {
        enviar_cod_enum(cliente_solicitante, ERROR_PEDIDO_LECTURA);
    }
}

void lectura_cpu(tipo_buffer *buffer_lectura, int cliente_solicitante)
{
    uint32_t direccion_fisica = leer_buffer_enteroUint32(buffer_lectura);
    uint32_t pid_ejecutando = leer_buffer_enteroUint32(buffer_lectura);
    uint32_t tamanio = leer_buffer_enteroUint32(buffer_lectura);

    uint32_t numero_pagina = direccion_fisica / valores_config->tam_pagina;
    uint32_t offset = direccion_fisica % valores_config->tam_pagina;

    void *valor_leido = leer_memoria(numero_pagina, offset, pid_ejecutando, tamanio);

    if (valor_leido != NULL)
    {
        enviar_cod_enum(cliente_solicitante, OK);
        tipo_buffer *buffer = crear_buffer();
        agregar_buffer_para_enterosUint32(buffer, valor_leido);
        enviar_buffer(buffer, cliente_solicitante);
    }
    else
    {
        enviar_cod_enum(cliente_solicitante, ERROR_PEDIDO_LECTURA);
    }
}

void iniciar_proceso(int cliente_fd, tipo_buffer *buffer)
{
    buffer = recibir_buffer(cliente_fd);
    t_cde *cde = armarCde(buffer);
    destruir_buffer(buffer);
    // agrego una tabla vacio de paginas asociada al proceso, auna lista de tablas globales
    crear_y_agregar_tabla_a_lista_global(cde->pid);

    cde->lista_instrucciones = leerArchivoConInstrucciones(cde->path);
    if (cde->path == NULL || cde->lista_instrucciones == NULL)
    {
        enviar_cod_enum(cliente_fd, ERROR_INICIAR_PROCESO);
    }
    else
    {
        list_add(lista_contextos, cde);
        list_add(lista_instrucciones, cde->lista_instrucciones);
        enviar_cod_enum(cliente_fd, INICIAR_PROCESO_CORRECTO);
        log_info(logger, "PID: <%d> - Iniciar Proceso: <%s>", cde->pid, cde->path);
    }
}

void crear_y_agregar_tabla_a_lista_global(int pid)
{
    t_tabla_paginas *tabla_proceso = malloc(sizeof(t_tabla_paginas));
    tabla_proceso->tabla_paginas_proceso = list_create();
    tabla_proceso->pid = pid;
    list_add(lista_global_tablas, tabla_proceso);
}

t_cde *armarCde(tipo_buffer *buffer)
{
    t_cde *cde = malloc(sizeof(t_cde));
    cde->pid = leer_buffer_enteroUint32(buffer);
    cde->path = leer_buffer_string(buffer);
    cde->lista_instrucciones = list_create();
    return cde;
}

t_list *leerArchivoConInstrucciones(char *nombre_archivo)
{
    t_list *list_instrucciones = list_create();
    char *ruta_completa = string_new();

    ruta_completa = obtener_ruta(nombre_archivo);

    FILE *archivo = fopen(ruta_completa, "r");

    if (archivo == NULL)
    {
        log_error(logger, "No se pudo abrir el archivo: <%s>", ruta_completa);
        return NULL;
    }
    char linea_instruccion[1024];
    while (fgets(linea_instruccion, sizeof(linea_instruccion), archivo) != NULL)
    {
        char *token = strdup(strtok(linea_instruccion, "\n"));
        // char *token_copia = token; sin esto tambien anda

        list_add(list_instrucciones, token);
    }
    fclose(archivo);
    free(ruta_completa);
    return list_instrucciones;
}

char *obtener_ruta(char *nombre_archivo)
{
    char *ruta_completa = string_new();
    char *ruta_acceso[1024];

    if (getcwd(ruta_acceso, sizeof(ruta_acceso)) == NULL)
    {
        log_info(logger, "No se pudo obtener la raiz");
        return NULL;
    }
    // TODO: esto debe tener configurada en realidad el path de config de memoria
    string_append(&ruta_completa, ruta_acceso);
    string_append(&ruta_completa, "/pruebas/checkpoint_3/archivos/");
    string_append(&ruta_completa, nombre_archivo);

    return ruta_completa;
}

void enviar_tamanio_pagina(int cpu)
{
    tipo_buffer *buffer_con_tam_pagina = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer_con_tam_pagina, valores_config->tam_pagina);
    enviar_buffer(buffer_con_tam_pagina, cpu);
    destruir_buffer(buffer_con_tam_pagina);
}

void pedido_instruccion_cpu_dispatch(int cliente_fd, t_list *contextos)
{
    // Ante cada petición se deberá esperar un tiempo determinado a modo de
    // retardo en la obtención de la instrucción, y este tiempo,
    // estará indicado en el archivo de configuración.
    sleep_ms(valores_config->retardo_respuesta); // cambiado a ms

    tipo_buffer *buffer = recibir_buffer(cliente_fd);
    uint32_t PID = leer_buffer_enteroUint32(buffer);
    uint32_t PC = leer_buffer_enteroUint32(buffer);

    t_cde *contexto = malloc(sizeof(t_cde));

    contexto = obtener_contexto_en_ejecucion(PID, contextos);

    tipo_buffer *buffer_instruccion = crear_buffer();
    char *instruccion = string_new();
    instruccion = list_get(contexto->lista_instrucciones, PC);

    enviar_cod_enum(cliente_fd, ENVIAR_INSTRUCCION_CORRECTO);
    agregar_buffer_para_string(buffer_instruccion, instruccion);

    enviar_buffer(buffer_instruccion, cliente_fd);
    destruir_buffer(buffer_instruccion);
    destruir_buffer(buffer);
    log_info(logger, "PID: <%d> - Instruccion: <%s>", PID, instruccion);
    free(instruccion);
}

t_cde *obtener_contexto_en_ejecucion(int PID, t_list *contextos)
{
    PID_buscado = PID;
    t_cde *cde_proceso = malloc(sizeof(t_cde));
    cde_proceso = list_find(contextos, estaElContextoConCiertoPID);
    cde_proceso->lista_instrucciones = list_get(lista_instrucciones, cde_proceso->pid);
    return cde_proceso;
}

char *obtener_char_instruccion(t_tipoDeInstruccion instruccion_code)
{
    if (instruccion_code == SET)
    {
        return "SET";
    }
    if (instruccion_code == MOV_IN)
    {
        return "MOV_IN";
    }
    if (instruccion_code == MOV_OUT)
    {
        return "MOV_OUT";
    }
    if (instruccion_code == SUM)
    {
        return "SUM";
    }
    if (instruccion_code == SUB)
    {
        return "SUB";
    }
    if (instruccion_code == JNZ)
    {
        return "JNZ";
    }
    if (instruccion_code == RESIZE)
    {
        return "RESIZE";
    }
    if (instruccion_code == COPY_STRING)
    {
        return "COPY_STRING";
    }
    if (instruccion_code == WAIT)
    {
        return "WAIT";
    }
    if (instruccion_code == SIGNAL)
    {
        return "SIGNAL";
    }
    if (instruccion_code == IO_GEN_SLEEP)
    {
        return "IO_GEN_SLEEP";
    }
    if (instruccion_code == IO_STDIN_READ)
    {
        return "IO_STDIN_READ";
    }
    if (instruccion_code == IO_STDOUT_WRITE)
    {
        return "IO_STDOUT_WRITE";
    }
    if (instruccion_code == IO_FS_CREATE)
    {
        return "IO_FS_CREATE";
    }
    if (instruccion_code == IO_FS_DELETE)
    {
        return "IO_FS_DELETE";
    }
    if (instruccion_code == IO_FS_TRUNCATE)
    {
        return "IO_FS_TRUNCATE";
    }
    if (instruccion_code == IO_FS_WRITE)
    {
        return "IO_FS_WRITE";
    }
    if (instruccion_code == IO_FS_READ)
    {
        return "IO_FS_READ";
    }
    if (instruccion_code == EXIT)
    {
        return "EXIT";
    }
    return NULL;
}

_Bool estaElContextoConCiertoPID(t_cde *contexto)
{
    return contexto->pid == PID_buscado;
}

void finalizar_proceso(int kernel, tipo_buffer *buffer)
{
    buffer = recibir_buffer(kernel);
    uint32_t pid = leer_buffer_enteroUint32(buffer);
    t_registros *registros = leer_buffer_registros(buffer);
    obtener_y_eliminar_cde(pid, registros);
    eliminar_tabla_paginas(pid);
    enviar_cod_enum(kernel, FINALIZAR_PROCESO);
}

void obtener_y_eliminar_cde(int pid, t_registros *reg)
{
    t_cde *cde = obtener_contexto_en_ejecucion(pid, lista_contextos);
    cde->registros = reg;
    eliminar_cde(cde);
}

void eliminar_cde(t_cde *cde)
{
    // VOLVER PARA LA ENTREGA FINAL
    /*     int instrucciones = list_size(cde->lista_instrucciones);
        for (int i = 0; i < instrucciones; i++)
        {
            t_instruccion *instruccion = list_get(cde->lista_instrucciones, i);
            int tamanio_lista = list_size(instruccion->parametros);
            for (int i = 0; i < tamanio_lista; i++)
            {
                char *par = list_get(instruccion->parametros, i);
                log_info(logger, "PARAMETRO A LIBERAR: %s", par);
                free(par);
            }
            list_destroy(instruccion->parametros);
        }
        free(cde->path);
        liberar_registros(cde->registros); */
}

void liberar_registros(t_registros *registros)
{
    free(registros->AX);
    free(registros->BX);
    free(registros->CX);
    free(registros->DI);
    free(registros->DX);
    free(registros->EAX);
    free(registros->EBX);
    free(registros->ECX);
    free(registros->EDX);
    free(registros->SI);
}

config_memoria *configuracion_memoria()
{
    config_memoria *valores_config = malloc(sizeof(config_memoria));

    valores_config->config = iniciar_config("memoria.config");
    valores_config->ip_memoria = config_get_string_value(valores_config->config, "IP");
    valores_config->puerto_memoria = config_get_string_value(valores_config->config, "PUERTO_ESCUCHA");
    valores_config->path_instrucciones = config_get_string_value(valores_config->config, "PATH_INSTRUCCIONES");
    valores_config->tam_memoria = config_get_int_value(valores_config->config, "TAM_MEMORIA");
    valores_config->tam_pagina = config_get_int_value(valores_config->config, "TAM_PAGINA");
    valores_config->retardo_respuesta = config_get_int_value(valores_config->config, "RETARDO_RESPUESTA");
    return valores_config;
}

t_pagina *crear_pagina(int bit_presencia, int marco, int pidProceso)
{
    t_pagina *pagina = malloc(sizeof(t_pagina));
    pagina->marco = marco;
    pagina->bit_validez = 1;
    pagina->pid = pidProceso;
    list_add(list_tabla_paginas, NULL); // la lista de paginas seria la tabla
    return pagina;
}

t_list *agregar_pagina(t_pagina *pagina, t_list *list_paginas)
{
    list_add(list_paginas, pagina);
    return list_paginas;
}

void eliminar_tabla_paginas(uint32_t pid)
{
    int cant_paginas = 0;
    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid); // busco en la lista global de  tabla del proceso

    int tamanio_tabla_pag = list_size(tabla_paginas->tabla_paginas_proceso);
    for (int i = 0; i < tamanio_tabla_pag; i++)
    {
        // voy recorriendo el proceso la cantida de epaginas
        t_pagina *pagina = list_get(tabla_paginas->tabla_paginas_proceso, i);
        if (pagina != NULL)
        {
            free(pagina);
            cant_paginas++; // voy contando las pag
        }
    }
    list_destroy(tabla_paginas->tabla_paginas_proceso); // elimino todas las paginas
    log_info(logger, "Destruccion :PID:%d  - Tamaño: %d ", pid, cant_paginas);
}

t_tabla_paginas *buscar_en_lista_global(int pid)
{
    int cantidad_tablas = list_size(lista_global_tablas);
    for (int i = 0; i < cantidad_tablas; i++)
    {
        t_tabla_paginas *tabla = list_get(lista_global_tablas, i);
        if (tabla->pid == pid)
        {
            return tabla;
        }
    }
    return NULL;
}

void colocar_pagina_en_marco(t_pagina *pagina)
{
    int *marco_libre = agarro_marco_que_este_libre(lista_global_marcos); // consigo el primer marco libre que encuentro
    if (marco_libre == NULL)
    {
        log_info(logger, "No hay ningun marco libre, OUT OF MEMORY");
        // puede ser que debamos finalizar el proceso
    }
    pagina->bit_validez = 1; // esta en memoria ffisica , o sea tiene un marco
    pagina->marco = *marco_libre;
}

int *agarro_marco_que_este_libre()
{
    int pos_marco_libre = obtener_marco_libre();           // busco el primer marco libre que encuentro en el bitmap
    return list_get(lista_global_marcos, pos_marco_libre); // hago list get para conseguir en la lista global de marcos,la posicion
}

int obtener_marco_libre()
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

int consultar_marco_de_una_pagina(t_tabla_paginas *tabla, t_pagina *pagina)
{ // para obtener el nro del marco que tiene una pagina asociado{
    int cant_paginas = list_size(tabla->tabla_paginas_proceso);

    for (int i = 0; i < cant_paginas; i++)
    {
        if (list_get(tabla->tabla_paginas_proceso, i) == pagina)
        {

            return pagina->marco;
        }
    }
    return -1;
}

void liberar_marco(int nroMarco)
{
    list_replace(lista_global_marcos, nroMarco, NULL);
}

void ampliar_proceso(uint32_t pid, uint32_t tamanio, int cliente_cpu)
{
    // Vemos cuantas paginas tenemos que agregar con este nuevo tamanio
    int paginas_adicionales = (tamanio + valores_config->tam_pagina - 1) / valores_config->tam_pagina;
    int tamanio_anterior = tamanio_proceso(pid);
    // 30
    //  Verificar si hay suficientes marcos disponibles
    int marcos_disponibles = 0;
    for (int i = 0; i < cant_marcos; i++)
    {
        if (array_bitmap[i].bit_ocupado == 0)
        { // si no esta ocupado
            marcos_disponibles++;
        }
    }
    // 32

    if (paginas_adicionales > marcos_disponibles)
    { // si hay mas paginas que  cant marcos le avisamos a cpu
        enviar_cod_enum(cliente_cpu, OUT_OF_MEMORY);
        log_error(logger, "OUT OF MEMORY");
        return;
    }
    // Buscar la tabla de páginas del proceso correspondiente
    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid);
    if (tabla_paginas == NULL)
    {
        log_info(logger, "NO se encontro la tabla de paginas con pid %d", pid);
        return;
    }
    int marco_libre;
    // Agrego las paginas
    t_list *paginas = tabla_paginas->tabla_paginas_proceso;
    for (int i = 0; i < paginas_adicionales; i++)
    {
        // Encontrar el próximo marco libre en el bitmap
        marco_libre = -1;
        for (int j = 0; j < cant_marcos; j++)
        {
            if (array_bitmap[j].bit_ocupado == 0)
                marco_libre = j;
            array_bitmap[j].bit_ocupado = 1; // Marcar el marco como ocupado
        }
        // Crear una nueva página y agregarla al proceso
        t_pagina *nueva_pagina = malloc(sizeof(t_pagina));
        nueva_pagina->marco = marco_libre; // Pongo el marco libre que encontre
        nueva_pagina->bit_validez = 1;
        list_add(paginas, nueva_pagina);
    }

    log_info(logger, "PID: <%d> - Tamaño Actual: <%d> - Tamaño a Ampliar: <%d>", pid, tamanio_anterior, tamanio);
    enviar_cod_enum(cliente_cpu, RESIZE_EXITOSO);
}

void reducir_proceso(uint32_t pid, uint32_t tamanio, int cliente_cpu)
{

    int paginas_requeridas = (tamanio + valores_config->tam_pagina - 1) / valores_config->tam_pagina; // cant de paginas que tiene que tener con este nuevo tamanio
    int tamanio_anterior = tamanio_proceso(pid);
    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid); // busco en la lista de listaa
    if (tabla_paginas == NULL)
    {
        log_info(logger, "NO se encontro la tabla de paginas con pid %d", pid);
        return;
    }
    int paginas_actuales = list_size(tabla_paginas->tabla_paginas_proceso); // cantida de paginas que hay jasta el momento
                                                                            // Si el proceso tiene más páginas de las requeridas, liberar las excesivas desde el final
    t_list *paginas = tabla_paginas->tabla_paginas_proceso;
    while (paginas_actuales > paginas_requeridas)
    {
        t_pagina *pagina_a_eliminar = list_remove(paginas, paginas_actuales - 1);
        pagina_a_eliminar->bit_validez = 0; // Marcar como no válida
        free(pagina_a_eliminar);            // Liberar la memoria de la página
        paginas_actuales--;
    }

    log_info(logger, "PID: %d - Tamaño Actual: <%d> - Tamaño a Reducir: <%d>", pid, tamanio_anterior, tamanio);
    enviar_cod_enum(cliente_cpu, RESIZE_EXITOSO);
}

void *leer_memoria(uint32_t numero_pagina, uint32_t offset, uint32_t pid, uint32_t tamanio)
{
    usleep(valores_config->retardo_respuesta * 1000);

    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid);
    t_pagina *pagina = list_get(tabla_paginas->tabla_paginas_proceso, numero_pagina);
    int marco = pagina->marco;

    void *valor = malloc(tamanio);

    memcpy(valor, espacio_usuario + marco * valores_config->tam_pagina + offset, tamanio);

    return valor;
}

void *escribir_memoria(uint32_t numero_pagina, uint32_t offset, uint32_t pid, void *valor_a_escribir, uint32_t tamanio)
{

    usleep(valores_config->retardo_respuesta * 1000);

    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid);
    if (tabla_paginas == NULL)
    {
        return (void *)-1;
    }
    t_pagina *pagina = list_get(tabla_paginas->tabla_paginas_proceso, numero_pagina);
    if (pagina == NULL)
    {
        return (void *)-1;
    }
    int marco = pagina->marco;
    if (marco < 0)
    {
        return (void *)-1;
    }
    void *destino = espacio_usuario + marco * valores_config->tam_pagina + offset;
    if (memcpy(destino, &valor_a_escribir, tamanio) == NULL)
    {
        return (void *)-1;
    }
    else
    {
        return (void *)1;
    }
}
