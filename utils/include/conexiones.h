#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include <stdlib.h>
#include <stdio.h>
#include "../src/utils.h"
#include "../include/sockets.h"

/* MENSAJE DE CPU A MEMORIA*/
typedef enum {
    PEDIDO_INSTRUCCION
}mensaje_cpu_mem;

/*MENSAJE DE CPU A KERNEL */
typedef enum {
    PEDIDO_PCB

}mensaje_cpu_kernel;





void levantarServidor(t_log *logger, char *puerto, char *ip, char *nombreServidor);
void levantarCliente(int unaConexion, t_log *logger, char *nombreDelServidor, char *ip, char *puerto, char *mensaje);
#endif