#ifndef DICCIONARIO_H_
#define DICCIONARIO_H_
#include "../include/cpu.h"

void iniciar_dic_instrucciones();
// char *registro_a_char(t_registros *registro);
uint32_t obtener_valor_origen(char *origen);
void exec_set(char *registro, uint32_t valor);
void exec_mov_in();
void exec_mov_out();
void exec_sum(char *destino, char *origen);
void exec_sub(char *destino, char* origen);
void exec_jnz(char *registro, uint32_t numeroInstruccion);
void exec_resize();
void exec_copy_string();
void exec_wait();
void exec_signal();
void exec_io_gen_sleep();
void exec_io_stdin_read();
void exec_io_stdout_write();
void exec_io_fs_create();
void exec_io_fs_delete();
void exec_io_fs_truncate();
void exec_io_fs_write();
void exec_io_fs_read();
void exec_exit();
#endif