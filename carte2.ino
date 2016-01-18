//ATOMIX Monitoring Carte 2

//24/04/2015 v1.3 JGR
//23/04/2015 v1.2 VGE delay enlevé
//22/04/2015 v1.0 TPT récupération des données issues du calculateur et de la boite de vitesse
//12/04/2015 v0.3 VGE mise a jour des id canbus et des infos calculo
//31/03/2015 v0.2 VGE


//--------------  assignation des pins  ------------------
const int btn_1 = 49;
const int btn_2 = 43;
const int btn_3 = 47;
const int btn_4 = 45;
const int led_homing = 41;
const int GND_1 = 39;

const int in_0 = 27;
const int in_1 = 25;
const int in_2 = 33;
const int in_3 = 31;
const int in_4 = 23;
const int out_1 = 37;
const int out_2 = 29;
const int GND_2 = 35;

const int out_rapport = 3;
const int shift_cut = 5;

//-------------------  données calcul  ----------------
boolean etat_btn_1;
boolean etat_btn_1_prec;
boolean etat_btn_2;
boolean etat_btn_2_prec;
boolean etat_btn_3;
boolean etat_btn_3_prec;
boolean etat_btn_4;
boolean etat_btn_4_prec;
boolean led_clignotante;
boolean led_force;
boolean homed;
boolean error;
long date_clignotement;
long date_coupure;
int delai_clignotement = 200;
int temps_coupure = 400;
const long refresh_can = 250;

//--------------------  variables stockage  -------------------

int id;
int rapport_recherche;
int rapport_engage;
boolean positions[16][4];
long temps_can;

int regime;
int rapport = 3;

//initialisation Canbus
#include <SPI.h>
#include "mcp_can.h"
MCP_CAN CAN(53);

unsigned char Flag_Recv = 0;
unsigned char len = 0; //à vérifier
unsigned char buf[8];
char str[20];

//variables mixou

void setup() {
  Serial.begin(115200);
  
    //initialisation canbus
  
START_INIT:
  if(CAN_OK == CAN.begin(CAN_1000KBPS))  
  {
    Serial.println("CAN BUS Shield init ok!");
  }
  else
  {
    Serial.println("CAN BUS Shield init fail");
    Serial.println("Init CAN BUS Shield again");
    delay(100);
    goto START_INIT;
  }

date_clignotement = millis();
  led_clignotante = true;
  led_force = false;
  homed = false;
  error = false;
  
  rapport_recherche = 0;    
  rapport_engage = rapport_recherche;
  
  pinMode(btn_1, INPUT);
  pinMode(btn_2, INPUT);
  pinMode(btn_3, INPUT);
  pinMode(btn_4, INPUT);  
  pinMode(led_homing, OUTPUT);
  pinMode(GND_1, OUTPUT);
  
  digitalWrite(btn_1, HIGH);
  digitalWrite(btn_2, HIGH);  
  digitalWrite(btn_3, HIGH);
  digitalWrite(btn_4, HIGH);
  digitalWrite(led_homing, LOW);
  digitalWrite(GND_1, LOW);
  
  pinMode(in_0, OUTPUT);
  pinMode(in_1, OUTPUT);
  pinMode(in_2, OUTPUT);
  pinMode(in_3, OUTPUT);
  pinMode(in_4, OUTPUT);
  pinMode(out_1, INPUT);
  pinMode(out_2, INPUT);
  pinMode(GND_2, OUTPUT);
    
  digitalWrite(in_0, LOW);
  digitalWrite(in_1, LOW);
  digitalWrite(in_2, LOW);
  digitalWrite(in_3, LOW);
  digitalWrite(in_4, LOW);  
  digitalWrite(GND_2, LOW);
  
  pinMode(out_rapport, OUTPUT);
  
  pinMode(shift_cut, OUTPUT);  
  digitalWrite(shift_cut, HIGH);
  
  
  etat_btn_1_prec = HIGH;
  etat_btn_2_prec = HIGH;
    
  positions[0][0] = 0;
  positions[0][1] = 0;
  positions[0][2] = 0;
  positions[0][3] = 0;  
  
  positions[1][0] = 1;
  positions[1][1] = 0;
  positions[1][2] = 0;
  positions[1][3] = 0;
  
  positions[2][0] = 0;
  positions[2][1] = 1;
  positions[2][2] = 0;
  positions[2][3] = 0;
  
  positions[3][0] = 1;
  positions[3][1] = 1;
  positions[3][2] = 0;
  positions[3][3] = 0;
  
  positions[4][0] = 0;
  positions[4][1] = 0;
  positions[4][2] = 1;
  positions[4][3] = 0;
  
  positions[5][0] = 1;
  positions[5][1] = 0;
  positions[5][2] = 1;
  positions[5][3] = 0;
  
  positions[6][0] = 0;
  positions[6][1] = 1;
  positions[6][2] = 1;
  positions[6][3] = 0;
  
  positions[7][0] = 1;
  positions[7][1] = 1;
  positions[7][2] = 1;
  positions[7][3] = 0;
  
  positions[8][0] = 0;
  positions[8][1] = 0;
  positions[8][2] = 0;
  positions[8][3] = 1;
  
  positions[9][0] = 1;
  positions[9][1] = 0;
  positions[9][2] = 0;
  positions[9][3] = 1;
  
  positions[10][0] = 0;
  positions[10][1] = 1;
  positions[10][2] = 0;
  positions[10][3] = 1;
  
  positions[11][0] = 1;
  positions[11][1] = 1;
  positions[11][2] = 0;
  positions[11][3] = 1;
  
  positions[12][0] = 0;
  positions[12][1] = 0;
  positions[12][2] = 1;
  positions[12][3] = 1;
  
  positions[13][0] = 1;
  positions[13][1] = 0;
  positions[13][2] = 1;
  positions[13][3] = 1;
  
  positions[14][0] = 0;
  positions[14][1] = 1;
  positions[14][2] = 1;
  positions[14][3] = 1;
  
  positions[15][0] = 1;
  positions[15][1] = 1;
  positions[15][2] = 1;
  positions[15][3] = 1;
 
}


