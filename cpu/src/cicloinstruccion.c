#include "../include/cicloinstruccion.h"

// SET
void exec_set(char *registro, uint32_t valor) //
{
    if (strcmp(registro, "AX") == 0)
    {
        registros->AX = valor;
    }
    if (strcmp(registro, "BX") == 0)
    {
        registros->BX = valor;
    }
    if (strcmp(registro, "CX") == 0)
    {
        registros->CX = valor;
    }
    if (strcmp(registro, "DX") == 0)
    {
        registros->DX = valor;
    }
    if (strcmp(registro, "EAX") == 0)
    {
        registros->EAX = valor;
    }
    if (strcmp(registro, "EBX") == 0)
    {
        registros->EBX = valor;
    }
    if (strcmp(registro, "ECX") == 0)
    {
        registros->ECX = valor;
    }
    if (strcmp(registro, "EDX") == 0)
    {
        registros->EDX = valor;
    }
}
void exec_mov_in() {}
void exec_mov_out() {}
// SUM
void exec_sum(char *destino, char *origen)
{
    uint32_t valor_origen = obtener_valor_origen(origen);
    if (strcmp(destino, "AX") == 0)
    {
        registros->AX = registros->AX + valor_origen;
    }
    if (strcmp(destino, "BX") == 0)
    {
        registros->BX = registros->BX + valor_origen;
    }
    if (strcmp(destino, "CX") == 0)
    {
        registros->CX = registros->CX + valor_origen;
    }
    if (strcmp(destino, "DX") == 0)
    {
        registros->DX = registros->DX + valor_origen;
    }
    if (strcmp(destino, "EAX") == 0)
    {
        registros->EAX = registros->EAX + valor_origen;
    }
    if (strcmp(destino, "EBX") == 0)
    {
        registros->EBX = registros->EBX + valor_origen;
    }
    if (strcmp(destino, "ECX") == 0)
    {
        registros->ECX = registros->ECX + valor_origen;
    }
    if (strcmp(destino, "EDX") == 0)
    {
        registros->EDX = registros->EDX + valor_origen;
    }
    /*Aclaracion origen no tiene que ser un puntero ya que no se modifica*/
}
// SUB
void exec_sub(char *destino, char *origen)
{
    uint32_t valor_origen = obtener_valor_origen(origen);
    if (strcmp(destino, "AX") == 0)
    {
        registros->AX = registros->AX - valor_origen;
    }
    if (strcmp(destino, "BX") == 0)
    {
        registros->BX = registros->BX - valor_origen;
    }
    if (strcmp(destino, "CX") == 0)
    {
        registros->CX = registros->CX - valor_origen;
    }
    if (strcmp(destino, "DX") == 0)
    {
        registros->DX = registros->DX - valor_origen;
    }
    if (strcmp(destino, "EAX") == 0)
    {
        registros->EAX = registros->EAX - valor_origen;
    }
    if (strcmp(destino, "EBX") == 0)
    {
        registros->EBX = registros->EBX - valor_origen;
    }
    if (strcmp(destino, "ECX") == 0)
    {
        registros->ECX = registros->ECX - valor_origen;
    }
    if (strcmp(destino, "EDX") == 0)
    {
        registros->EDX = registros->EDX - valor_origen;
    }
    /*Aclaracion origen no tiene que ser un puntero ya que no se modifica*/
}
// JNZ
void exec_jnz(char *registro, uint32_t numeroInstruccion)
{
    t_cde *cde = malloc(sizeof(t_cde)); // esto es temporal para poder definir las instuurcciones
    if (strcmp(registro, "AX") == 0)
    {
        if (registros->AX != 0)
        {
            cde->registro->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "BX") == 0)
    {
        if (registros->BX != 0)
        {
            cde->registro->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "CX") == 0)
    {
        if (registros->CX != 0)
        {
            cde->registro->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "DX") == 0)
    {
        if (registros->DX != 0)
        {
            cde->registro->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "EAX") == 0)
    {
        if (registros->EAX != 0)
        {
            cde->registro->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "EBX") == 0)
    {
        if (registros->EBX != 0)
        {
            cde->registro->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "ECX") == 0)
    {
        if (registros->ECX != 0)
        {
            cde->registro->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "EDX") == 0)
    {
        if (registros->EDX != 0)
        {
            cde->registro->PC = numeroInstruccion;
        }
    }
}
// RESIZE
void exec_resize() {}
// COPY_STRING
void exec_copy_string() {}
// WAIT
void exec_wait() {}
// SIGNAL
void exec_signal() {}
// IO_GEN_SLEEP
void exec_io_gen_sleep(char *nombre_interfaz, uint32_t unidades_trabajo)
{
    int socket_kernel; // declaracion momentania
    enviar_cod_enum(socket_kernel, SOLICITUD_INTERFAZ_GENERICA);
    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_string(buffer, nombre_interfaz);
    agregar_buffer_para_enterosUint32(buffer, unidades_trabajo);
    enviar_buffer(buffer, socket_kernel); // falta definir quien es cliente_kernel
}
// IO_STDIN_READ
void exec_io_stdin_read() {}
void exec_io_stdout_write() {}
void exec_io_fs_create() {}
void exec_io_fs_delete() {}
void exec_io_fs_truncate() {}
void exec_io_fs_write() {}
void exec_io_fs_read() {}
// EXIT
void exec_exit()
{
    /*EXIT: Esta instrucción representa la syscall de finalización del proceso. Se deberá devolver el
Contexto de Ejecución actualizado al Kernel para su finalización.*/
    int socket_kernel;
    enviar_cod_enum(socket_kernel, FINALIZAR_PROCESO);
    // cuando enviamos el finalizar_proceso, el kernel debe enviar el proceso a exit
    tipo_buffer *buffer = crear_buffer();
    t_cde *cde = malloc(sizeof(t_cde));
    // suponemos que el cde es global
    agregar_buffer_para_enterosUint32(buffer, cde->pid);
    agregar_buffer_para_enterosUint32(buffer, cde->registro->PC);
    enviar_buffer(buffer, socket_kernel);
}
// OBTENER VALORES
uint32_t obtener_valor_origen(char *origen)
{
    if (strcmp(origen, "AX") == 0)
    {
        return registros->AX;
    }
    if (strcmp(origen, "BX") == 0)
    {
        return registros->BX;
    }
    if (strcmp(origen, "CX") == 0)
    {
        return registros->CX;
    }
    if (strcmp(origen, "DX") == 0)
    {
        return registros->DX;
    }
    if (strcmp(origen, "EAX") == 0)
    {
        return registros->EAX;
    }
    if (strcmp(origen, "EBX") == 0)
    {
        return registros->EBX;
    }
    if (strcmp(origen, "ECX") == 0)
    {
        return registros->ECX;
    }
    if (strcmp(origen, "EDX") == 0)
    {
        return registros->EDX;
    }
    return 0;
}