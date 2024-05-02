#include "../include/kernel.h"

colaEstado *cola_new_global;
colaEstado *cola_ready_global;
colaEstado *cola_exec_global;
colaEstado *cola_bloqueado_global;
colaEstado *cola_exit_global;
int socket_memoria;
int socket_cpu_dispatch;
int socket_cpu_interrupt;
int QUANTUM;
//t_log *logger;
//uint32_t PID_GLOBAL;

int main(int argc, char *argv[])
{
	int conexion_memoria_desde_kernel, conexion_cpu;

	// INICIALIZACION DE ESTADOS
	inicializarEstados();

	logger = iniciar_logger("kernel.log", "KERNEL");

	// CONFIG
	config_kernel *valores_config = inicializar_config_kernel();

	// KERNEL COMO SERVER DE I0
	levantarServidor(logger, valores_config->puerto_escucha, valores_config->ip_memoria, "SERVIDOR KERNEL");
	// KERNEL COMO CLIENTE
	conexion_memoria_desde_kernel = levantarCliente(logger, "MEMORIA", valores_config->ip_memoria, valores_config->puerto_memoria, "KERNEL SE CONECTO A MEMORIA");
	conexion_cpu = levantarCliente(logger, "CPU", valores_config->ip_cpu, valores_config->puerto_cpu_dispatch, "KERNEL SE CONECTO A CPU");

	// iniciar_consola_interactiva(logger);
	terminar_programa(conexion_cpu, logger, valores_config->config);
	liberarConexion(conexion_memoria_desde_kernel);
}

void gestionar_peticiones_memoria()
{
	printf("Gestionar Peticion Memoria");
}

void gestionar_peticiones_interfaces()
{
	printf("Gestionar Peticion Interfaces");
}

void planificar_ejecucion_procesos()
{
	printf("Planificar Ejecucion procesos");
}

colaEstado *constructorColaEstado(char *nombre)
{
	colaEstado *cola_estado_generica;
	cola_estado_generica = malloc(sizeof(colaEstado));
	cola_estado_generica->nombreEstado = nombre;
	// strcpy(cola_estado_generica->nombreEstado, nombre);
	cola_estado_generica->estado = queue_create();
	cola_estado_generica->mutex_estado = malloc(sizeof(pthread_mutex_t));

	pthread_mutex_init(cola_estado_generica->mutex_estado, NULL);

	return cola_estado_generica;
}

// INICIALIZAR COLAS DE ESTADOS
void inicializarEstados()
{
	cola_new_global = constructorColaEstado("NEW");
	cola_ready_global = constructorColaEstado("READY");
	cola_exec_global = constructorColaEstado("EXEC");
	cola_bloqueado_global = constructorColaEstado("BLOCK");
	cola_exit_global = constructorColaEstado("EXIT");
}

config_kernel *inicializar_config_kernel()
{
	config_kernel *configuracion = (config_kernel *)malloc(sizeof(config_kernel));

	configuracion->config = iniciar_config("kernel.config");

	configuracion->ip_memoria = config_get_string_value(configuracion->config, "IP_MEMORIA");
	configuracion->ip_cpu = config_get_string_value(configuracion->config, "IP_CPU");
	configuracion->puerto_memoria = config_get_string_value(configuracion->config, "PUERTO_MEMORIA");
	configuracion->puerto_escucha = config_get_string_value(configuracion->config, "PUERTO_ESCUCHA");
	configuracion->puerto_cpu_dispatch = config_get_string_value(configuracion->config, "PUERTO_CPU_DISPATCH");
	configuracion->puerto_cpu_interrupt = config_get_string_value(configuracion->config, "PUERTO_CPU_INTERRUPT");

	return configuracion;
}

// /*------------------------------------MENU-------------------------------------*/

