
#include "../include/cortoPlazo.h"

int temporal;
int tiempo_extra;

//Despues veo si lo muevo a cortoPlazo.c

//REPENSAR EL HILO DISPATCH, EL TP NO TE PIDE UN HILO, SOLO QUE TENGAS LA CONEXION, TRANQUILAMENTE PODES HACER EL RECV EN EL LA FUNCION VRR/RR/FIFO.

void planificar_por_vrr(){

    //transicion ready exec
    transicion_ready_exec();
    //...


    //send() //a la conexión de dispatch
    enviar_proceso_a_cpu();
    
    //Iniciar temporal

    iniciar_temporal();


    t_pcb *proceso = recibir_proceso_de_cpu(); //VER COMO GESTIONAMOS EL CAMBIO DE ESTADO, DEBERIA MANDARLO A BLOCKED PERO GUARDAR EL QUANTUM

    detener_temporal();
    //Detener temporal
    

    if( temporal < QUANTUM){
        tiempo_extra = QUANTUM - temporal;
        proceso->quantum = tiempo_extra;
        agregar_a_estado(proceso, cola_bloqueado_global);    // ENTRA A BLOCK, se guardo el quantum EXTRA, cuando vuelva de BLOCK hay que evaluar si no tiene Quantum de mas
    }

}

void enviar_proceso_a_cpu(){
    enviar_cod_enum(socket_cpu_dispatch, EJECUTAR_PROCESO); // PASA A ESTADO EXEC 
    enviar_cde(socket_cpu_dispatch, proceso->cde);
}

void temporizador(){
    timer = temporal_create();
    esperar_a_que_desaloje(pcb); //crea temporizador y desaloja
    temporal_stop(timer);
    ms_transcurridos = temporal_gettime(timer);
    temporal_destroy(timer);
}

void iniciar_temporal(){

}


t_pcb *recibir_proceso_de_cpu(){
    op_code cod = recibir_operacion(socket_cpu_dispatch);
	tipo_buffer *buffer_cpu;
    t_cde *cde = malloc(sizeof(t_cde));

    if(cod == FINALIZAR_PROCESO){
        buffer_cpu = recibir_buffer(socket_cpu_dispatch);
		cde = leer_cde(buffer_cpu);

        t_pcb *proceso = sacar_procesos_cola(cola_exec_global);
        proceso->cde = cde;
        //VER COMO GESTIONAMOS EL CAMBIO DE ESTADO, DEBERIA MANDARLO A BLOCKED PERO GUARDAR EL QUANTUM

        return proceso;
    }
    else{
        return NULL;
    }
}