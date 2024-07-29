#include "../include/operaciones_es.h"

void realizar_operacion_gen(t_interfaz *interfaz)
{
    sleep_ms(interfaz->tiempo_unidad_trabajo);
    tipo_buffer *buffer_gen = recibir_buffer(conexion_kernel);
    t_tipoDeInstruccion instruccion = leer_buffer_enteroUint32(buffer_gen);
    int unidades_tiempo = leer_buffer_enteroUint32(buffer_gen);
    int pid = leer_buffer_enteroUint32(buffer_gen);
    destruir_buffer(buffer_gen);

    if (instruccion == IO_GEN_SLEEP)
    {
        sleep_ms(unidades_tiempo * interfaz->tiempo_unidad_trabajo);
        // log obligatorio
        log_info(logger, "PID: <%d> - Operacion: <IO_GEN_SLEEP>", pid);
    }
    else
    {
        log_error(logger, "ERROR - INSTRUCCION INCORRECTA");
    }

    estoy_libre = 1;
}

void enviar_a_memoria(int socket_memoria, uint32_t direccion_fisica, int size, void *dato, int pid)
{
    enviar_op_code(socket_memoria, ACCESO_ESPACIO_USUARIO);
    enviar_op_code(socket_memoria, PEDIDO_ESCRITURA);
    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer, direccion_fisica);
    agregar_buffer_para_enterosUint32(buffer, pid);
    agregar_buffer_para_enterosUint32(buffer, size);
    agregar_buffer_para_enterosUint32(buffer, STRING);
    agregar_buffer_para_string(buffer, dato);
    enviar_buffer(buffer, conexion_memoria);
    destruir_buffer(buffer);
    op_code escritura_memoria = recibir_op_code(socket_memoria);
    if (escritura_memoria != OK)
    {
        log_error(logger, "DIRECCION INCORRECTA");
    }
}

int calcular_marco(int dir_fisica, int tamanio_marco) { return floor(dir_fisica / tamanio_marco); }

uint32_t escribir_dato_memoria(uint32_t direccion_fisica, int tamanio_marco, void *dato, int size, int pid)
{
    uint32_t df_del_byte_inicial_dato = direccion_fisica;
    uint32_t df_del_byte_final_dato = direccion_fisica + size - 1;

    int marco_del_byte_inicial = calcular_marco(df_del_byte_inicial_dato, tamanio_marco);
    int marco_del_byte_final = calcular_marco(df_del_byte_final_dato, tamanio_marco);

    if (marco_del_byte_inicial == marco_del_byte_final)
    {
        // Si el dato cabe en una sola página, se escribe directamente.
        if (direccion_fisica == -1)
        {
            log_error(logger, "DIRECCION INCORRECTA");
            // interrupcion_exit = 1;
            return -1;
        }
        enviar_a_memoria(conexion_memoria, direccion_fisica, size, dato, pid);
    }
    else
    {
        // Si el dato cruza el límite de la página, se divide la escritura.
        // Leo 1 mas por cada iteracion
        uint32_t df_byte_final_marco = (marco_del_byte_inicial + 1) * tamanio_marco - 1;
        int cant_bytes = df_byte_final_marco - df_del_byte_inicial_dato + 1;

        // Escribir la primera parte del dato que cabe en la página actual.

        enviar_a_memoria(conexion_memoria, direccion_fisica, cant_bytes, dato, pid);
        // Llamada recursiva para escribir el resto del dato en la siguiente página.
        // Nota: se usar (char*) por una cuestion aritmetica de punteros, para sumar de a bytes
        escribir_dato_memoria(df_byte_final_marco + 1, tamanio_marco, dato + cant_bytes, size - cant_bytes, pid);
    }

    return direccion_fisica;
}

