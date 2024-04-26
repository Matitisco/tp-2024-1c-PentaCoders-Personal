
#include "../include/serializacion.h"
#include <stdlib.h>
#include <stdio.h>


tipo_buffer*crear_buffer(){
    tipo_buffer*unBuffer = malloc(sizeof(unBuffer));
    //completo los atributos bah incializao
    return unBuffer;
}
void destroy_buffer(tipo_buffer*unBuffer){
    free(unBuffer);
    //Tal vez deberia hacer free miembro a miembro
    
}
void envio_buffer(tipo_buffer *unBuffer,int unSocket){
    send(unSocket, (&unBuffer),sizeof(unBuffer),0);
}
