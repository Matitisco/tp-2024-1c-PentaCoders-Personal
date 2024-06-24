#ifndef MENU_H_
#define MENU_H_

#include "kernel.h"
#include "../include/operaciones.h"
extern char *comandos[];

// MENU
void iniciar_consola_interactiva();
void mostrar_operaciones_realizables();
void ejecutar_operacion(char *codigo_operacion, t_log *logger);

// MENU 2
void iniciar_consola_interactiva2();
void ejecutar_comando(char *comando, int tokens);
char *obtener_comando(char *entrada);
char *generator(const char *text, int state);
char **autocompletado(const char *text, int start, int end);

// AUXILIARES
opciones_menu obtenerCodigo(char *opcion, t_log *logger);

#endif