#ifndef CICLOINSTRUCCION_H_
#define CICLOINSTRUCCION_H_

#include "cpu.h"

// INSTRUCCIONES

void exec_set(char *registro, uint32_t valor);
void exec_mov_in(char *datos, char *direccion);
void exec_mov_out(char *direccion, char *datos);
void exec_sum(char *destino, char *origen);
void exec_sub(char *destino, char *origen);
void exec_jnz(char *registro, uint32_t numeroInstruccion);
void exec_resize(char *tamanio, t_cde *contextoProceso);
void exec_copy_string(char *tamanio);
void exec_wait(char *recurso, t_cde *cde);
void exec_signal(char *recurso, t_cde *cde);
void exec_io_gen_sleep(char *nombre_interfaz, uint32_t unidades_trabajo);
void exec_io_stdin_read(char *interfaz, char *reg_direccion, char *reg_tamanio);
void exec_io_stdout_write(char *interfaz, char *reg_direccion, char *reg_tamanio);
void exec_io_fs_create();
void exec_io_fs_delete();
void exec_io_fs_truncate();
void exec_io_fs_write();
void exec_io_fs_read();
void exec_exit(t_cde *cde);

// AUXILIARES

uint32_t obtener_valor_origen(char *origen);

#endif