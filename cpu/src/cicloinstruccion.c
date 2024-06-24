#include "../include/cicloinstruccion.h"

t_registros *registros;

void exec_set(char *registro, uint32_t valor)
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

void exec_mov_in(char *datos, char *direccion, t_cde *cde)
{
    uint32_t direccion_logica = obtener_valor(direccion);
    uint32_t direccion_fisica = direccion_logica_a_fisica(direccion_logica);
    log_info(logger, "DIRECCION FISICA ENVIADA POR CPU: %u", direccion_fisica);
    enviar_op_code(socket_memoria, ACCESO_ESPACIO_USUARIO);
    enviar_op_code(socket_memoria, PEDIDO_LECTURA);
    enviar_op_code(socket_memoria, LECTURA_CPU);
    uint32_t tamanio = sizeof(direccion);

    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer, direccion_fisica);
    agregar_buffer_para_enterosUint32(buffer, cde->pid);
    agregar_buffer_para_enterosUint32(buffer, tamanio);

    // direccion que se envia a la memoria y con la cual esta debe buscar el valor y guardarlo
    enviar_buffer(buffer, socket_memoria);

    buffer->offset = 0;
    log_info(logger, "\033[38;2;255;105;180m Direccion fisica enviada: %u \033[0m", leer_buffer_enteroUint32(buffer));
    log_info(logger, "\033[38;2;255;105;180m PID enviada: %u \033[0m", leer_buffer_enteroUint32(buffer));
    log_info(logger, "\033[38;2;255;105;180m TAMANIO enviada: %u \033[0m", leer_buffer_enteroUint32(buffer));

    op_code lectura_memoria = recibir_op_code(socket_memoria);
    if (lectura_memoria == OK)
    {
        tipo_buffer *buffer_valor = recibir_buffer(socket_memoria);
        uint32_t valor = leer_buffer_enteroUint32(buffer_valor);
        exec_set(datos, valor);
        destruir_buffer(buffer_valor);
        log_info(logger, "PID: <%d> - Accion: <LEER> - Direccion Fisica: <%d> - Valor: <%d>", cde->pid, direccion_fisica, valor);
    }
    else
    {
        log_error(logger, "DIRECCION INCORRECTA");
        // exec_exit(cde); Ver si esto esta bien
    }
    destruir_buffer(buffer);
}

void exec_mov_out(char *direccion, char *datos, t_cde *cde)
{
    uint32_t reg_valor = obtener_valor(datos);
    uint32_t direccion_logica = obtener_valor(direccion);
    uint32_t direccion_fisica = direccion_logica_a_fisica(direccion_logica);

    enviar_op_code(socket_memoria, ACCESO_ESPACIO_USUARIO);
    enviar_op_code(socket_memoria, PEDIDO_ESCRITURA);
    enviar_op_code(socket_memoria, SOLICITUD_ESCRITURA_CPU);

    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer, direccion_fisica);
    agregar_buffer_para_enterosUint32(buffer, reg_valor);
    agregar_buffer_para_enterosUint32(buffer, cde->pid);
    agregar_buffer_para_enterosUint32(buffer, sizeof(reg_valor));
    enviar_buffer(buffer, socket_memoria);

    op_code escritura_memoria = recibir_op_code(socket_memoria);
    if (escritura_memoria == OK)
    {
        log_info(logger, "PID: <%d> - Accion: <ESCRIBIR> - Direccion Fisica: <%d> - Valor: <%d>", cde->pid, direccion_fisica, reg_valor);
    }
    else
    {
        log_error(logger, "DIRECCION INCORRECTA");
    }
    destruir_buffer(buffer);
}

void exec_sum(char *destino, char *origen)
{
    uint32_t valor_origen = obtener_valor(origen);
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
}

void exec_sub(char *destino, char *origen)
{
    uint32_t valor_origen = obtener_valor(origen);
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
}

