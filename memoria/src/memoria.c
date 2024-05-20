#include "../include/memoria.h"
// CRAER HILOS COMO VARIABLES GLOBALES
pthread_t hiloCpu;
pthread_t hiloKernel;
pthread_t hiloIO;
struct config_memoria *valores_config;
int server_fd;
int main(int argc, char *argv[])
{

    logger = iniciar_logger("memoria.log", "MEMORIA");
    valores_config = config_memoria();

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
    return EXIT_SUCCESS;
}

void *recibirKernel()
{
    tipo_buffer *buffer = malloc(sizeof(tipo_buffer));
    int cliente_fd = esperar_cliente(logger, "Memoria", "Kernel", server_fd);
    while (1)
    {
        // puede ser que haya un porblema de sincro, talvez agregar un sem?
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
            log_error(logger, "El KERNEL se desconecto. Terminando servidor");
            return (void *)EXIT_FAILURE;
            break;

        default:
            log_warning(logger, "Operacion desconocida. No quieras meter la pata");
            return (void *)EXIT_FAILURE;
            ;
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
    cde->lista_instrucciones = leerArchivoConInstrucciones(cde->path); // implementar
    if (cde->path != NULL)
    {
        enviar_cod_enum(cliente_fd, INICIAR_PROCESO_CORRECTO);
        log_info(logger, "Se inicio el proceso de PID: %d y PATH: %s", cde->pid, cde->path);
    }
    else
    {
        enviar_cod_enum(cliente_fd, ERROR_INICIAR_PROCESO);
    }
}
t_cde *armarCde(tipo_buffer *buffer)
{
    t_cde *cde = malloc(sizeof(t_cde));
    cde->pid = leer_buffer_enteroUint32(buffer);
    char *string = leer_buffer_string(buffer);
    cde->path = string;
    return cde;
}
t_list *leerArchivoConInstrucciones(char *nombre_archivo)
{
    t_list *list_instrucciones = list_create(); // creo el puntero a la lista
    char *ruta_completa = string_new();
    char *ruta_acceso = "/home/utnso/tp-2024-1c-PentaCoders/memoria/pruebas/";
    string_append(&ruta_completa, ruta_acceso);
    string_append(&ruta_completa, nombre_archivo);
    log_info(logger, "El path del archivo es : %s", ruta_completa);
    FILE *archivo = fopen(ruta_completa, "r");

    if (archivo == NULL)
    {
        log_warning(logger, "No se pudo abrir el archivo: %s", ruta_acceso);
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
            /*             tipo_buffer *buffer = recibir_buffer(socket_cpu);

                        int pid = leer_buffer(buffer);
                        int pc = leer_buffer(buffer);
                        destruir_buffer(buffer);
                        t_pcb*proceso= buscarPCBEnColaPorPid(pid);

                        t_list *listaInstrucciones = list_get(proceso->cde->instrucciones);
                        buffer = crear_buffer();
                        escribir_buffer(buffer, instruccion);
                        enviar_buffer(buffer, socket_cpu);
                        destruir_buffer(); */
            printf("Se aprueba pedido instruccion");
        // case ACCESO_ESPACIO_USUARIO:
        case -1:
            log_error(logger, "El cliente se desconecto. Terminando servidor");
            return (void *)EXIT_FAILURE;
            // return EXIT_FAILURE; // version de la catedra, pero da un warning si no anda comentar el de arriba
        default:
            log_warning(logger, "Operacion desconocida. No quieras meter la pata");
            break;
        }
        return EXIT_SUCCESS;
    }
}
void finalizar_proceso(int cliente_fd, tipo_buffer *buffer)
{
    buffer = recibir_buffer(cliente_fd);
    uint32_t pid_a_eliminar = leer_buffer_enteroUint32(buffer);
    eliminar_proceso(pid_a_eliminar);
}
void eliminar_proceso(int pid_a_eliminar)
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