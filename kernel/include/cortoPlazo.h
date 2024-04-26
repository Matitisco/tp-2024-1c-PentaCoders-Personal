#ifndef CORTOPLAZO_H_
#define CORTOPLAZO_H_
#include <unistd.h>
#include "../include/kernel.h"
#include "../../utils/src/utils.h"
#include "../../utils/include/instrucciones.h"
// Todas las definiciones en el .h

void ready_a_execute();
t_pcb *obtener_siguiente_ready();
void simular_ejecucion_proceso(t_pcb *proceso);
t_cde *obtener_cde(t_pcb *proceso);
void enviar_cde(int conexion, t_cde *cde, int codOP);
void agregar_cde_a_paquete(t_paquete *paquete, t_cde *cde);
void agregar_lista_instrucciones_a_paquete(t_paquete *paquete, t_list *instrucciones);
void agregar_instruccion_a_paquete(t_paquete *paquete, t_instruccion *instruccion);
void agregar_tipo_instruccion_a_paquete(t_paquete *paquete, t_tipoDeInstruccion tipo);
void agregar_entero_a_paquete(t_paquete *paquete, uint32_t x);
void agregar_string_a_paquete(t_paquete *paquete, char *palabra);


void agregar_a_estado(t_pcb *pcb,colaEstado *cola_estado); // Añade un proceso a la cola estado


#endif