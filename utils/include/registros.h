#ifndef REGISTROS_H_
#define REGISTROS_H_

#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "../../utils/include/instrucciones.h"
#include <stdint.h>

typedef struct
{
    // uint32_t PC;
    uint8_t AX;
    uint8_t BX;
    uint8_t CX;
    uint8_t DX;
    uint32_t EAX;
    uint32_t EBX;
    uint32_t ECX;
    uint32_t EDX;
    uint32_t SI;
    uint32_t DI;
} t_registros;
typedef struct
{
    uint32_t pid;
    uint32_t pc;
    t_registros *registro;
    t_list *lista_instrucciones;

} t_cde;



#endif