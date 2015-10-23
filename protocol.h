#ifndef protocolostpte_practicas_headerfile
#define protocolostpte_practicas_headerfile
#endif

// COMANDOS DE APLICACION
#define SC "USER"  // SOLICITUD DE CONEXION USER usuario 
#define PW "PASS"  // Password del usuario  PASS password
#define HELO "HELO"	//HELO command
#define SD  "QUIT"  // Finalizacion de la conexion de aplicacion
#define SD2 "EXIT"  // Finalizacion de la conexion de aplicacion y finaliza servidor

// RESPUESTAS A COMANDOS DE APLICACION
#define OK  "OK"
#define ER  "ERROR"

//FIN DE RESPUESTA
#define CRLF "\r\n"

//ESTADOS
#define S_CONNECT 0	//initial state
#define S_HELO 1	//once the connection has been established, it'll send the HELO command to the server
#define S_DATA 2	//in this state will be added the options for sending an e-mail
#define S_MAIL_F 3	//new state for inserting the email source
#define S_RCPT_T 4	//new state for inserting the email destination
#define S_SEND_D 5	//just for sending the DATA command to the server
#define S_MAIL_B 6	//the body of the message
#define S_QUIT 7
#define S_EXIT 8