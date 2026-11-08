#ifndef SATURNE_COULEUR_H
#define SATURNE_COULEUR_H

// ============================================================================
//  Module couleur : représentation et mélange des couleurs.
//
//  Une couleur est décrite par ses composantes rouge, vert et bleu, chacune
//  entre 0 et 255. Pendant les calculs, elles restent des réels (double) pour
//  ne pas accumuler d'erreurs d'arrondi ; elles ne sont converties en octets
//  qu'au moment de ranger le pixel dans l'image.
// ============================================================================

typedef struct {
    double rouge;
    double vert;
    double bleu;
} TCouleur;

/**
 * Rôle : construit une couleur à partir de ses trois composantes.
 * But : écrire les couleurs de façon lisible, en une seule ligne.
 * Paramètres :
 *   - rouge, vert, bleu : les composantes, normalement entre 0 et 255
 * Retour : la couleur correspondante
 */
TCouleur creerCouleur(double rouge, double vert, double bleu);

/**
 * Rôle : mélange deux couleurs. t = 0 donne c1, t = 1 donne c2, et une valeur
 *        intermédiaire donne une couleur entre les deux (alpha blending).
 * But : dégradés, transparence des anneaux, bandes nuageuses de la planète.
 * Paramètres :
 *   - c1 : la première couleur
 *   - c2 : la seconde couleur
 *   - t  : la proportion de c2 dans le mélange, entre 0 et 1
 * Retour : (1 - t) × c1 + t × c2, composante par composante
 */
TCouleur melangerCouleurs(TCouleur c1, TCouleur c2, double t);

/**
 * Rôle : multiplie les trois composantes d'une couleur par un même facteur.
 * But : assombrir (facteur < 1) ou éclaircir (facteur > 1) une couleur,
 *       par exemple selon l'éclairage reçu ou dans une ombre.
 * Paramètres :
 *   - c       : la couleur de départ
 *   - facteur : le coefficient multiplicateur (positif)
 * Retour : la couleur modifiée
 */
TCouleur multiplierCouleur(TCouleur c, double facteur);

/**
 * Rôle : additionne deux couleurs, composante par composante.
 * But : ajouter de la lumière (reflet sur la planète, éclat d'une étoile) ou
 *       faire la somme des échantillons d'un pixel avant d'en faire la moyenne.
 * Paramètres :
 *   - c1, c2 : les deux couleurs à additionner
 * Retour : la somme (elle peut dépasser 255, voir composanteVersOctet)
 */
TCouleur ajouterCouleurs(TCouleur c1, TCouleur c2);

/**
 * Rôle : convertit une composante réelle en octet : arrondi à l'entier le plus
 *        proche, puis ramené entre 0 et 255.
 * But : une image BMP stocke chaque composante sur un octet. Une couleur
 *       éclaircie peut dépasser 255 : il faut alors la plafonner (écrêtage).
 * Paramètres :
 *   - composante : la valeur réelle à convertir
 * Retour : l'octet correspondant, entre 0 et 255
 */
unsigned char composanteVersOctet(double composante);

#endif
