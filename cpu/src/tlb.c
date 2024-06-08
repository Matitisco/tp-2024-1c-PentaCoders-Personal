#include "../include/tlb.h"

tlb_tabla *tlb;

void tlb_iniciar(char *algoritmo, int cant_entradas)
{
    if (tlb_cant_entradas <= 0)
    {
        log_info(logger, "TLB - DESHABILITADA");
    }
    else
    {
        tlb_crear(algoritmo, cant_entradas);
        log_info(logger, "TLB - ENTRADAS: <%d> ", tlb->cant_entradas);
    }
}

void tlb_crear(char *algoritmo, int cant_entradas)
{
    tlb = malloc(sizeof(tlb_tabla));

    tlb->algoritmo = *algoritmo;
    tlb->entradas = malloc(sizeof(tlb_entrada) * cant_entradas);
    tlb->cant_entradas = cant_entradas;

    for (int i = 0; i < cant_entradas; i++)
    {
        tabla_tlb->entradas[i] = NULL;
    }
}

int tlb_consultar_info_pagina(int pagina_buscada)
{

    for (int i = 0; i < tlb->cant_entradas; i++)
    {
        int pagina_tlb = tlb->entradas[i].pagina;

        if (pagina_buscada = pagina_tlb)
        {
            return tlb->entradas[i].marco; // TLB HIT
        }
    }

    return -1; // TLB MISS
}

void tlb_agregar_entrada(int pid, int pagina, int marco)
{
    int entrada_libre;
    tlb_entrada *entrada;
    if ((entrada_libre = entrada_obtener()) < 0) // no hay lugares libres
    {
        entrada = entrada_crear(pid, pagina, marco);
        tlb_reemplazar(entrada);
        free(entrada);
    }
    else // hay lugares disponibles
    {
        entrada = entrada_crear(pid, pagina, marco);
        tlb->entradas[entrada_libre] = entrada;
        free(entrada);
    }
}

void tlb_reemplazar(tlb_entrada *entrada)
{
    if (tlb->algoritmo == "FIFO")
    {
        log_info(logger, "TLB - REEMPLAZO POR FIFO");
        tlb_reemplazo_fifo(entrada);
    }
    else if (tlb->algoritmo == "LRU")
    {
        log_info(logger, "TLB - REEMPLAZO POR LRU");
        tlb_reemplazo_lru(entrada);
    }
}

void tlb_reemplazo_fifo(tlb_entrada *entrada) // IMPLEMENTAR
{
}

void tlb_reemplazo_lru(tlb_entrada *entrada) // IMPLEMENTAR
{
}

tlb_entrada *entrada_crear(int pid, int pagina, int marco)
{
    tlb_entrada *entrada_nueva = malloc(sizeof(tlb_entrada));
    entrada_nueva->pid = pid;
    entrada_nueva->pagina = pagina;
    entrada_nueva->marco = marco;
    return entrada_nueva;
}

int entrada_obtener()
{
    for (int i = 0; i < tlb->cant_entradas; i++)
    {
        if (tlb->entradas[i] == NULL)
        {
            return i;
        }
    }
    return -1;
}