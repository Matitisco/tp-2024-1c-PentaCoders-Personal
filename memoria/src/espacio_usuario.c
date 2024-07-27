#include "../include/espacio_usuario.h"
extern void *espacio_usuario;

void imprimir_rango_memoria(void *espacio_usuario, uint32_t inicio, uint32_t fin)
{
    // Validar los rangos de inicio y fin
    if (inicio > fin)
    {
        printf("El rango de memoria es inválido: inicio > fin.\n");
        return;
    }

    // Asegúrate de que el puntero no sea NULL
    if (espacio_usuario == NULL)
    {
        printf("El puntero a la memoria es NULL.\n");
        return;
    }

    // Convertir el puntero a un puntero a unsigned char para imprimir bytes
    unsigned char *memoria = (unsigned char *)espacio_usuario;
}

void *escribir_espacio_usuario(uint32_t direccion_fisica, void *valor_a_escribir, size_t tamanio, t_log *logger, int pid)
{
    log_info(logger, "PID: <%d> - Accion: <ESCRIBIR> - Direccion fisica: <%d> - Tamaño <%zu>", pid, direccion_fisica, tamanio);

    char *valor_a_escribir_char = malloc(tamanio + 1);
    valor_a_escribir_char[tamanio+1] = '\0';
    memcpy(valor_a_escribir_char, valor_a_escribir, tamanio);
    log_info(logger, "PID: <%d> - Valor a escribir en Espacio de Usuario: <%s>", pid, valor_a_escribir_char);
    free(valor_a_escribir_char);

    // chequeo si el marco esta asignado al proceso
    int pagina = chequear_lectura_escritura_en_espacio_usuario(direccion_fisica, pid);
    if (pagina != -1)
    {
        // log_info(logger, "Se accedio al espacio de escritura del proceso");
        void *destino = espacio_usuario + direccion_fisica;
        memcpy(destino, valor_a_escribir, tamanio);
        imprimir_rango_memoria(espacio_usuario, direccion_fisica, direccion_fisica + tamanio);
        return (void *)1;
    }
    else
    {
        log_error(logger, "Se quiere escribir por fuera del espacio del proceso asignado");
    }


    return NULL;
}

int chequear_lectura_escritura_en_espacio_usuario(int direccion_fisica, int pid)
{
    int marco = direccion_fisica / valores_config->tam_pagina;
    t_tabla_paginas *tabla = buscar_en_lista_global(pid);
    if (tabla == NULL)
    {
        return -1;
    }
    int pagina = consultar_pagina_de_un_marco(tabla, marco);

    return pagina;
}

void *leer_espacio_usuario(uint32_t direccion_fisica, size_t tamanio, t_log *logger, int pid)
{
    // log obligatorio
    log_info(logger, "PID: <%d> - Accion: <LEER> - Direccion fisica: <%d> - Tamaño <%zu>", pid, direccion_fisica, tamanio);

    int pagina = chequear_lectura_escritura_en_espacio_usuario(direccion_fisica, pid);
    if (pagina != -1)
    {
        void *valor = malloc(tamanio);
        if (valor == NULL)
        {
            return NULL;
        }
        size_t bytes_leidos = 0;
        size_t pagina_size = valores_config->tam_pagina;
        uint32_t direccion_pagina = direccion_fisica;

        while (bytes_leidos < tamanio)
        {
            // Calcular el desplazamiento dentro de la página
            size_t offset = direccion_pagina % pagina_size;
            // Calcular el tamaño a copiar en esta iteración
            size_t bytes_a_copiar = pagina_size - offset;
            if (bytes_leidos + bytes_a_copiar > tamanio)
            {
                bytes_a_copiar = tamanio - bytes_leidos;
            }
            // Copiar los datos desde la página
            memcpy(valor + bytes_leidos, espacio_usuario + direccion_pagina, bytes_a_copiar);
            bytes_leidos += bytes_a_copiar;
            direccion_pagina += bytes_a_copiar;
        }

        return valor;
    }
    else
    {
        log_error(logger, "Se quiere leer por fuera del espacio del proceso asignado");
    }

    return NULL;
}

