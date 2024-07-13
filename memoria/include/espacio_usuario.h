#ifndef ESPACIO_USUARIO_H
#define ESPACIO_USUARIO_H

#include <stdint.h>
#include <stdlib.h>
#include "../../utils/include/utils.h"

extern void *espacio_usuario;

void *escribir_espacio_usuario(uint32_t direccion_fisica, void *valor_a_escribir, size_t tamanio,config_memoria *valores_config,t_log *logger);
void *leer_espacio_usuario(uint32_t direccion_fisica, size_t tamanio,config_memoria *valores_config,t_log *logger);
void crear_espacio_usuario(int tam_memoria);
#endif 