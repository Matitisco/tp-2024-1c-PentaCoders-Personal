#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include <stdlib.h>
#include <stdio.h>

#include "utils.h"
#include "sockets.h"
#include "serializacion.h"

int levantarCliente(t_log *logger, char *nombreDelServidor, char *ip, char *puerto);
#endif