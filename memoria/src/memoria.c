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
t_bit_map* array_bitmap;

int main(int argc, char *argv[])
{
    

    logger = iniciar_logger("memoria.log", "MEMORIA");
    log_info(logger,"JOLA");
    valores_config = configuracion_memoria();
    
    lista_global_marcos = list_create(); // esta seria la tabla de marcos. Todos los marcos componen a la memoria
    lista_global_tablas = list_create();
    cant_marcos = valores_config->tam_memoria / valores_config->tam_pagina; // va a ser la cantida de amrcos

    
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
    array_bitmap= malloc(sizeof(t_bit_map)*cant_marcos);
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
    pthread_create(&hiloIO, NULL, recibirIO, NULL);
}

void *recibirIO()
{
    // va a recibir interfaces que le van a pedir acceso al espacio del usuario (stdin,stdout y dialfs) :D
    return EXIT_SUCCESS;
}

void *recibirKernel()
{
    tipo_buffer *buffer = crear_buffer();
    int cliente_fd = esperar_cliente(logger, "Memoria", "Kernel", server_fd);
    while (1)
    {
        op_code cod_op = recibir_operacion(cliente_fd);
        switch (cod_op)
        {
        case SOLICITUD_INICIAR_PROCESO:
            iniciar_proceso(cliente_fd, buffer);
            break;
        case SOLICITUD_FINALIZAR_PROCESO:
            finalizar_proceso(cliente_fd, buffer);
            break;
        case AMPLIACION_PROCESO:
            ampliar_proceso(cliente_fd, buffer);
            break;
        case REDUCION_PROCESO:
            reduccion_proceso(cliente_fd, buffer);
            break;
        case ACCESO_ESPACIO_USUARIO:
            acceso_a_espacio_usuario(cliente_fd, buffer);
            break;
        case -1:
            log_error(logger, " El KERNEL se desconecto. Terminando servidor");
            return (void *)EXIT_FAILURE;
            break;
        default:
            log_warning(logger, "Operacion desconocida. No quieras meter la pata");
            return (void *)EXIT_FAILURE;
            break;
        }
    }
}
void ampliar_proceso(int cliente, tipo_buffer *buffer)
{
}
void reduccion_proceso(int cliente, tipo_buffer *buffer)
{
}
void acceso_a_espacio_usuario(int cliente, tipo_buffer *buffer)
{
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
        char *token_copia = token;

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
    string_append(&ruta_completa, "/pruebas/");
    string_append(&ruta_completa, nombre_archivo);

    return ruta_completa;
}

void *recibirCPU()
{
    int cliente_cpu = esperar_cliente(logger, "Memoria", "CPU", server_fd);
    enviar_tamanio_pagina(cliente_cpu);
    while (1)
    {
        op_code cod_op = recibir_operacion(cliente_cpu);
        switch (cod_op)
        {
        case PEDIDO_INSTRUCCION:
            pedido_instruccion_cpu_dispatch(cliente_cpu, lista_contextos);
            break;
            // case ACCESO_ESPACIO_USUARIO:
        case -1:
            log_error(logger, "El cliente se desconecto. Terminando servidor");
            exit(EXIT_FAILURE);
            return (void *)EXIT_FAILURE;
            break;
            // return EXIT_FAILURE; // version de la catedra, pero da un warning si no anda comentar el de arriba
        default:
            log_warning(logger, "Operacion desconocida. No quieras meter la pata");
            break;
        }
    }
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
    usleep(valores_config->retardo_respuesta);

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
    return -1;
}

_Bool estaElContextoConCiertoPID(t_cde *contexto)
{
    return contexto->pid == PID_buscado;
}

void finalizar_proceso(int kernel, tipo_buffer *buffer)
{
    buffer = recibir_buffer(kernel);
    uint32_t pid = leer_buffer_enteroUint32(buffer);
    obtener_y_eliminar_cde(pid);
    enviar_cod_enum(kernel, FINALIZAR_PROCESO);
    eliminar_tabla_paginas(pid);
}

void obtener_y_eliminar_cde(int pid)
{
    t_cde *cde = obtener_contexto_en_ejecucion(pid, lista_contextos);
    eliminar_cde(cde);
}

void eliminar_cde(t_cde *cde)
{
    list_clean_and_destroy_elements(cde->lista_instrucciones, destroy_instruccion);
    free(cde->path);
    liberar_registros(cde->registros);
}

void *destroy_instruccion(void *element)
{
    char *instruccion = (char *)element;
    free(instruccion);
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
    pagina->bit_modificado = false;
    pagina->bit_presencia = true;
    pagina->pid = pidProceso;
    list_add(list_tabla_paginas, NULL); // la lista de paginas seria la tabla
}
t_list *agregar_pagina(t_pagina *pagina, t_list *list_paginas)
{
    list_add(list_paginas, pagina);
}
/*
uint32_t hay_marco_libre()
{
    for (int i = 0; i < tam_marco; i++)
    {
        t_pagina *pag = list_get(lista_marcos, i);
        if (pag == NULL)
            return 1;
        else
            return 0;
    }
}
*/


void eliminar_tabla_paginas(uint32_t pid)
{
    int cant_paginas = 0;
    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid); // busco en la lista global de  tabla del proceso

    int tamanio_tabla_pag = list_size(tabla_paginas->tabla_paginas_proceso);
    for (int i = 0; i < tamanio_tabla_pag; i++)
    {
        // voy recorriendo el proceso la cantida de epaginas
        t_pagina *pagina= list_get(tabla_paginas->tabla_paginas_proceso,i);
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
        log_info(logger,"No hay ningun marco libre, OUT OF MEMORY");
        // puede ser que debamos finalizar el proceso
    }
    pagina->bit_presencia = 1; // esta en memoria ffisica , o sea tiene un marco
    pagina->marco = marco_libre;
}
int *agarro_marco_que_este_libre()
{
    int pos_marco_libre = obtener_marco_libre();                         // busco el primer marco libre que encuentro en el bitmap
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
}

// direccion física = nro marco * tamaño marco + OFFSET