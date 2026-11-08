#ifndef SATURNE_VECTEUR_H
#define SATURNE_VECTEUR_H

// ============================================================================
//  Module vecteur : calculs sur les vecteurs de l'espace (x, y, z).
//
//  Convention utilisée dans tout le projet : x vers la droite, y vers le bas
//  (comme les lignes d'une image) et z vers l'observateur, qui regarde
//  l'écran de face.
// ============================================================================

typedef struct {
    double x;
    double y;
    double z;
} TVecteur;

/**
 * Rôle : construit un vecteur à partir de ses trois coordonnées.
 * But : écrire les vecteurs de façon lisible, en une seule ligne.
 * Paramètres :
 *   - x, y, z : les coordonnées du vecteur
 * Retour : le vecteur (x, y, z)
 */
TVecteur creerVecteur(double x, double y, double z);

/**
 * Rôle : additionne deux vecteurs.
 * But : se déplacer dans l'espace, par exemple avancer le long d'un rayon
 *       lumineux (point + distance × direction).
 * Paramètres :
 *   - a, b : les deux vecteurs
 * Retour : a + b
 */
TVecteur additionnerVecteurs(TVecteur a, TVecteur b);

/**
 * Rôle : multiplie un vecteur par un nombre.
 * But : allonger, raccourcir ou retourner (k < 0) un vecteur.
 * Paramètres :
 *   - a : le vecteur
 *   - k : le coefficient
 * Retour : k × a
 */
TVecteur multiplierVecteur(TVecteur a, double k);

/**
 * Rôle : calcule le produit scalaire a · b = ax × bx + ay × by + az × bz.
 * But : c'est l'outil central de l'éclairage et des ombres. Pour deux vecteurs
 *       de longueur 1, il vaut le cosinus de l'angle qui les sépare : 1 s'ils
 *       sont alignés, 0 s'ils sont perpendiculaires, négatif s'ils s'opposent.
 * Paramètres :
 *   - a, b : les deux vecteurs
 * Retour : le produit scalaire
 */
double produitScalaire(TVecteur a, TVecteur b);

/**
 * Rôle : calcule la longueur (norme) d'un vecteur, racine de a · a.
 * But : mesurer des distances, par exemple entre un point et le centre de la planète.
 * Paramètres :
 *   - a : le vecteur
 * Retour : sa longueur
 */
double norme(TVecteur a);

/**
 * Rôle : renvoie le vecteur de même direction mais de longueur 1.
 * But : les formules d'éclairage utilisent des directions, donc des vecteurs
 *       de longueur 1 (vecteurs unitaires).
 * Paramètres :
 *   - a : le vecteur ; s'il est nul, il est renvoyé tel quel (il n'a pas de direction)
 * Retour : a divisé par sa norme
 */
TVecteur normaliser(TVecteur a);

/**
 * Rôle : fait tourner un vecteur d'un angle donné autour de l'axe z, c'est-à-dire
 *        dans le plan de l'image. La coordonnée z ne change pas.
 * But : pencher les anneaux dans l'image, ou au contraire les "redresser".
 * Paramètres :
 *   - a     : le vecteur à faire tourner
 *   - angle : l'angle de rotation en radians (sens des aiguilles d'une montre
 *             à l'écran, puisque l'axe y est dirigé vers le bas)
 * Retour : le vecteur tourné
 */
TVecteur tournerAutourDeZ(TVecteur a, double angle);

#endif
