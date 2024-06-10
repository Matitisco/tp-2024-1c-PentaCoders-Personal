#include "../include/tlb.h"

tlb_tabla *tlb;
int iterador_fifo;

void tlb_iniciar(char *algoritmo, int cant_entradas)
{
    if (cant_entradas <= 0)
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
        tlb->entradas[i].pid = NULL;
        tlb->entradas[i].pagina = NULL;
        tlb->entradas[i].marco = NULL;
        tlb->entradas[i].ultima_referencia = (char *)NULL;
    }

    iterador_fifo = 0;
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
    entrada_tipo entrada_tipo = entrada_obtener(pid, pagina, marco);
    tlb_entrada *entrada;

    if (entrada_tipo.pos_entrada < 0) // no hay lugares libres
    {
        entrada = entrada_crear(pid, pagina, marco);
        tlb_reemplazar(entrada);
        free(entrada);
    }
    else // hay lugares disponibles o ya esta la pagina cargada
    {

        if (entrada_tipo.estaba_cargada)
        {
            log_info(logger, "ESA ENTRADA SE ENCUENTRA CARGADA");
        }
        else
        {
            entrada = entrada_crear(pid, pagina, marco);
            tlb->entradas[entrada_tipo.pos_entrada].pid = entrada->pid;
            tlb->entradas[entrada_tipo.pos_entrada].pagina = entrada->pagina;
            tlb->entradas[entrada_tipo.pos_entrada].marco = entrada->marco;
            tlb->entradas[entrada_tipo.pos_entrada].ultima_referencia = entrada->ultima_referencia;
            free(entrada);
        }
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

void tlb_reemplazo_fifo(tlb_entrada *entrada)
{
    tlb->entradas[iterador_fifo].pid = entrada->pid;       // asigno la nueva entrada en el array
    tlb->entradas[iterador_fifo].pagina = entrada->pagina; // asigno la pagina
    tlb->entradas[iterador_fifo].marco = entrada->marco;   // asigno el marco

    iterador_fifo++;

    if (iterador_fifo == tlb->cant_entradas)
    {
        iterador_fifo = 0;
    }
    /*
    PID PAGINA MARCO INDICE
    0    1     10     0
    1    2     11     1
    0    3     14     2
    */
}

void tlb_reemplazo_lru(tlb_entrada *entrada)
{
    /*
 PID PAGINA MARCO INDICE ULT
 0    1     10     0      100
 1    2     11     1      150
 0    3     14     2      200
 0    3     14     3      90
 */
    int pos_entrada = 0;
    int tiempo_victima = obtener_tiempo_en_miliSegundos(tlb->entradas[0].ultima_referencia); // 100
    for (int i = 0; i < tlb->cant_entradas; i++)
    {
        int tiempo_posible_victima = obtener_tiempo_en_miliSegundos(tlb->entradas[i].ultima_referencia); // 90
        if (tiempo_posible_victima < tiempo_victima)
        {
            tiempo_victima = tiempo_posible_victima; // 90
            pos_entrada = i;
        }
    }
    tlb->entradas[pos_entrada].pid = entrada->pid;       // asigno la nueva entrada en el array
    tlb->entradas[pos_entrada].pagina = entrada->pagina; // asigno la pagina
    tlb->entradas[pos_entrada].marco = entrada->marco;
}

int obtener_tiempo_en_miliSegundos(char *tiempo)
{
    char **nueva_hora = string_split(tiempo, ":");

    int horas = atoi(nueva_hora[0]);
    int minutos = atoi(nueva_hora[1]) + horas * 60;
    int segundos = atoi(nueva_hora[2]) + minutos * 60;
    int miliSegundos = atoi(nueva_hora[3]) + segundos * 1000;

    string_array_destroy(nueva_hora);

    return miliSegundos;
}

tlb_entrada *entrada_crear(int pid, int pagina, int marco)
{
    tlb_entrada *entrada_nueva = malloc(sizeof(tlb_entrada));
    entrada_nueva->pid = pid;
    entrada_nueva->pagina = pagina;
    entrada_nueva->marco = marco;
    entrada_nueva->ultima_referencia = temporal_get_string_time("%H:%M:%S:%MS");
    return entrada_nueva;
}

entrada_tipo entrada_obtener(int pid, int pagina, int marco)
{
    entrada_tipo tipo;
    for (int i = 0; i < tlb->cant_entradas; i++)
    {
        if (tlb->entradas[i].pid == NULL && tlb->entradas[i].marco == NULL && tlb->entradas[i].marco == NULL && tlb->entradas[i].ultima_referencia == NULL) // hay lugar libre
        {
            tipo.pos_entrada = i;
            tipo.estaba_cargada = 0;
            return tipo; // retornamos la posicion que esta libre
        }
        else if (tlb->entradas[i].pid == pid && tlb->entradas[i].pagina == pagina && tlb->entradas[i].marco == marco) // consultiamos si hay una pagina que esta cargada
        {
            log_info(logger, "PID: <%d> - PAGINA: <%d> - MARCO: <%d> - ESTABA CARGADA EN TLB", pid, pagina, marco);
            tlb->entradas[i].ultima_referencia = temporal_get_string_time("%H:%M:%S:%MS"); // actualizamos la entrada con la hora actual
            tipo.pos_entrada = i;
            tipo.estaba_cargada = 1;
            return tipo;
        }
    }
    tipo.estaba_cargada = -1;
    tipo.pos_entrada = -1;
    return tipo;
}