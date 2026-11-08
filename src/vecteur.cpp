// ============================================================================
//  Module vecteur : opérations sur les vecteurs de l'espace.
//  La documentation de chaque fonction se trouve dans include/vecteur.h.
// ============================================================================

#include <math.h>
#include "vecteur.h"

TVecteur creerVecteur(double x, double y, double z) {
    TVecteur vecteur;

    vecteur.x = x;
    vecteur.y = y;
    vecteur.z = z;
    return vecteur;
}

TVecteur additionnerVecteurs(TVecteur a, TVecteur b) {
    return creerVecteur(a.x + b.x, a.y + b.y, a.z + b.z);
}

TVecteur multiplierVecteur(TVecteur a, double k) {
    return creerVecteur(k * a.x, k * a.y, k * a.z);
}

double produitScalaire(TVecteur a, TVecteur b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double norme(TVecteur a) {
    return sqrt(produitScalaire(a, a));
}

TVecteur normaliser(TVecteur a) {
    double longueur = norme(a);

    if (longueur == 0) {
        return a;
    }
    return multiplierVecteur(a, 1.0 / longueur);
}

TVecteur tournerAutourDeZ(TVecteur a, double angle) {
    // Formule de rotation dans un plan : x' = x cos - y sin, y' = x sin + y cos
    double c = cos(angle);
    double s = sin(angle);

    return creerVecteur(a.x * c - a.y * s, a.x * s + a.y * c, a.z);
}
