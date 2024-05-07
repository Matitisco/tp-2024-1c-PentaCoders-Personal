#include "../include/operaciones.h"

uint32_t PID_GLOBAL = 0;
op_code estado_planificacion;
// int socket_memoria;

// EJECUTAR SCRIPT
void ejecutar_script(char *PATH)
{
}
// INICIAR PROCESO
void iniciar_proceso(char *PATH) // CONSULTAR FUNCION
{
    t_pcb *proceso = crear_proceso(PATH);

    tipo_buffer *buffer = crear_buffer();

    op_code codigo = SOLICITUD_INICIAR_PROCESO; // SOLICITUD_INICIAR_PROCESO;

    send(socket_memoria, &codigo, sizeof(uint32_t), 0); // enviar codigo

    agregar_buffer_para_enterosUint32(buffer, proceso->cde->pid);
    agregar_buffer_para_string(buffer, proceso->cde->path);

    enviar_buffer(buffer, socket_memoria);
    destruir_buffer(buffer);

    // OBTENEMOS OP_CODE DESDE MEMORIA
    op_code respuestaDeMemoria = recibir_operacion(socket_memoria);
    if (respuestaDeMemoria == INICIAR_PROCESO_CORRECTO)
    {
        agregar_a_estado(proceso, cola_new_global);
        log_info(logger, "Se creo un proceso con PID: %u en NEW\n", mostrarPID(proceso)); // se muestra el logger
    }
    else if (respuestaDeMemoria == ERROR_INICIAR_PROCESO)
    {
        log_info(logger, "No se pudo crear el proceso %u", proceso->cde->pid); // se muestra que no se pudo
    }
}
// DETENER PROCESO
void finalizar_proceso(uint32_t PID)
{

    t_pcb *proceso = buscarProceso(PID); // buscamos en las colas
    log_info(logger, "Se finalizo el proceso %u \n", PID);

    op_code otroCodigo = recibir_operacion(socket_memoria);
    if (otroCodigo == FINALIZAR_PROCESO)
    {
        log_info(logger, "PID %u -Destruir pcb", proceso->cde->pid);

        agregar_a_estado(proceso, cola_exit_global); // moverlo a la cola de exit
        liberar_proceso(proceso);
    }
    else if (otroCodigo == ERROR_FINALIZAR_PROCESO)
    {
        // FALTA VER COMO MOSTRAMOS EL MOTIVO POR EL QUE HA FINALIZADO EL PROCESO
        // log_info(logger, "Finalizar el proceso %u - Motivo: %s\n", PID, mostrarMotivo(motivoFinalizar));
    }
}
// INICIAR PLANIFICACION
void iniciar_planificacion()
{
    // tenemos un proceso en new y lo tenemos que pasar a ready
    // habilita a los hilos de los planificadores a que dejen de estar en pausa
    if (estado_planificacion == PLANIFICACION_PAUSADA)
    {
        renaudar_corto_plazo();
        renaudar_largo_plazo();
        estado_planificacion = PLANIFICACION_EN_FUNCIONAMIENTO;
    }
    else if (estado_planificacion == PLANIFICACION_EN_FUNCIONAMIENTO)
    {
        log_info(logger, "La planificacion ya se encuentra iniciada");
    }
    printf("Iniciar Planificacion");
}
// DETENER PLANIFICACION
void detener_planificacion()
{
    pausar_corto_plazo();
    pausar_largo_plazo();
    estado_planificacion = PLANIFICACION_PAUSADA;
}
// MODIFICAR GRADO DE MULTIPROGRAMACION
void grado_multiprogramacion(int valor)
{
    modificar_grado_multiprogramacion(valor);
}
// LISTAR PROCESOS POR ESTADO
void listar_procesos_x_estado()
{
    mostrar_procesos(cola_new_global);
    mostrar_procesos(cola_ready_global);
    mostrar_procesos(cola_exec_global);
    mostrar_procesos(cola_bloqueado_global);
    mostrar_procesos(cola_exit_global);
}
/*----------------------------------FUNCIONES AUXILIARES--------------------------------------*/
uint32_t mostrarPID(t_pcb *proceso)
{
    return proceso->cde->pid;
}

t_pcb *buscarProceso(uint32_t pid)
{
    t_pcb *pcb_buscada = NULL;
    colaEstado *colas[] = {cola_new_global, cola_ready_global, cola_exec_global, cola_exit_global}; // Hace vector de colas

    for (int i = 0; i < sizeof(colas) / sizeof(colas[0]); i++) // va fijandose si en cada posicion del vector esta ese pid
    {

        if ((pcb_buscada = buscarPCBEnColaPorPid(pid, colas[i]->estado, colas[i]->nombreEstado)) != NULL) // busqueda
        {
            // pcb_buscada = buscarPCBEnColaPorPid(pid, colas[i]->estado, colas[i]->nombreEstado);
            return pcb_buscada;
            break;
        }
    }
    if (pcb_buscada == NULL)
    {
        printf("No se pudo encontrar ningun PCB asociado al PID %u\n", pid);
    }

    return pcb_buscada;
}