void exec_jnz(char *registro, uint32_t numeroInstruccion, t_cde *cde)
{
    if (strcmp(registro, "AX") == 0)
    {
        if (registros->AX != 0)
        {
            cde->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "BX") == 0)
    {
        if (registros->BX != 0)
        {
            cde->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "CX") == 0)
    {
        if (registros->CX != 0)
        {
            cde->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "DX") == 0)
    {
        if (registros->DX != 0)
        {
            cde->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "EAX") == 0)
    {
        if (registros->EAX != 0)
        {
            cde->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "EBX") == 0)
    {
        if (registros->EBX != 0)
        {
            cde->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "ECX") == 0)
    {
        if (registros->ECX != 0)
        {
            cde->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "EDX") == 0)
    {
        if (registros->EDX != 0)
        {
            cde->PC = numeroInstruccion;
        }
    }
}

void exec_resize(char *tamanio, t_cde *cde)
{
    enviar_op_code(socket_memoria, RESIZE_EXTEND);
    uint32_t tamanioValor = atoi(tamanio);
    tipo_buffer *buffer = crear_buffer();

    agregar_buffer_para_enterosUint32(buffer, tamanioValor);
    agregar_cde_buffer(buffer, cde);

    enviar_buffer(buffer, socket_memoria);
    destruir_buffer(buffer);
    op_code resize_memoria = recibir_op_code(socket_memoria);
    if (resize_memoria == OUT_OF_MEMORY)
    {
        salida_exit = 0;
        enviar_op_code(socket_kernel_dispatch, OUT_OF_MEMORY);
        tipo_buffer *buffer_out_memory = crear_buffer();
        agregar_cde_buffer(buffer_out_memory, cde);
        enviar_buffer(buffer, socket_kernel_dispatch);
        destruir_buffer(buffer_out_memory);
    }
    else if (resize_memoria == RESIZE_EXITOSO)
    {
        return;
    }
}

void exec_copy_string(char *tamanio, t_cde *cde)
{
    uint32_t direccion_logica_SI = cde->registros->SI;

    uint32_t direccion_fisica_SI = direccion_logica_a_fisica(direccion_logica_SI);
    enviar_op_code(socket_memoria, PEDIDO_LECTURA);

    tipo_buffer *buffer_copy_string = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer_copy_string, direccion_fisica_SI);
    agregar_buffer_para_enterosUint32(buffer_copy_string, cde->pid);
    enviar_buffer(buffer_copy_string, socket_memoria);
    destruir_buffer(buffer_copy_string);

    tipo_buffer *buffer_lectura = recibir_buffer(socket_memoria);
    char *unString = leer_buffer_string(buffer_lectura);
    uint32_t tamanio_string = sizeof(atoi(tamanio));
    uint32_t direccion_logica_DI = cde->registros->DI;
    uint32_t direccion_fisica_DI = direccion_logica_a_fisica(direccion_logica_DI);
    enviar_op_code(socket_memoria, PEDIDO_ESCRITURA);
    enviar_op_code(socket_memoria, ESCRITURA_CPU);
    tipo_buffer *buffer_DI = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer_DI, direccion_fisica_DI);
    agregar_buffer_para_enterosUint32(buffer_DI, cde->pid);
    agregar_buffer_para_enterosUint32(buffer_DI, tamanio_string);
    agregar_buffer_para_string(buffer_DI, unString);
    enviar_buffer(buffer_DI, socket_memoria);
    destruir_buffer(buffer_DI);
}

void exec_wait(char *recurso, t_cde *cde)
{
    enviar_op_code(socket_kernel_dispatch, WAIT_RECURSO);

    tipo_buffer *buffer_recurso;
    buffer_recurso = crear_buffer();
    desalojo_wait = 1;
    agregar_cde_buffer(buffer_recurso, cde);
    agregar_buffer_para_string(buffer_recurso, recurso);
    enviar_buffer(buffer_recurso, socket_kernel_dispatch);

    destruir_buffer(buffer_recurso);
}

void exec_signal(char *recurso, t_cde *cde)
{
    enviar_op_code(socket_kernel_dispatch, SIGNAL_RECURSO);

    tipo_buffer *buffer_recurso;
    buffer_recurso = crear_buffer();
    desalojo_signal = 1;
    agregar_cde_buffer(buffer_recurso, cde);
    agregar_buffer_para_string(buffer_recurso, recurso);
    enviar_buffer(buffer_recurso, socket_kernel_dispatch);

    destruir_buffer(buffer_recurso);
}

