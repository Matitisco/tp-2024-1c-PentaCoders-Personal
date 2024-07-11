#ifndef CICLOINSTRUCCION_H_
#define CICLOINSTRUCCION_H_

#include "cpu.h"

// INSTRUCCIONES

void exec_set(char *registro, uint32_t valor);
void exec_mov_in(char *datos, char *direccion, t_cde *cde);
void exec_mov_out(char *direccion, char *datos, t_cde *cde);
void exec_sum(char *destino, char *origen);
void exec_sub(char *destino, char *origen);
void exec_jnz(char *registro, uint32_t numeroInstruccion, t_cde *cde);
void exec_resize(char *tamanio, t_cde *cde);
void exec_copy_string(char *tamanio, t_cde *cde);
void exec_wait(char *recurso, t_cde *cde);
void exec_signal(char *recurso, t_cde *cde);
void exec_io_gen_sleep(char *nombre_interfaz, uint32_t unidades_trabajo);
void exec_io_stdin_read(char *interfaz, char *reg_direccion, char *reg_tamanio, t_cde *cde);
void exec_io_stdout_write(char *interfaz, char *reg_direccion, char *reg_tamanio, t_cde *cde);
void exec_io_fs_create(char *nombre_interfaz, char *nombre_archivo,t_cde *cde);
void exec_io_fs_delete(char *nombre_interfaz, char *nombre_archivo,t_cde *cde);
void exec_io_fs_truncate(char *nombre_interfaz, char *nombre_archivo, char *reg_tamanio, t_cde *cde);
void exec_exit(t_cde *cde);
void exec_io_fs_write(char *nombre_interfaz, char *nombre_archivo, char *reg_tamanio, char *reg_direccion, uint32_t puntero_archivo, t_cde *cde);
void exec_io_fs_read(char *nombre_interfaz, char *nombre_archivo, char *reg_tamanio, char *reg_direccion, char *puntero_archivo, t_cde *cde);

// AUXILIARES

uint32_t obtener_valor_origen(char *origen, t_cde *cde);
uint32_t obtener_valor(char *origen);
uint32_t direccion_logica_a_fisica(int direccion_logica);

#endif