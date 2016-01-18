/*ATOMIX Monitoring Carte 3

 15/04/2015 v1   TPT : correction de bugs, mise à jour des id des cartes
 14/04/2015 v0.6 VGE : mise au point des communications
 09/04/2015 v0.5 TPT : récupération des données du calculateur
 04/04/2015 v0.4 TPT : création automatique des fichiers sur la carte SD
 31/03/2015 v0.3 TPT : ajout du module de carte SD, stockage des variables sur la carte SD
 31/03/2015 v0.2 VGE : Récupération des données par le CANbus et les capteurs
 
  
 	
 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 8	 
 */

#include <SD.h>
#include <SPI.h>
#include "mcp_can.h"

//--------------  assignation des pins  ------------------

const int cpt_debat_arg = 0;
const int cpt_debat_ard = 1;
const int cpt_vit_arg = 2;
const int cpt_vit_ard = 3;
const int cpt_press_ar = 2;

//-------------------  donnÃ©es calcul  ----------------
const int diametre = 440;
const int nbr_trous = 24;
const int limite_rafraich = 12;

//--------------------  variables stockage  -------------------
float vitesse_arg ;
float vitesse_ard ;
int vitesse_avg ;
int vitesse_avd ;

int cpt_vitesse_g ;
int cpt_vitesse_d ;
long temps_g;
long temps_gdif;
long temps_d;
long temps_ddif;

int debat_arg ;
int debat_ard ;
int debat_avg ;
int debat_avd ;
int debat_cre ;

int press_av;
int press_ar;

int regime;
int rapport;
int temperature;

long temps;

int id;

//--------------------  initialisation Canbus  -------------------

unsigned char Flag_Recv = 0;
unsigned char len = 0;
unsigned char buf[8];
char str[20];
MCP_CAN CAN(10);

//--------------------  initialisation Shield SD  -------------------

const int chipSelect = 8;
File myFile;
char datafile[11];
Sd2Card card;
int numero = 0;

/*----------------------------------------------
 --------------------  Setup  -------------------
 ----------------------------------------------*/

void setup()
{
  Serial.begin(115200);

  //initialisation des pins
  pinMode(cpt_vit_arg,INPUT);
  pinMode(cpt_vit_ard,INPUT);
  //on active les résistances pull-up
  digitalWrite(cpt_vit_arg, HIGH);
  digitalWrite(cpt_vit_ard, HIGH);
  //on définit les interruption
  attachInterrupt(0,inter_vitesse_gauche,FALLING);
  attachInterrupt(1,inter_vitesse_droite,FALLING);

  temps_g=millis();
  temps_d=millis();

  //initialisation du shield SD
START_INIT_SD:  

  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (SD.begin(chipSelect)){
    Serial.println("SD Shield init ok!");
  }
  else
  {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    delay(100);
    goto START_INIT_SD;
  }

  //initialisation du shield CANbus
START_INIT_CAN:

  Serial.print("Initializing CAN Shield...");
  if(CAN_OK == CAN.begin(CAN_1000KBPS))                   // init can bus : baudrate = 1000k
  {
    Serial.println("CAN BUS Shield init ok!");
  }
  else
  {
    Serial.println("CAN BUS Shield init fail");
    delay(100);
    goto START_INIT_CAN;
  }

  //Creation d'un nouveau fichier d'enregistrement
  
  do{
    sprintf(datafile,"data%d.txt",numero);  
    numero++;
  }
  while(SD.exists(datafile));
  Serial.print(datafile);  
  Serial.println(" doesn't exist. Creation...");
  myFile = SD.open(datafile, FILE_WRITE);
  myFile.close();
}


/*----------------------------------------------
 ----------------  Interruption  ---------------
 ----------------------------------------------*/

void inter_vitesse_gauche() {
  cpt_vitesse_g++ ; 
}

void inter_vitesse_droite() {
  cpt_vitesse_d++ ;
}

/*----------------------------------------------
 --------------------  Loop  -------------------
 ----------------------------------------------*/

void loop()
{
 if (!card.init(SPI_HALF_SPEED, chipSelect)) {
asm volatile ("  jmp 0"); 
 }
 else{

  //--------------------  Stockage des données dans les variables  -------------------

  debat_arg=analogRead(cpt_debat_arg);
  debat_ard=analogRead(cpt_debat_ard);
  press_ar=analogRead(cpt_press_ar);

 if(cpt_vitesse_g >= limite_rafraich){
    temps_gdif = millis()-temps_g;
    temps_g = millis();  
    vitesse_arg = 3.14*diametre*3.6/((nbr_trous/limite_rafraich)*temps_gdif);
    cpt_vitesse_g=0;
    }
    
      
  if(cpt_vitesse_d >= limite_rafraich){
    temps_ddif = millis()-temps_d;
    temps_d = millis();   
    vitesse_ard = 3.14*diametre*3.6/((nbr_trous/limite_rafraich)*temps_ddif);
    cpt_vitesse_d=0;
    }
    
 if(millis()>=temps_d+1000){
   vitesse_ard = 0;
 }
 
 if(millis()>=temps_g+1000){
   vitesse_arg = 0;
 }

  //--------------------  Réception des données du CANbus  -------------------  
  for(int i=0; i<3; i++){
  if(CAN_MSGAVAIL == CAN.checkReceive())      // check if data coming
  {  
    CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
    id = CAN.getCanId();
    Serial.println(id);
    if(id==10)
    {
      vitesse_avg = buf[0];
      vitesse_avd = buf[1];
      debat_avg = buf[2]*4+buf[3];
      debat_avd = buf[4]*4+buf[5];
      debat_cre = buf[6]*4;
      press_av = buf[7]*4;
    }

    if(id==20)
    {
      rapport = buf[0];
    }

    if(id==8192)
    {
      regime = buf[0];
      temperature =buf[4];
    }

  }
 }
 
  temps=millis();

  File myFile = SD.open(datafile, FILE_WRITE);   
  if (myFile) {
    myFile.print(temps);
    myFile.print("\t");
    myFile.print(vitesse_arg);
    myFile.print("\t");
    myFile.print(vitesse_ard);
    myFile.print("\t");
    myFile.print(vitesse_avg);
    myFile.print("\t");
    myFile.print(vitesse_avd);
    myFile.print("\t");
    myFile.print(debat_arg);
    myFile.print("\t");
    myFile.print(debat_ard);
    myFile.print("\t");
    myFile.print(debat_avg);
    myFile.print("\t");
    myFile.print(debat_avd);
    myFile.print("\t");
    myFile.print(debat_cre);
    myFile.print("\t");
    myFile.print(press_av);
    myFile.print("\t");
    myFile.print(press_ar);
    myFile.print("\t");
    myFile.print(regime);
    myFile.print("\t");
    myFile.print(rapport);
    myFile.print("\t");
    myFile.print(temperature);
    myFile.println(""); 
    myFile.close();    
  }

  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening the file");
  } 
 }
}
















