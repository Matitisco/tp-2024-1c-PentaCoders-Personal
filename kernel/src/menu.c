#include "../include/menu.h"

char *generator(const char *text, int state)
{
    static int list_index, len;
    char *name;

    if (!state)
    {
        list_index = 0;
        len = strlen(text);
    }

    while ((name = comandos[list_index++]))
    {
        if (strncmp(name, text, len) == 0)
        {
            return strdup(name);
        }
    }

    return NULL;
}

char **autocompletado(const char *text, int start, int end)
{
    rl_attempted_completion_over = 1;

    char **matches = NULL;
    if (start == 0)
    {
        matches = rl_completion_matches(text, generator);
    }

    return matches;
}

char *obtener_comando(char *entrada)
{
    return strtok(entrada, " ");
}

int contar_tokens(char *cadena)
{
    int contador = 0;
    char *token;

    char *cadena_copia = strdup(cadena);

    token = strtok(cadena_copia, " ");
    while (token != NULL)
    { // copia de la cadena, ya que strtok modifica la cadena original
        contador++;
        token = strtok(NULL, " ");
    }

    free(cadena_copia);
    return contador;
}

void ejecutar_comando(char *comando, int tokens)
{
    bool argumentros_incorrectos = false;

    if (strcmp(comando, "EJECUTAR_SCRIPT") == 0)
    {
        if (tokens == 2)
        {
            ejecutar_script(strtok(NULL, "\0"));
        }
        else
        {
            argumentros_incorrectos = true;
        }
    }
    else if (strcmp(comando, "INICIAR_PROCESO") == 0)
    {
        if (tokens == 2)
        {
            iniciar_proceso(strtok(NULL, "\0"));
        }
        else
        {
            argumentros_incorrectos = true;
        }
    }
    else if (strcmp(comando, "FINALIZAR_PROCESO") == 0)
    {
        if (tokens == 2)
        {
            sem_post(sem_finalizar_proceso);
            finalizar_proceso(atoi(strtok(NULL, "\0")), INTERRUPTED_BY_USER);
        }
        else
        {
            argumentros_incorrectos = true;
        }
    }
    else if (strcmp(comando, "DETENER_PLANIFICACION") == 0)
    {
        if (tokens == 1)
        {
            detener_planificacion();
        }
        else
        {
            argumentros_incorrectos = true;
        }
    }
    else if (strcmp(comando, "INICIAR_PLANIFICACION") == 0)
    {
        if (tokens == 1)
        {
            iniciar_planificacion();
        }
        else
        {
            argumentros_incorrectos = true;
        }
    }
    else if (strcmp(comando, "MULTIPROGRAMACION") == 0 && tokens == 2)
    {
        if (tokens == 2)
        {
            grado_multiprogramacion(atoi(strtok(NULL, "\0")));
        }
        else
        {
            argumentros_incorrectos = true;
        }
    }
    else if (strcmp(comando, "PROCESO_ESTADO") == 0 && tokens == 1)
    {
        if (tokens == 1)
        {
            proceso_estado();
        }
        else
        {
            argumentros_incorrectos = true;
        }
    }
    else if (strcmp(comando, "HELP") == 0 && tokens == 1)
    {
        if (tokens == 1)
        {
            printf("EJECUTAR_SCRIPT [PATH]\n");
            printf("INICIAR_PROCESO [PATH]\n");
            printf("FINALIZAR_PROCESO [PID]\n");
            printf("DETENER_PLANIFICACION\n");
            printf("INICIAR_PLANIFICACION\n");
            printf("MULTIPROGRAMACION [VALOR]\n");
            printf("PROCESO_ESTADO\n");
        }
        else
        {
            argumentros_incorrectos = true;
        }
    }
    else
    {
        printf("%s: command not found \n", comando);
    }

    if (argumentros_incorrectos)
    {
        printf("Invalid arguments\n\n");
    }
}

void iniciar_consola_interactiva2()
{
    using_history();
    rl_attempted_completion_function = autocompletado;
    while (1)
    {
        sleep(1);
        sem_post(binario_menu_lp); // Habilita largo plazo

        char *entrada = readline("\033[1;32mPentaOS\033[0m:\033[1;34m~\033[0m$ ");
        add_history(entrada);

        int tokens = contar_tokens(entrada);
        char *comando = obtener_comando(entrada); // palabra reservada
        ejecutar_comando(comando, tokens);

        free(entrada);
    }
}

void iniciar_consola_interactiva()
{
    while (1)
    {
        sleep(1);
        sem_post(binario_menu_lp); // Habilita largo plazo
        mostrar_operaciones_realizables();
        ejecutar_operacion(readline("Ingrese un valor para realizar una de las siguientes acciones: "), logger);
    }
}

void mostrar_operaciones_realizables()
{
    printf("-----------------------------------------\n");
    printf("1-Ejecutar Script\n");
    printf("2-Iniciar Proceso\n");
    printf("3-Finalizar Proceso\n");
    printf("4-Iniciar Planificacion\n");
    printf("5-Detener Planificacion\n");
    printf("6-Modificar Grado Multiprogramacion\n");
    printf("7-Listar Procesos Estado\n");
    printf("-----------------------------------------\n");
}

void ejecutar_operacion(char *opcion, t_log *logger)
{
    opciones_menu cod_op = obtenerCodigo(opcion, logger);
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
    case FINALIZAR_PROCESO_MENU:
        char *PID = readline("Ingrese el PID del Proceso a Finalizar: ");
        //sem_post(sem_finalizar_proceso);
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

opciones_menu obtenerCodigo(char *opcion, t_log *logger)
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
        return FINALIZAR_PROCESO_MENU;
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