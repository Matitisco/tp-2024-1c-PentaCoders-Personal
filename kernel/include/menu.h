#ifndef MENU_H_
#define MENU_H_

#include "../include/kernel.h"
//#include <commons/log.h>

enum codigoOp{
    INICIAR_PROCESO,
    EJECUTAR_SCRIPT
};

void iniciar_consola_interactiva(t_log *logger);
void mostrar_operaciones_realizables();
void ejecutar_operacion(char *codigo_operacion, t_log *logger);
#endif