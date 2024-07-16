#include "../include/operaciones_es.h"

/*
Cada interfaz en la vida real tiene diferentes velocidades,
por lo que para simplificar esto en nuestro TP vamos a tener en
la configuración del módulo el Tiempo de unidad de trabajo, este valor
luego se va a multiplicar por otro valor que va a estar dado según el
tipo de interfaz que tengamos, en este TP vamos a trabajar con 4 tipos
de Interfaces: Genéricas, STDIN, STDOUT y DialFS.
*/

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
        log_info(logger, "PID: <%d> - Operacion: <IO_GEN_SLEEP>", pid);
    }
    else
    {
        log_error(logger, "ERROR - INSTRUCCION INCORRECTA");
    }

    estoy_libre = 1;
}

void realizar_operacion_stdin(t_interfaz *interfaz)
{
    sleep_ms(interfaz->tiempo_unidad_trabajo);
    tipo_buffer *buffer_stdin = recibir_buffer(conexion_kernel);
    t_tipoDeInstruccion instruccion = leer_buffer_enteroUint32(buffer_stdin);
    int tamanio = leer_buffer_enteroUint32(buffer_stdin);
    int direccion_fisica = leer_buffer_enteroUint32(buffer_stdin);
    log_info(logger, "DIRECCIONES FISICAS %d", direccion_fisica);
    int pid = leer_buffer_enteroUint32(buffer_stdin);
    log_info(logger, "CANT DE BYTES A COPIAR: %d", tamanio);
    destruir_buffer(buffer_stdin);

    if (instruccion == IO_STDIN_READ)
    {
        char *texto_ingresado = readline("Ingrese un texto por teclado: ");
        char *texto_truncado = truncar_texto(texto_ingresado, tamanio);
        log_info(logger, "TEXTO A ENVIAR A MEMORIA : %s", texto_truncado);

        enviar_op_code(conexion_memoria, ACCESO_ESPACIO_USUARIO);
        enviar_op_code(conexion_memoria, PEDIDO_ESCRITURA);
        tipo_buffer *buffer_stdin = crear_buffer();
        agregar_buffer_para_enterosUint32(buffer_stdin, direccion_fisica);
        agregar_buffer_para_enterosUint32(buffer_stdin, pid);
        agregar_buffer_para_enterosUint32(buffer_stdin, tamanio);
        agregar_buffer_para_enterosUint32(buffer_stdin, STRING);
        agregar_buffer_para_string(buffer_stdin, texto_truncado);
        enviar_buffer(buffer_stdin, conexion_memoria);
        destruir_buffer(buffer_stdin);

        op_code codigo_memoria = recibir_op_code(conexion_memoria);
        if (codigo_memoria == OK)
        {
            log_info(logger, "PID: <%d> - Operacion: <IO_STDIN_READ>", pid);
        }
        else if (codigo_memoria == ERROR_PEDIDO_ESCRITURA)
        {
            log_error(logger, "PID: <%d> - ERROR Operacion: <IO_STDIN_READ>", pid);
        }
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
    int tamanio = leer_buffer_enteroUint32(buffer_sol_operacion); // con este valor, se lo envio a la memoria para que
    log_info(logger, "TAMANIO A LEER %d", tamanio);
    int direccion_fisica = leer_buffer_enteroUint32(buffer_sol_operacion); // donde voy a pedirle a memoria que busque el dato
    log_info(logger, "DIRECCIONES FISICAS %d", direccion_fisica);
    int pid = leer_buffer_enteroUint32(buffer_sol_operacion);
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
            tipo_buffer *lectura = recibir_buffer(conexion_memoria);
            char *texto_recibido = leer_buffer_string(lectura);
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
    instrucciones_dialfs();
}

// AUXILIARES

t_list *convertir_a_numeros(char *texto)
{
    int i;
    t_list *lista = list_create();
    for (i = 0; texto[i] != '\0'; i++)
    {
        int numero = (int)texto[i];
        list_add(lista, &numero);
    }
    return lista;
}

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

void int_a_char_y_concatenar_a_string(int valor, char *cadena)
{
    char caracter = (char)valor;
    int longitud_cadena = strlen(cadena);
    cadena[longitud_cadena] = caracter;
    cadena[longitud_cadena + 1] = '\0';
}

char *truncar_texto(char *texto_ingresado, int tamanio)
{
    char *nuevo_texto;
    truncar_valor(&nuevo_texto, texto_ingresado, tamanio);
    return nuevo_texto;
}