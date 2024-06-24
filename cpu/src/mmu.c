#include "../include/mmu.h"

int TLB_HABILITADA;

uint32_t direccion_logica_a_fisica(int direccion_logica)
{
    int numero_pagina = calcular_pagina(direccion_logica);
    int desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;

    if (TLB_HABILITADA)
    {
        int direccion_fisica = tlb_consultar_df_pagina(numero_pagina, desplazamiento);

        if (direccion_fisica == -1)
        {
            log_info(logger, "PID: <%d> - TLB MISS - Pagina: <%d>", cde_recibido->pid, numero_pagina);
            int marco = enviar_peticion_frame(numero_pagina);
            int direccion_fisica = marco * tamanio_pagina + desplazamiento;
            return direccion_fisica;
        }
        else
        {
            log_info(logger, "PID: <%d> - TLB HIT - Pagina: <%d>", cde_recibido->pid, numero_pagina);
            return direccion_fisica;
        }
    }
    else
    {
        int marco = enviar_peticion_frame(numero_pagina);
        int direccion_fisica = marco * tamanio_pagina + desplazamiento;
        return direccion_fisica;
    }
}

int calcular_pagina(int direccion_logica)
{
    return floor(direccion_logica / tamanio_pagina);
}

int enviar_peticion_frame(int pagina)
{
    int frame_buscado;

    enviar_op_code(socket_memoria, PEDIDO_FRAME);

    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer, cde_recibido->pid);
    agregar_buffer_para_enterosUint32(buffer, pagina);
    enviar_buffer(buffer, socket_memoria);
    destruir_buffer(buffer);

    op_code respuesta_mmu = recibir_op_code(socket_memoria);
    if (respuesta_mmu == PEDIDO_FRAME_CORRECTO)
    {
        tipo_buffer *buffer_memoria_tlb = recibir_buffer(socket_memoria);
        frame_buscado = leer_buffer_enteroUint32(buffer_memoria_tlb);
        destruir_buffer(buffer_memoria_tlb);
        if (TLB_HABILITADA)
        {
            tlb_agregar_entrada(cde_recibido->pid, pagina, frame_buscado);
        }
        log_info(logger, "PID: <%d> - OBTENER MARCO - Pagina: <%d> - Marco: <%d>", cde_recibido->pid, pagina, frame_buscado);
    }
    else if (respuesta_mmu == PEDIDO_FRAME_INCORRECTO)
    {
        log_info(logger, "PID: <%d> - ERROR OBTENER MARCO - Paguna: <%d>", cde_recibido->pid, pagina);
    }

    return frame_buscado;
}