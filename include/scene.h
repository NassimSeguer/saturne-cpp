#ifndef SATURNE_SCENE_H
#define SATURNE_SCENE_H

// ============================================================================
//  Module scene : la planète, ses anneaux et la lumière du Soleil.
//
//  Les calculs se font dans le "repère de la scène" : son origine est le
//  centre de la planète, l'unité est le pixel, et il est tourné par rapport à
//  l'image pour que les anneaux y soient horizontaux. Un point de l'image
//  (x, y) y devient (u, v) ; la troisième coordonnée z va vers l'observateur.
// ============================================================================

#include "couleur.h"
#include "vecteur.h"

#define NB_ANNEAUX 3

/**
 * Un anneau : une couronne plate centrée sur la planète.
 *   - nom            : sa désignation ("C", "B" ou "A")
 *   - rayonInterieur : distance de son bord intérieur au centre, en rayons de la planète
 *   - rayonExterieur : distance de son bord extérieur, en rayons de la planète
 *   - couleur        : sa couleur de base
 *   - opacite        : 0 pour un anneau transparent, 1 pour un anneau opaque
 */
typedef struct {
    const char *nom;
    double rayonInterieur;
    double rayonExterieur;
    TCouleur couleur;
    double opacite;
} TAnneau;

/**
 * Tous les paramètres de la scène.
 *   - centreX, centreY : centre de la planète dans l'image, en pixels
 *   - rayon            : rayon de la planète, en pixels
 *   - rotation         : angle des anneaux dans l'image, en radians
 *   - inclinaison      : angle sous lequel on voit le plan des anneaux, en
 *                        radians (0 = vus par la tranche, 90° = vus de dessus)
 *   - normaleAnneaux   : vecteur de longueur 1 perpendiculaire au plan des anneaux
 *   - lumiere          : vecteur de longueur 1 dirigé vers le Soleil
 *   - anneaux          : les anneaux, du plus proche au plus éloigné de la planète
 */
typedef struct {
    double centreX;
    double centreY;
    double rayon;
    double rotation;
    double inclinaison;
    TVecteur normaleAnneaux;
    TVecteur lumiere;
    TAnneau anneaux[NB_ANNEAUX];
} TScene;

/**
 * Rôle : règle tous les paramètres de la scène pour une image de taille
 *        donnée : planète centrée, anneaux aux proportions réelles, Soleil
 *        placé à gauche et légèrement devant, au-dessus du plan des anneaux.
 * But : préparer la scène avant le rendu. La taille de la planète s'adapte
 *       à l'image pour que l'ensemble tienne avec une marge.
 * Paramètres :
 *   - scene   : la scène à remplir
 *   - largeur : largeur de l'image, en pixels
 *   - hauteur : hauteur de l'image, en pixels
 */
void initialiserScene(TScene *scene, int largeur, int hauteur);

/**
 * Rôle : convertit un point de l'image (x, y) en coordonnées (u, v) du repère
 *        de la scène : on se place au centre de la planète, puis on tourne
 *        pour que les anneaux deviennent horizontaux.
 * But : toutes les formules de la scène sont plus simples dans ce repère.
 * Paramètres :
 *   - scene : la scène
 *   - x, y  : le point de l'image, en pixels
 *   - u, v  : reçoivent les coordonnées dans le repère de la scène
 */
void imageVersScene(const TScene *scene, double x, double y, double *u, double *v);

/**
 * Rôle : conversion inverse de imageVersScene : repère de la scène -> image.
 * But : retrouver où se trouve dans l'image un point de la scène (utile pour
 *       les tests).
 * Paramètres :
 *   - scene : la scène
 *   - u, v  : le point dans le repère de la scène
 *   - x, y  : reçoivent la position dans l'image, en pixels
 */
void sceneVersImage(const TScene *scene, double u, double v, double *x, double *y);

/**
 * Rôle : renvoie le point du plan des anneaux que l'on voit à la position
 *        (u, v). En regardant la scène de face, tous les points (u, v, z) se
 *        superposent ; un seul d'entre eux appartient au plan des anneaux.
 * But : savoir sur quel anneau tombe un pixel, et si ce morceau d'anneau
 *       passe devant la planète (z > 0) ou derrière (z < 0).
 * Paramètres :
 *   - scene : la scène
 *   - u, v  : la position dans le repère de la scène
 * Retour : le point (u, v, z) du plan des anneaux
 */
TVecteur pointSurAnneaux(const TScene *scene, double u, double v);

/**
 * Rôle : cherche quel anneau se trouve à une distance donnée du centre.
 * But : déterminer la couleur et la transparence d'un point du plan des anneaux.
 * Paramètres :
 *   - scene    : la scène
 *   - distance : distance au centre de la planète, en rayons de la planète
 * Retour : l'indice de l'anneau dans scene->anneaux, ou -1 s'il n'y a pas
 *          d'anneau à cette distance (par exemple dans la division de Cassini)
 */
int indiceAnneau(const TScene *scene, double distance);

/**
 * Rôle : indique si un point de l'espace est dans l'ombre de la planète.
 *        On suit le rayon lumineux qui part du point vers le Soleil : s'il
 *        rencontre la planète, le point ne reçoit pas de lumière directe.
 * But : dessiner l'ombre de la planète projetée sur ses anneaux.
 * Paramètres :
 *   - scene : la scène
 *   - point : le point à tester, situé hors de la planète
 * Retour : 1 si le point est dans l'ombre, 0 sinon
 */
int estDansOmbreDeLaPlanete(const TScene *scene, TVecteur point);

/**
 * Rôle : calcule la couleur vue en un point de l'image : la planète, un
 *        anneau, ou le fond, en tenant compte de l'éclairage, des ombres, de
 *        la transparence des anneaux et de ce qui passe devant quoi.
 * But : c'est la fonction appelée pour chaque échantillon lors du rendu.
 * Paramètres :
 *   - scene : la scène
 *   - x, y  : le point de l'image, en pixels (valeurs réelles : on peut viser
 *             une fraction de pixel pour le sur-échantillonnage)
 *   - fond  : la couleur du ciel à cet endroit, visible à travers les anneaux
 * Retour : la couleur du point
 */
TCouleur couleurPoint(const TScene *scene, double x, double y, TCouleur fond);

#endif
