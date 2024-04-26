#ifndef OPERACIONES_H_
#define OPERACIONES_H_

#include "kernel.h"
#include "cortoPlazo.h"
void ejecutar_script();
void iniciar_proceso();
void detener_proceso();
void iniciar_planificacion();
void detener_planificacion();
void listar_procesos_x_estado();

t_pcb *crear_proceso();
t_cde *iniciar_cde();

#endif