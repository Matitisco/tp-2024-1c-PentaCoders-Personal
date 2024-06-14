/* 
void *leer_memoria(uint32_t direccion_fisica, uint32_t pid, uint32_t tamanio)
{
    usleep(valores_config->retardo_respuesta * 1000);

    uint32_t numero_pagina = direccion_fisica / valores_config->tam_pagina;
    uint32_t offset = direccion_fisica % valores_config->tam_pagina;

    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid);
    t_pagina *pagina = list_get(tabla_paginas->tabla_paginas_proceso, numero_pagina);
    int marco = pagina->marco;

    void *valor = malloc(tamanio);

    memcpy(valor, espacio_usuario + marco * valores_config->tam_pagina + offset, tamanio);

    return valor;
}

void *escribir_memoria(uint32_t direccion_fisica, uint32_t pid, void *valor_a_escribir, uint32_t tamanio)
{
    usleep(valores_config->retardo_respuesta * 1000);

    uint32_t numero_pagina = direccion_fisica / valores_config->tam_pagina;
    uint32_t offset = direccion_fisica % valores_config->tam_pagina;

    valor_a_escribir = malloc(tamanio);

    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid);
    if (tabla_paginas == NULL)
    {
        return (void *)-1;
    }
    t_pagina *pagina = list_get(tabla_paginas->tabla_paginas_proceso, numero_pagina);
    if (pagina == NULL)
    {
        return (void *)-1;
    }
    int marco = pagina->marco;
    if (marco < 0)
    {
        return (void *)-1;
    }
    if (memcpy(espacio_usuario + marco * valores_config->tam_pagina + offset, valor_a_escribir, tamanio) == NULL)
    {
        return (void *)-1;
    }
    else
    {
        return (void *)1;
    }
}

void ampliar_proceso(uint32_t pid, uint32_t tamanio, int cliente_cpu)
{
    int paginas_adicionales = (tamanio + valores_config->tam_pagina - 1) / valores_config->tam_pagina;
    int tamanio_anterior = tamanio_proceso(pid);

    int marcos_disponibles = 0;
    for (int i = 0; i < cant_marcos; i++)
    {
        if (array_bitmap[i].bit_ocupado == 0)
        {
            marcos_disponibles++;
        }
    }

    if (paginas_adicionales > marcos_disponibles)
    {
        enviar_cod_enum(cliente_cpu, OUT_OF_MEMORY);
        log_error(logger, "OUT OF MEMORY");
        return;
    }
    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid);
    if (tabla_paginas == NULL)
    {
        log_info("NO se encontro la tabla de paginas con pid %d", pid);
        return;
    }
    int marco_libre;
    t_list *paginas = tabla_paginas->tabla_paginas_proceso;
    for (int i = 0; i < paginas_adicionales; i++)
    {

        marco_libre = -1;
        for (int j = 0; j < cant_marcos; j++)
        {
            if (array_bitmap[j].bit_ocupado == 0)
                marco_libre = j;
            array_bitmap[j].bit_ocupado = 1; // Marcar el marco como ocupado
        }
        // Crear una nueva página y agregarla al proceso
        t_pagina *nueva_pagina = malloc(sizeof(t_pagina));
        nueva_pagina->marco = marco_libre; // Pongo el marco libre que encontre
        nueva_pagina->bit_validez = 1;
        list_add(paginas, nueva_pagina);
    }

    log_info(logger, "PID: <%d> - Tamaño Actual: <%d> - Tamaño a Ampliar: <%d>", pid, tamanio_anterior, tamanio);
    enviar_cod_enum(cliente_cpu, RESIZE_EXITOSO);
}

void reducir_proceso(uint32_t pid, uint32_t tamanio, int cliente_cpu)
{
    int paginas_requeridas = (tamanio + valores_config->tam_pagina - 1) / valores_config->tam_pagina; // cant de paginas que tiene que tener con este nuevo tamanio
    int tamanio_anterior = tamanio_proceso(pid);
    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid); // busco en la lista de listaa
    if (tabla_paginas == NULL)
    {
        log_info("No se encontro la tabla de paginas con pid %d", pid);
        return;
    }
    int paginas_actuales = list_size(tabla_paginas->tabla_paginas_proceso); // cantida de paginas que hay jasta el momento
                                                                            // Si el proceso tiene más páginas de las requeridas, liberar las excesivas desde el final
    t_list *paginas = tabla_paginas->tabla_paginas_proceso;
    while (paginas_actuales > paginas_requeridas)
    {
        t_pagina *pagina_a_eliminar = list_remove(paginas, paginas_actuales - 1);
        pagina_a_eliminar->bit_validez = 0; // Marcar como no válida
        free(pagina_a_eliminar);            // Liberar la memoria de la página
        paginas_actuales--;
    }

    log_info(logger, "PID: %d - Tamaño Actual: <%d> - Tamaño a Reducir: <%d>", pid, tamanio_anterior, tamanio);
    enviar_cod_enum(cliente_cpu, RESIZE_EXITOSO);
}

int tamanio_proceso(int pid)
{
    t_tabla_paginas *tabla_paginas = buscar_en_lista_global(pid); // devuelve mi tabla de proceso
    int cant_paginas_proceso = list_size(tabla_paginas->tabla_paginas_proceso);
    return cant_paginas_proceso * valores_config->tam_pagina;
}

void crear_y_agregar_tabla_a_lista_global(int pid)
{
    t_tabla_paginas *tabla_proceso = malloc(sizeof(t_tabla_paginas));
    tabla_proceso->tabla_paginas_proceso = list_create();
    tabla_proceso->pid = pid;
    list_add(lista_global_tablas, tabla_proceso);
}

t_cde *armarCde(tipo_buffer *buffer)
{
    t_cde *cde = malloc(sizeof(t_cde));
    cde->pid = leer_buffer_enteroUint32(buffer);
    cde->path = leer_buffer_string(buffer);
    cde->lista_instrucciones = list_create();
    return cde;
}

char *obtener_char_instruccion(t_tipoDeInstruccion instruccion_code)
{
    if (instruccion_code == SET)
    {
        return "SET";
    }
    if (instruccion_code == MOV_IN)
    {
        return "MOV_IN";
    }
    if (instruccion_code == MOV_OUT)
    {
        return "MOV_OUT";
    }
    if (instruccion_code == SUM)
    {
        return "SUM";
    }
    if (instruccion_code == SUB)
    {
        return "SUB";
    }
    if (instruccion_code == JNZ)
    {
        return "JNZ";
    }
    if (instruccion_code == RESIZE)
    {
        return "RESIZE";
    }
    if (instruccion_code == COPY_STRING)
    {
        return "COPY_STRING";
    }
    if (instruccion_code == WAIT)
    {
        return "WAIT";
    }
    if (instruccion_code == SIGNAL)
    {
        return "SIGNAL";
    }
    if (instruccion_code == IO_GEN_SLEEP)
    {
        return "IO_GEN_SLEEP";
    }
    if (instruccion_code == IO_STDIN_READ)
    {
        return "IO_STDIN_READ";
    }
    if (instruccion_code == IO_STDOUT_WRITE)
    {
        return "IO_STDOUT_WRITE";
    }
    if (instruccion_code == IO_FS_CREATE)
    {
        return "IO_FS_CREATE";
    }
    if (instruccion_code == IO_FS_DELETE)
    {
        return "IO_FS_DELETE";
    }
    if (instruccion_code == IO_FS_TRUNCATE)
    {
        return "IO_FS_TRUNCATE";
    }
    if (instruccion_code == IO_FS_WRITE)
    {
        return "IO_FS_WRITE";
    }
    if (instruccion_code == IO_FS_READ)
    {
        return "IO_FS_READ";
    }
    if (instruccion_code == EXIT)
    {
        return "EXIT";
    }
    return -1;
}
 */