// void iniciar_consola_interactiva(t_log *logger)
// {
// 	char *opcion;
// 	while (1)
// 	{
// 		mostrar_operaciones_realizables();
// 		opcion = readline("Ingrese un valor para realizar una de las siguientes acciones: ");
// 		ejecutar_operacion(opcion, logger);
// 		free(opcion);
// 	}
// }

// void mostrar_operaciones_realizables()
// {
// 	printf("1-Ejecutar Script de Operaciones\n");
// 	printf("2-Iniciar Proceso\n");
// 	printf("3-Finalizar Proceso\n");
// 	printf("4-Iniciar Planificacion\n");
// 	printf("5-Detener Planificacion\n");
// 	printf("6-Listar Procesos por Estado\n");
// }

// void ejecutar_operacion(char *opcion, t_log *logger)
// {
// 	op_code cod_op = obtenerCodigo(opcion, logger);
// 	// EL PATH DEBE ARREGLARSE PARA QUE SE PUEDA MANDAR CUALQUIERA
// 	char *PATH = malloc(sizeof(FILE));
// 	switch (cod_op)
// 	{
// 	case EJECUTAR_SCRIPT:
// 		ejecutar_script();
// 		exit(1);
// 		break;
// 	case INICIAR_PROCESO:
// 		iniciar_proceso(PATH);
// 		// exit(1); // momentaneo es solo para que no quede en loop
// 		break;
// 	case FINALIZAR_PROCESO:
// 		char *pid = readline("Ingrese el PID del Proceso a Finalizar: ");
// 		finalizar_proceso(atoi(pid));
// 		// exit(1);
// 		break;
// 	case INICIAR_PLANIFICACION:
// 		iniciar_planificacion();
// 		exit(1);
// 		break;
// 	case DETENER_PLANIFICACION:
// 		detener_planificacion();
// 		exit(1);
// 		break;
// 	case LISTAR_ESTADOS:
// 		listar_procesos_x_estado();
// 		exit(1);
// 	default:
// 		printf("No es una opcion valida, ingrese otra vez");
// 		iniciar_consola_interactiva(logger);
// 		exit(1);
// 		break;
// 	}
// }

// op_code obtenerCodigo(char *opcion, t_log *logger)
// {
// 	if (strcmp(opcion, "1") == 0)
// 	{
// 		return EJECUTAR_SCRIPT;
// 	}
// 	if (strcmp(opcion, "2") == 0)
// 	{
// 		return INICIAR_PROCESO;
// 	}
// 	if (strcmp(opcion, "3") == 0)
// 	{
// 		return FINALIZAR_PROCESO;
// 	}
// 	if (strcmp(opcion, "4") == 0)
// 	{
// 		return INICIAR_PLANIFICACION;
// 	}
// 	if (strcmp(opcion, "5") == 0)
// 	{
// 		return DETENER_PLANIFICACION;
// 	}
// 	if (strcmp(opcion, "6") == 0)
// 	{
// 		return LISTAR_ESTADOS;
// 	}
// 	return 0;
// }

// /*------------------------------------OPERACIONES-------------------------------------*/
// // EJECUTAR SCRIPT
// void ejecutar_script()
// {
// 	printf("Ejecutar Script\n");
// }
// // VER TEMA DEL PATH QUE NO ESTA DEL TODO CLARO
// void iniciar_proceso(char *PATH)
// {
// 	t_pcb *proceso = crear_proceso();							// Creo nuestro nuevo proceso
// 	enviar_cod_enum(socket_memoria, SOLICITUD_INICIAR_PROCESO); // Le pido si puedo iniciar el proceso
// 	// Envia buffer a memoria despues de que recibio el OpCode
// 	tipo_buffer *buffer = crear_buffer();
// 	escribir_buffer(buffer, proceso->cde->pid);
// 	enviar_buffer(buffer, socket_memoria);
// 	destruir_buffer(buffer);

