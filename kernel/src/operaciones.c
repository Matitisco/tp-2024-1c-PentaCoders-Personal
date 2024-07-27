#include "../include/operaciones.h"

uint32_t PID_GLOBAL = 0;
uint32_t pid_a_finalizar;
int primera_ejecucion = 1;

void ejecutar_script(char *PATH)
{

    char linea[1024];

    char directorioActual[1024];
    char *ultimo_dir = basename(directorioActual);
    if (primera_ejecucion)
    {
        if (strcmp(ultimo_dir, "bin") == 0)
        {
            chdir("..");
        }
        chdir("..");
        primera_ejecucion = 0;
    }

    getcwd(directorioActual, sizeof(directorioActual));
    strcat(directorioActual, "/memoria/scripts_kernel/");
    strcat(directorioActual, PATH);

    FILE *archivo_script = fopen(directorioActual, "r");

    if (archivo_script == NULL)
    {
        log_info(logger, "No se pudo leer el script con PATH: <%s>", directorioActual);
        iniciar_consola_interactiva();
    }
    while (fgets(linea, sizeof(linea), archivo_script) != NULL)
    {
        strtok(linea, "\n");
        char **lineas_script = string_split(linea, " ");

        if (strcmp(lineas_script[0], "INICIAR_PROCESO") == 0)
        {
            iniciar_proceso(lineas_script[1]);
        }
        if (strcmp(lineas_script[0], "FINALIZAR_PROCESO") == 0)
        {
            finalizar_proceso(atoi(lineas_script[1]), INTERRUPTED_BY_USER);
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
        liberar_lineas_script(lineas_script);
    }
    
    fclose(archivo_script);
}
void liberar_lineas_script(char **lineas_script)
{
    if (lineas_script == NULL)
    {
        return;
    }

    for (int i = 0; lineas_script[i] != NULL; i++)
    {
        free(lineas_script[i]);
    }
    free(lineas_script);
}
void iniciar_proceso(char *PATH)
{
    t_pcb *proceso = crear_proceso(PATH);

    tipo_buffer *buffer = crear_buffer();

    enviar_op_code(socket_memoria, SOLICITUD_INICIAR_PROCESO);
    agregar_buffer_para_enterosUint32(buffer, proceso->cde->pid);
    agregar_buffer_para_string(buffer, proceso->cde->path);

    enviar_buffer(buffer, socket_memoria);
    destruir_buffer(buffer);

    op_code respuestaDeMemoria = recibir_op_code(socket_memoria);
    if(respuestaDeMemoria == -1){
        log_error(logger, "Error en la conexión con memoria, finalizando kernel.");
        free(proceso->cde->path);
        free(proceso->cde);
        free(proceso);
        return;
    }
    if (respuestaDeMemoria == INICIAR_PROCESO_CORRECTO)
    {
        agregar_a_estado(proceso, cola_new_global);

        log_info(logger, "Se crea el proceso <%u> en NEW", proceso->cde->pid); // log obligatorio
    }
    else if (respuestaDeMemoria == ERROR_INICIAR_PROCESO)
    {
        PID_GLOBAL--;
        free(proceso->cde->path);
        free(proceso->cde);
        free(proceso);
    }
}

void finalizar_proceso(uint32_t pid, motivoFinalizar motivo)
{
    pid_a_finalizar = pid;
    t_pcb *proceso = list_find(cola_exec_global->estado, buscar_por_pid); // busca en EXEC

    if (proceso == NULL) // NEW, READY, READY+ y BLOCKED
    {
        proceso = buscar_pcb_en_colas(pid); // se fija en el resto
        if (proceso == NULL)
        {
            log_warning(logger, "PID: <%d> - No existe o ya fue eliminado", pid);
            return;
        }
    }
    else // EXEC
    {
        enviar_op_code(socket_cpu_interrupt, SOLICITUD_EXIT); // Si esta en CPU le aviso que lo finalice
        sem_wait(sem_finalizar_proceso);                      // recibe el OK de la CPU
        finalizar_proceso_final(proceso, pid, motivo);        // finaliza el proceso en memoria
        return;
    }
    finalizar_proceso_final(proceso, pid, motivo);
    return;
}

void finalizar_proceso_final(t_pcb *proceso, int pid, motivoFinalizar motivo)
{
    enviar_op_code(socket_memoria, SOLICITUD_FINALIZAR_PROCESO);
    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer, pid);
    enviar_buffer(buffer, socket_memoria);
    destruir_buffer(buffer);
    op_code codigo = recibir_op_code(socket_memoria);
    if(codigo == -1){
        log_error(logger, "Error en la conexión con memoria, finalizando kernel.");
        return;
    }
    if (codigo == FINALIZAR_PROCESO)
    {
        transicion_generica_exit(proceso->cde->pid);
        eliminar_proceso(proceso);

        log_info(logger, "Finaliza el proceso %d - Motivo: <%s>", proceso->cde->pid, mostrar_motivo(motivo));
    }
    else
    {
        log_error(logger, "PID <%d> - Error al finalizar el proceso", pid);
    }
}

