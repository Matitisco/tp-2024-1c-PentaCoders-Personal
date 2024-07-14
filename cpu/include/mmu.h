#ifndef MMU_H_
#define MMU_H_

#include "cpu.h"
#include "tlb.h"
#include <math.h>

// TRADUCCION
uint32_t direccion_logica_a_fisica(int direccion_logica);
uint32_t traducir_direccion_mmu(uint32_t direccion_logica);
// AUXILIARES
int calcular_pagina(int direccion_logica);
int enviar_peticion_frame(int pagina);

int obtener_frame(int pagina);
int pedir_frame_memoria(int pid, int nroPagina);
#endif