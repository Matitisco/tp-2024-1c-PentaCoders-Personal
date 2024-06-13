#ifndef MENU_H_
#define MENU_H_

#include "kernel.h"
#include "../include/operaciones.h"

// MENU
void iniciar_consola_interactiva();
void mostrar_operaciones_realizables();
void ejecutar_operacion(char *codigo_operacion, t_log *logger);

// AUXILIARES
op_code obtenerCodigo(char *opcion, t_log *logger);

#endif