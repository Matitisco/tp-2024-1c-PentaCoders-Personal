#ifndef MMU_H_
#define MMU_H_

#include "cpu.h"
#include "tlb.h"
#include <math.h>

// TRADUCCION
uint32_t traducir_direccion_mmu(uint32_t direccion_logica);
// AUXILIARES
int calcular_pagina(int direccion_logica);
int obtener_frame(int pagina);
int pedir_frame_memoria(int pid, int nroPagina);

void tlb_iniciar(char *algoritmo, int cant_entradas);
#endif