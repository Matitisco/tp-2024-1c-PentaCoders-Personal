#include "../include/espacio_usuario.h"
extern void *espacio_usuario; // espacio_usuario está definido en otro lugar

void *escribir_espacio_usuario(uint32_t direccion_fisica, void *valor_a_escribir, size_t tamanio, t_log *logger, int pid)
{
    log_info(logger, "PID: <%d> - Accion: <ESCRIBIR> - Direccion fisica: <%d> - Tamaño <%zu>", pid, direccion_fisica, tamanio);

    char* valor_a_escribir_char = valor_a_escribir;
    log_info(logger,"VALOR A ESCRIBIR EU: %s", valor_a_escribir_char);

    // chequeo si el marco esta asignado al proceso
    int pagina = chequear_lectura_escritura_en_espacio_usuario(direccion_fisica, pid);
    if (pagina != -1)
    {
        log_info(logger, "Se accedio al espacio de escritura del proceso");
        void *destino = espacio_usuario + direccion_fisica;
        memcpy(destino, valor_a_escribir, tamanio);
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
    if(tabla == NULL){
        return -1;
    }
    int pagina = consultar_pagina_de_un_marco(tabla, marco);
    return pagina;
}

void *leer_espacio_usuario(uint32_t direccion_fisica, size_t tamanio, t_log *logger, int pid)
{
    log_info(logger, "PID: <%d> - Accion: <LEER> - Direccion fisica: <%d> - Tamaño <%zu>", pid, direccion_fisica, tamanio);

    int pagina = chequear_lectura_escritura_en_espacio_usuario(direccion_fisica, pid);
    if (pagina != -1)
    {
        log_info(logger, "Se accedio al espacio de lectura del proceso");
        void *valor = malloc(tamanio);
        memcpy(valor, espacio_usuario + direccion_fisica, tamanio);         //<--- La funcion es esto, lo demas son chequeos
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
    if (espacio_usuario == NULL)
    {
        log_error(logger, "ERROR ESPACIO USUARIO");
    }
}