// ============================================================================
//  Module couleur : création, mélange et conversion des couleurs.
//  La documentation de chaque fonction se trouve dans include/couleur.h.
// ============================================================================

#include "couleur.h"

TCouleur creerCouleur(double rouge, double vert, double bleu) {
    TCouleur couleur;

    couleur.rouge = rouge;
    couleur.vert = vert;
    couleur.bleu = bleu;
    return couleur;
}

TCouleur melangerCouleurs(TCouleur c1, TCouleur c2, double t) {
    return creerCouleur((1 - t) * c1.rouge + t * c2.rouge,
                        (1 - t) * c1.vert + t * c2.vert,
                        (1 - t) * c1.bleu + t * c2.bleu);
}

TCouleur multiplierCouleur(TCouleur c, double facteur) {
    return creerCouleur(c.rouge * facteur, c.vert * facteur, c.bleu * facteur);
}

TCouleur ajouterCouleurs(TCouleur c1, TCouleur c2) {
    return creerCouleur(c1.rouge + c2.rouge, c1.vert + c2.vert, c1.bleu + c2.bleu);
}

unsigned char composanteVersOctet(double composante) {
    if (composante <= 0) {
        return 0;
    }
    if (composante >= 255) {
        return 255;
    }
    // Ajouter 0,5 puis tronquer revient à arrondir à l'entier le plus proche
    return (unsigned char) (composante + 0.5);
}
