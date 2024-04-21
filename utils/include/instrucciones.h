#ifndef INSTRUCCIONES_H_
#define INSTRUCCIONES_H_

#include <stdlib.h>
#include <stdio.h>
#include "../../utils/src/utils.h"
#include <stdint.h>


typedef enum {
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
}tipoDeInstruccion;


#endif