void realizar_operacion_stdin(t_interfaz *interfaz)
{
    sleep_ms(interfaz->tiempo_unidad_trabajo);
    tipo_buffer *buffer_stdin = recibir_buffer(conexion_kernel);
    t_tipoDeInstruccion instruccion = leer_buffer_enteroUint32(buffer_stdin);
    int tamanio = leer_buffer_enteroUint32(buffer_stdin);
    int tamanio_marco = leer_buffer_enteroUint32(buffer_stdin);
    int direccion_fisica = leer_buffer_enteroUint32(buffer_stdin);
    int pid = leer_buffer_enteroUint32(buffer_stdin);
    destruir_buffer(buffer_stdin);

    if (instruccion == IO_STDIN_READ)
    {
        char *texto_ingresado = readline("Ingrese un texto por teclado: ");
        char *texto_truncado = truncar_texto(texto_ingresado, tamanio);

        escribir_dato_memoria(direccion_fisica, tamanio_marco, texto_truncado, tamanio, pid);
        log_info(logger, "PID: <%d> - Operacion: <IO_STDIN_READ>", pid);
    }
    else
    {
        log_error(logger, "ERROR - INSTRUCCION INCORRECTA");
    }
    estoy_libre = 1;
}

void realizar_operacion_stdout(t_interfaz *interfaz)
{
    sleep_ms(interfaz->tiempo_unidad_trabajo);
    tipo_buffer *buffer_sol_operacion = recibir_buffer(conexion_kernel);
    t_tipoDeInstruccion sol_operacion = leer_buffer_enteroUint32(buffer_sol_operacion);
    int tamanio = leer_buffer_enteroUint32(buffer_sol_operacion);
    int direccion_fisica = leer_buffer_enteroUint32(buffer_sol_operacion);
    int pid = leer_buffer_enteroUint32(buffer_sol_operacion);
    log_info(logger, "TAMANIO : %d", tamanio);
    if (sol_operacion == IO_STDOUT_WRITE)
    {
        enviar_op_code(conexion_memoria, ACCESO_ESPACIO_USUARIO);
        enviar_op_code(conexion_memoria, PEDIDO_LECTURA);

        tipo_buffer *buffer_a_memoria = crear_buffer();
        agregar_buffer_para_enterosUint32(buffer_a_memoria, direccion_fisica);
        agregar_buffer_para_enterosUint32(buffer_a_memoria, pid);
        agregar_buffer_para_enterosUint32(buffer_a_memoria, tamanio);
        agregar_buffer_para_enterosUint32(buffer_a_memoria, STRING);
        enviar_buffer(buffer_a_memoria, conexion_memoria);

        destruir_buffer(buffer_a_memoria);

        op_code codigo_memoria = recibir_op_code(conexion_memoria);
        if (codigo_memoria == OK)
        {
            char *texto_recibido = calloc(1, tamanio);
            tipo_buffer *lectura = recibir_buffer(conexion_memoria);
            texto_recibido = leer_buffer_string(lectura);
            log_info(logger, "Texto hallado: %s", texto_recibido);
            destruir_buffer(lectura);

            log_info(logger, "PID: <%d> - Operacion: <IO_STDOUT_WRITE>", pid);
        }
        else if (codigo_memoria == ERROR_PEDIDO_LECTURA)
        {
            log_error(logger, "PID: <%d> - ERROR Operacion: <IO_STDOUT_WRITE>", pid);
        }
    }
    else
    {
        log_info(logger, "Hubo un error al traer la operacion IO_STDOUT_WRITE");
    }
    estoy_libre = 1;
    destruir_buffer(buffer_sol_operacion);
}

void realizar_operacion_dialfs(t_interfaz *interfaz)
{
    sleep_ms(interfaz->tiempo_unidad_trabajo);
    instrucciones_dialfs();
    estoy_libre = 1;
}

// AUXILIARES

void truncar_valor(char **valor_nuevo, char *valor_viejo, int limitante)
{
    *valor_nuevo = malloc(limitante * sizeof(char));

    if (*valor_nuevo == NULL)
    {
        return;
    }

    for (int i = 0; i < limitante; i++)
    {
        (*valor_nuevo)[i] = valor_viejo[i];
    }
    (*valor_nuevo)[limitante] = '\0';
    if (valor_viejo != NULL && valor_viejo[0] != '\0')
    {
        free(valor_viejo);
    }
}

char *truncar_texto(char *texto_ingresado, int tamanio)
{
    char *nuevo_texto;
    truncar_valor(&nuevo_texto, texto_ingresado, tamanio);
    return nuevo_texto;
}