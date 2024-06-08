#ifndef TLB_H_
#define TLB_H_

#include "cpu.h"

// ESTRUCTURAS
typedef struct
{
    int pid;
    int pagina;
    int marco;
} tlb_entrada;

typedef struct
{
    tlb_entrada *entradas;
    char *algoritmo;
    int cant_entradas;
} tlb_tabla;

// FUNCIONES

void tlb_iniciar(char *algoritmo, int cant_entradas);
void tlb_crear(char *algoritmo, int cant_entradas);
int tlb_consultar_info_pagina(int pagina_buscada);
void tlb_agregar_entrada(int pid, int pagina, int marco);



#endif