// 	op_code codigo = recibir_cod(socket_memoria);
// 	if (codigo == INICIAR_PROCESO_CORRECTO)
// 	{
// 		agregar_a_estado(proceso, cola_new_global);
// 		// Aca ella agrega semaforos pero creo que nosotros al tener la cola sincronizada nos lo evitamos
// 		log_info(logger, "Se creo un proceso con PID: %u en NEW\n", mostrarPID(proceso)); // se muestra el logger
// 	}
// 	else if (codigo == INICIAR_PROCESO_ERROR)
// 	{
// 		log_info(logger, "No se pudo crear el proceso %u", proceso->cde->pid); // se muestra que no se pudo
// 	}
// }
// t_pcb *crear_proceso()
// {
// 	t_pcb *proceso_nuevo = malloc(sizeof(t_pcb)); // reservamos memoria para el proceso           // por ahora en 0;
// 	proceso_nuevo->estado = NEW;
// 	proceso_nuevo->path = "";
// 	proceso_nuevo->archivosAsignados = list_create();
// 	proceso_nuevo->recursosAsignados = list_create();
// 	proceso_nuevo->cde = iniciar_cde();
// 	return proceso_nuevo;
// }

// t_cde *iniciar_cde()
// {
// 	t_cde *cde = malloc(sizeof(t_cde));

// 	cde->pid = PID_GLOBAL;
// 	PID_GLOBAL++;
// 	cde->pc = 0; // LA CPU lo va a ir cambiando

// 	cde->registro = malloc(sizeof(t_registros));
// 	cde->registro = NULL;
// 	cde->lista_instrucciones = malloc(sizeof(t_list));
// 	cde->lista_instrucciones = list_create();
// 	return cde;
// }
// // DETENER PROCESO
// void finalizar_proceso(uint32_t PID)
// {

// 	t_pcb *proceso = buscarProceso(PID); // buscamos en las colas
// 	log_info(logger, "Se finalizo el proceso %u \n", PID);

// 	op_code otroCodigo = recibir_cod(socket_memoria);
// 	if (otroCodigo == FINALIZAR_PROCESO)
// 	{
// 		log_info(logger, "PID %u -Destruir pcb", proceso->cde->pid);

// 		agregar_a_estado(proceso, cola_exit_global); // moverlo a la cola de exit
// 		liberar_proceso(proceso);
// 	}
// 	else if (otroCodigo == ERROR_FINALIZAR_PROCESO)
// 	{
// 		// FALTA VER COMO MOSTRAMOS EL MOTIVO POR EL QUE HA FINALIZADO EL PROCESO
// 		// log_info(logger, "Finalizar el proceso %u - Motivo: %s\n", PID, mostrarMotivo(motivoFinalizar));
// 	}
// }
// // INICIAR PLANIFICACION
// void iniciar_planificacion()
// {
// 	// tenemos un proceso en new y lo tenemos que pasar a ready
// 	// habilita a los hilos de los planificadores a que dejen de estar en pausa

// 	printf("Iniciar Planificacion");
// }
// // DETENER PLANIFICACION
// void detener_planificacion()
// {
// 	printf("Detener Planificacion");
// }
// // LISTAR PROCESOS POR ESTADO
// void listar_procesos_x_estado()
// {
// 	printf("Listar Procesos Por Estado");
// }

// uint32_t mostrarPID(t_pcb *proceso)
// {
// 	return proceso->cde->pid;
// }

// t_pcb *buscarProceso(uint32_t pid)
// {
// 	t_pcb *pcb_buscada = NULL;
// 	colaEstado *colas[] = {cola_new_global, cola_ready_global, cola_exec_global, cola_exit_global}; // Hace vector de colas

// 	for (int i = 0; i < sizeof(colas) / sizeof(colas[0]); i++) // va fijandose si en cada posicion del vector esta ese pid
// 	{

