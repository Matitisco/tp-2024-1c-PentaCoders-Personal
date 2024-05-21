#include "../include/memoria.h"
// CRAER HILOS COMO VARIABLES GLOBALES
pthread_t hiloCpu;
pthread_t hiloKernel;
pthread_t hiloIO;
struct config_memoria *valores_config;
int server_fd;
// sem_t *sem_kernel;

t_list *lista_procesos;

int main(int argc, char *argv[])
{

    logger = iniciar_logger("memoria.log", "MEMORIA");
    valores_config = config_memoria();

    lista_procesos = list_create();
    // iniciar_sem_globales();
    crearHilos();

    pthread_join(hiloCpu, NULL);
    pthread_join(hiloKernel, NULL);
    pthread_join(hiloIO, NULL);
    destruirConfig(valores_config->config);
    destruirLog(logger);
}
void crearHilos()
{
    server_fd = iniciar_servidor(logger, "Memoria", valores_config->ip_memoria, valores_config->puerto_memoria);
    log_info(logger, "Memoria lista para recibir KERNEL, CPU Y dispositivos de ENTRADA/SALIDA");

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
        // sem_wait(sem_kernel); // espero a que el kernel me envie un mensaje
        op_code cod_op = recibir_operacion(cliente_fd);
        switch (cod_op)
        {
        case SOLICITUD_INICIAR_PROCESO:
            iniciar_proceso(cliente_fd, buffer);
            break;
        case SOLICITUD_FINALIZAR_PROCESO:
            finalizar_proceso(cliente_fd, buffer);
            break;
        case ERROR_CLIENTE_DESCONECTADO:
            log_error(logger, " El KERNEL se desconecto. Terminando servidor");
            return (void *)EXIT_FAILURE;
            break;
        default:
            log_warning(logger, "Operacion desconocida. No quieras meter la pata");
            // return (void *)EXIT_FAILURE;
            break;
        }
    }
}
void iniciar_proceso(int cliente_fd, tipo_buffer *buffer)
{
    log_info(logger, "Me llego la Solicitud de Iniciar Proceso");
    buffer = recibir_buffer(cliente_fd);
    t_cde *cde = armarCde(buffer);
    destruir_buffer(buffer);
    cde->lista_instrucciones = leerArchivoConInstrucciones(cde->path);
    if (cde->path == NULL || cde->lista_instrucciones == NULL)
    {
        enviar_cod_enum(cliente_fd, ERROR_INICIAR_PROCESO);
    }
    else
    {
        list_add(lista_procesos,cde);
        enviar_cod_enum(cliente_fd, INICIAR_PROCESO_CORRECTO);
        log_info(logger, "Se inicio el proceso de PID: %d y PATH: %s", cde->pid, cde->path);
    }
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
    t_list *list_instrucciones = list_create(); // creo el puntero a la lista
    char *ruta_completa = string_new();
    char *ruta_acceso[1024];

    if (getcwd(ruta_acceso, sizeof(ruta_acceso)) == NULL)
    {
        log_info(logger, "No se pudo obtener la raiz");
        return NULL;
    }
    // char *ruta_acceso = "tp-2024-1c-PentaCoders/memoria/pruebas/";
    // string_append(&ruta_completa, ruta_acceso);
    string_append(&ruta_completa, ruta_acceso);
    string_append(&ruta_completa, "/pruebas/");
    string_append(&ruta_completa, nombre_archivo);
    log_info(logger, "El path del archivo es : %s", ruta_completa);
    FILE *archivo = fopen(ruta_completa, "r");
    if (archivo == NULL)
    {
        // log_warning(logger, "No se pudo abrir el archivo");
        log_warning(logger, "No se pudo abrir el archivo: %s", ruta_completa);

        return NULL;
    }
    char linea_instruccion[1024]; // este seria el buffer para ir leyendo el archivo
    while (fgets(linea_instruccion, sizeof(linea_instruccion), archivo) != NULL)
    { // voy leyendo el archivo
        strtok(linea_instruccion, "\n");
        list_add(list_instrucciones, linea_instruccion); // agrego una instruccion a la lista
        log_info(logger, "Se agrego la instruccion: %s", linea_instruccion);
    }
    fclose(archivo);
    free(ruta_completa);
    return list_instrucciones;
}
void *recibirCPU()
{
    int cliente_fd = esperar_cliente(logger, "Memoria", "CPU", server_fd);
    while (1)
    {
        op_code cod_op = recibir_operacion(cliente_fd);
        switch (cod_op)
        {
        case PEDIDO_INSTRUCCION:
            pedido_instruccion_cpu_dispatch(cliente_fd);
            break;
            // case ACCESO_ESPACIO_USUARIO:
        case ERROR_CLIENTE_DESCONECTADO:
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
void pedido_instruccion_cpu_dispatch(int cliente_fd)
{
    log_info(logger, "Me llego desde CPU Dispatch un pedido de instruccion");
    tipo_buffer *buffer = recibir_buffer(cliente_fd);

    uint32_t PID = leer_buffer_enteroUint32(buffer);
    uint32_t PC = leer_buffer_enteroUint32(buffer);
    destruir_buffer(buffer);

    t_cde *cde_proceso = malloc(sizeof(t_cde));
    //proceso->cde = malloc(sizeof())
    cde_proceso = list_get(lista_procesos, PID);

    //t_list *lista_instrucciones = cde_proceso->lista_instrucciones;

    char *instruccion = list_get(cde_proceso->lista_instrucciones, PC);

    tipo_buffer *buffer_instruccion = crear_buffer();

    agregar_buffer_para_string(buffer_instruccion, instruccion);
    enviar_buffer(buffer_instruccion, socket_cpu);
    destruir_buffer(buffer_instruccion);

    log_info(logger, "Se va a enviar la instruccion: %s", instruccion);
    log_info(logger, "Se aprueba Pedido Instruccion");
    enviar_cod_enum(cliente_fd,ENVIAR_INSTRUCCION_CORRECTO);
}
void finalizar_proceso(int cliente_fd, tipo_buffer *buffer) // HACER
{
    buffer = recibir_buffer(cliente_fd);
    uint32_t pid_a_eliminar = leer_buffer_enteroUint32(buffer);
    eliminar_proceso(pid_a_eliminar);
}
void eliminar_proceso(int pid_a_eliminar) // HACER
{
}
struct config_memoria *config_memoria()
{
    struct config_memoria *valores_config = malloc(sizeof(struct config_memoria));

    // creo el config
    valores_config->config = iniciar_config("memoria.config");

    valores_config->ip_memoria = config_get_string_value(valores_config->config, "IP");
    valores_config->puerto_memoria = config_get_string_value(valores_config->config, "PUERTO_ESCUCHA");

    return valores_config;
}