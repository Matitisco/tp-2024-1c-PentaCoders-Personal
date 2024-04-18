#ifndef CPU_H_
#define CPU_H_

#include <stdlib.h>
#include <stdio.h>
#include <utils.h>
#include <sockets.c>
#include "../../utils/src/utils.h"
#include "../include/conexiones.h"

char *PUERTO_ESCUCHA_DISPATCH;
char *IP;
char *PUERTO_MEMORIA;
int CONEXION_A_MEMORIA;
t_config *config;
t_log *logger;

#endif