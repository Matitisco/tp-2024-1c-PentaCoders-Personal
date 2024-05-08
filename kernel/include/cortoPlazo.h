#ifndef CORTOPLAZO_H_
#define CORTOPLAZO_H_
#include <unistd.h>
#include "../include/kernel.h"

bool proceso_completado();
t_pcb *obtener_siguiente_ready();
t_paquete* crear_paquete_op_code(int codOP);
t_cde *obtener_cde(t_pcb *proceso);
void enviar_cde(int conexion, t_cde *cde, int codOP);
void simular_ejecucion_proceso(t_pcb *proceso);
void ready_a_execute();

void agregar_lista_instrucciones_a_paquete(t_paquete *paquete, t_list *instrucciones);
void agregar_instruccion_a_paquete(t_paquete *paquete, t_instruccion *instruccion);
void agregar_tipo_instruccion_a_paquete(t_paquete *paquete, t_tipoDeInstruccion tipo);
void agregar_entero_a_paquete(t_paquete *paquete, uint32_t x);
void agregar_string_a_paquete(t_paquete *paquete, char *palabra);

void agregar_cde_a_paquete(t_paquete *paquete, t_cde *cde);



#endif