#include "../include/menu.h"

void iniciar_consola_interactiva()
{
    while (1)
    {
        sleep(1);
        sem_post(binario_menu_lp); // Habilita largo plazo
        /* if (habilitar_planificadores)
        {
            sem_post(b_detener_planificacion);
            
        } */
        mostrar_operaciones_realizables();
        ejecutar_operacion(readline("Ingrese un valor para realizar una de las siguientes acciones: "), logger);
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
        finalizar_proceso(atoi(PID), INTERRUPTED_BY_USER);
        free(PID);
        break;
    case INICIAR_PLANIFICACION:
        iniciar_planificacion();
        break;
    case DETENER_PLANIFICACION:
        detener_planificacion();
        break;
    case MULTIPROGRAMACION:
        char *valor = readline("Ingrese el PID del Proceso a Finalizar: ");
        grado_multiprogramacion(atoi(valor));
        free(valor);
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