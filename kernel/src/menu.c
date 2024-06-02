#include "../include/menu.h"


char * obtener_comando(char *entrada){
    return strtok(entrada, " ");
}
//hacer funcion controlar argumentos
bool cantidad_argumentos_correcta(char *entrada){

}
void ejecutar_comando(char *comando)
{
    if(strcmp(comando, "EJECUTAR_SCRIPT") == 0){
		//printf("EJECUTAR_SCRIPT \n");
        ejecutar_script(strtok(NULL, "\0"));
	}
	else if(strcmp(comando, "INICIAR_PROCESO") == 0){
		//printf("INICIAR_PROCESO \n");
        iniciar_proceso(strtok(NULL, "\0"));
        /*
        char *PATH_INSTRUCCIONES = readline("Ingrese el nombre del archivo: ");
        iniciar_proceso(PATH_INSTRUCCIONES);
        free(PATH_INSTRUCCIONES);
        */
	}
	else if(strcmp(comando, "FINALIZAR_PROCESO") == 0){
		//printf("FINALIZAR_PROCESO \n");
        finalizar_proceso(atoi(strtok(NULL, "\0")));
	}
	else if(strcmp(comando, "DETENER_PLANIFICACION") == 0){
		//printf("DETENER_PLANIFICACION \n");
        detener_planificacion();
	}
	else if(strcmp(comando, "INICIAR_PLANIFICACION") == 0){
		//printf("INICIAR_PLANIFICACION \n");
        iniciar_planificacion();
	}
	else if(strcmp(comando, "MULTIPROGRAMACION") == 0){
		//printf("MULTIPROGRAMACION \n");
        grado_multiprogramacion(atoi( strtok(NULL, "\0") ));
	}
	else if(strcmp(comando, "PROCESO_ESTADO") == 0){
		//printf("PROCESO_ESTADO \n");
        proceso_estado();
	}
    else{
        printf("%s: command not found \n",comando);
    }
}

void iniciar_consola_interactiva2()
{
    habilitar_planificadores = 0;
    while (1)
    {
        sleep(1);
        sem_post(binario_menu_lp); // Habilita largo plazo
        if (habilitar_planificadores)
        {
            sem_post(b_reanudar_largo_plazo);
            sem_post(b_reanudar_corto_plazo);
        }

        char *entrada = readline("\033[1;32mkernel\033[0m:\033[1;34m~\033[0m$ ");
        char *comando =  obtener_comando(entrada);
        ejecutar_comando(comando);
        free(entrada);
    }
}
void iniciar_consola_interactiva()
{
    habilitar_planificadores = 0;
    char *opcion = string_new();
    while (1)
    {
        sleep(1);
        sem_post(binario_menu_lp); // Habilita largo plazo
        if (habilitar_planificadores)
        {
            sem_post(b_reanudar_largo_plazo);
            sem_post(b_reanudar_corto_plazo);
        }
        mostrar_operaciones_realizables();
        ejecutar_operacion(readline("Ingrese un valor para realizar una de las siguientes acciones: "), logger);
        // free(opcion);
    }
}

void mostrar_operaciones_realizables()
{
    printf("\n\n");
    printf("1-Ejecutar Script\n");
    printf("2-Iniciar Proceso\n");
    printf("3-Finalizar Proceso\n");
    printf("4-Iniciar Planificacion\n");
    printf("5-Detener Planificacion\n");
    printf("6-Modificar Grado Multiprogramacion\n");
    printf("7-Listar Procesos por Estado\n");
    printf("\n\n");
}

void ejecutar_operacion(char *opcion, t_log *logger)
{
    op_code cod_op = obtenerCodigo(opcion, logger);
    switch (cod_op)
    {
    case EJECUTAR_SCRIPT:
        char *PATH_SCRIPT = readline("Ingrese el nombre del archivo con comandos: ");
        ejecutar_script(PATH_SCRIPT);
        free(PATH_SCRIPT);
        break;
    case INICIAR_PROCESO:
        char *PATH_INSTRUCCIONES = readline("Ingrese el nombre del archivo: ");
        iniciar_proceso(PATH_INSTRUCCIONES);
        free(PATH_INSTRUCCIONES);
        break;
    case FINALIZAR_PROCESO:
        char *PID = readline("Ingrese el PID del Proceso a Finalizar: ");
        finalizar_proceso(atoi(PID));
        free(PID);
        break;
    case INICIAR_PLANIFICACION:
        iniciar_planificacion();
        // exit(1);
        break;
    case DETENER_PLANIFICACION:
        detener_planificacion();
        // exit(1);
        break;
    case MULTIPROGRAMACION:
        char *valor = readline("Ingrese el PID del Proceso a Finalizar: ");
        grado_multiprogramacion(atoi(valor));
        break;
    case LISTAR_ESTADOS:
        proceso_estado();
        break;
    default:
        printf("No es una opcion valida, ingrese otra vez\n");
        iniciar_consola_interactiva(logger);
        break;
    }
}

op_code obtenerCodigo(char *opcion, t_log *logger)
{
    const char *opcion_elegida = opcion;
    if (strcmp(opcion_elegida, "1") == 0)
    {
        log_info(logger, "Se eligio la opcion EJECUTAR SCRIPT");
        return EJECUTAR_SCRIPT;
    }
    if (strcmp(opcion_elegida, "2") == 0)
    {
        log_info(logger, "Se eligio la opcion INICIAR PROCESO");
        return INICIAR_PROCESO;
    }
    if (strcmp(opcion_elegida, "3") == 0)
    {
        log_info(logger, "Se eligio la opcion FINALIZAR PROCESO");
        return FINALIZAR_PROCESO;
    }
    if (strcmp(opcion_elegida, "4") == 0)
    {
        log_info(logger, "Se eligio la opcion INICIAR PLANIFICACION");
        return INICIAR_PLANIFICACION;
    }
    if (strcmp(opcion_elegida, "5") == 0)
    {
        log_info(logger, "Se eligio la opcion DETENER PLANIFICACION");
        return DETENER_PLANIFICACION;
    }
    if (strcmp(opcion_elegida, "6") == 0)
    {
        log_info(logger, "Se eligio la opcion MODIFICAR GRADO DE MULTIPROGRAMACION");
        return MULTIPROGRAMACION;
    }
    if (strcmp(opcion_elegida, "7") == 0)
    {
        log_info(logger, "Se eligio la opcion LISTAR PROCESOS POR ESTADO");
        return LISTAR_ESTADOS;
    }
    return -1;
}