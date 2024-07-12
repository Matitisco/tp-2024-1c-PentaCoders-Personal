
#include "../include/memoria.h"

pthread_t hiloCpu;
pthread_t hiloKernel;
pthread_t hiloIO;

config_memoria *valores_config;

int server_fd;
int PID_buscado;
int cliente_cpu;
int cliente_kernel;
int dispositivo_io;
int CLIENTE_ESPACIO_USUARIO;
int pid_a_buscar_o_eliminar;

t_list *lista_contextos;
t_list *lista_instrucciones;
t_list *lista_global_tablas;
t_list *lista_global_marcos;
t_tabla_paginas *tabla_actual;
t_bit_map *array_bitmap;

int main(int argc, char *argv[])
{
    logger = iniciar_logger("memoria.log", "MEMORIA");
    valores_config = configuracion_memoria();

    lista_global_tablas = list_create();
    cant_marcos = valores_config->tam_memoria / valores_config->tam_pagina;
    crear_espacio_usuario();

    tabla_actual = malloc(sizeof(t_tabla_paginas));

    inicializar_bitmap(cant_marcos);
    crear_marcos(cant_marcos);

    lista_contextos = list_create();
    lista_instrucciones = list_create();

    crearHilos();
    pthread_join(hiloCpu, NULL);
    pthread_join(hiloKernel, NULL);
    pthread_join(hiloIO, NULL);
    destruirConfig(valores_config->config);
    destruirLog(logger);
}

void crear_marcos(int cant_marcos)
{
    lista_global_marcos = list_create();
    for (int i = 0; i < cant_marcos; i++)
    {
        list_add(lista_global_marcos, NULL); // creo mi lista de marcos-tabla. En principio todo en null
    }
}
void crear_espacio_usuario()
{
    espacio_usuario = malloc(valores_config->tam_memoria);
    if (espacio_usuario == NULL)
    {
        log_error(logger, "ERROR ESPACIO USUARIO");
    }
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

    cliente_cpu = esperar_cliente(logger, "Memoria", "CPU", server_fd);
    cliente_kernel = esperar_cliente(logger, "Memoria", "Kernel", server_fd);

    pthread_create(&hiloCpu, NULL, recibirCPU, NULL);
    pthread_create(&hiloKernel, NULL, recibirKernel, NULL);
    pthread_create(&hiloIO, NULL, recibir_interfaces_io, NULL);
}

