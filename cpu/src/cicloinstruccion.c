#include "../include/cicloinstruccion.h"

t_registros *registros;

void exec_set(char *registro, uint32_t valor)
{
    if (strcmp(registro, "AX") == 0)
    {
        registros->AX = valor;
    }
    else if (strcmp(registro, "BX") == 0)
    {
        registros->BX = valor;
    }
    else if (strcmp(registro, "CX") == 0)
    {
        registros->CX = valor;
    }
    else if (strcmp(registro, "DX") == 0)
    {
        registros->DX = valor;
    }
    else if (strcmp(registro, "EAX") == 0)
    {
        registros->EAX = valor;
    }
    else if (strcmp(registro, "EBX") == 0)
    {
        registros->EBX = valor;
    }
    else if (strcmp(registro, "ECX") == 0)
    {
        registros->ECX = valor;
    }
    else if (strcmp(registro, "EDX") == 0)
    {
        registros->EDX = valor;
    }
    else if (strcmp(registro, "PC") == 0)
    {
        cde_recibido->PC = 0;
    }
    else if (strcmp(registro, "DI") == 0)
    {
        registros->DI = valor;
    }
    else if (strcmp(registro, "SI") == 0)
    {
        registros->SI = valor;
    }
    else
    {
        log_error(logger, "No es un registro o el PC");
    }
}

/*

4/16 = 0,25 -> Una Pagina
32/16 = 2 -> Dos Paginas

SOLUCION: La cpu debe generar tantas direcciones fisicas como se requieran
luego estas se van enviando de a una a la memoria para que esta escriba la informacion en los blqoues correspondientes
idem interfaces.

*/

double obtenerMantisa(double x)
{
    int exponente;
    double mantisa = frexp(x, &exponente);
    return mantisa;
}

¿

void exec_mov_in(char *datos, char *direccion, char *tamanio, t_cde *cde)
{
    uint32_t direccion_logica = obtener_valor(direccion);
    uint32_t direccion_fisica = traducir_direccion_mmu(direccion_logica); //1

    // enviamos la primera direccion fisica encontrada y guardamos lo que retorna
    
    /* float posicion_pagina = direccion_logica / tamanio_pagina; //2

    double itamanio = atof(tamanio);
    float paginas_necesarias = ceil(itamanio / tamanio_pagina);
    log_info_(logger, "Paginas necesarias para guardar la informacion: %d", paginas_necesarias);


    for (int i = 0; i++; i < paginas_necesarias)
    {
        //enviar()
        //guardar()
    } */
    

    if (direccion_fisica == -1)
    {
        log_error(logger, "DIRECCION INCORRECTA");
        // interrupcion_exit = 1;
        return;
    }
    log_info(logger, "DIRECCION FISICA ENVIADA POR CPU: %u", direccion_fisica);
    enviar_op_code(socket_memoria, ACCESO_ESPACIO_USUARIO);
    enviar_op_code(socket_memoria, PEDIDO_LECTURA);

    uint32_t int_tamanio = atoi(tamanio);
    log_info(logger, "TAMANIO A ENVIAR %d", int_tamanio);
    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer, direccion_fisica);
    agregar_buffer_para_enterosUint32(buffer, cde->pid);
    agregar_buffer_para_enterosUint32(buffer, int_tamanio);
    agregar_buffer_para_enterosUint32(buffer, INTEGER);

    // direccion que se envia a la memoria y con la cual esta debe buscar el valor y guardarlo
    enviar_buffer(buffer, socket_memoria);

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
    uint32_t direccion_fisica = traducir_direccion_mmu(direccion_logica);
    if (direccion_fisica == -1)
    {
        log_error(logger, "DIRECCION INCORRECTA");
        interrupcion_exit = 1;
        return;
    }

    enviar_op_code(socket_memoria, ACCESO_ESPACIO_USUARIO);
    enviar_op_code(socket_memoria, PEDIDO_ESCRITURA);
    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer, direccion_fisica);
    agregar_buffer_para_enterosUint32(buffer, cde->pid);
    agregar_buffer_para_enterosUint32(buffer, sizeof(reg_valor));
    agregar_buffer_para_enterosUint32(buffer, INTEGER);
    agregar_buffer_para_enterosUint32(buffer, reg_valor);
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
            registros->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "BX") == 0)
    {
        if (registros->BX != 0)
        {
            registros->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "CX") == 0)
    {
        if (registros->CX != 0)
        {
            registros->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "DX") == 0)
    {
        if (registros->DX != 0)
        {
            registros->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "EAX") == 0)
    {
        if (registros->EAX != 0)
        {
            registros->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "EBX") == 0)
    {
        if (registros->EBX != 0)
        {
            registros->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "ECX") == 0)
    {
        if (registros->ECX != 0)
        {
            registros->PC = numeroInstruccion;
        }
    }
    if (strcmp(registro, "EDX") == 0)
    {
        if (registros->EDX != 0)
        {
            registros->PC = numeroInstruccion;
        }
    }
}

