#include <SPI.h>
#include <EthernetBonjour.h>
#include <Dhcp.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <util.h>


//defines
#define bufferMax 50
#define nreles 3
#define ndimms 2


//variables de configuración de interfaz ethernet
byte mac[] = {0x90, 0xA2, 0xDA, 0x00, 0x58, 0xD3};
byte ip[] = {192,168,0,101};
byte gateway[] = {192,168,0,1};
byte subnet[]={255,255,255,0};
EthernetServer server(80);// Port 80 is http.
EthernetClient client;

unsigned long Timer = 0;//timer para bonjour
String buffer = "";//buffer para almacenar peticiones de los clientes
String metodo = "";
String clave = "";//string para almacenar el comando
String valor = "";//string para almacenar el valor

//variables de status
int relevador1 = 0;
int relevador2 = 0;
int dimmer1 = 0;
int dimmer2 = 0;
int relevadores[nreles];
int dimmers[ndimms];

void setup(){
  Serial.begin(9600);
  Serial.println("iniciando el programa");
  if(Ethernet.begin(mac)==0){
    Serial.println("fallo al obtener direccion DHCP, asignando IP Local");
    Ethernet.begin(mac,ip,gateway,subnet);
    Serial.println(Ethernet.localIP());
  }
  else 
    Serial.println("Asignacion DHCP correcta");
  EthernetBonjour.begin("lm001");
  EthernetBonjour.addServiceRecord("Modulo de Luces 001._http",
                                   80,
                                   MDNSServiceTCP);    
}

void loop(){
  if ((millis() - Timer) >= 10000){//si ya pasaron mas de 10 segundos desde que se anuncio el servicio bonjour
    EthernetBonjour.run();
    Timer = millis();
  }
  client = server.available();
  if (client)
  {
    Serial.println("conexion realizada");
    leePeticion(client, buffer);
    Serial.println(buffer);
    metodo = obtenerMetodo(buffer);
    Serial.println(metodo);
    if ( metodo == "GET"){
      imprimirStatus(client);
    }
    else if (metodo == "POST"){
      parse(buffer, clave, valor);
      Serial.println(clave);
      Serial.println(valor);
      ejecuta(clave, valor);
      imprimirStatus(client);
    }
    delay(100);
    client.stop();
  }
}




void leePeticion(EthernetClient &client, String &buffer) //FUNCIÓN PARA ALMACENAR LA PETICIÓN DEL CLIENTE EN EL BUFFER
{
  buffer = "";
  char nuevochar;
  char charanterior = '\r';
  char charanterior2 = '\n';
  int nuevasLineas = 0;
while(client.available()){//verifico si hay algo por leer
    nuevochar = client.read();//lo leo
      if (nuevochar == '\n'){//si es un caracter de nueva linea
	  nuevasLineas++;//aumento el contador de lineas nuevas 
			if (charanterior == '\r' && charanterior2 == '\n'){//además si lo anterior también fue una linea nueva es que se acabaron los headers
                                if(client.available()){//si hay datos despues de los headers 
                                buffer.concat('\r'); //concateno un salto de linea para marcar el inicio de los datos en el buffer
                                buffer.concat('\n');
                                }
				while(client.available() && (buffer.length() < bufferMax)){//por tanto mientras quede algo por leer y no se llene el buffer
					nuevochar = client.read();//lo leo
					buffer.concat(nuevochar);//lo guardo en el buffer
				}
				client.flush();//si se gastó o se llenó el buffer de todas maneras me aseguro de que no quede nada por leer
			}
      }
      else if ((nuevasLineas < 1) && (buffer.length() < bufferMax) && (nuevochar != '\r') ){//si no fue caracter de nueva línea, ni carriage return, y aun estoy en la primera linea
		buffer.concat(nuevochar);//guardo en mi buffer
		}
    charanterior2 = charanterior;
    charanterior = nuevochar;     
  }
}


String obtenerMetodo(String &buffer){//FUNCIÓN QUE DETERMINA EL TIPO DE PETICIÓN (SOLO SOPORTA GET Y POST, SI NO LA CONSIDERA INVÁLIDA)
  if (buffer.substring(0,3)=="GET"){
    return "GET";
  }
  else if (buffer.substring(0,4)=="POST"){
    return "POST"; 
    }
  else return "error";
}

void parse(String &buffer, String &clave, String &valor){
    String query = "";
    int inicioquery;
    inicioquery = buffer.indexOf("\r\n");//busco el salto de linea que marca el inicio de los datos
    if (inicioquery >= 0){
      query = buffer.substring(inicioquery+2);
      int igual = query.indexOf("=");
      clave = query.substring(0, igual);
      valor = query.substring(igual+1);
    }
}

void imprimirStatus(EthernetClient &client){
  client.println("Tipo = Modulo de luces");
  client.print("Relevador1 = ");
  client.println(relevador1);
  client.print("Relevador2 = ");
  client.println(relevador2);
  client.print("Dimmer1 = ");
  client.println(dimmer1);
  client.print("Dimmer2 = ");
  client.println(dimmer2);
}

void ejecuta(String& clave, String& valor){
  char charnumero = clave.charAt(3);
  int numero = atoi(&charnumero);
  char charestado[3];
  valor.toCharArray(charestado,3);
  int estado = atoi(charestado);
  if (clave.substring(0,3)=="rel"){
    relevadores[numero]=estado;
  }
  else if (clave.substring(0,3)=="dim"){
    dimmers[numero]=estado;
  }
}
//this
//comentarioPrueba
  
