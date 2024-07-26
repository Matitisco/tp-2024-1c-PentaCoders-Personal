#ifndef TLB_H_
#define TLB_H_
#include <commons/temporal.h>
#include <commons/collections/list.h>
#include "cpu.h"

// ESTRUCTURAS
typedef struct {
    int pid;
    int nro_pagina;
    int nro_marco;
    int last_reference; //tiempo en ms para usar para el algoritmo
} t_entrada_tlb;
typedef enum {FIFO, LRU} algoritmo_reemplazo_tlb;

typedef struct {
    t_list *entradas_tlb;
    int size_tlb;
    algoritmo_reemplazo_tlb algoritmo;
} t_tlb;

extern t_tlb * tlb_cpu;
// TLB
t_tlb *crear_tlb(int size_tlb, algoritmo_reemplazo_tlb algoritmo);
t_entrada_tlb *crear_entrada_tlb(int pid, int nro_pagina, int nro_marco);
void eliminar_tlb();
// ENTRADAS
void reemplazar_entrada_tlb(t_tlb *tlb, t_entrada_tlb *entrada);
void agregar_entrada_a_tlb(t_tlb *tlb, t_entrada_tlb *entrada);
int obtener_marco_tlb(t_tlb *tlb, int pid, int nro_pagina);

// AUXILIARES
int obtener_tiempo_en_miliSegundos(char *tiempo);
bool tlb_llena(t_tlb *tlb);
static void* entrada_tlb_last_reference_min(t_entrada_tlb* entrada1, t_entrada_tlb* entrada2);
void imprimir_tlb(t_tlb * tlb);

#endif