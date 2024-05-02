#include "../include/operaciones.h"

uint32_t PID_GLOBAL = 0;
//int socket_memoria;

// EJECUTAR SCRIPT
void ejecutar_script()
{
    printf("Ejecutar Script\n");
}
// INICIAR PROCESO
void iniciar_proceso(char *PATH)
{

    printf("\nsocket memoria: %d\n", socket_memoria);
    t_pcb *proceso = crear_proceso(PATH);                       // Creo nuestro nuevo proceso
    enviar_cod_enum(socket_memoria, SOLICITUD_INICIAR_PROCESO); // Le pido si puedo iniciar el proceso
    // ENVIAMOS BUFFER A MEMORIA 
    //tipo_buffer *buffer = crear_buffer();
    //escribir_buffer_entero(buffer, proceso->cde->pid);
    //escribir_buffer_string(buffer, proceso->cde->path);

    enviar_buffer(buffer, socket_memoria);      //Enviado a memoria
    destruir_buffer(buffer);
    // OBTENEMOS OP_CODE DESDE MEMORIA
    op_code codigo = recibir_cod(socket_memoria);
    if (codigo == INICIAR_PROCESO_CORRECTO)
    {
        agregar_a_estado(proceso, cola_new_global);
        // Aca ella agrega semaforos pero creo que nosotros al tener la cola sincronizada nos lo evitamos
        log_info(logger, "Se creo un proceso con PID: %u en NEW\n", mostrarPID(proceso)); // se muestra el logger
    }
    else if (codigo == INICIAR_PROCESO_ERROR)
    {
        log_info(logger, "No se pudo crear el proceso %u", proceso->cde->pid); // se muestra que no se pudo
    }
}
t_pcb *crear_proceso(char *PATH)
{
    t_pcb *proceso_nuevo = malloc(sizeof(t_pcb)); // reservamos memoria para el proceso           // por ahora en 0;
    proceso_nuevo->estado = NEW;

    proceso_nuevo->archivosAsignados = list_create();
    proceso_nuevo->recursosAsignados = list_create();
    proceso_nuevo->cde = iniciar_cde();
    return proceso_nuevo;
}

t_cde *iniciar_cde()
{
    t_cde *cde = malloc(sizeof(t_cde));

    cde->pid = PID_GLOBAL;
    PID_GLOBAL++;
    cde->pc = 0; // LA CPU lo va a ir cambiando
    cde->path=malloc(strlen(PATH) + 1); //reservar memoria para el path
    strcpy(cde->path,PATH); //y asignarle con la funcion
    cde->registro = malloc(sizeof(t_registros));
    cde->registro = NULL;
    cde->lista_instrucciones = malloc(sizeof(t_list));
    cde->lista_instrucciones = list_create();
    return cde;
}
// DETENER PROCESO
void finalizar_proceso(uint32_t PID)
{

    t_pcb *proceso = buscarProceso(PID); // buscamos en las colas
    log_info(logger, "Se finalizo el proceso %u \n", PID);

    op_code otroCodigo = recibir_cod(socket_memoria);
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

    printf("Iniciar Planificacion");
}
// DETENER PLANIFICACION
void detener_planificacion()
{
    printf("Detener Planificacion");
}
// LISTAR PROCESOS POR ESTADO
void listar_procesos_x_estado()
{
    printf("Listar Procesos Por Estado");
}

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
    // PREGUNTAR QUE HACE ESTA FUNCION RAWRA
}

/* void enviar_cde(int conexion, t_cde *cde, int codOP) //----IMPLEMENTAR----
{
    t_paquete *paquete = crear_paquete_op_code(codOP);

    agregar_cde_a_paquete(paquete, cde);

    enviar_paquete(paquete, conexion);

    eliminar_paquete(paquete);
}
 */
/* void enviar_cde(t_cde *cde)
{
    enviar_codigo(socket_cpu_dispatch, EJECUTAR_PROCESO); // Le pido si pueod iniciar el proceso
    tipo_buffer *buffer = crear_buffer();
    escribir_buffer(buffer, cde->pid);
    escribir_buffer(buffer, cde->pc);
    enviar_buffer(buffer, socket_cpu_dispatch);
    destruir_buffer(buffer);
}*/

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

