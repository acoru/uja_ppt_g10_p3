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
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "protocol.h"


int getTimeZone()

{

	TIME_ZONE_INFORMATION tziOld;
	DWORD dwRet;

	dwRet = GetTimeZoneInformation(&tziOld);

	if(dwRet == TIME_ZONE_ID_STANDARD || dwRet == TIME_ZONE_ID_UNKNOWN)
	{
		tziOld.StandardBias/60;
	}
	else if( dwRet == TIME_ZONE_ID_DAYLIGHT )
	{
		return tziOld.DaylightBias/60;
	}
	else
	{
		printf("GTZI failed (%d)\n", GetLastError());
		return 0;
	}
}


int main(int *argc, char *argv[])
{
	//code for taking the date of the system
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	SOCKET sockfd;
	struct sockaddr_in server_in;
	char buffer_in[1024], buffer_out[1024],input[1024];
	int recibidos=0,enviados=0;
	int estado=S_CONNECT;
	char option = 's';
	char d_option[5] = "";	//DATA option, for selecting the option in the DATA state
	int i_d_option = 0;		//for working with integer values on the DATA state option
	int error = 0;			//for controlling some possible errors during the insertion of the mail data
	char date[50] = "";		//for taking the date into a variable for later use
	char m_from[50] = "";	//source email
	char m_to[50] = "";		//destination email
	char subject[50] = "";	//subject of the message
	char from[50] = "";		//variable for the from of the mail header
	char to[50] = "";		//variable for the to of the mail header
	char message[1000] = "";//the message's body

	//for getting the time zone
	int time_zone = 0;
	char timezone[5];

	//variables for date
	int d_day = 0;
	int d_month = 0;
	char c_day[4] = "";
	char c_month[4] = "";

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
	char r_c_code[20] = "";
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
						do
						{
							printf("CLIENTE> Select an option\r\n");
							printf("1) Write email\r\n");
							printf("2) QUIT (close session)\r\n");
							printf("3) EXIT (from program)\r\n");
							gets(d_option);
							sscanf_s(d_option, "%d", &i_d_option);
							switch (i_d_option)
							{
								case 1:
									/* if the user choose to write an email, it will change the state to S_MAIL_F, but this state 
									*  is what you can see hear, after the user insert the email, the state will change to S_RCPT_T
									*/
									estado++;
									printf("CLIENT> Insert the email source:");
									gets(input);
									strcpy(m_from, input);
									if(strcmp(input, "QUIT") == 0)
									{
										sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
									}
									else
									{
										sprintf_s(buffer_out, sizeof(buffer_out), "MAIL FROM: %s%s", input, CRLF);
									}
									break;
								case 2:
									sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
									estado=S_QUIT;
									break;
								case 3:
									sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
									estado=S_QUIT;
									option = 'n';
									break;
								default:
									printf("CLIENT> Incorrect option, please, choose one of the following options");
									break;
							}
						}while(i_d_option < 1 || i_d_option > 3);
						break;
					case S_RCPT_T:
						printf("CLIENT> Insert the destination email:");
						gets(input);
						strcpy(m_to, input);
						if(strcmp(input, "QUIT") == 0)
						{
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
						}
						else
						{
							sprintf_s(buffer_out, sizeof(buffer_out), "RCPT TO:%s%s", input, CRLF);
						}
						break;
					case S_SEND_D:
						printf("DATA%s", CRLF);
						sprintf_s(buffer_out, sizeof(buffer_out), "DATA%s", CRLF);
						break;
					case S_MAIL_B:
						//code for taking the date from the system, it must be taken foreach time a user write an email
						t = time(NULL);
						tm = *localtime(&t);
						//constructing the date format
						d_day = tm.tm_wday;
						d_month = tm.tm_mon;
						//taking the day

						/** NOTA DEL PROFESOR *************************
						La función strtime aporta toda esta información
						automáticamente
						**********************************************/
						switch (d_day)
						{
							case 0:
								sprintf_s(c_day, sizeof(c_day), "Sun");
								break;
							case 1:
								sprintf_s(c_day, sizeof(c_day), "Mon");
								break;
							case 2:
								sprintf_s(c_day, sizeof(c_day), "Tue");
								break;
							case 3:
								sprintf_s(c_day, sizeof(c_day), "Wed");
								break;
							case 4:
								sprintf_s(c_day, sizeof(c_day), "Thu");
								break;
							case 5:
								sprintf_s(c_day, sizeof(c_day), "Fri");
								break;
							case 6:
								sprintf_s(c_day, sizeof(c_day), "Sat");
								break;
						}
						//taking the month
						switch (d_month)
						{
							case 0:
								sprintf_s(c_month, sizeof(c_month), "Jan");
								break;
							case 1:
								sprintf_s(c_month, sizeof(c_month), "Feb");
								break;
							case 2:
								sprintf_s(c_month, sizeof(c_month), "Mar");
								break;
							case 3:
								sprintf_s(c_month, sizeof(c_month), "Apr");
								break;
							case 4:
								sprintf_s(c_month, sizeof(c_month), "May");
								break;
							case 5:
								sprintf_s(c_month, sizeof(c_month), "Jun");
								break;
							case 6:
								sprintf_s(c_month, sizeof(c_month), "Jul");
								break;
							case 7:
								sprintf_s(c_month, sizeof(c_month), "Aug");
								break;
							case 8:
								sprintf_s(c_month, sizeof(c_month), "Sep");
								break;
							case 9:
								sprintf_s(c_month, sizeof(c_month), "Oct");
								break;
							case 10:
								sprintf_s(c_month, sizeof(c_month), "Nov");
								break;
							case 11:
								sprintf_s(c_month, sizeof(c_month), "Dec");
								break;
						}

						time_zone = getTimeZone();
						if(time_zone < 10)
						{
							sprintf_s(timezone, sizeof(timezone), "0%d", time_zone);
						}
						else
						{
							sprintf_s(timezone, sizeof(timezone), "%d", time_zone);
						}

						sprintf_s(date, sizeof(date), "%s, %d %s %d %d:%d:%d +%s00", c_day, tm.tm_mday, c_month, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, timezone);
						//user must insert a subjet, if user do not insert anything, it'll loop until user insert it
						printf("subjet: ");
						do
						{
							if(error == 1)
							{
								printf("CLIENT> subjet required, please, insert a subjet: ");
							}
							gets(input);
							error = 1;
						}while(strcmp(input, "") == 0);
						error = 0;
						strcpy(subject, input);
						//if user don't insert an alias for the source of the message and the destination, it'll be set
						//the source and destination email address
						printf("CLIENT> from (if empty it'll be set the mail inserted on the MAIL FROM before): ");
						gets(input);
						if(strcmp(input, "") == 0)
						{
							strcpy(from, m_from);
						}
						else
						{
							strcpy(from, input);
						}
						printf("CLIENT> to (if empty it'll be set the mail inserted on the MAIL FROM before): ");
						gets(input);
						if(strcmp(input, "") == 0)
						{
							strcpy(to, m_to);
						}
						else
						{
							strcpy(to, input);
						}
						printf("CLIENT> Insert the body of the message: %s", CRLF);
						printf("NOTE: insert CRLF.CRLF for ending the text typing%s", CRLF);
						/*	this loop sentence will take endless lines until user insert in a new line just the "." char
						*	the way it works is by getting all the lines the user insert, and concatenating the new inputs
						*	with the old ones in a vector of char (string message). Just one the user insert in a new line
						*	a simple char ".", the condition in the while sentence will detect it and will make the loop end
						*/
						do
						{
							gets(input);
							strcat(input, CRLF);
							strcat(message, input);
						}while(strcmp(input, ".\r\n") != 0);
						/*** NOTA DEL PROFESOR ******************************+
						No se pueden enviar correos más largos que la longitude de buffer_out
						por lo que se cumple con uno de los objetivos
						
						*/

						sprintf_s(buffer_out, sizeof(buffer_out), "date:%s\r\nsubject:%s\r\nto:%s\r\nfrom:%s\r\n\r\n%s", date, subject, to, from, message);
						break;
					}
					//Envio
					// Ejercicio: Comprobar el estado de envio
					if(estado != S_CONNECT)
					{
						enviados=send(sockfd,buffer_out,(int)strlen(buffer_out),0);
					}
					if(enviados <= 0 && estado != S_CONNECT)
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

					if(recibidos<=0 && estado != S_CONNECT)
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
						//takes the error code (or non error code) from the server response
						sscanf_s(buffer_in, "%d %s", &r_code, r_c_code, sizeof(r_c_code));
						printf(buffer_in);
						switch (r_code)
						{
							//response for the initial connection with the server
							case 220:
								estado++;
								break;
							//server code response to a QUIT message
							case 221:
									estado = S_QUIT;
								break;
							case 250:
								//Controlling the server response for a helo message, if cliente is in HELO state and a 250 code message is received, it means that
								//the helo message was valid
								if(estado == S_HELO)
								{
									estado++;
								}
								//for controlling the MAIL FROM and RCPT TO message
								else if(estado == S_MAIL_F || estado == S_RCPT_T)
								{
									estado++;
								}
								//for controlling that the message has been sent correctly to it's destination
								else if(estado = S_MAIL_B)
								{
									printf("CLIENT> Write more email? (Y/N)%s", CRLF);
									option=_getch();
									if(option == 'y' || option == 'Y')
									{
										//cleaning the buffer_out
										sprintf_s(message, sizeof(message), "");
										estado = S_DATA;
									}
									else if(option == 'n' || option == 'N')
									{
										estado = S_QUIT;
									}
								}
								break;
							case 354:
								//recieve from the server after the DATA message is send
								estado++;
								break;
							default:
								break;
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
		if(option != 'n' && option != 'N')
		{
			printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
			//if user want to send another mail, the state will be set to the initial state
			option=_getche();
			if(option == 's' || option == 'S')
			{
				estado=S_CONNECT;
				sprintf_s(buffer_out, sizeof(buffer_out), "0x00");	//cleaning the buffer_out for new connections
			}
		}
	}while(option!='n' && option!='N');

	
	
	return(0);

}