// 		if ((pcb_buscada = buscarPCBEnColaPorPid(pid, colas[i]->estado, colas[i]->nombreEstado)) != NULL) // busqueda
// 		{
// 			// pcb_buscada = buscarPCBEnColaPorPid(pid, colas[i]->estado, colas[i]->nombreEstado);
// 			return pcb_buscada;
// 			break;
// 		}
// 	}
// 	if (pcb_buscada == NULL)
// 	{
// 		printf("No se pudo encontrar ningun PCB asociado al PID %u\n", pid);
// 	}

// 	return pcb_buscada;
// }

// // FUNCION MOSTRAR MOTIVO
// /* char *mostrarMotivo(enum motivoFinalizar motivo)
// {
// 	if (motivo == SUCCESS)
// 	{
// 		return "SUCCESS";
// 	}
// 	else if (motivo == INVALID_RESOURCE)
// 	{
// 		return "INVALID_RESOURCE";
// 	}
// 	else if (motivo == INVALID_WRITE)
// 	{
// 		return "INVALID_WRITE";
// 	}
// } */

// // FUNCION DE LIBERACIONES
// void liberar_proceso(t_pcb *proceso)
// {
// 	liberar_cde(proceso);
// 	liberar_recursos(proceso);
// 	liberar_archivos(proceso);
// 	// liberar_memoria(proceso); CONSULTAR QUE HACE
// }
// void liberar_cde(t_pcb *proceso)
// {
// 	free(proceso->cde->lista_instrucciones);
// 	free(proceso->cde->registro);
// 	free(proceso->cde);
// 	// free(proceso->cde->pc);
// 	// free(proceso->cde->pid);
// 	// list_destroy(proceso->cde->instrucciones->parametros);
// 	// free(proceso->cde->registro);
// 	//  free(proceso->cde->instrucciones->codigo);
// }

// void liberar_recursos(t_pcb *proceso)
// {
// 	list_destroy(proceso->recursosAsignados);
// }
// void liberar_archivos(t_pcb *proceso)
// {
// 	free(proceso->archivosAsignados);
// }
// /*void liberar_memoria(t_pcb *proceso)
// {
// 	//PREGUNTAR QUE HACE ESTA FUNCION RAWRA
// }
// */
// /*
// void enviar_cde(int conexion, t_cde *cde, int codOP) //----IMPLEMENTAR----
// {
// 	t_paquete *paquete = crear_paquete_op_code(codOP);

// 	agregar_cde_a_paquete(paquete, cde);

// 	enviar_paquete(paquete, conexion);

// 	eliminar_paquete(paquete);
// }*/

// /*
// void enviar_cde(t_cde *cde)
// {
// 	enviar_codigo(socket_cpu_dispatch, EJECUTAR_PROCESO); // Le pido si pueod iniciar el proceso
// 	tipo_buffer *buffer = crear_buffer();
// 	escribir_buffer(buffer, cde->pid);
// 	escribir_buffer(buffer, cde->pc);
// 	enviar_buffer(buffer, socket_cpu_dispatch);
// 	destruir_buffer(buffer);
// }*/

// t_pcb *buscarPCBEnColaPorPid(int pid_buscado, t_queue *cola, char *nombreCola)
// {

// 	t_pcb *pcb_buscada;

// 	// Verificar si la lista está vacía
// 	if (queue_is_empty(cola))
// 	{
// 		printf("El estado %s está vacío.\n", nombreCola);
// 		return NULL;
// 	}

// 	t_queue *colaAux = queue_create();
// 	// Copiar los elementos a una cola auxiliar y mostrarlos

// 	while (!queue_is_empty(cola)) // vacia la cola y mientras buscar el elemento
// 	{
// 		t_pcb *pcb = queue_pop(cola);

// 		if (pcb->cde->pid == pid_buscado)
// 		{
// 			pcb_buscada = pcb;
// 		}

// 		queue_push(colaAux, pcb_buscada);
// 	}

// 	// Restaurar la cola original
// 	while (!queue_is_empty(colaAux))
// 	{
// 		queue_push(cola, queue_pop(colaAux));
// 	}

