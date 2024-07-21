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
void exec_io_fs_write(char *nombre_interfaz, char *nombre_archivo,  char *reg_direccion,char *reg_tamanio, char *puntero_archivo, t_cde *cde);
void exec_io_fs_create(char *nombre_interfaz, char *nombre_archivo, t_cde *cde);
void exec_io_fs_delete(char *nombre_interfaz, char *nombre_archivo,t_cde *cde);
void exec_io_fs_truncate(char *nombre_interfaz, char *nombre_archivo, char *reg_tamanio, t_cde *cde);
void exec_exit(t_cde *cde, motivoFinalizar motivo);
void exec_io_fs_read(char *nombre_interfaz, char *nombre_archivo,  char *reg_direccion,char *reg_tamanio, char *puntero_archivo, t_cde *cde);

// AUXILIARES

void* obtener_valor(char *origen);
uint32_t traducir_direccion_mmu(uint32_t direccion_logica);
int calcular_pagina(int direccion_logica);
void escribir_memoria(int socket_memoria, uint32_t direccion_fisica, int size, void *dato, int pid);
uint32_t escribir_dato_memoria(uint32_t direccion_logica, void *dato, int size, int pid);
uint32_t leer_dato_memoria(uint32_t direccion_logica, int size, int pid);
uint32_t leer_memoria(uint32_t direccion_fisica, int size, int pid);

#endif