void finalizar_proceso_success(uint32_t pid, motivoFinalizar motivo)
{
    enviar_op_code(socket_memoria, SOLICITUD_FINALIZAR_PROCESO);
    pid_a_finalizar = pid;
    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer, pid);
    enviar_buffer(buffer, socket_memoria);
    destruir_buffer(buffer);
    op_code codigo = recibir_op_code(socket_memoria);
    if(codigo == -1){
        log_error(logger, "Error en la conexión con memoria, finalizando kernel.");
        return;
    }
    if (codigo == FINALIZAR_PROCESO)
    {
        t_pcb *proceso = list_find(cola_exec_global->estado, buscar_por_pid);
        eliminar_proceso(proceso);

        log_info(logger, "Finaliza el proceso %d - Motivo: <%s>", pid, mostrar_motivo(motivo));
    }
    else
    {
        log_error(logger, "PID <%d> - Error Finalizar", pid);
    }
}

void eliminar_proceso(t_pcb *proceso)
{
    liberar_recursos(proceso);
    free(proceso->cde->registros); // se liberan los registros
    proceso->estado = EXIT;
}

void detener_planificacion()
{
    if (habilitar_planificadores == 1)
    {
        habilitar_planificadores = 0;
        sem_wait(b_reanudar_largo_plazo);
        sem_wait(b_reanudar_corto_plazo);
        log_info(logger, "PLANIFICACION PAUSADA");
    }
    else
    {
        log_info(logger, "PLANIFICACION YA ESTA PAUSADA");
    }
}

void iniciar_planificacion()
{
    if (habilitar_planificadores == 0)
    {
        habilitar_planificadores = 1;
        sem_post(b_reanudar_largo_plazo);
        sem_post(b_reanudar_corto_plazo);
        log_info(logger, "PLANIFICACION EN FUNCIONAMIENTO");
    }
    else
    {
        log_info(logger, "PLANIFICACION YA ESTA EN FUNCIONAMIENTO");
    }
}

void grado_multiprogramacion(int valor)
{
    modificar_grado_multiprogramacion(valor);
}

void proceso_estado()
{
    mostrar_procesos(cola_new_global);
    mostrar_procesos(cola_ready_global);
    mostrar_procesos(cola_ready_plus);
    mostrar_procesos(cola_exec_global);
    mostrar_procesos(cola_bloqueado_global);
    mostrar_procesos(cola_exit_global);
    // imprimir_recursos();
}

// FUNCIONES AUXILIARES OPERACIONES

bool buscar_por_pid(t_pcb *proceso)
{
    return proceso->cde->pid == pid_a_finalizar;
}

t_pcb *buscar_pcb_en_colas(int pid)
{
    pid_a_finalizar = pid;
    colaEstado *colas[5] = {cola_new_global, cola_ready_global, cola_ready_plus, cola_exec_global, cola_bloqueado_global};
    for (int i = 0; i < 5; i++)
    {
        t_pcb *pcb = list_find(colas[i]->estado, buscar_por_pid);
        if (pcb != NULL)
            return pcb;
    }
    return NULL;
}