// 	// Liberar memoria de la cola auxiliar y sus elementos
// 	while (!queue_is_empty(colaAux))
// 	{
// 		free(queue_pop(colaAux));
// 	}
// 	queue_destroy(colaAux);

// 	if (pcb_buscada != NULL)
// 	{
// 		if (pcb_buscada->cde != NULL)
// 		{
// 			printf("Se encontró el proceso PID: %i en %s \n", pcb_buscada->cde->pid, nombreCola);
// 		}
// 		else
// 		{
// 			printf("El PCB encontrado no tiene un puntero válido a cde.\n");
// 		}
// 	}
// 	else
// 	{
// 		printf("No se encontró el proceso en %s.\n", nombreCola);
// 	}

// 	return pcb_buscada;
// }
// /*------------------------------------CORTO PLAZO-------------------------------------*/

// pthread_mutex_t *mutex_pcb_ejecutando;
// pthread_mutex_t *mutex_estado_ejecutando;

// // pthread_mutex_init(mutex, NULL) INICIALIZAR EN EL MAIN -ACORDARSE
// sem_t *semaforo_ready;

// void agregar_a_estado(t_pcb *pcb, colaEstado *cola_estado) // Añade un proceso a la cola New
// {
// 	pthread_mutex_lock(cola_estado->mutex_estado);
// 	queue_push(cola_estado->estado, pcb);
// 	pthread_mutex_unlock(cola_estado->mutex_estado);
// }

// // PASAR PROCESO DE READY A EXECUTE
// void ready_a_execute(colaEstado *cola_ready)
// { // Esto sirve solo para FIFO y RR    - VER DE CAMBIAR LOGICA EN CASO DE VRR CON UN IF
// 	while (1)
// 	{
// 		pthread_mutex_lock(mutex_pcb_ejecutando); // Deberia ser un binario que marque cuando hay 1 procesaso en EXEC para que no haya 2 a la vez
// 		sem_wait(semaforo_ready);				  // Contador que resta uno, cuando un proceso pasa de ready a execute

// 		// if(FIFO or RR) =>

// 		t_pcb *pcb = obtener_siguiente_ready(cola_ready);
// 		// else (VRR)
// 		agregar_a_estado(pcb, cola_ready);

// 		// simular_ejecucion_proceso(pcb, "asd"); ----IMPLEMENTAR----

// 		log_info(logger, "Se agrego un proceso %d  a Execute desde Ready\n", pcb->cde->pid);
// 	}
// }

// // OBTENER SIGUIENTE PROCESO
// t_pcb *obtener_siguiente_ready(colaEstado *cola_ready)
// {
// 	pthread_mutex_lock(cola_ready->mutex_estado); // mutex para estado ready y controlar la condicion de carrera

// 	t_pcb *pcb_ready = queue_pop(cola_ready->estado); // obtiene y quita el proceso de la cola de ready || SECCION CRITICA

// 	pthread_mutex_unlock(cola_ready->mutex_estado);
// 	return pcb_ready;
// }

// void simular_ejecucion_proceso(t_pcb *proceso)
// {
// 	// int CE;

// 	// t_cde *cde = obtener_cde(proceso);

// 	// enviar_paquete(paquete_a_exec, conexion_cpu);
// 	// enviar_cde(conexion_cpu, cde, CE); // ENVIO PROCESO A PUERTO DISPATCH

// 	// semaforo de iniciar ejecucion
// 	while (QUANTUM > 0)
// 	{
// 		if (proceso_completado())
// 		{
// 			break;
// 		}

// 		usleep(1000); // 1000 (1000MicroSegundos) ?? = 1 Quantum // Consultar con ayudante el tiempo
// 		QUANTUM--;

// 		if (QUANTUM == 0)
// 		{
// 			break;
// 		}
// 	}
// }

// t_cde *obtener_cde(t_pcb *proceso)
// {
// 	return proceso->cde;
// }

