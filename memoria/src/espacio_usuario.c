#include "../include/espacio_usuario.h"
extern void *espacio_usuario;

void *escribir_espacio_usuario(uint32_t direccion_fisica, void *valor_a_escribir, size_t tamanio, t_log *logger, int pid)
{
    log_info(logger, "PID: <%d> - Accion: <ESCRIBIR> - Direccion fisica: <%d> - Tamaño <%zu>", pid, direccion_fisica, tamanio);

    int pagina = chequear_lectura_escritura_en_espacio_usuario(direccion_fisica, pid);
    if (pagina != -1)
    {
        void *destino = espacio_usuario + direccion_fisica;
        memcpy(destino, valor_a_escribir, tamanio);
        return (void *)1;
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
            size_t offset = direccion_pagina % pagina_size;
            size_t bytes_a_copiar = pagina_size - offset;
            if (bytes_leidos + bytes_a_copiar > tamanio)
            {
                bytes_a_copiar = tamanio - bytes_leidos;
            }
            memcpy(valor + bytes_leidos, espacio_usuario + direccion_pagina, bytes_a_copiar);
            bytes_leidos += bytes_a_copiar;
            direccion_pagina += bytes_a_copiar;
        }
        return valor;
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