// ============================================================================
//  Module etoiles : ciel dégradé et étoiles placées au hasard.
//  La documentation de la fonction publique se trouve dans include/etoiles.h.
// ============================================================================

#include <math.h>
#include <stdlib.h>
#include "couleur.h"
#include "image.h"
#include "etoiles.h"

static const TCouleur CIEL_HAUT = {6, 8, 20};       // bleu nuit très sombre
static const TCouleur CIEL_BAS = {1, 1, 5};         // presque noir
static const TCouleur ETOILE_BLEUTEE = {200, 215, 255};
static const TCouleur ETOILE_CHAUDE = {255, 236, 210};

/**
 * Rôle : tire un nombre réel au hasard dans [0, 1[, chaque valeur ayant la
 *        même probabilité (loi uniforme).
 * But : choisir la position, l'éclat et la teinte de chaque étoile.
 * Retour : un réel dans [0, 1[
 */
static double tirageUniforme() {
    return rand() / (RAND_MAX + 1.0);
}

/**
 * Rôle : dessine une étoile comme une petite tache lumineuse : la lumière est
 *        maximale au centre et diminue doucement avec la distance (courbe en
 *        cloche, exp(-d² / 2r²)). Elle s'ajoute à ce qui est déjà dessiné.
 * But : une étoile réduite à un seul pixel paraîtrait carrée et crénelée ;
 *       une tache floue est plus naturelle.
 * Paramètres :
 *   - image  : l'image dans laquelle dessiner
 *   - x, y   : position du centre de l'étoile, en pixels (valeurs réelles)
 *   - eclat  : intensité au centre, entre 0 et 1
 *   - rayon  : taille de la tache, en pixels
 *   - teinte : couleur de l'étoile à pleine intensité
 */
static void dessinerEtoile(TImage *image, double x, double y, double eclat, double rayon, TCouleur teinte) {
    // Au-delà de 3 rayons, la lumière est devenue invisible : inutile d'aller plus loin
    int portee = (int) ceil(3 * rayon);

    for (int ligne = (int) y - portee; ligne <= (int) y + portee; ligne++) {
        for (int colonne = (int) x - portee; colonne <= (int) x + portee; colonne++) {
            // Distance entre le centre de l'étoile et le centre du pixel
            double dx = colonne + 0.5 - x;
            double dy = ligne + 0.5 - y;
            double intensite = eclat * exp(-(dx * dx + dy * dy) / (2 * rayon * rayon));

            TCouleur couleur = lirePixel(image, ligne, colonne);
            ecrirePixel(image, ligne, colonne, ajouterCouleurs(couleur, multiplierCouleur(teinte, intensite)));
        }
    }
}

void dessinerFondEtoile(TImage *image, unsigned int graine) {
    // 1. Le ciel : un dégradé vertical, ligne par ligne
    for (int ligne = 0; ligne < image->hauteur; ligne++) {
        double t = (double) ligne / image->hauteur;
        TCouleur ciel = melangerCouleurs(CIEL_HAUT, CIEL_BAS, t);
        for (int colonne = 0; colonne < image->largeur; colonne++) {
            ecrirePixel(image, ligne, colonne, ciel);
        }
    }

    // 2. Les étoiles. La graine fixe la suite de nombres produite par rand().
    srand(graine);
    int nbEtoiles = image->largeur * image->hauteur / PIXELS_PAR_ETOILE;

    for (int i = 0; i < nbEtoiles; i++) {
        double x = tirageUniforme() * image->largeur;
        double y = tirageUniforme() * image->hauteur;

        // u puissance 4 est le plus souvent proche de 0 : beaucoup d'étoiles
        // faibles et seulement quelques étoiles brillantes
        double u = tirageUniforme();
        double u4 = u * u * u * u;
        double eclat = 0.15 + 0.85 * u4;
        double rayon = 0.45 + 0.9 * u4;

        TCouleur teinte = melangerCouleurs(ETOILE_BLEUTEE, ETOILE_CHAUDE, tirageUniforme());
        dessinerEtoile(image, x, y, eclat, rayon, teinte);
    }
}
