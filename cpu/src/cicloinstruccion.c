#include "../include/cicloinstruccion.h"

t_registros *registros;
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
//auxiliares instrucciones
/*
void* leer_memoria(direccionFisica){
    
    enviar_cod_enum(socket_memoria, PEDIDO_LECTURA);
    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_direccionFisica(buffer,direccionFisica);
    enviar_buffer(buffer, socket_memoria);


    op_code lectura_memoria = recibir_operacion(socket_memoria);
    if (lectura_memoria == DIRECCION_CORRECTA){
        tipo_buffer *bufferValor = recibir_buffer(socket_memoria);
        void * valorLeido;//pensar tipo generico que manda memoria que luego se lo castea
        free(bufferValor);
    }
    else{//Informar direccion incorrecta
        
    }
    free(buffer);
    return valorLeido;
}
void escribir_memoria(direccionFisica2,void * valor){

}
void solicitar_stdin_kernel(char * interfaz,char * reg_tamanio){
    //hay que enviar a kernel cod_op 
    //kernel lo recibe y mediante char *line = readline("texto"); (usar otra funcion que lea solo un tamanio) realiza la lectura
    //kernel devuelve lo leido  
}
*/
/*

MOV_IN EDX ECX
MOV_IN (Registro Datos, Registro Dirección): Lee el valor de memoria correspondiente a la
Dirección Lógica que se encuentra en el Registro Dirección y lo almacena en el Registro
Datos.
*/
void exec_mov_in(char *datos, char *direccion) {
    uint32_t direccionLogica = obtener_valor_origen(direccion);
    
    //uint32_t direccionFisica = direccion_logica_a_fisica(direccionLogica);//definido en mmu
    uint32_t direccionFisica = 3000; //invento una direccion fisica hasta tener terminada la conversión

    enviar_cod_enum(socket_memoria, PEDIDO_LECTURA);
    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer,direccionFisica);
    enviar_buffer(buffer, socket_memoria); //-> hay que enviar la direccion física

    /*
    op_code lectura_memoria = recibir_operacion(socket_memoria); Implementar control de direccion correcta
    if (lectura_memoria == DIRECCION_CORRECTA){
    */
        tipo_buffer *bufferValor = recibir_buffer(socket_memoria);
        uint32_t valor = leer_buffer_enteroUint32(bufferValor);
        exec_set(datos, valor);
        destruir_buffer(bufferValor);
    /*} 
    else{//Informar direccion incorrecta

    }*/
    destruir_buffer(buffer);

}


/*
MOV_OUT EDX ECX
MOV_OUT (Registro Dirección, Registro Datos): Lee el valor del Registro Datos y lo escribe en
la dirección física de memoria obtenida a partir de la Dirección Lógica almacenada en el
Registro Dirección.
*/
void exec_mov_out(char *direccion, char *datos) {
    uint32_t valor = obtener_valor_origen(datos);
    uint32_t direccionLogica = obtener_valor_origen(direccion);

    //uint32_t direccionFisica = direccion_logica_a_fisica(direccionLogica);
    uint32_t direccionFisica = 125;
    t_escrituraMemoria valores = {valor,direccionFisica};

    enviar_cod_enum(socket_memoria, PEDIDO_ESCRITURA);

    tipo_buffer *buffer = crear_buffer();
    agregar_escrituraMemoria_buffer(buffer,valores);
    enviar_buffer(buffer, socket_memoria);
    
    //memoria hace la escritura si está todo ok -> revisar si es necesario
    /*
    op_code escritura_memoria = recibir_operacion(socket_memoria);
    if(escritura_memoria == OK) //escritura correcta
    */

   destruir_buffer(buffer);
}

