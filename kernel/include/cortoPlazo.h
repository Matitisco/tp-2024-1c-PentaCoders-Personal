#ifndef CORTOPLAZO_H_
#define CORTOPLAZO_H_
#include <unistd.h>
#include "../include/kernel.h"

bool proceso_completado();
t_pcb *obtener_siguiente_ready();
t_cde *obtener_cde(t_pcb *proceso);
void enviar_cde(int conexion, t_cde *cde);
void simular_ejecucion_proceso(t_pcb *proceso);
void ready_a_execute();
void iniciar_sem_cp();

void planificar_por_fifo();
void cambiar_procesoActual_readyARunning();
int hayInstruccionBloqueante();
void planificar_por_rr();
void replanificar_por_rr(t_pcb *proceso);
void planificar_por_vrr();
void *transicion_exec_ready();
void *transicion_exit_largo_plazo();
void *hilo_quantum();

void enviar_a_cpu_cde(t_cde* cde);
t_pcb *transicion_ready_exec();
void inicio_quantum();
_Bool esta_bloqueado_por_falta_de_recurso(t_recurso *recurso);
#endif