// FUNCION MOSTRAR MOTIVO
/* char *mostrarMotivo(enum motivoFinalizar motivo)
{
    if (motivo == SUCCESS)
    {
        return "SUCCESS";
    }
    else if (motivo == INVALID_RESOURCE)
    {
        return "INVALID_RESOURCE";
    }
    else if (motivo == INVALID_WRITE)
    {
        return "INVALID_WRITE";
    }
} */

// FUNCION DE LIBERACIONES

void liberar_proceso(t_pcb *proceso)
{
    liberar_cde(proceso);
    liberar_recursos(proceso);
    liberar_archivos(proceso);
    // liberar_memoria(proceso); CONSULTAR QUE HACE
}
void liberar_cde(t_pcb *proceso)
{
    free(proceso->cde->lista_instrucciones);
    free(proceso->cde->registro);
    free(proceso->cde);
    // free(proceso->cde->pc);
    // free(proceso->cde->pid);
    // list_destroy(proceso->cde->instrucciones->parametros);
    // free(proceso->cde->registro);
    //  free(proceso->cde->instrucciones->codigo);
}

void liberar_recursos(t_pcb *proceso)
{
    list_destroy(proceso->recursosAsignados);
}
void liberar_archivos(t_pcb *proceso)
{
    free(proceso->archivosAsignados);
}
void liberar_memoria(t_pcb *proceso)
{
}

void renaudar_corto_plazo()
{
}
void renaudar_largo_plazo()
{
}
void modificar_grado_multiprogramacion(int valor)
{
}
void mostrar_procesos(colaEstado *cola)
{
}
void pausar_corto_plazo() {}
void pausar_largo_plazo() {}
t_pcb *buscarPCBEnColaPorPid(int pid_buscado, t_queue *cola, char *nombreCola)
{

    t_pcb *pcb_buscada;

    // Verificar si la lista está vacía
    if (queue_is_empty(cola))
    {
        printf("El estado %s está vacío.\n", nombreCola);
        return NULL;
    }

    t_queue *colaAux = queue_create();
    // Copiar los elementos a una cola auxiliar y mostrarlos

    while (!queue_is_empty(cola)) // vacia la cola y mientras buscar el elemento
    {
        t_pcb *pcb = queue_pop(cola);

        if (pcb->cde->pid == pid_buscado)
        {
            pcb_buscada = pcb;
        }

        queue_push(colaAux, pcb_buscada);
    }

    // Restaurar la cola original
    while (!queue_is_empty(colaAux))
    {
        queue_push(cola, queue_pop(colaAux));
    }

    // Liberar memoria de la cola auxiliar y sus elementos
    while (!queue_is_empty(colaAux))
    {
        free(queue_pop(colaAux));
    }
    queue_destroy(colaAux);

    if (pcb_buscada != NULL)
    {
        if (pcb_buscada->cde != NULL)
        {
            printf("Se encontró el proceso PID: %i en %s \n", pcb_buscada->cde->pid, nombreCola);
        }
        else
        {
            printf("El PCB encontrado no tiene un puntero válido a cde.\n");
        }
    }
    else
    {
        printf("No se encontró el proceso en %s.\n", nombreCola);
    }

    return pcb_buscada;
}

t_pcb *crear_proceso(char *PATH)
{
    t_pcb *proceso_nuevo = malloc(sizeof(t_pcb)); // reservamos memoria para el proces
    proceso_nuevo->estado = NEW;

    proceso_nuevo->archivosAsignados = list_create();
    proceso_nuevo->recursosAsignados = list_create();
    proceso_nuevo->cde = iniciar_cde(PATH);
    return proceso_nuevo;
}

t_cde *iniciar_cde(char *PATH)
{
    t_cde *cde = malloc(sizeof(t_cde));

    cde->pid = PID_GLOBAL;
    PID_GLOBAL++;
    cde->pc = 0;                          // LA CPU lo va a ir cambiando
    cde->path = malloc(strlen(PATH) + 1); // reservar memoria para el path
    strcpy(cde->path, PATH);              // y asignarle con la funcion
    cde->registro = malloc(sizeof(t_registros));
    cde->registro = NULL;
    cde->lista_instrucciones = list_create();
    return cde;
}