char *mostrar_motivo(motivoFinalizar motivo)
{
    if (motivo == SUCCESS) // FUNCION EXIT DE CPU
    {
        return "SUCCESS";
    }
    else if (motivo == INVALID_RESOURCE) // SE PIDIO UN RECURSO INVALIDO
    {
        return "INVALID_RESOURCE";
    }
    else if (motivo == INVALID_INTERFACE) // NO SE PUDO CONECTAR A UNA INTERFAZ
    {
        return "INVALID_INTERFACE";
    }
    else if (motivo == OUT_OF_MEMORY_END) // OUT OF MEMORY DE MEMORIA
    {
        return "OUT_OF_MEMORY";
    }
    else if (motivo == INTERRUPTED_BY_USER) // DESDE CONSOLA
    {
        return "INTERRUPTED_BY_USER";
    }
    return NULL;
}

void liberar_recursos(t_pcb *proceso)
{
    int cantidad_recursos_asignados = list_size(proceso->recursosAsignados);
    for (int i = 0; i < cantidad_recursos_asignados; i++)
    {
        t_recurso *recurso_a_liberar_del_proceso = list_get(proceso->recursosAsignados, i);

        int instancias_recurso;
        sem_getvalue(recurso_a_liberar_del_proceso->instancias, &instancias_recurso);

        log_info(logger, "%d INSTANCIAS DEL RECURSO %s QUE TIENE EL PROCESO <%d>", instancias_recurso, recurso_a_liberar_del_proceso->nombre, proceso->cde->pid);
        for (int i = 0; i < instancias_recurso; i++)
        {
            sem_wait(recurso_a_liberar_del_proceso->instancias);
            int recursos_SO = list_size(valores_config->recursos);
            for (int i = 0; i < (recursos_SO); i++)
            {
                t_recurso *recurso_SO = list_get(valores_config->recursos, i);
                if (strcmp(recurso_SO->nombre, recurso_a_liberar_del_proceso->nombre) == 0)
                {
                    sem_post(recurso_SO->instancias);
                    if (!list_is_empty(recurso_SO->cola_bloqueados->estado))
                    {
                        for (int i = 0; i < list_size(recurso_SO->cola_bloqueados->estado); i++)
                        {
                            t_pcb *pcb = list_get(recurso_SO->cola_bloqueados->estado, i);
                            if (pcb->cde->pid == proceso->cde->pid)
                            {
                                sem_wait(recurso_SO->cola_bloqueados->contador);
                                pthread_mutex_lock(recurso_SO->cola_bloqueados->mutex_estado);
                                t_pcb *pcb = list_remove(recurso_SO->cola_bloqueados->estado, i);
                                pthread_mutex_unlock(recurso_SO->cola_bloqueados->mutex_estado);
                            }
                        }
                        t_pcb *pcb_ = list_get(recurso_SO->cola_bloqueados->estado, 0);
                        asignar_recurso(recurso_SO, pcb_);
                    }
                }
            }
        }
        free(recurso_a_liberar_del_proceso->nombre);
        sem_destroy(recurso_a_liberar_del_proceso->instancias);
    }
    list_destroy(proceso->recursosAsignados);
}

int cant_recursos_SO(t_recurso **recursos)
{
    int i = 0;
    while (recursos[i] != NULL)
    {
        i++;
    }
    return i;
}

void modificar_grado_multiprogramacion(int valor)
{
    for (int i = 0; i < valor; i++)
    {
        sem_post(GRADO_MULTIPROGRAMACION);
    }
    log_info(logger, "Se modifico el grado de multiprogramacion a %d", valor);
}

void mostrar_procesos(colaEstado *cola)
{
    log_info(logger, "PROCESOS: <%d> - COLA: <%s>", list_size(cola->estado), cola->nombreEstado);
    list_iterate(cola->estado, imprimir_registro);
}

void imprimir_registro(void *element)
{
    t_pcb *pcb = (t_pcb *)element;
    log_info(logger, "PID: <%d>", pcb->cde->pid);
}

t_pcb *crear_proceso(char *PATH)
{
    t_pcb *proceso_nuevo = malloc(sizeof(t_pcb));
    proceso_nuevo->estado = NEW;
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
    cde->PC = 0;
    cde->lista_instrucciones = list_create();
    return cde;
}