/*


void iniciar_proceso(char* linea) {
    //char argumento = (char) args;


    char* PATH = NULL;
    char* SIZE = NULL;
    char* PRIORIDAD = NULL;

    int i = 0;

    // Verificar si hay argumentos disponibles
    if (linea[i]) {
        PATH = linea + i;

        // Avanzar hasta el final del primer argumento
        while (linea[i] && !isspace(linea[i]))
            i++;

        if (linea[i]) //GENERA SEG FAULT
            linea[i++] = '\0'; // Terminar el primer argumento si hay más en la línea

        // Ahora, verificar si hay un segundo argumento
        while (linea[i] && isspace(linea[i]))
            i++;

        if (linea[i]) {
            SIZE = linea + i;

            // Avanzar hasta el final del segundo argumento
            while (linea[i] && !isspace(linea[i]))
                i++;

            //if (linea[i]) GENERA SEG FAULT
             //   linea[i] = '\0';  Terminar el segundo argumento si hay más en la línea



            if (linea[i]){
                PRIORIDAD = linea + i;
                 // Avanzar hasta el final del tercer argumento
                while (linea[i] && !isspace(linea[i]))
                    i++;

                //if (linea[i]) GENERA SEG FAULT
                   // linea[i] = '\0'; // Terminar el tercer argumento si hay más en la línea
            }

        }



    }



    printf("Iniciando proceso... \n"); // %s %d %d \n", PATH, SIZE, PRIORIDAD);



    Proceso* pcb = iniciarPcb(PATH, SIZE, PRIORIDAD);
    ingresarNew(pcb);
    //printf("Argumento: %s\n", argumento);

    //printf("Se agrega el proceso: %i \n",pcb->pid);
}


Proceso* iniciarPcb(char* path, char* size, char* prioridad){ //t_proceso* proceso estaria bueno para definir la data que venga de lo que se lea en consola


    Proceso* pcb = malloc(sizeof(Proceso));

    int sizeValor = atoi(size);
    //printf("%d", sizeValor);
    int prioridadValor = atoi(prioridad);
    //printf("%d", prioridadValor);

    pcb->instrucciones = list_create();

    leerInstrucciones(path, pcb->instrucciones);

    nro_procesos++;
    pcb->pid=nro_procesos;
    pcb->prioridad = prioridadValor;
    pcb->program_counter=0;
    pcb->tamanio=sizeValor;

    pcb->estado_actual= ESTADO_NEW;

    //pcb->registros = NULL;
    //pcb->archivos_abiertos = NULL;

    printf("Se creo el proceso %i con %i \n", pcb->pid, pcb->program_counter);

    return pcb;
}


void leerInstrucciones(char* path, t_list* listaInstrucciones) {
    printf("\n%s\n",path);

    FILE* archivo;
    archivo = fopen(path, "r");

    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    char buffer[100];

    while (fgets(buffer, sizeof(buffer), archivo) != NULL) {
        char* tipo_str = strtok(buffer, " ");
        t_tipo_instruccion tipo = -1;


        if (strcmp(tipo_str, "SET") == 0) {
            tipo = INSTRUCCION_SET;
        } else if (strcmp(tipo_str, "SUM") == 0) {
            tipo = INSTRUCCION_SUM;
        } else if (strcmp(tipo_str, "SUB") == 0) {
            tipo = INSTRUCCION_SUB;
        } else if (strcmp(tipo_str, "EXIT") == 0) {
            tipo = INSTRUCCION_EXIT;
        }  else {
            perror("Tipo de instrucción desconocida");
            exit(EXIT_FAILURE);
        }


        char* arg1 = strtok(NULL, " ");
        char* arg2 = strtok(NULL, " \n");


        t_instruccion* nuevaInstruccion = agregarInstruccionALista(listaInstrucciones, tipo, arg1, arg2);
        printf("Instruccion: %s %s %s\n", tipo_str, arg1, arg2);

        list_add(listaInstrucciones, nuevaInstruccion);
    }

    fclose(archivo);
}






*/