void exec_io_stdin_read(char *interfaz, char *reg_direccion, char *reg_tamanio, t_cde *cde)
{
    interrupcion_io = 1;
    uint32_t direccion_logica = obtener_valor(reg_direccion);
    uint32_t direccion_fisica = direccion_logica_a_fisica(direccion_logica);
    uint32_t tamanio = sizeof(atoi(reg_tamanio));

    buffer_instruccion_io = crear_buffer();
    enviar_op_code(socket_kernel_dispatch, INSTRUCCION_INTERFAZ);

    agregar_buffer_para_enterosUint32(buffer_instruccion_io, SOLICITUD_INTERFAZ_STDIN);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, IO_STDIN_READ);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, tamanio);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, direccion_fisica);
    agregar_buffer_para_string(buffer_instruccion_io, interfaz);
}

void exec_io_stdout_write(char *interfaz, char *reg_direccion, char *reg_tamanio, t_cde *cde)
{
    interrupcion_io = 1;
    uint32_t direccion_logica = obtener_valor(reg_direccion);
    uint32_t direccion_fisica = direccion_logica_a_fisica(direccion_logica);
    uint32_t tamanio = sizeof(atoi(reg_tamanio));

    buffer_instruccion_io = crear_buffer();

    enviar_op_code(socket_kernel_dispatch, INSTRUCCION_INTERFAZ);

    agregar_buffer_para_enterosUint32(buffer_instruccion_io, SOLICITUD_INTERFAZ_STDOUT);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, IO_STDOUT_WRITE);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, tamanio);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, direccion_fisica);
    agregar_buffer_para_string(buffer_instruccion_io, interfaz);
}

void exec_io_gen_sleep(char *nombre_interfaz, uint32_t unidades_trabajo)
{
    interrupcion_io = 1;
    buffer_instruccion_io = crear_buffer();

    enviar_op_code(socket_kernel_dispatch, INSTRUCCION_INTERFAZ);

    agregar_buffer_para_enterosUint32(buffer_instruccion_io, SOLICITUD_INTERFAZ_GENERICA);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, IO_GEN_SLEEP);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, unidades_trabajo);
    agregar_buffer_para_string(buffer_instruccion_io, nombre_interfaz);
}

// IMPLEMENTAR PARA ENTREGA FINAL

void exec_io_fs_create() {}
void exec_io_fs_delete() {}
void exec_io_fs_truncate() {}
void exec_io_fs_write() {}
void exec_io_fs_read() {}

void exec_exit(t_cde *cde)
{
    salida_exit = 0;
    enviar_op_code(socket_kernel_dispatch, FINALIZAR_PROCESO);
    tipo_buffer *buffer = crear_buffer();
    agregar_cde_buffer(buffer, cde);
    enviar_buffer(buffer, socket_kernel_dispatch);
    destruir_buffer(buffer);
}

// FUNCIONES AUXILIARES

uint32_t obtener_valor_origen(char *origen, t_cde *cde)
{
    if (strcmp(origen, "AX") == 0)
    {
        return cde->registros->AX;
    }
    if (strcmp(origen, "BX") == 0)
    {
        return cde->registros->BX;
    }
    if (strcmp(origen, "CX") == 0)
    {
        return cde->registros->CX;
    }
    if (strcmp(origen, "SI") == 0)
    {
        return cde->registros->SI;
    }
    if (strcmp(origen, "DX") == 0)
    {
        return cde->registros->DX;
    }
    if (strcmp(origen, "DI") == 0)
    {
        return cde->registros->DI;
    }
    if (strcmp(origen, "EAX") == 0)
    {
        return cde->registros->EAX;
    }
    if (strcmp(origen, "EBX") == 0)
    {
        return cde->registros->EBX;
    }
    if (strcmp(origen, "ECX") == 0)
    {
        return cde->registros->ECX;
    }
    if (strcmp(origen, "EDX") == 0)
    {
        return cde->registros->EDX;
    }
    return 0;
}

uint32_t obtener_valor(char *origen)
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
    if (strcmp(origen, "SI") == 0)
    {
        return registros->SI;
    }
    if (strcmp(origen, "DX") == 0)
    {
        return registros->DX;
    }
    if (strcmp(origen, "DI") == 0)
    {
        return registros->DI;
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