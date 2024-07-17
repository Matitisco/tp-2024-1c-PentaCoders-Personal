#include "../include/espacio_usuario.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../../utils/include/utils.h"
extern void *espacio_usuario; // espacio_usuario está definido en otro lugar


void *escribir_espacio_usuario(uint32_t direccion_fisica, void *valor_a_escribir, size_t tamanio,t_log *logger) {
    log_info(logger, "PID: <PID> - Accion: <ESCRIBIR> - Direccion fisica: <%d> - Tamaño <%zu>", direccion_fisica, tamanio);

    void *destino = espacio_usuario + direccion_fisica;
    memcpy(destino, valor_a_escribir, tamanio);
    // implementar chequeo de si se puede escribir dicho espacio
    return (void *)1;
}

void *leer_espacio_usuario(uint32_t direccion_fisica, size_t tamanio,t_log *logger) {
    log_info(logger, "PID: <PID> - Accion: <LEER> - Direccion fisica: <%d> - Tamaño <%zu>", direccion_fisica, tamanio);

    void *valor = malloc(tamanio);
    memcpy(valor, espacio_usuario + direccion_fisica, tamanio);
    // implementar chequeo de si se puede leer dicho espacio
    return valor;
}

void crear_espacio_usuario(int tam_memoria, t_log *logger)
{
    espacio_usuario = malloc(tam_memoria);
    if (espacio_usuario == NULL)
    {
        log_error(logger, "ERROR ESPACIO USUARIO");
    }
}