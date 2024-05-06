#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "../include/sockets.h"
#include "serializacion.h"

void levantarServidor(t_log *logger, char *puerto, char *ip, char *nombreServidor);
int levantarCliente(t_log *logger, char *nombreDelServidor, char *ip, char *puerto, char *mensaje);
void iterator(char *value);
void iterador_interfaz(t_interfaz *interfaz);
#endif