//-----------------------------------------------
//-------------------  Loop ---------------------

void loop() {
  
etat_btn_1 = digitalRead(btn_1);

  if(etat_btn_1 != etat_btn_1_prec)
  {
    if(!etat_btn_1)
    {
      action_btn_1();
      Serial.write("bouton 1");
    }
    etat_btn_1_prec = !etat_btn_1_prec;
  }

  etat_btn_2 = digitalRead(btn_2);

  if(etat_btn_2 != etat_btn_2_prec)
  {
    if(!etat_btn_2)
    {
      action_btn_2();
      Serial.write("bouton 2");
    }
    etat_btn_2_prec = !etat_btn_2_prec;
  }
  
  etat_btn_3 = digitalRead(btn_3);

  if(etat_btn_3 != etat_btn_3_prec)
  {
    if(!etat_btn_3)
    {
      action_btn_3();
      Serial.write("bouton 3");
    }
    etat_btn_3_prec = !etat_btn_3_prec;
  }

  etat_btn_4 = digitalRead(btn_4);

  if(etat_btn_4 != etat_btn_4_prec)
  {
    if(!etat_btn_4)
    {
      action_btn_4();
      Serial.write("bouton 3");
    }
    etat_btn_4_prec = !etat_btn_4_prec;
  }
  
  engager_rapport(rapport_recherche);
  
  
  if(true) //if(moteur électrique stoppé)
  {
    rapport_engage = rapport_recherche;
  }
  
  if(!digitalRead(out_1) && !digitalRead(out_2))
  {
    error = true;
    homed = false;
    delai_clignotement = 500;
    led_clignotante = true;
    led_force = false;
  }
  
  else
  {  
    error = false;    
    delai_clignotement = 200;
    led_clignotante = false;
    led_force = false;
  }
  
  analogWrite(out_rapport, max(0, rapport_engage-2)*255/6);
  
  clignoter_led();
  
  if(millis() - date_coupure > temps_coupure)
    digitalWrite(shift_cut, HIGH);
  
//envoyer CANBUS
if(millis()>=temps_can + refresh_can){
unsigned char stmp[8] = {rapport_engage,0,0,0,0,0,0,0};  
  // send data:  id = 0x00, standard flame, data len = 8, stmp: data buf
  CAN.sendMsgBuf(20, 0, 8, stmp);
temps_can = millis();  
}
}

void clignoter_led()
{
  
  if(millis() - date_clignotement > delai_clignotement)
    date_clignotement = millis();
  
  if(millis() - date_clignotement < (delai_clignotement/2) && led_clignotante)
  {
    digitalWrite(led_homing, HIGH);
  }
  else
  {
    if(!led_force)
      digitalWrite(led_homing, LOW);
    else
      digitalWrite(led_homing, HIGH);
  }
  
}

void engager_rapport(int rapport)
{

  digitalWrite(in_0, LOW);
  digitalWrite(in_1, positions[rapport][0]);
  digitalWrite(in_2, positions[rapport][1]);
  digitalWrite(in_3, positions[rapport][2]);
  digitalWrite(in_4, positions[rapport][3]); 
  
}

void action_btn_1()
{
  if(rapport_recherche > 3)
  {
    rapport_recherche--;
  }
}

void action_btn_2()
{
  if(millis() > 1000)
  {
    if(rapport_recherche < 8 && rapport_recherche > 1)
    {
      rapport_recherche++;
      digitalWrite(shift_cut, LOW);
      date_coupure = millis();
    }
    else if(rapport_recherche == 1)
      rapport_recherche = 3;
      
    
  }
}

void action_btn_3()
{
  if(error)
  {
    rapport_recherche = 0;
  }
  
  else if (!error && millis() > 1000)
  {  
    if(rapport_recherche == 1)
    {
      rapport_recherche = 2;
      delay(50);
      rapport_recherche = 1; 
    }
    else
      rapport_recherche = 1;
  }
  
}

void action_btn_4()
{
  rapport_recherche = 2;
}
