#include "../include/operaciones.h"


uint32_t PID_GLOBAL = 0;
op_code estado_planificacion = PLANIFICACION_PAUSADA;
// int socket_memoria;
// sem_t * sem_kernel;

void ejecutar_script(char *PATH)
{
    char *linea_script = string_new();
    char **lineas_script = string_array_new();
    char linea[1024];
    FILE *archivo_script = fopen(PATH, "r");

    if (archivo_script == NULL)
    {
        log_info(logger, "No se pudo leer el script con PATH: %s", PATH);
        iniciar_consola_interactiva();
    }
    while (fgets(linea, sizeof(linea), archivo_script) != NULL)
    {
        log_info(logger, "Linea leida: %s", linea);
        strtok(linea, "\n");
        log_info(logger, "Linea ahora: %s", linea);
        lineas_script = string_split(linea, " ");
        log_info(logger, "Linea leida: %s", linea);

        if (strcmp(lineas_script[0], "INICIAR_PROCESO") == 0)
        {
            iniciar_proceso(lineas_script[1]);
        }
        if (strcmp(lineas_script[0], "FINALIZAR_PROCESO") == 0)
        {
            finalizar_proceso(atoi(lineas_script[1]));
        }
        if (strcmp(lineas_script[0], "DETENER_PLANIFICACION") == 0)
        {
            detener_planificacion();
        }
        if (strcmp(lineas_script[0], "INICIAR_PLANIFICACION") == 0)
        {
            iniciar_planificacion();
        }
        if (strcmp(lineas_script[0], "MULTIPROGRAMACION") == 0)
        {
            grado_multiprogramacion(atoi(lineas_script[1]));
        }
        if (strcmp(lineas_script[0], "PROCESO_ESTADO") == 0)
        {
            proceso_estado();
        }
    }
    free(lineas_script);
    free(linea_script);
    fclose(archivo_script);
}

void iniciar_proceso(char *PATH)
{
    t_pcb *proceso = crear_proceso(PATH);

    tipo_buffer *buffer = crear_buffer();

    op_code codigo = SOLICITUD_INICIAR_PROCESO;
    enviar_cod_enum(socket_memoria, codigo);

    agregar_buffer_para_enterosUint32(buffer, proceso->cde->pid);
    agregar_buffer_para_string(buffer, proceso->cde->path);

    enviar_buffer(buffer, socket_memoria);
    destruir_buffer(buffer);

    op_code respuestaDeMemoria = recibir_operacion(socket_memoria);
    if (respuestaDeMemoria == INICIAR_PROCESO_CORRECTO)
    {
        agregar_a_estado(proceso, cola_new_global);
        log_info(logger, "Se crea el proceso %u en NEW\n", proceso->cde->pid);
    }
    else if (respuestaDeMemoria == ERROR_INICIAR_PROCESO)
    {
        log_info(logger, "No se pudo crear el proceso %u", proceso->cde->pid);
        PID_GLOBAL--;
    }
}

// Eliminación de Procesos
// puede ser por pedido de cpu, un error, o consola
// memoria debe liberar todas las estructuras asociadas
// buscamos al proceso y nos fijamos que no este en cpu
// si esta en cpu entonces mandamos a cpu_interrupt una interrupcion
// pidiendo que desaloje el proceso de la cpu y retorne el cde
// al eliminar se habilita +1 grado multiprogramacion

void finalizar_proceso(uint32_t PID) 
{
    //buscamos el proceso en la cola de exec
    t_pcb *proceso = buscarPCBEnColaPorPid(PID, cola_exec_global->estado, cola_exec_global->nombreEstado);

    if (proceso == NULL){// puede estar en new, ready, blocked
        enviar_cod_enum(socket_memoria, SOLICITUD_FINALIZAR_PROCESO); // enviamos solicitud a la memoria
      }  // para que finalice el proceso
    else{
        enviar_cod_enum(socket_cpu_interrupt, SOLICITUD_EXIT); // pero si el proceso esta ejecutandose en la cpu
        tipo_buffer* buffer_cpu_interrupt= crear_buffer();
        agregar_buffer_para_enterosUint32(buffer_cpu_interrupt, PID);
        enviar_buffer(buffer_cpu_interrupt,socket_cpu_interrupt);
        destruir_buffer(buffer_cpu_interrupt);
    }
 //si esta en cpu entonces mandamos a cpu_interrupt una interrupcion
// pidiendo que desaloje el proceso de la cpu y retorne el cde

    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer, PID);
    enviar_buffer(buffer, socket_memoria);//    le solicito a MEMORIA que ELIMINE el proceso
    destruir_buffer(buffer);
    
}

// INICIAR PLANIFICACION
void iniciar_planificacion()
{
    habilitar_planificadores = 1;
    sem_post(b_reanudar_largo_plazo);
    sem_post(b_reanudar_corto_plazo);
    estado_planificacion = PLANIFICACION_EN_FUNCIONAMIENTO;
} 
// DETENER PLANIFICACION
void detener_planificacion()
{
    habilitar_planificadores = 0;
    //sem_wait(b_reanudar_largo_plazo);
    //sem_wait(b_reanudar_corto_plazo);
    estado_planificacion = PLANIFICACION_PAUSADA;
}

// MODIFICAR GRADO DE MULTIPROGRAMACION
void grado_multiprogramacion(int valor)
{
    modificar_grado_multiprogramacion(valor);
}
// LISTAR PROCESOS POR ESTADO
void proceso_estado()
{
    mostrar_procesos(cola_new_global);
    mostrar_procesos(cola_ready_global);
    mostrar_procesos(cola_exec_global);
    mostrar_procesos(cola_bloqueado_global);
    mostrar_procesos(cola_exit_global);
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

/* void liberar_proceso(t_pcb *proceso)
{
    liberar_recursos(proceso);
    liberar_archivos(proceso);
    proceso->estado = EXIT;
} */

/* void liberar_recursos(t_pcb *proceso)
{
    list_destroy_and_destroy_elements(proceso->recursosAsignados, destroy_recursos);
}

void* destroy_recursos(void* element) {
    void* recurso = (void*)element;
    free(recurso);
}
 */

void liberar_archivos(t_pcb *proceso)
{
    list_destroy_and_destroy_elements(proceso->archivosAsignados, destroy_archivos);
}

void* destroy_archivos(void* element) {
    char* archivo = (char*)element;
    free(archivo);
}


void modificar_grado_multiprogramacion(int valor)
{
    for (int i = 0; i < valor; i++)
    {
        sem_post(&GRADO_MULTIPROGRAMACION);
    }
    log_info(logger, "Se modifico el grado de multiprogramacion a %d", valor);
}

void mostrar_procesos(colaEstado *cola)
{
    log_info(logger, "Hay en total %d procesos en la cola %s", queue_size(cola->estado), cola->nombreEstado);

    t_queue *cola_aux = queue_create();

    while (!queue_is_empty(cola->estado))
    {
        t_pcb *pcb = queue_pop(cola->estado);
        log_info(logger, "Proceso PID: %d", pcb->cde->pid);

        queue_push(cola_aux, pcb);
    }

    // Restaurar la cola original
    while (!queue_is_empty(cola_aux))
    {
        queue_push(cola->estado, queue_pop(cola_aux));
    }
}

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
    cde->path = malloc(strlen(PATH) + 1);
    strcpy(cde->path, PATH);
    cde->registros = malloc(sizeof(t_registros));
    cde->PC = 0; // LA CPU lo va a ir cambiando
    cde->lista_instrucciones = list_create();
    return cde;
}