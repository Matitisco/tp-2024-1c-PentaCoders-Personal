#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/string.h>
#include <string.h>
#include <stdlib.h>

int iniciar_servidor(t_log *logger, const char *name, char *ip, char *puerto);
int esperar_cliente(t_log *logger, const char *name_server, const char *name_client, int socket_servidor);
int crear_conexion(t_log *logger, const char *server_name, char *ip, char *puerto);
void liberar_conexion(int *socket_cliente);
int crear_servidor(int puerto);
char *obtener_ip_local();
#endif