/*
RESIZE 128
RESIZE (Tamaño): Solicitará a la Memoria ajustar el tamaño del proceso al tamaño pasado
por parámetro. En caso de que la respuesta de la memoria sea Out of Memory, se deberá
devolver el contexto de ejecución al Kernel informando de esta situación.
*/
void exec_resize(char * tamanio) {
//tiene que ser el proceso actual
//hay que enviarle el tamaño pasado por parametro a Memoria 
//devolver el cde si la respuesta es ...
    /*
    enviar_cod_enum(socket_memoria, RESIZE_EXTEND);
    uint32_t tamanioValor = string_to_in(tamanio); //hacer string_to_int
    
    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_enterosUint32(buffer,tamanioValor);
    enviar_buffer(buffer,socket_memoria);

    op_code resize_memoria = recibir_operacion(socket_memoria);
    if(resize_memoria == OUT_OF_MEMORY){
        //enviar cod_op a kernel
        //enviar cde a kernel
    }
    */

}
/*
COPY_STRING 8
COPY_STRING (Tamaño): Toma del string apuntado por el registro SI y copia la cantidad de
bytes indicadas en el parámetro tamaño a la posición de memoria apuntada por el registro
DI.
*/
// COPY_STRING
void exec_copy_string(char * tamanio) {// hace lectura y escritura de memoria
    uint32_t direccionLogica1 = registros->SI;
    uint32_t direccionLogica2 = registros->DI;

    /*
    direccionFisica1 = direccion_logica_a_fisica(direccionLogica1);
    char * string_SI = (char *)leer_memoria(direccionFisica1);

    int bytes_a_copiar = string_to_int(tamanio);
    char *subcadena = (char *)malloc(bytes_a_copiar + 1);
    strncpy(subcadena, string_SI, bytes_a_copiar);
    subcadena[bytes_a_copiar] = '\0'; //por las dudas

    direccionFisica2 = direccion_logica_a_fisica(direccionLogica2);
    escribir_memoria(direccionFisica2,subcadena);

    free(string_SI);
    free(subcadena);
    */
    //primero hay que tener todo el valor del string  


    //uint32_t longitud = strin_to_int(tamanio);
//SI tiene una direccion lógica a un string 
//hay que obtener el string que tiene dicha direccion
//hay que copiar la cantidad de bytes indicada por tamanio
//hay que obtener la direccion lógiaca de DI
//luego hay que asignar lo copiado a dicha direccion
//usar string to int para tamanio
}

/*
IO_STDIN_READ Int2 EAX AX
IO_STDIN_READ (Interfaz, Registro Dirección, Registro Tamaño): Esta instrucción solicita al
Kernel que mediante la interfaz ingresada se lea desde el STDIN (Teclado) un valor cuyo
tamaño está delimitado por el valor del Registro Tamaño y el mismo se guarde a partir de la
Dirección Lógica almacenada en el Registro Dirección.

-Kernel le manda a CPU algo que leyó por consola, CPU le solicita la escritura de lo recibido de
kernel en la direccion 
*/
//IO_STDIN_READ
void exec_io_stdin_read(char * interfaz,char * reg_direccion,char * reg_tamanio) {
    uint32_t direccionLogica1 = obtener_valor_origen(reg_direccion);
    /*
    direccionFisica1 = direccion_logica_a_fisica(direccionLogica1);

    char * lectura_kernel = solicitar_stdin_kernel(interfaz,reg_tamanio);
    escribir_memoria(direccionFisica1,(void*)lectura_kernel);
    */
}

/*
IO_STDOUT_WRITE Int3 BX EAX
IO_STDOUT_WRITE (Interfaz, Registro Dirección, Registro Tamaño): Esta instrucción solicita
al Kernel que mediante la interfaz seleccionada, se lea desde la posición de memoria
indicada por la Dirección Lógica almacenada en el Registro Dirección, un tamaño indicado
por el Registro Tamaño y se imprima por pantalla.
*/

/*

*/
/*
typedef struct
{
    uint32_t direccionLogica;
    char * interfaz;
    uint32_t tamanio;

}stdout_kernel
*/
void exec_io_stdout_write(char * interfaz,char * reg_direccion,char * reg_tamanio) {
    /*
    stdout_kernel enviar = {obtener_valor_origen(reg_direccion),interfaz,string_to_int(reg_tamanio)};
    enviar_cod_enum(socket_kernel_dispatch, STDOUT);

    tipo_buffer *buffer = crear_buffer();
    agregar_buffer_para_stdout(buffer,enviar);
    enviar_buffer(buffer,socket_kernel_dispatch);
    free(buffer);
    */

}


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

// WAIT
void exec_wait() {}
// SIGNAL
void exec_signal() {}
// IO_GEN_SLEEP
void exec_io_gen_sleep(char *nombre_interfaz, uint32_t unidades_trabajo)
{
    buffer_instruccion_io = crear_buffer();
    interrupcion_io = 1;
    enviar_cod_enum(socket_kernel_dispatch, INSTRUCCION_INTERFAZ);

    agregar_buffer_para_enterosUint32(buffer_instruccion_io, SOLICITUD_INTERFAZ_GENERICA);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, IO_GEN_SLEEP);
    agregar_buffer_para_enterosUint32(buffer_instruccion_io, unidades_trabajo);
    agregar_buffer_para_string(buffer_instruccion_io, nombre_interfaz);
}
// IO_STDIN_READ

void exec_io_fs_create() {}
void exec_io_fs_delete() {}
void exec_io_fs_truncate() {}
void exec_io_fs_write() {}
void exec_io_fs_read() {}

// EXIT
void exec_exit(t_cde *cde)
{
    salida_exit = 0;
    enviar_cod_enum(socket_kernel_dispatch, FINALIZAR_PROCESO);

    tipo_buffer *buffer = crear_buffer();
    
    agregar_cde_buffer(buffer, cde);
    enviar_buffer(buffer, socket_kernel_dispatch);
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