void exec_resize(char *tamanio, t_cde *cde)
{
    enviar_op_code(socket_memoria, RESIZE_OP);
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
    // LEER STRING

    uint32_t direccion_logica_SI = registros->SI; // SET SI 0
    uint32_t direccion_fisica_SI = traducir_direccion_mmu(direccion_logica_SI);
    log_info(logger, "DIRECCION FISICA ENVIADA POR CPU: %u", direccion_fisica_SI);

    enviar_op_code(socket_memoria, ACCESO_ESPACIO_USUARIO);
    enviar_op_code(socket_memoria, PEDIDO_LECTURA);

    tipo_buffer *buffer_copy_string = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer_copy_string, direccion_fisica_SI);
    agregar_buffer_para_enterosUint32(buffer_copy_string, cde->pid);
    agregar_buffer_para_enterosUint32(buffer_copy_string, atoi(tamanio));
    agregar_buffer_para_enterosUint32(buffer_copy_string, STRING);

    enviar_buffer(buffer_copy_string, socket_memoria);
    destruir_buffer(buffer_copy_string);
    char *valor = string_new();
    op_code lectura_memoria = recibir_op_code(socket_memoria);
    if (lectura_memoria == OK)
    {
        tipo_buffer *buffer_valor = recibir_buffer(socket_memoria);
        valor = leer_buffer_string(buffer_valor);
        destruir_buffer(buffer_valor);
        log_info(logger, "PID: <%d> - Accion: <LEER> - Direccion Fisica: <%d> - Valor: <%s>", cde->pid, direccion_fisica_SI, valor);
        log_info(logger, "Cadena recibida %s", valor);
    }
    else
    {
        log_error(logger, "DIRECCION INCORRECTA");
    }

    // ESCRIBIR STRING LEIDO

    uint32_t tamanio_string = atoi(tamanio);
    uint32_t direccion_logica_DI = registros->DI;
    uint32_t direccion_fisica_DI = traducir_direccion_mmu(direccion_logica_DI);
    enviar_op_code(socket_memoria, ACCESO_ESPACIO_USUARIO);
    enviar_op_code(socket_memoria, PEDIDO_ESCRITURA);
    tipo_buffer *buffer_DI = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer_DI, direccion_fisica_DI);
    agregar_buffer_para_enterosUint32(buffer_DI, cde->pid);
    agregar_buffer_para_enterosUint32(buffer_DI, tamanio_string);
    agregar_buffer_para_enterosUint32(buffer_DI, STRING);
    agregar_buffer_para_string(buffer_DI, valor);
    enviar_buffer(buffer_DI, socket_memoria);

    op_code escritura_memoria = recibir_op_code(socket_memoria);
    if (escritura_memoria == OK)
    {
        log_info(logger, "PID: <%d> - Accion: <ESCRIBIR> - Direccion Fisica: <%d> - Valor: <%s>", cde->pid, direccion_fisica_DI, valor);
    }
    else
    {
        log_error(logger, "DIRECCION INCORRECTA");
    }
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
    uint32_t direccion_fisica = traducir_direccion_mmu(direccion_logica);
    uint32_t tamanio = obtener_valor(reg_tamanio);

    buffer_instruccion_io = crear_buffer();
    enviar_op_code(socket_kernel_dispatch, INSTRUCCION_INTERFAZ);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, SOLICITUD_INTERFAZ_STDIN);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, IO_STDIN_READ);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, tamanio);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, direccion_fisica);
    agregar_buffer_para_string(buffer_instruccion_io, interfaz);
    enviar_buffer(buffer_instruccion_io, socket_kernel_dispatch);
}

void exec_io_stdout_write(char *interfaz, char *reg_direccion, char *reg_tamanio, t_cde *cde)
{
    interrupcion_io = 1;
    uint32_t direccion_logica = obtener_valor(reg_direccion);
    uint32_t direccion_fisica = traducir_direccion_mmu(direccion_logica);
    uint32_t tamanio = obtener_valor(reg_tamanio);

    buffer_instruccion_io = crear_buffer();

    enviar_op_code(socket_kernel_dispatch, INSTRUCCION_INTERFAZ);

    agregar_buffer_para_enterosUint32(buffer_instruccion_io, SOLICITUD_INTERFAZ_STDOUT);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, IO_STDOUT_WRITE);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, tamanio);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, direccion_fisica);
    agregar_buffer_para_string(buffer_instruccion_io, interfaz);
    enviar_buffer(buffer_instruccion_io, socket_kernel_dispatch);
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
    enviar_buffer(buffer_instruccion_io, socket_kernel_dispatch);
}

