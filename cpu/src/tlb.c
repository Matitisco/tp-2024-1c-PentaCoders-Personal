#include "../include/tlb.h"

// tlb_tabla *tlb;
t_tlb *tlb_cpu;

int obtener_tiempo_en_miliSegundos(char *tiempo)
{
    char **nueva_hora = string_split(tiempo, ":");

    int horas = atoi(nueva_hora[0]);
    int minutos = atoi(nueva_hora[1]) + horas * 60;
    int segundos = atoi(nueva_hora[2]) + minutos * 60;
    int miliSegundos = atoi(nueva_hora[3]) + segundos * 1000;

    string_array_destroy(nueva_hora);
    free(tiempo);
    return miliSegundos;
}

t_tlb *crear_tlb(int size_tlb, algoritmo_reemplazo_tlb algoritmo)
{
    t_tlb *tlb = malloc(sizeof(t_tlb));
    tlb->entradas_tlb = list_create();
    tlb->size_tlb = size_tlb;
    tlb->algoritmo = algoritmo;
    return tlb;
}

t_entrada_tlb *crear_entrada_tlb(int pid, int nro_pagina, int nro_marco)
{
    t_entrada_tlb *entrada = malloc(sizeof(t_entrada_tlb));
    entrada->pid = pid;
    entrada->nro_pagina = nro_pagina;
    entrada->nro_marco = nro_marco;
    entrada->last_reference = obtener_tiempo_en_miliSegundos(temporal_get_string_time("%H:%M:%S:%MS"));
    return entrada;
}

bool tlb_llena(t_tlb *tlb)
{
    return list_size(tlb->entradas_tlb) == tlb->size_tlb;
}

static void *entrada_tlb_last_reference_min(t_entrada_tlb *entrada1, t_entrada_tlb *entrada2)
{
    return entrada1->last_reference <= entrada2->last_reference ? entrada1 : entrada2;
}

void reemplazar_entrada_tlb(t_tlb *tlb, t_entrada_tlb *entrada)
{
    t_entrada_tlb *entrada_modificar;
    entrada_modificar = (t_entrada_tlb *)list_get_minimum(tlb->entradas_tlb, (void *)entrada_tlb_last_reference_min);
    entrada_modificar->pid = entrada->pid;
    entrada_modificar->nro_pagina = entrada->nro_pagina;
    entrada_modificar->nro_marco = entrada->nro_marco;
    entrada_modificar->last_reference = entrada->last_reference;
    free(entrada);
}

void agregar_entrada_a_tlb(t_tlb *tlb, t_entrada_tlb *entrada)
{
    if (tlb_llena(tlb))
    {
        reemplazar_entrada_tlb(tlb, entrada);
    }
    else
    {
        list_add(tlb->entradas_tlb, entrada);
    }
}

int obtener_marco_tlb(t_tlb *tlb, int pid, int nro_pagina)
{

    if (!list_is_empty(tlb->entradas_tlb))
    {
        for (int i = 0; i < list_size(tlb->entradas_tlb); i++)
        {
            t_entrada_tlb *entrada = list_get(tlb->entradas_tlb, i);
            if (entrada->pid == pid && entrada->nro_pagina == nro_pagina)
            {
                if (tlb->algoritmo == LRU)
                {
                    entrada->last_reference = obtener_tiempo_en_miliSegundos(temporal_get_string_time("%H:%M:%S:%MS"));
                }
                return entrada->nro_marco;
            }
        }
    }

    return -1;
}

void imprimir_tlb(t_tlb *tlb)
{
    if (!list_is_empty(tlb->entradas_tlb))
    {
        printf_green("           TLB");
        printf_green("------------------------");
        printf_green("| PID | PAGINA | MARCO |");
        for (int i = 0; i < list_size(tlb->entradas_tlb); ++i)
        {
            t_entrada_tlb *entrada = list_get(tlb->entradas_tlb, i);
            printf_green("|  %d  |    %d   |   %d   |", entrada->pid, entrada->nro_pagina, entrada->nro_marco);
            printf_green("------------------------");
        }
    }
    else
    {
        printf_green("TLB VACIA\n");
    }
    return;
}

void tlb_iniciar(char *algoritmo, int cant_entradas)
{
    if (cant_entradas <= 0)
    {
        TLB_HABILITADA = 0; // false
        printf_purple("TLB DESHABILITADA\n");
    }
    else
    {
        if (strcmp(algoritmo, "FIFO") == 0)
        {
            tlb_cpu = crear_tlb(cant_entradas, FIFO);
        }
        else if (strcmp(algoritmo, "LRU") == 0)
        {
            tlb_cpu = crear_tlb(cant_entradas, LRU);
        }
        TLB_HABILITADA = 1; // true
        printf_purple("TLB HABILITADA\n");
        imprimir_tlb(tlb_cpu);
    }
}
void eliminar_tlb()
{
    list_destroy_and_destroy_elements(tlb_cpu->entradas_tlb, (void *)free);
    free(tlb_cpu);
}