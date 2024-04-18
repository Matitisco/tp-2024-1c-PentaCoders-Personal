#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdlib.h>
#include <stdio.h>
#include "../../utils/src/utils.h"
#include "../include/conexiones.h"

t_log *logger;
t_config *config;
char *PUERTO_ESCUCHA;
char *PUERTO_CPU;
char *PUERTO_MEMORIA;
char *IP;
int conexion_memoria;
int conexion_cpu;

#endif