// void enviar_cde(int conexion, t_cde *cde, int codOP) //----IMPLEMENTAR----
// {
// 	t_paquete *paquete = crear_paquete_op_code(codOP);

// 	agregar_cde_a_paquete(paquete, cde);

// 	enviar_paquete(paquete, conexion);

// 	eliminar_paquete(paquete);
// }
// // Agregamos el Contexto de Ejecucion a Paquete
// void agregar_cde_a_paquete(t_paquete *paquete, t_cde *cde)
// {
// 	// Agregamos PID
// 	agregar_entero_a_paquete(paquete, cde->pid);
// 	log_trace(logger, "Se agrego el ID del proceso");

// 	// Agregamos INSTRUCCIONES
// 	agregar_lista_instrucciones_a_paquete(paquete, cde->lista_instrucciones);
// 	log_trace(logger, "Se agregaron las instrucciones del proceso");

// 	// Agregamos PC
// 	agregar_entero_a_paquete(paquete, cde->pc);
// 	log_trace(logger, "Se agrego el program counter");

// 	// Agregamos REGISTROS
// 	//  agregar_registros_a_paquete(paquete, ce->registros_cpu);          //----IMPLEMENTAR----
// 	log_trace(logger, "Se agregaron los registros"); // crear la funcion para mandar los registros.

// 	// agregar_tabla_segmentos_a_paquete(paquete, ce->tabla_segmentos);  //----IMPLEMENTAR----
// 	//  log_trace(logger, "agrego tabla de segmentos");
// }

// // serializaciones

// void agregar_lista_instrucciones_a_paquete(t_paquete *paquete, t_list *instrucciones)
// {
// 	int tamanio = list_size(instrucciones);

// 	agregar_entero_a_paquete(paquete, tamanio);

// 	for (int i = 0; i < tamanio; i++)
// 	{
// 		t_instruccion *instruccion = list_get(instrucciones, i);
// 		agregar_instruccion_a_paquete(paquete, instruccion);
// 	}
// }

// void agregar_instruccion_a_paquete(t_paquete *paquete, t_instruccion *instruccion)
// {
// 	agregar_tipo_instruccion_a_paquete(paquete, instruccion->codigo);
// 	// agregar_entero_a_paquete(paquete, string_length(instruccion->argumento1));
// 	// agregar_string_a_paquete(paquete, instruccion->argumento1);
// 	// agregar_entero_a_paquete(paquete, string_length(instruccion->argumento1));
// 	// agregar_string_a_paquete(paquete, instruccion->argumento2);
// }

// void agregar_tipo_instruccion_a_paquete(t_paquete *paquete, t_tipoDeInstruccion tipo)
// {
// 	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(t_tipoDeInstruccion));
// 	memcpy(paquete->buffer->stream + paquete->buffer->size, &tipo, sizeof(t_tipoDeInstruccion));
// 	paquete->buffer->size += sizeof(t_tipoDeInstruccion);
// }

// void agregar_entero_a_paquete(t_paquete *paquete, uint32_t x)
// {
// 	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
// 	memcpy(paquete->buffer->stream + paquete->buffer->size, &x, sizeof(uint32_t));
// 	paquete->buffer->size += sizeof(uint32_t);
// }

// void agregar_string_a_paquete(t_paquete *paquete, char *palabra)
// {
// 	agregar_entero_a_paquete(paquete, (int)strlen(palabra));
// 	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(char *));
// 	memcpy(paquete->buffer->stream + paquete->buffer->size, &palabra, sizeof(char *));
// 	paquete->buffer->size += (sizeof(char *));
// }
// bool proceso_completado()
// {
// 	return true;
// }
// t_paquete *crear_paquete_op_code(int codOP)
// {
// 	// IMPLEMENTAR
// 	t_paquete *paqueteEjemplo = crear_paquete();
// 	return paqueteEjemplo;
// }
// AGREGADOS A COLAS DE ESTADOS CON SEMAFOROS