void crear_espacio_usuario(int tam_memoria, t_log *logger)
{
    espacio_usuario = malloc(tam_memoria);
    memset(espacio_usuario, ' ', tam_memoria);
    if (espacio_usuario == NULL)
    {
        log_error(logger, "ERROR ESPACIO USUARIO");
    }
}

int contarDigitos(int numero)
{
    if (numero == 0)
    {
        return 1; // Caso especial para el número 0
    }
    return (int)log10(numero) + 1; // Usa logaritmo base 10 y suma 1
}
void insertarStringEnPosicion(char *destino, const char *fuente, int posicion)
{
    int longitudFuente = strlen(fuente);
    memcpy(destino + posicion, fuente, longitudFuente);
}

int imprimir_linea_guiones(int tam_memoria, int tam_pagina)
{
    int cant_marcos = tam_memoria / tam_pagina;

    int tamanio_celda_marco = strlen(" Marco ");
    int digitos = contarDigitos(cant_marcos - 1);
    if (digitos > tamanio_celda_marco)
    {
        tamanio_celda_marco = digitos;
    }

    int longitud_ocupado = strlen(" OCUPADO ") + 2;
    int longitud_contenido = tam_pagina + 1;

    int longitud_total = tamanio_celda_marco + 1 + longitud_ocupado + longitud_contenido;

    char *linea_guiones = malloc(longitud_total + 1);

    memset(linea_guiones, '-', longitud_total);
    linea_guiones[longitud_total] = '\0';

    printf_yellow("%s", linea_guiones);
    free(linea_guiones);
    return longitud_total;
}

void imprimir_espacio_usuario(void *espacio_usuario, int tam_memoria, int tam_pagina, t_bit_map *array_bitmap)
{
    printf_yellow("     ESPACIO DE USUARIO");
    int longitud_total = imprimir_linea_guiones(tam_memoria, tam_pagina);

    if (tam_pagina > 11)
    {
        char *celda_contanido = malloc(tam_pagina + 1);
        celda_contanido[tam_pagina] = '\0';
        memset(celda_contanido, ' ', tam_pagina);
        insertarStringEnPosicion(celda_contanido, " CONTENIDO ", 0);
        printf_yellow("| Marco | OCUPADO |%s|", celda_contanido);
        free(celda_contanido);
    }
    else
    {
        printf_yellow("| Marco | OCUPADO | CONTENIDO |");
    }
    imprimir_linea_guiones(tam_memoria, tam_pagina);
    int cant_marcos = tam_memoria / tam_pagina;

    int tamanio_celda_marco = strlen(" Marco ");
    int digitos = contarDigitos(cant_marcos - 1);
    if (digitos > tamanio_celda_marco)
    {
        tamanio_celda_marco = digitos;
    }

    for (int i = 0; i < cant_marcos; ++i)
    {
        char *celda_marco = malloc(tamanio_celda_marco + 1);

        memset(celda_marco, ' ', tamanio_celda_marco); // Llenar con espacios
        celda_marco[tamanio_celda_marco] = '\0';       // Terminador nulo

        char *contenido_marco = malloc(tam_pagina + 1);
        contenido_marco[tam_pagina] = '\0';
        memcpy(contenido_marco, (char *)espacio_usuario + i * tam_pagina, tam_pagina);

        char *nro_marco = malloc(digitos + 1);
        sprintf(nro_marco, "%d", i);
        insertarStringEnPosicion(celda_marco, nro_marco, 2);

        printf("\033[93m|%s|    %d    |", celda_marco, array_bitmap[i].bit_ocupado);
        for (int a = 0; a < tam_pagina; a++) // imprimo uno a uno los bytes ese marco, pero como caracter
        {
            char caracter;
            memcpy(&caracter, (char *)espacio_usuario + i * tam_pagina + a, 1);
            printf("%c", caracter);
        }
        printf("|");
        printf("\033[0m\n"); // Resetear color
        // printf_yellow("|%s|    %d    |%s|", celda_marco, array_bitmap[i].bit_ocupado, contenido_marco);
        imprimir_linea_guiones(tam_memoria, tam_pagina);

        free(contenido_marco);
        free(nro_marco);
        free(celda_marco);
    }
}