/*******************************************************
Protocolos de Transporte
Grado en Ingeniería Telemática
Dpto. Ingeníería de Telecomunicación
Univerisdad de Jaén

Fichero: cliente.c
Versión: 1.0
Fecha: 23/09/2012
Descripción:
	Cliente de eco sencillo TCP.

Autor: Juan Carlos Cuevas Martínez

*******************************************************/
#include <stdio.h>
#include <winsock.h>
#include <time.h>
#include <conio.h>

#include "protocol.h"





int main(int *argc, char *argv[])
{
	SOCKET sockfd;
	struct sockaddr_in server_in;
	char buffer_in[1024], buffer_out[1024],input[1024];
	char buffer_out2[10] = "";
	int recibidos=0,enviados=0;
	int estado=S_CONNECT;
	char option;

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

    char ipdest[16];
	char default_ip[16]="127.0.0.1";
	//variables for the port of the server, 25 as default port
	char tcp_port[16];
	char default_tcp_port[16]="25";
	int s_tcp_port = 0;
	//variable for the recived code as response from the server for a command
	int r_code;
	
	//Inicialización Windows sockets
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1)
	{
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets

	do{
		sockfd=socket(AF_INET,SOCK_STREAM,0);

		if(sockfd==INVALID_SOCKET)
		{
			printf("CLIENTE> ERROR AL CREAR SOCKET\r\n");
			exit(-1);
		}
		else
		{
			printf("CLIENTE> SOCKET CREADO CORRECTAMENTE\r\n");

		
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets(ipdest);

			if(strcmp(ipdest,"")==0)
				strcpy(ipdest,default_ip);
			
			//user must insert the TCP port of the SMTP server, 25 as default
			printf("CLIENTE> Insert the TCP port of the server (intro for default PORT): ");
			gets(tcp_port);

			if(strcmp(tcp_port, "") == 0)
			{
				strcpy(tcp_port, default_tcp_port);
			}
			//once the user has inserted the port, it's converted into short
			sscanf_s(tcp_port, "%d", &s_tcp_port);

			server_in.sin_family=AF_INET;
			server_in.sin_port=htons(s_tcp_port);
			server_in.sin_addr.s_addr=inet_addr(ipdest);
			
			estado=S_CONNECT;
		
			// establece la conexion de transporte
			if(connect(sockfd,(struct sockaddr*)&server_in,sizeof(server_in))==0)
			{
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n",ipdest,s_tcp_port);
			
		
				//Inicio de la máquina de estados
				do{
					switch(estado)
					{
					case S_CONNECT:
						break;
					case S_HELO:
						// Se recibe el mensaje de bienvenida
						sprintf_s(buffer_out, sizeof(buffer_out), "HELO hi\r\n");
						break;
					case S_DATA:
						printf("CLIENTE> Introduzca datos (enter o QUIT para salir): ");
						gets(input);
						if(strlen(input)==0)
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}
						else
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input,CRLF);
						break;
				 
				
					}
					//Envio
					// Ejercicio: Comprobar el estado de envio
					enviados=send(sockfd,buffer_out,(int)strlen(buffer_out),0);
					if(enviados <= 0)
					{
						DWORD error = GetLastError();
						if(enviados < 0)
						{
							printf("CLIENTE> FAIL SENDING DATA TO THE SERVER: %d%s", error, CRLF);
							estado=S_QUIT;
						}
						else
						{
							printf("CLIENTE> END CONEXION WITH THE SERVER%s", CRLF);
							estado=S_QUIT;
						}
					}
					//Recibo
					recibidos=recv(sockfd,buffer_in,512,0);

					if(recibidos<=0)
					{
						DWORD error=GetLastError();
						if(recibidos<0)
						{
							printf("CLIENTE> Error %d en la recepción de datos\r\n",error);
							estado=S_QUIT;
						}
						else
						{
							printf("CLIENTE> Conexión con el servidor cerrada\r\n");
							estado=S_QUIT;
						
					
						}
					}
					else
					{
						buffer_in[recibidos]=0x00;
						printf(buffer_in); 
						//takes the error code (or non error code) from the server response
						sscanf_s(buffer_in, "%d", &r_code);
						//if received code is 220, the conecction with the server was successful
						//then, the state machine changes it's state to HELO
						if(estado != S_DATA && r_code == 220)
						{
							estado++;
						}
						//response for the HELO message
						if(estado != S_DATA && r_code == 250)
						{
							estado++;
						}
					}

				}while(estado!=S_QUIT);
				
	
		
			}
			else
			{
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n",ipdest,s_tcp_port);
			}		
			// fin de la conexion de transporte
			closesocket(sockfd);
			
		}	
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option=_getche();

	}while(option!='n' && option!='N');

	
	
	return(0);

}
