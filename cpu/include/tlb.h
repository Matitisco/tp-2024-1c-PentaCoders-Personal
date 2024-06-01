#ifndef TLB_H_
#define TLB_H_

#include "cpu.h"

typedef struct{
    int pid;
    int pagina;
    int marco;
}t_tlb;

#endif