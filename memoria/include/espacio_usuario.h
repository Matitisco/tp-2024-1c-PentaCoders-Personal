#ifndef ESPACIO_USUARIO_H
#define ESPACIO_USUARIO_H

#include "memoria.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern void *espacio_usuario;

void *escribir_espacio_usuario(uint32_t direccion_fisica, void *valor_a_escribir, size_t tamanio, t_log *logger,int pid);
void *leer_espacio_usuario(uint32_t direccion_fisica, size_t tamanio, t_log *logger,int pid);
void crear_espacio_usuario(int tam_memoria, t_log *logger);
int chequear_lectura_escritura_en_espacio_usuario(int direccion_fisica, int pid);
#endif