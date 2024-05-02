/* #ifndef MENU_H_
#define MENU_H_

#include "kernel.h"
#include "utils_kernel.h"

void iniciar_consola_interactiva(t_log *logger);
void mostrar_operaciones_realizables();
op_code obtenerCodigo(char *opcion, t_log *logger);
void ejecutar_operacion(char *codigo_operacion, t_log *logger);

#endif */