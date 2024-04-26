#ifndef MENU_H_
#define MENU_H_

#include "../include/kernel.h"
// #include <commons/log.h>

typedef enum 
{
    EJECUTAR_SCRIPT,
    FINALIZAR_PROCESO,
    INICIAR_PROCESO,
    INICIAR_PLANIFICACION,
    DETENER_PLANIFICACION,
    LISTAR_ESTADOS
}codigoOp;

void iniciar_consola_interactiva(t_log *logger);
void mostrar_operaciones_realizables();
codigoOp obtenerCodigo(char *opcion, t_log *logger);
void ejecutar_operacion(char *codigo_operacion, t_log *logger);

#endif