#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include <stdlib.h>
#include <stdio.h>
#include "../src/utils.h"
#include "../include/sockets.h"

void levantarServidor(t_log *logger, char *puerto, char *ip, char *nombreServidor);
void levantarCliente(int unaConexion, t_log *logger, char *nombreDelServidor, char *ip, char *puerto, char *mensaje);
#endif