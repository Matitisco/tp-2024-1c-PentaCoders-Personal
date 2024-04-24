#include "../include/menu.h"

void iniciar_consola_interactiva(t_log *logger)
{

    char *opcion;
    while (1)
    {
        mostrar_operaciones_realizables();
        opcion = readline("Ingrese un valor para realizar una de las siguientes acciones: ");
        ejecutar_operacion(cod_operacion, logger);
        free(cod_operacion);
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

void ejecutar_operacion(char *opcion, t_log *logger)
{
    codigoOp cod_op = obtenerCodigo(opcion, logger);
    // hay que convertir el char a int o algo similar usando cmp
    switch (cod_op)
    {
    case EJECUTAR_SCRIPT:
        ejecutar_script();
        exit(1);
        break;
    case 2:
        iniciar_proceso();
        exit(1);
        break;
    case 3:
        detener_proceso();
        exit(1);
        break;
    case 4:
        iniciar_planificacion();
        exit(1);
        break;
    case 5:
        detener_planificacion();
        exit(1);
        break;
    case 6:
        listar_procesos_x_estado();
        exit(1);
    default:
        printf("No es una opcion valida, ingrese otra vez");
        iniciar_consola_interactiva(logger);
        exit(1);
        break;
    }
}

codigoOp obtenerCodigo(char* opcion,t_log* logger){
    switch (opcion)
    {
    case strcmp(opcion,"1"):
        return EJECUTAR_SCRIPT;
        break;
    
    default:
        break;
    }
}