#include "../include/mmu.h"
#include "../include/tlb.h"

int TLB_HABILITADA;


uint32_t traducir_direccion_mmu(uint32_t direccion_logica){
    int numero_pagina = calcular_pagina(direccion_logica);
    int desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;
    int marco = obtener_frame(numero_pagina);
    if (marco == -1)
    {
        return -1;
    }
    int direccion_fisica = marco * tamanio_pagina + desplazamiento;
    return direccion_fisica;
}

int calcular_pagina(int direccion_logica)
{
    return floor(direccion_logica / tamanio_pagina);
}

int obtener_frame(int pagina){
    int frame;
    if (TLB_HABILITADA)
    {
        frame = obtener_marco_tlb(tlb_cpu, cde_recibido->pid, pagina);
        if(frame == -1){//TLB MISS
            log_info(logger, "PID: <%d> - TLB MISS - Pagina: <%d>", cde_recibido->pid, pagina);
            frame = pedir_frame_memoria(cde_recibido->pid, pagina);
            agregar_entrada_a_tlb(tlb_cpu, crear_entrada_tlb(cde_recibido->pid, pagina, frame));
        }
        else{//TLB HIT
            log_info(logger, "PID: <%d> - TLB HIT - Pagina: <%d>", cde_recibido->pid, pagina);
        }
        imprimir_tlb(tlb_cpu);
    }
    else{
        frame = pedir_frame_memoria(cde_recibido->pid, pagina);
    }
    return frame;
}

int pedir_frame_memoria(int pid, int nroPagina)
{
    int frame_pedido;
    enviar_op_code(socket_memoria, PEDIDO_FRAME);

    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer, pid);
    agregar_buffer_para_enterosUint32(buffer, nroPagina);
    enviar_buffer(buffer, socket_memoria);
    destruir_buffer(buffer);

    op_code respuesta_mmu = recibir_op_code(socket_memoria);
    if (respuesta_mmu == PEDIDO_FRAME_CORRECTO)
    {
        tipo_buffer *buffer_memoria_tlb = recibir_buffer(socket_memoria);
        frame_pedido = leer_buffer_enteroUint32(buffer_memoria_tlb);
        destruir_buffer(buffer_memoria_tlb);
        log_info(logger, "PID: <%d> - MARCO OBTENIDO - Pagina: <%d> - Marco: <%d>", pid, nroPagina, frame_pedido);
    }
    else if (respuesta_mmu == PEDIDO_FRAME_INCORRECTO)
    {
        log_error(logger, "PID: <%d> - ERROR OBTENER MARCO - Pagina: <%d>", pid, nroPagina);
        frame_pedido = -1;
    }
    return frame_pedido;
}

