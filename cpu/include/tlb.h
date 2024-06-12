#ifndef TLB_H_
#define TLB_H_

#include "cpu.h"

// ESTRUCTURAS
typedef struct
{
    int pid;
    int pagina;
    int marco;
    char *ultima_referencia;

} tlb_entrada;

typedef struct
{
    int pos_entrada;
    int estaba_cargada;
} entrada_tipo;

typedef struct
{
    tlb_entrada *entradas;
    char *algoritmo;
    int cant_entradas;
} tlb_tabla;

// TLB
void tlb_iniciar(char *algoritmo, int cant_entradas);
void tlb_crear(char *algoritmo, int cant_entradas);
int tlb_consultar_info_pagina(int pagina_buscada);
void tlb_agregar_entrada(int pid, int pagina, int marco);

// ALGORITMOS TLB
void tlb_reemplazar(tlb_entrada *entrada);
void tlb_reemplazo_fifo(tlb_entrada *entrada);
void tlb_reemplazo_lru(tlb_entrada *entrada);

// ENTRADAS
entrada_tipo entrada_obtener(int pid, int pagina, int marco);
tlb_entrada *entrada_crear(int pid, int pagina, int marco);

// AUXILIARES
int obtener_tiempo_en_miliSegundos(char *tiempo);

#endif