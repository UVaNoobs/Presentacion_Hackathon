#include <aes_keyschedule.h>
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

#include <SoftwareSerial.h>
#include <SD.h>
#include <string.h>


#define TAMANONOMBREMOVIL 40
#define TAMANOCLAVESIMETRICA 32
#define TAMANOMENSAJECIFRADO 128    //TODO
#define DIGITOSNUMEROAUTENTICACION 4

SoftwareSerial bluetooth(10, 11);
File ficheroClaves;
uint8_t claveSimetrica[TAMANOCLAVESIMETRICA];
boolean primeraConexion;

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

int fase1() {
  //Devuelve el numero de autenticacion enviado al final de la fase 1 de conexion en plano o -1 si se aborto la conexion
  //Recibe por BT el nombre del movil que solicita la conexion y envia por BT el numero de autenticacion para la conexion cifrado con su clave simetrica
  //si la conexion se permite o "NO" si la conexion se aborta
  Serial.println("Fase 1 de conexion");
  char nombreMovil[TAMANONOMBREMOVIL];
  int contador = 0;

  while (true) {
    if (bluetooth.available() > 0) {
      char caracterEnLectura = bluetooth.read();
      while (contador < TAMANONOMBREMOVIL && caracterEnLectura != '\0') {   //Arduino recibe nombre del movil
        nombreMovil[contador] = caracterEnLectura;
        contador ++;
        caracterEnLectura = bluetooth.read();
      }
      Serial.print(nombreMovil);
      Serial.println(" solicita conexion");

      if (nombreEnFichero(nombreMovil) == true) {       //Arduino comprueba que el movil esta en el fichero de nombres
        claveSimetrica = getClaveSimetrica(nombreMovil);
        Serial.print(nombreMovil);
        Serial.println(" SI se encuentra en fichero");
        int numeroDeAutenticacion = (int)random(pow(10,DIGITOSNUMEROAUTENTICACION));   //Arduino envia numero de autenticacion de identidad de movil en plano
        char *numeroDeAutenticacionCifrado = toString(numeroDeAutenticacion);
        aes256_enc_single(claveSimetrica, numeroDeAutenticacionCifrado);
        bluetooth.write(numeroDeAutenticacionCifrado);

        return numeroDeAutenticacion;
      } else {
        //Nombre no esta en fichero
        Serial.print(nombreMovil);
        Serial.println(" NO se encuentra en fichero");
        bluetooth.write("NO");

        return -1;
      }
    }
  }
}
boolean fase2(int numeroDeAutenticacion) {
  //Autentica la identidad del movil
  //Recibe por BT el numero de autenticacion cifrado con la clave simetrica del movil que solicita la conexion
  //Envia por BT "OK" o "NO" en funcion de si la conexion se aborto o no
  //Devuelve true si la conexion continua, false si se aborta
  char numeroDeAutenticacionPlano[] = toString(numeroDeAutenticacion);
  char numeroDeAutenticacionCifrado[TAMANOMENSAJECIFRADO];
  int contador = 0;

  while (true) {
    if (bluetooth.available() > 0) {
      char caracterEnLectura = bluetooth.read();

      while (contador < TAMANOMENSAJECIFRADO && caracterEnLectura != '\0') {   //Arduino recibe nombre del movil
        numeroDeAutenticacionCifrado[contador] = caracterEnLectura;
        contador ++;
        caracterEnLectura = bluetooth.read();
      }

      aes256_dec_single(claveSimetrica, numeroDeAutenticacionCifrado);
      if (strcmp(numeroDeAutenticacionCifrado, numeroDeAutenticacionPlano) == 0) {
        //Autenticado
        bluetooth.write("OK");
        Serial.println("Identidad confirmada");
        return true;
      } else {
        //Falso
        bluetooth.write("NO");
        Serial.println("Identidad falsa. Abortando conexion");
        return false;
      }
    }
  }
}
boolean fase3() {
  //Devuelve true si el Arduino trabaja en modo MODIFY DATABASE o false si trabaja en modo OPEN
  //Recibe por BT "0" para entrar en modo MODIFY DATABASE o "1" para entrar en modo OPEN
  //Envia por BT "OK" cuando accede al modo solicitado
  boolean configuracionDeModo = true;
  while (true) {  //Arduino espera "0" para entrar en modo modificacion o "1" para modo abrir la cerradura
    if (bluetooth.available() > 0) {
      switch (bluetooth.read()) {
        case '0':
          bluetooth.write("OK");
          Serial.println("Entrando en modo MODIFY DATABASE");
          return true;
        case '1':
          bluetooth.write("OK");

          Serial.println("Entrando en modo OPEN");
          return false;
      }
    }
  }
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
  int numeroDeAutenticacion = fase1();
  if (numeroDeAutenticacion != -1) {
    boolean continuar = fase2((numeroDeAutenticacion+1)%((int)pow(10,DIGITOSNUMEROAUTENTICACION)));
    if (continuar == true) {
      boolean modoModificacion = fase3();


      if (modoModificacion == true) {   //Arduino trabaja en modo MODIFY DATABASE

      } else {      //Arduino trabaja en modo OPEN

      }
    } else {
      Serial.println("");
      Serial.println("----------------------REINICIO DE CONEXION-------------------");
      Serial.println("");
    }

  }
  else {
    Serial.println("");
    Serial.println("----------------------REINICIO DE CONEXION-------------------");
    Serial.println("");
  }
}