void *recibir_interfaces_io()
{
    while (1)
    {
        int dispositivo_io = esperar_cliente(logger, "Memoria", "Interfaz IO", server_fd); // lo dejamos en el hilo para que se conecten varias
        op_code codigo_io = recibir_op_code(dispositivo_io);
        switch (codigo_io)
        {
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

void *recibirKernel()
{
    tipo_buffer *buffer = crear_buffer();
    while (1)
    {
        op_code cod_op = recibir_op_code(cliente_kernel);

        if (cod_op == -1)
        {
            log_error(logger, " El KERNEL se desconecto. Terminando servidor");
            return (void *)EXIT_FAILURE;
        }

        switch (cod_op)
        {
        case SOLICITUD_INICIAR_PROCESO:
            iniciar_proceso(cliente_kernel, buffer);
            break;
        case SOLICITUD_FINALIZAR_PROCESO:
            finalizar_proceso(cliente_kernel, buffer);
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
    enviar_tamanio_pagina(cliente_cpu);
    while (1)
    {
        op_code cod_op = recibir_op_code(cliente_cpu);

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
            CLIENTE_ESPACIO_USUARIO = cliente_cpu;
            acceso_a_espacio_usuario();
            break;
        case RESIZE_EXTEND:
            tipo_buffer *buffer_cpu = recibir_buffer(cliente_cpu);
            uint32_t nuevo_tamanio = leer_buffer_enteroUint32(buffer_cpu);
            t_cde *cde = leer_cde(buffer_cpu);
            int tamanio = tamanio_proceso(cde->pid);
            if (tamanio > nuevo_tamanio)
            {
                reducir_proceso(cde->pid, nuevo_tamanio, cliente_cpu);
            }
            else
            {
                ampliar_proceso(cde->pid, nuevo_tamanio, cliente_cpu);
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

// PEDIDO DE MMU

void pedido_frame_mmu(int cliente_cpu)
{

    tipo_buffer *buffer_mmu_cpu = recibir_buffer(cliente_cpu);
    int pid = leer_buffer_enteroUint32(buffer_mmu_cpu);
    int pagina = leer_buffer_enteroUint32(buffer_mmu_cpu);
    destruir_buffer(buffer_mmu_cpu);

    t_tabla_paginas *tabla = buscar_en_lista_global(pid);

    if (pagina >= list_size(tabla->paginas_proceso)  )
    {
        enviar_op_code(cliente_cpu, PEDIDO_FRAME_INCORRECTO);
        return;
    }

    t_pagina *pagina_buscada = list_get(tabla->paginas_proceso, pagina);
    int marco = pagina_buscada->marco;

    if (marco < 0)
    {
        enviar_op_code(cliente_cpu, PEDIDO_FRAME_INCORRECTO);
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

void *acceso_a_espacio_usuario()
{
    op_code codigo, solicitud;
    codigo = recibir_op_code(CLIENTE_ESPACIO_USUARIO);
    switch (codigo)
    {
    case PEDIDO_ESCRITURA:
        solicitud = recibir_op_code(CLIENTE_ESPACIO_USUARIO);

        if (solicitud == SOLICITUD_INTERFAZ_STDIN)
        {
            tipo_buffer *buffer_escritura = recibir_buffer(CLIENTE_ESPACIO_USUARIO);
            escritura_interfaz(buffer_escritura, CLIENTE_ESPACIO_USUARIO);
        }

        else if (solicitud == SOLICITUD_ESCRITURA_CPU)
        {
            tipo_buffer *buffer_escritura = recibir_buffer(CLIENTE_ESPACIO_USUARIO);
            escritura_cpu(buffer_escritura, CLIENTE_ESPACIO_USUARIO);
        }
        else if (solicitud == SOLICITUD_ESCRITURA_DIALFS)
        {
            tipo_buffer *buffer_escritura = recibir_buffer(CLIENTE_ESPACIO_USUARIO);
            escritura_cpu(buffer_escritura, CLIENTE_ESPACIO_USUARIO);
        }
        break;

    case PEDIDO_LECTURA:
        solicitud = recibir_op_code(CLIENTE_ESPACIO_USUARIO);

        if (solicitud == SOLICITUD_INTERFAZ_STDOUT)
        {
            tipo_buffer *buffer_lectura = recibir_buffer(CLIENTE_ESPACIO_USUARIO);
            lectura_interfaz(buffer_lectura, CLIENTE_ESPACIO_USUARIO);
        }
        else if (solicitud == LECTURA_CPU)
        {
            tipo_buffer *buffer_lectura = recibir_buffer(CLIENTE_ESPACIO_USUARIO);
            lectura_cpu(buffer_lectura, CLIENTE_ESPACIO_USUARIO);
            destruir_buffer(buffer_lectura);
        }
        else if (solicitud == LECTURA_DIALFS)
        {
            tipo_buffer *buffer_lectura = recibir_buffer(CLIENTE_ESPACIO_USUARIO);
            lectura_interfaz(buffer_lectura, CLIENTE_ESPACIO_USUARIO);
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

    uint32_t numero_marco = direccion_fisica / valores_config->tam_pagina;
    uint32_t offset = direccion_fisica % valores_config->tam_pagina;

    for (int i = 0; i < cant_caracteres; i++)
    {
        uint32_t valor = leer_buffer_enteroUint32(buffer);
        resultado = (uint32_t)escribir_memoria(numero_marco, offset, pid_ejecutando, &valor, sizeof(valor));
        if (resultado == -1)
        {
            log_error(logger, "ERROR AL ESCRIBIR EL VALOR %d", valor);
        }
        else
        {
            log_info(logger, "Se escribio el valor: %d", valor);
            log_info(logger, "PID: <%d> - Accion: <ESCRIBIR> - Direccion fisica: <%d> - Tamaño <%d> ", pid_ejecutando, direccion_fisica, cant_caracteres);
        }
        offset += sizeof(valor);
    }
    if (resultado != -1)
    {
        enviar_op_code(cliente_solicitante, OK);
        agregar_buffer_para_enterosUint32(buffer, resultado);
        enviar_buffer(buffer, cliente_solicitante);
    }
    else
    {
        enviar_op_code(cliente_solicitante, ERROR_PEDIDO_ESCRITURA);
    }

    destruir_buffer(buffer);
}

void escritura_cpu(tipo_buffer *buffer, int cliente_solicitante)
{
    uint32_t direccion_fisica = leer_buffer_enteroUint32(buffer);
    uint32_t valor_a_escribir = leer_buffer_enteroUint32(buffer);
    uint32_t pid_ejecutando = leer_buffer_enteroUint32(buffer);
    uint32_t tamanio = leer_buffer_enteroUint32(buffer);

    uint32_t numero_marco = direccion_fisica / valores_config->tam_pagina;
    uint32_t offset = direccion_fisica % valores_config->tam_pagina;
    // chequear si dicho pid puede ecribir
    int resultado = escribir_espacio_usuario(direccion_fisica, &valor_a_escribir, tamanio);

    if (resultado != -1)
    {
        log_info(logger, "Se escribio el valor: %d", valor_a_escribir);
        enviar_op_code(cliente_solicitante, OK);
    }
    else
    {
        enviar_op_code(cliente_solicitante, ERROR_PEDIDO_ESCRITURA);
    }
    destruir_buffer(buffer);
}

void lectura_interfaz(tipo_buffer *buffer_lectura, int cliente_solicitante)
{
    uint32_t direccion_fisica = leer_buffer_enteroUint32(buffer_lectura);
    uint32_t pid_ejecutando = leer_buffer_enteroUint32(buffer_lectura);
    uint32_t limite = leer_buffer_enteroUint32(buffer_lectura);

    uint32_t numero_marco = direccion_fisica / valores_config->tam_pagina;
    uint32_t offset = direccion_fisica % valores_config->tam_pagina;

    tipo_buffer *buffer_stdout = crear_buffer();
    int valor;
    int *valor_void;

    for (int i = 0; i < limite; i++)
    {
        valor_void = leer_memoria(numero_marco, offset, pid_ejecutando, sizeof(limite));
        if (valor_void != NULL)
        {
            agregar_buffer_para_enterosUint32(buffer_stdout, *valor_void);
            log_info(logger, "SE LEYO EL VALOR : %d", *valor_void);
        }
        else
        {
            log_error(logger, "ERROR AL TRAER EL VALOR");
        }
        offset += (sizeof(limite));
    }

    if ((valor_void) != NULL)
    {
        enviar_op_code(cliente_solicitante, OK);
        log_info(logger, "PID: <%d> - Accion: <LEER> - Direccion fisica: <%d> - Tamaño <%d>", pid_ejecutando, direccion_fisica, limite);
        enviar_buffer(buffer_stdout, cliente_solicitante);
        destruir_buffer(buffer_stdout);
    }
    else
    {
        enviar_op_code(cliente_solicitante, ERROR_PEDIDO_LECTURA);
    }
}

void lectura_cpu(tipo_buffer *buffer_lectura, int cliente_solicitante)
{
    uint32_t direccion_fisica = leer_buffer_enteroUint32(buffer_lectura);
    log_info(logger, "DIRECCION FISICA ENVIADA POR CPU: %u", direccion_fisica); // 32
    uint32_t pid_ejecutando = leer_buffer_enteroUint32(buffer_lectura);
    uint32_t tamanio = leer_buffer_enteroUint32(buffer_lectura);

    // uint32_t numero_pagina = direccion_fisica / valores_config->tam_pagina;
    // uint32_t offset = direccion_fisica % valores_config->tam_pagina;

    void *valor_leido = leer_espacio_usuario(direccion_fisica, tamanio);
    uint32_t valor = *(uint32_t *)valor_leido;

    if (valor_leido != NULL)
    {
        enviar_op_code(cliente_solicitante, OK);
        tipo_buffer *buffer = crear_buffer();
        agregar_buffer_para_enterosUint32(buffer, valor);
        log_info(logger, "SE LEYO EL VALOR : %u", valor);
        enviar_buffer(buffer, cliente_solicitante);
    }
    else
    {
        enviar_op_code(cliente_solicitante, ERROR_PEDIDO_LECTURA);
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
        enviar_op_code(cliente_fd, ERROR_INICIAR_PROCESO);
    }
    else
    {
        list_add(lista_contextos, cde);
        list_add(lista_instrucciones, cde->lista_instrucciones);
        enviar_op_code(cliente_fd, INICIAR_PROCESO_CORRECTO);
        log_info(logger, "PID: <%d> - Iniciar Proceso: <%s>", cde->pid, cde->path);
    }
}

void crear_y_agregar_tabla_a_lista_global(int pid)
{
    t_tabla_paginas *tabla_proceso = malloc(sizeof(t_tabla_paginas));
    tabla_proceso->paginas_proceso = list_create();
    tabla_proceso->pid = pid;
    list_add(lista_global_tablas, tabla_proceso);
    log_info(logger, "PID: <%d> - Tamaño : <%d>", pid, list_size(tabla_proceso->paginas_proceso));
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
    string_append(&ruta_completa, valores_config->path_instrucciones);
    // string_append(&ruta_completa, "/");
    string_append(&ruta_completa, nombre_archivo);
    // ruta_completa = obtener_ruta(nombre_archivo);

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
        list_add(list_instrucciones, token);
    }
    fclose(archivo);
    free(ruta_completa);
    return list_instrucciones;
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
    sleep_ms(valores_config->retardo_respuesta);

    tipo_buffer *buffer = recibir_buffer(cliente_fd);
    uint32_t PID = leer_buffer_enteroUint32(buffer);
    uint32_t PC = leer_buffer_enteroUint32(buffer);

    t_cde *contexto = malloc(sizeof(t_cde));

    contexto = obtener_contexto_en_ejecucion(PID);

    tipo_buffer *buffer_instruccion = crear_buffer();
    char *instruccion = string_new();
    instruccion = list_get(contexto->lista_instrucciones, PC);

    enviar_op_code(cliente_fd, ENVIAR_INSTRUCCION_CORRECTO);
    agregar_buffer_para_string(buffer_instruccion, instruccion);

    enviar_buffer(buffer_instruccion, cliente_fd);
    destruir_buffer(buffer_instruccion);
    destruir_buffer(buffer);
    log_info(logger, "PID: <%d> - Instruccion: <%s>", PID, instruccion);
    free(instruccion);
}

// FINALIZAR PROCESO

void finalizar_proceso(int kernel, tipo_buffer *buffer)
{
    buffer = recibir_buffer(kernel);
    uint32_t pid = leer_buffer_enteroUint32(buffer);

    
    eliminar_tabla_paginas(pid);

    //eliminar_cde(pid);
    enviar_op_code(kernel, FINALIZAR_PROCESO);
}

t_cde *obtener_contexto_en_ejecucion(int pid)
{
    t_cde *cde_proceso = malloc(sizeof(t_cde));
    pid_a_buscar_o_eliminar = pid;
    cde_proceso = list_find(lista_contextos, estaElContextoConCiertoPID);
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
    //imprimir todas las intrucciones que tiene el cde
    for (int i = 0; i < list_size(cde->lista_instrucciones); i++)
    {
        log_info(logger, "Instruccion: %s", list_get(cde->lista_instrucciones, i));
    }

    list_remove_by_condition(lista_contextos, estaElContextoConCiertoPID);
    list_destroy_and_destroy_elements(cde->lista_instrucciones, element_destroyer);

    //free(cde->path);
    //free(cde);
}

void element_destroyer(void *element)
{
    log_info(logger, "Elemento eliminado: %s", element);
    free(element);
}

// CONFIGURACION DE LA MEMORIA

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

t_pagina *crear_pagina(int bit_validez, int marco, int pidProceso)
{
    t_pagina *pagina = malloc(sizeof(t_pagina));
    pagina->marco = marco;
    pagina->bit_validez = 1;
    pagina->pid = pidProceso;
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
    list_destroy_and_destroy_elements(tabla_paginas->paginas_proceso, (void*) destruir_pagina);

    log_info(logger, "PID: <%d> - Tamaño: <%d> ", pid, cant_paginas);
}

void destruir_pagina(void *pagina) {
    t_pagina *pagina2 = (t_pagina *)pagina;
    printf("Pagina destruida:marco:%d pid: %d\n",pagina2->marco,pagina2->pid);
    free(pagina2);
}

t_tabla_paginas *buscar_en_lista_global(int pid)
{
    int cantidad_tablas = list_size(lista_global_tablas); // 1

    for (int i = 0; i < cantidad_tablas; i++)
    {
        if (i > list_size(lista_global_tablas))
        {
            break;
        }

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
    if (marco_libre == -1)
    {
        log_info(logger, "No hay ningun marco libre, OUT OF MEMORY");
        // puede ser que debamos finalizar el proceso
    }
    pagina->bit_validez = 1; // esta en memoria fisica , o sea tiene un marco
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

int consultar_marco_de_una_pagina(t_tabla_paginas *tabla, t_pagina *pagina_buscada)
{
    int cant_paginas = list_size(tabla->paginas_proceso);

    for (int i = 0; i < cant_paginas; i++)
    {
        if (list_get(tabla->paginas_proceso, i) == pagina_buscada)
        {

            return pagina_buscada->marco;
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
    log_info(logger, "CANTIDAD MARCOS A ASIGNAR: %d", paginas_adicionales);
    int tamanio_anterior = tamanio_proceso(pid);
    int paginas_actuales = (tamanio_anterior + valores_config->tam_pagina - 1) / valores_config->tam_pagina;

    //  Verificar si hay suficientes marcos disponibles
    hay_marcos_suficientes(paginas_adicionales, cliente_cpu);

    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid);

    if (tabla_paginas == NULL)
    {
        log_error(logger, "PID: <%d> - NO SE ENCONTRO TABLA PAGINAS", pid);
        return;
    }

    int marco_libre, j = 0;

    t_list *tp_paginas_proceso = tabla_paginas->paginas_proceso;

    for (int i = 0; i < cant_marcos; i++)
    {
        marco_libre = -1;
        if (j == paginas_adicionales)
        {
            break;
        }
        while (j < (paginas_adicionales))
        {
            if (array_bitmap[i].bit_ocupado == 0)
            {
                marco_libre = i;

                agregar_pagina(crear_pagina(1, marco_libre, pid), tp_paginas_proceso);
                array_bitmap[i].bit_ocupado = 1;
                j++;
                break;
            }
            else
            {
                j++;
                break;
            }
        }
    }

    for (int i = 0; i < list_size(tp_paginas_proceso); i++)
    {
        t_pagina *pagina = list_get(tp_paginas_proceso, i);
        log_info(logger, "PAGINA: %d, PID:%d, MARCO:%d, VALIDEZ:%d ", i, pagina->pid, pagina->marco, pagina->bit_validez);
    }

    log_info(logger, "PID: <%d> - Tamaño Actual: <%d> - Tamaño a Ampliar: <%d>", pid, tamanio_anterior, tamanio);
    enviar_op_code(cliente_cpu, RESIZE_EXITOSO);
}

void hay_marcos_suficientes(int paginas_adicionales, int cliente_cpu)
{
    int marcos_disponibles = 0;
    for (int i = 0; i < cant_marcos; i++)
    {
        if (array_bitmap[i].bit_ocupado == 0)
        { // si no esta ocupado
            marcos_disponibles++;
        }
    }
    // 32
    log_info(logger, "MARCOS DISPONIBLES: <%d>", marcos_disponibles);
    if (paginas_adicionales > marcos_disponibles)
    { // si hay mas paginas que  cant marcos le avisamos a cpu
        enviar_op_code(cliente_cpu, OUT_OF_MEMORY);
        log_error(logger, "OUT OF MEMORY");
        return;
    }
    // Buscar la tabla de páginas del proceso correspondiente
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
    int paginas_actuales = list_size(tabla_paginas->paginas_proceso); // cantida de paginas que hay jasta el momento
                                                                      // Si el proceso tiene más páginas de las requeridas, liberar las excesivas desde el final
    t_list *paginas = tabla_paginas->paginas_proceso;
    while (paginas_actuales > paginas_requeridas)
    {
        t_pagina *pagina_a_eliminar = list_remove(paginas, paginas_actuales - 1);
        pagina_a_eliminar->bit_validez = 0; // Marcar como no válida
        free(pagina_a_eliminar);            // Liberar la memoria de la página
        paginas_actuales--;
    }

    log_info(logger, "PID: %d - Tamaño Actual: <%d> - Tamaño a Reducir: <%d>", pid, tamanio_anterior, tamanio);
    enviar_op_code(cliente_cpu, RESIZE_EXITOSO);
}

void *leer_memoria(uint32_t numero_pagina, uint32_t offset, uint32_t pid, uint32_t tamanio)
{
    usleep(valores_config->retardo_respuesta * 1000);
    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid);
    t_pagina *pagina = list_get(tabla_paginas->paginas_proceso, numero_pagina);
    int marco = pagina->marco;
    log_info(logger, "PID: <%d> - Pagina: <%d> - Marco: <%d>", pid, numero_pagina, marco);
    void *valor = malloc(tamanio);
    memcpy(&valor, espacio_usuario + (marco * valores_config->tam_pagina + offset), tamanio);

    return valor;
}

void *escribir_memoria(uint32_t numero_pagina, uint32_t offset, uint32_t pid, void *valor_a_escribir, uint32_t tamanio)
{
    usleep(valores_config->retardo_respuesta * 1000);
    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid);
    t_pagina *pagina = list_get(tabla_paginas->paginas_proceso, numero_pagina);
    int marco = pagina->marco;
    log_info(logger, "PID: <%d> - Pagina: <%d> - Marco: <%d>", pid, numero_pagina, marco);
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

void *escribir_espacio_usuario(uint32_t direccion_fisica, void *valor_a_escribir, size_t tamanio)
{
    sleep_ms(valores_config->retardo_respuesta);
    log_info(logger, "PID: <PID> - Accion: <ESCRIBIR> - Direccion fisica: <%d> - Tamaño <%zu>", direccion_fisica, tamanio);

    void *destino = espacio_usuario + direccion_fisica;
    memcpy(destino, valor_a_escribir, tamanio);
    // implementar chequeo de si se puede escribir dicho espacio
    return (void *)1;
}
void *leer_espacio_usuario(uint32_t direccion_fisica, size_t tamanio)
{
    sleep_ms(valores_config->retardo_respuesta);
    log_info(logger, "PID: <PID> - Accion: <LEER> - Direccion fisica: <%d> - Tamaño <%zu>", direccion_fisica, tamanio);

    void *valor = malloc(tamanio);
    memcpy(valor, espacio_usuario + direccion_fisica, tamanio);
    // implementar chequeo de si se puede leer dicho espacio
    return valor;
}
