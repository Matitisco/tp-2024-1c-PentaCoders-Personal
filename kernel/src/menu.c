#include "../include/menu.h"

void iniciar_consola_interactiva(t_log *logger)
{
    char *opcion;
    while (1)
    {
        mostrar_operaciones_realizables();
        opcion = readline("Ingrese un valor para realizar una de las siguientes acciones: ");
        ejecutar_operacion(opcion, logger);
        free(opcion);
    }
}

void mostrar_operaciones_realizables()
{
    printf("1-Ejecutar Script de Operaciones\n");
    printf("2-Iniciar Proceso\n");
    printf("3-Finalizar Proceso\n");
    printf("4-Iniciar Planificacion\n");
    printf("5-Detener Planificacion\n");
    printf("6-Listar Procesos por Estado\n");
}

// 2 NEW => IP => 1 a ready =>

void ejecutar_operacion(char *opcion, t_log *logger)
{
    codigoOp cod_op = obtenerCodigo(opcion, logger);
    switch (cod_op)
    {
    case EJECUTAR_SCRIPT:
        ejecutar_script();
        exit(1);
        break;
    case INICIAR_PROCESO:
        iniciar_proceso();
        // exit(1); // momentaneo es solo para que no quede en loop
        break;
    case FINALIZAR_PROCESO:
        char *pid = readline("Ingrese el PID del Proceso a Finalizar: ");
        finalizar_proceso(atoi(pid));
        //exit(1);
        break;
    case INICIAR_PLANIFICACION:
        iniciar_planificacion();
        exit(1);
        break;
    case DETENER_PLANIFICACION:
        detener_planificacion();
        exit(1);
        break;
    case LISTAR_ESTADOS:
        listar_procesos_x_estado();
        exit(1);
    default:
        printf("No es una opcion valida, ingrese otra vez");
        iniciar_consola_interactiva(logger);
        exit(1);
        break;
    }
}

codigoOp obtenerCodigo(char *opcion, t_log *logger)
{
    if (strcmp(opcion, "1") == 0)
    {
        return EJECUTAR_SCRIPT;
    }
    if (strcmp(opcion, "2") == 0)
    {
        return INICIAR_PROCESO;
    }
    if (strcmp(opcion, "3") == 0)
    {
        return FINALIZAR_PROCESO;
    }
    if (strcmp(opcion, "4") == 0)
    {
        return INICIAR_PLANIFICACION;
    }
    if (strcmp(opcion, "5") == 0)
    {
        return DETENER_PLANIFICACION;
    }
    if (strcmp(opcion, "6") == 0)
    {
        return LISTAR_ESTADOS;
    }
}