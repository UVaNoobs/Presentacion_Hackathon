/*#include <aes_keyschedule.h>
  #include <bcal-cbc.h>
  #include <aes_sbox.h>
  #include <aes_types.h>
  #include <blockcipher_descriptor.h>
  #include <memxor.h>
  #include <bcal_aes128.h>
  #include <aes256_dec.h>
  #include <aes_dec.h>
  #include <aes_enc.h>
  #include <bcal-ofb.h>
  #include <aes_invsbox.h>
  #include <aes192_enc.h>
  #include <bcal-cmac.h>
  #include <aes.h>
  #include <bcal_aes256.h>
  #include <keysize_descriptor.h>
  #include <aes128_enc.h>
  #include <bcal-basic.h>
  #include <AESLib.h>
  #include <aes128_dec.h>
  #include <gf256mul.h>
  #include <aes192_dec.h>
  #include <bcal_aes192.h>
  #include <aes256_enc.h>
*/

#include <string.h>
#include <SD.h>

#include <SoftwareSerial.h>

#define TAMANOTEMP 100
#define TAMANOCLAVEPUBLICA 100
#define TAMANOMENSAJECIFRADORSA 100

SoftwareSerial bluetooth(10, 11);
File ficheroClaves;
char temp[TAMANOTEMP];
boolean primeraConexion;
boolean modoModificacion;

int cuentaLineas() {
    ficheroClaves = SD.open("ficheroClaves.txt");
  int nLineas = 0;
  while (ficheroClaves.peek() != -1) {
    if (ficheroClaves.read() == '\n') {
      nLineas ++;
    }
  }
  return nLineas;
}
int enFichero(char *texto){
  //Devuelve la linea en la que se encuentra el texto parametro, comenzando a contar por 1, o 0 si no esta
    ficheroClaves = SD.open("ficheroClaves.txt");
  char linea[100];
  int contador = 0;
  while(ficheroClaves.peek() != '\n' && contador<100){
    linea[contador] = ficheroClaves.read();
    }
    linea[contador] =
    if()
  }
void limpiarTemp() {
  for (int i = 0; i < TAMANOTEMP; i++) {
    temp[i] = '\0';
  }
}

void sendOK() {
  bluetooth.write("OK");
}
void sendDENY() {
  bluetooth.write("DENY");
}
//--------------------METODOS RELATIVOS A FASES DE LA CONEXION----------------------------------------
void primerHello() {
  limpiarTemp();
  int contador = 0;
  boolean enConfiguracion = true;
  boolean recibidoHello = false;

  while (enConfiguracion) {
    if (bluetooth.available() > 0) {
      if (!recibidoHello) {       //Recibe el primer HELLO de la conexion
        temp[contador] = bluetooth.read();
        contador = (contador + 1) % 5;
        if (strcmp(temp, "HELLO") == 0) {
          recibidoHello = true;
          sendOK();
          enConfiguracion = false;
        }
      }
    }
  }
}

uint8_t recibeClaveSesion(char *clavePublica){
//Recibe por BT la clave de sesion de la conexion cifrada con la clave privada asociada a la clave publica del parametro
//Devuelve la clave de sesion plana para la conexion AES  
  }
void setup() {
  SD.begin();
  Serial.begin(9600);
  bluetooth.begin(38400);
  ficheroClaves = SD.open("ficheroClaves.txt");
  primeraConexion = false;
  if (ficheroClaves.size() == 0 || cuentaLineas() == 1) {
    primeraConexion = true;
  }
}

void loop() {
  ficheroClaves = SD.open("ficheroClaves.txt");

  primerHello();  //Arduino recibe "HELLO" y responde con "OK"

  boolean configuracionDeModo = true;
  while (configuracionDeModo) {  //Arduino espera "0" para entrar en modo modificacion o "1" para modo abrir la cerradura
    if (bluetooth.available() > 0) {
      switch (bluetooth.read()) {
        case '0':
          modoModificacion = true;
          configuracionDeModo = false;
          sendOK();
          break;
        case '1':
          modoModificacion = false;
          configuracionDeModo = false;
          sendOK();
          break;
      }
    }
  }

  char clavePublica[TAMANOCLAVEPUBLICA] = recibeClavePublica();   //Arduino recibe la clave publica del movil que solicita la conexion en plano
  if (modoModificacion) { //Arduino trabaja en modo modificacion del fichero de claves publicas

  } else {    //Arduino trabaja en modo apertura de cerradura
    if (enFichero(clavePublica)>0) {  //Clave publica ha sido previamente registrada
      sendOK();
      Serial.println("La clave publica recibida: " + clavePublica + " se encuentra en el fichero");
      char okCifrado[TAMANOMENSAJECIFRADORSA];
      if (bluetooth.available() > 0) {
        int contador = 0;
        char caracter;
        while (contador < TAMANOMENSAJECIFRADORSA && caracter != '\0') {  //Arduino recibe "OK" cifrado con clave privada
          mensajeCifrado[contador] = bluetooth.read();
          contador++;
        }
      }
    } else { //Clave publica no se encuentra en fichero, se reinicia la conexion
      sendDENY();
      Serial.println("La clave publica recibida: " + clavePublica + " NO se encuentra en el fichero");
      Serial.println("----------------------------REINICIANDO CONEXION-------------------------");

      continue;
    }
    if (strcmp(descifraRSA(okCifrado, clavePublica), "OK") == 0) { //La clave publica recibida pertenece al movil que la envio
      Serial.println("La identidad del movil es legitima");

      uint8_t claveDeSesion[16] = recibeClaveSesion(clavePublica);  //Arduino recibe la clave de sesion cifrada mediante la clave privada
    } else { //El movil ha enviado una clave publica de la que no posee la privada, se reinica la conexion
      sendDENY();
      Serial.println("La identidad del movil es ILEGITIMA");
      Serial.println("----------------------------REINICIANDO CONEXION-------------------------");

      continue;
    }
  }
}
