#ifndef LARGOPLAZO_H_
#define LARGOPLAZO_H_
#include <unistd.h>
#include "../include/kernel.h"

void *largo_plazo();
t_pcb *transicion_new_a_ready();

void *transicion_exit_largo_plazo();
char *lista_pid();
void transicion_generica_exit(uint32_t pid);

bool coincide_pid(t_pcb *pcb);
colaEstado *buscarCola(uint32_t pid);
t_pcb *sacar_proceso_cola(colaEstado *cola_estado, uint32_t pid);

#endif