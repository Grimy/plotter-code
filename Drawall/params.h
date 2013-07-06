/*
Ce travail est sous licence Creative Commons Attribution - Pas d’Utilisation Commerciale - Partage dans les Mêmes Conditions 3.0 France License. Pour voir voir une copie de cette licence, ouvrez le fichier LIENCE.txt ou connectez-vous sur http://creativecommons.org/licenses/by-nc-sa/3.0/fr/.
*/

/**
 * \file    params.h
 * \author  Nathanaël Jourdane
 * \version 1.0
 * \date    1er juillet 2013
 * \brief   Fichier de paramètres
 */

/**********************
* Définition des pins *
**********************/

// Pins 0 et 1 : utilisées par le port série (communication avec Processing)

/// Pin du capteur fin de course du moteur gauche.
#define PIN_LEFT_CAPTOR 2

/// Pin du capteur fin de course du moteur droit.
#define PIN_RIGHT_CAPTOR 3

/// Pin pour desactivation des moteurs (pin /en sur pololu).
#define PIN_OFF_MOTORS 4

/// Pin de commande du servo-moteur.
#define PIN_SERVO 5

/// Pin STEP (les pas) du moteur gauche.
#define PIN_LEFT_MOTOR_STEP 6

/// Pin DIR (la direction) du moteur gauche.
#define PIN_LEFT_MOTOR_DIR 7

/// Pin STEP (les pas) du moteur droit.
#define PIN_RIGHT_MOTOR_STEP 8 //

/// Pin DIR (la direction) du moteur droit.
#define PIN_RIGHT_MOTOR_DIR 9

/// Pin CS de la carte SD.
/// Snootlab et Adafruit : 10 - Sparkfun : 8
#define PIN_SD_CS 10

// Pins 11, 12 et 13 : utilisés par la carte SD (MOSI, MISO, SCK)

/// Pin de la diode infra-rouge pour télécommande.
#define PIN_REMOTE A0

/// Pin SCE de l'écran LCD
#define PIN_SCREEN_SCE A1

/// Pin RESET de l'écran LCD
#define PIN_SCREEN_RST A2

/// Pin DC de l'écran LCD
#define PIN_SCREEN_DC A3

/// Pin SDIN de l'écran LCD
#define PIN_SCREEN_SDIN A4

/// Pin SCLK de l'écran LCD
#define PIN_SCREEN_SCLK A5

/*************
* paramètres *
*************/

/***** du servo-moteur *****/

/// position lorsque le robot n'écrit pas (en degrés)
#define MIN_SERVO 30

/// position lorsque le robot écrit (en degrés)
#define MAX_SERVO 90

/// Pause avant chaque déplacement (en ms)
#define DELAY_BEFORE_SERVO 200

/// Pause après chaque déplacement (en ms)
#define DELAY_AFTER_SERVO 200

/***** des moteurs *****/

/// nombre de pas (prendre en compte les micros-pas générés par le driver-moteur)
#define STEPS 400

/// Diamètre du pignon (en mm)
#define DIAMETER 12

/// Direction du moteur gauche
#define LEFT_DIRECTION true

/// Direction du moteur droit
#define RIGHT_DIRECTION true

/*******************
* Autres variables *
*******************/

/// Pause avant de commencer à desiner (en ms)
#define DELAY_BEFORE_STARTING 2000

/// Vitesse de la communication série (en bauds)
#define SERIAL_BAUDS 9600

/// Échelle horizontale appliquée au dessin, permetant de le calibrer.
#define SCALE_X 1.5

/// Échelle verticale appliquée au dessin, permetant de le calibrer.
#define SCALE_Y 1.5

/// Offset horizontal appliqué au dessin, permetant de le décaler.
#define OFFSET_X 0

/// Offset vertical appliqué au dessin, permetant de le décaler.
#define OFFSET_Y 0

/// Vitesse par défaut du crayon, avant l'appel éventuel de setSpeed() (en m/s).
#define DEFAULT_SPEED 10

/// Position par défaut du point de départ du crayon, avant l'appel éventuel de setPosition().
#define DEFAULT_POSITION CENTER