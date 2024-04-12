/***************************************************
 HUSKYLENS An Easy-to-use AI Machine Vision Sensor
 <https://www.dfrobot.com/product-1922.html>
 
 ***************************************************
 This example shows the basic function of library for HUSKYLENS via Serial.
 
 Created 2020-03-13
 By [Angelo qiao](Angelo.qiao@dfrobot.com)
 
 GNU Lesser General Public License.
 See <http://www.gnu.org/licenses/> for details.
 All above must be included in any redistribution
 ****************************************************/

/***********Notice and Trouble shooting***************
 1.Connection and Diagram can be found here
 <https://wiki.dfrobot.com/HUSKYLENS_V1.0_SKU_SEN0305_SEN0336#target_23>
 2.This code is tested on Arduino Uno, Leonardo, Mega boards.
 ****************************************************/

#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

//******************************************************
// Déclaration des constantes
//******************************************************

const float D = 61;     // diamètre de la roue sur axe du moteur pas à pas ou 61.1 mm
const int Np = 512;     //  nombre de pas par tour      
const int duree = 2;    //  vitesse de rotation en ms (durée entre chaque pas)
const float voie = 108; //  voie: distance en mm entre les roues
const int a = 10;       // a longueur en mm

// Sequence orange -> rose -> bleu -> jaune
byte brocheMppD[] = {11, 9, 8, 10};
byte brocheMppG[] = {7, 5, 4, 6};

boolean masque_SensHoraire[][4] =  
{ {1, 0, 1, 0},
  {0, 1, 1, 0},
  {0, 1, 0, 1},
  {1, 0, 0, 1}
};

boolean masque_SensAntiHoraire[][4] =  
{ {1, 0, 0, 1},
  {0, 1, 0, 1},
  {0, 1, 1, 0},
  {1, 0, 1, 0}
};

HUSKYLENS huskylens;
SoftwareSerial mySerial(2, 3); // RX, TX
//HUSKYLENS green line >> Pin 10; blue line >> Pin 11
void printResult(HUSKYLENSResult result);

void setup() {
    Serial.begin(115200);
    mySerial.begin(9600);
    for (byte broche = 0; broche < 4; broche++) 
    {
      pinMode(brocheMppG[broche], OUTPUT);
      pinMode(brocheMppD[broche], OUTPUT);
    }
    arret();
    while (!huskylens.begin(mySerial))
    {
        Serial.println(F("Begin failed!"));
        Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
        Serial.println(F("2.Please recheck the connection."));
        delay(100);
    }
}

void loop() {
    if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
    else if(!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
    else if(!huskylens.available()) {
      Serial.println(F("No block or arrow appears on the screen!"));
      avance(900);
      droite(90);
      avance(300);
      droite(90);
      avance(900);
      
    }
    else
    {
        Serial.println(F("###########"));
        while (huskylens.available())
        {
            HUSKYLENSResult result = huskylens.read();
            printResult(result);
        }    
    }
}

void printResult(HUSKYLENSResult result){
    if (result.command == COMMAND_RETURN_BLOCK){
        Serial.println(String()+F("Block:xCenter=")+result.xCenter+F(",yCenter=")+result.yCenter+F(",width=")+result.width+F(",height=")+result.height+F(",ID=")+result.ID);
        if (result.xCenter > 140 and result.xCenter < 180){
          Serial.println("balle au milieu"); 
          float d;
          d = (15.0 / result.width)* 100;
          Serial.println(String(d));
          
          avance((d-5)*10);
          delay(10000);
        }
        else {
          int pos = result.xCenter;

          int centre = 160; 
          
          int largeur = 320;
        
          int diff = pos - centre;
        
          float angle = atan((float) diff / largeur) * 180.0 / PI;
          if (angle < 0){
            angle = angle * (-1);
          }
          Serial.println(angle);
          if (pos > 160) {
            droite(angle);
            delay(1000);
          }

          if (pos < 160){
            gauche(angle);
            delay(1000);
          }
          
        }
    }
    else if (result.command == COMMAND_RETURN_ARROW){
        Serial.println(String()+F("Arrow:xOrigin=")+result.xOrigin+F(",yOrigin=")+result.yOrigin+F(",xTarget=")+result.xTarget+F(",yTarget=")+result.yTarget+F(",ID=")+result.ID);
    }
    else{
        Serial.println("Object unknown!");
    }
}



//******************************************************
// Déclarations des fonctions
//******************************************************

void arret()
{
  for (byte masque = 0; masque < 4; masque++)
  {
    for (byte broche = 0; broche < 4; broche++)
    {
      digitalWrite(brocheMppG[broche], LOW);
      digitalWrite(brocheMppD[broche], LOW);
    }
    delay(duree);
  }
}

int step(float distance)
{
  int steps = distance * Np / (D * 3.1412); 
//  Serial.print(distance);
//  Serial.print(" ");
//  Serial.print(Np);
//  Serial.print(" ");
//  Serial.print(D);
//  Serial.print(" ");
//  Serial.println(steps);
  return steps;
}

void avance(float distance)
{
  int steps = step(distance);
  for (int step = 0; step < steps; step++)
  {
    for (byte masque = 0; masque < 4; masque++)
    {
      for (byte broche = 0; broche < 4; broche++)
      {
        digitalWrite(brocheMppG[broche], masque_SensAntiHoraire[masque][broche]);
        digitalWrite(brocheMppD[broche], masque_SensHoraire[masque][broche]);
      }
      delay(duree);
    }
  }
}

void recule(float distance)
{
  int steps = step(distance);
  for (int step = 0; step < steps; step++)
  {
    for (byte masque = 0; masque < 4; masque++)
    {
      for (byte broche = 0; broche < 4; broche++)
      {
        digitalWrite(brocheMppG[broche], masque_SensHoraire[masque][broche]);
        digitalWrite(brocheMppD[broche], masque_SensAntiHoraire[masque][broche]);
      }
      delay(duree);
    }
  }
}

void droite(float degres)
{
  float rotation = degres / 360.0;
  float dist = voie * 3.1416 * rotation;
  int steps = step(dist);
  for (int step = 0; step < steps; step++)
  {
    for (byte masque = 0; masque < 4; masque++)
    {
      for (byte broche = 0; broche < 4; broche++)
      {
        digitalWrite(brocheMppG[broche], masque_SensAntiHoraire[masque][broche]);
        digitalWrite(brocheMppD[broche], masque_SensAntiHoraire[masque][broche]);
      }
      delay(duree);
    }
  }
}

void gauche(float degres)
{
  float rotation = degres / 360.0;
  float dist = voie * 3.1416 * rotation;
  int steps = step(dist);
  for (int step = 0; step < steps; step++)
  {
    for (byte masque = 0; masque < 4; masque++)
    {
      for (byte broche = 0; broche < 4; broche++)
      {
        digitalWrite(brocheMppG[broche], masque_SensHoraire[masque][broche]);
        digitalWrite(brocheMppD[broche], masque_SensHoraire[masque][broche]);
      }
      delay(duree);
    }
  }
}