void exec_io_fs_create(char *nombre_interfaz, char *nombre_archivo, t_cde *cde)
{
    buffer_instruccion_io = crear_buffer();
    enviar_op_code(socket_kernel_dispatch, INSTRUCCION_INTERFAZ);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, SOLICITUD_INTERFAZ_DIALFS); // enviar como buffer para que no haya problemas de sincronizacion
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, IO_FS_CREATE);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, cde->pid);
    agregar_buffer_para_string(buffer_instruccion_io, nombre_interfaz);
    agregar_buffer_para_string(buffer_instruccion_io, nombre_archivo);
    enviar_buffer(buffer_instruccion_io, socket_kernel_dispatch);
}

void exec_io_fs_delete(char *nombre_interfaz, char *nombre_archivo, t_cde *cde)
{
    buffer_instruccion_io = crear_buffer();
    enviar_op_code(socket_kernel_dispatch, INSTRUCCION_INTERFAZ);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, SOLICITUD_INTERFAZ_DIALFS);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, IO_FS_DELETE);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, cde->pid);
    agregar_buffer_para_string(buffer_instruccion_io, nombre_interfaz);
    agregar_buffer_para_string(buffer_instruccion_io, nombre_archivo);
    enviar_buffer(buffer_instruccion_io, socket_kernel_dispatch);
}

void exec_io_fs_truncate(char *nombre_interfaz, char *nombre_archivo, char *reg_tamanio, t_cde *cde)
{
    buffer_instruccion_io = crear_buffer();
    enviar_op_code(socket_kernel_dispatch, INSTRUCCION_INTERFAZ);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, SOLICITUD_INTERFAZ_DIALFS);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, IO_FS_TRUNCATE);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, cde->pid);
    uint32_t tamanio = atoi(reg_tamanio);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, tamanio);
    agregar_buffer_para_string(buffer_instruccion_io, nombre_interfaz);
    agregar_buffer_para_string(buffer_instruccion_io, nombre_archivo);
    enviar_buffer(buffer_instruccion_io, socket_kernel_dispatch);
}

void exec_io_fs_write(char *nombre_interfaz, char *nombre_archivo, char *reg_tamanio, char *reg_direccion, uint32_t puntero_archivo, t_cde *cde)
{
    uint32_t direccion_logica = obtener_valor_origen(reg_direccion, cde);
    uint32_t direccion_fisica = traducir_direccion_mmu(direccion_logica);
    buffer_instruccion_io = crear_buffer();
    enviar_op_code(socket_kernel_dispatch, INSTRUCCION_INTERFAZ);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, SOLICITUD_INTERFAZ_DIALFS);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, IO_FS_WRITE);
    uint32_t tamanio = atoi(reg_tamanio);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, tamanio);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, direccion_fisica);
    uint32_t puntero = obtener_valor_origen(puntero_archivo, cde);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, puntero_archivo);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, cde->pid);
    agregar_buffer_para_string(buffer_instruccion_io, nombre_interfaz);
    agregar_buffer_para_string(buffer_instruccion_io, nombre_archivo);

    enviar_buffer(buffer_instruccion_io, socket_kernel_dispatch);
}

void exec_io_fs_read(char *nombre_interfaz, char *nombre_archivo, char *reg_tamanio, char *reg_direccion, char *puntero_archivo, t_cde *cde)
{
    uint32_t direccion_logica = obtener_valor_origen(reg_direccion, cde);
    uint32_t direccion_fisica = traducir_direccion_mmu(direccion_logica);
    buffer_instruccion_io = crear_buffer();
    enviar_op_code(socket_kernel_dispatch, INSTRUCCION_INTERFAZ);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, SOLICITUD_INTERFAZ_DIALFS);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, IO_FS_WRITE);
    uint32_t tamanio = atoi(reg_tamanio);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, tamanio);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, direccion_fisica);
    uint32_t puntero = obtener_valor_origen(puntero_archivo, cde);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, puntero);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, cde->pid);
    agregar_buffer_para_string(buffer_instruccion_io, nombre_interfaz);
    agregar_buffer_para_string(buffer_instruccion_io, nombre_archivo);
    enviar_buffer(buffer_instruccion_io, socket_kernel_dispatch);
}

void exec_exit(t_cde *cde, motivoFinalizar motivo)
{
    salida_exit = 0;
    enviar_op_code(socket_kernel_dispatch, FINALIZAR_PROCESO);
    tipo_buffer *buffer = crear_buffer();
    agregar_cde_buffer(buffer, cde);
    agregar_buffer_para_enterosUint32(buffer, motivo);
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