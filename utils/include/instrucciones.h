#ifndef INSTRUCCIONES_H_
#define INSTRUCCIONES_H_

#include <stdlib.h>
#include <stdio.h>
#include "../../utils/src/utils.h"
#include <stdint.h>

typedef enum
{
    SET,
    MOV_IN,
    MOV_OUT,
    SUM,
    SUB,
    JNZ,
    RESIZE,
    COPY_STRING,
    WAIT,
    SIGNAL,
    IO_GEN_SLEEP,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_WRITE,
    IO_FS_TRUNCATE,
    IO_FS_READ,
    EXIT
} t_tipoDeInstruccion;

typedef struct
{
    t_tipoDeInstruccion codigo;
    // parametros que puede llegar a recibir la instruccion ( ej. mov ac bx son dos parametros)
    char *par1;
    char *par2;
    char *par3;
    char *par4;
    char *par5;
    //
} t_instruccion;

#endif
