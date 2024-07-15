#ifndef OPERACIONES_ES_H_
#define OPERACIONES_ES_H_

#include "entrada_salida.h"

// OPERACIONES
void realizar_operacion_gen(t_interfaz *interfaz);
void realizar_operacion_stdin(t_interfaz *interfaz);
void realizar_operacion_stdout(t_interfaz *interfaz);
void realizar_operacion_dialfs(t_interfaz *interfaz);

// AUXILIARES
t_list *convertir_a_numeros(char *texto);
void truncar_valor(char **valor_nuevo, char *valor_viejo, int limitante);
void int_a_char_y_concatenar_a_string(int valor, char *cadena);
char *truncar_texto(char *texto_ingresado, int tamanio);
void enviar_buffer_stdin_memoria(int direccion_fisica, int pid, int cant_enteros, t_list *lista_enteros);
void instrucciones_dialfs();
#endif