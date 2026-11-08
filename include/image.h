#ifndef SATURNE_IMAGE_H
#define SATURNE_IMAGE_H

// ============================================================================
//  Module image : un tableau de pixels en mémoire, et son enregistrement au
//  format BMP, écrit octet par octet sans aucune bibliothèque.
// ============================================================================

#include "couleur.h"

/**
 * Un pixel tel qu'il est stocké : une composante par octet (0 à 255).
 */
typedef struct {
    unsigned char rouge;
    unsigned char vert;
    unsigned char bleu;
} TPixel;

/**
 * Une image de largeur × hauteur pixels. Les pixels sont rangés ligne par
 * ligne, en partant du haut : le pixel (ligne, colonne) se trouve à la case
 * ligne × largeur + colonne du tableau pixels.
 */
typedef struct {
    int largeur;
    int hauteur;
    TPixel *pixels;
} TImage;

/**
 * Rôle : réserve en mémoire une image dont tous les pixels sont noirs.
 * But : préparer la "toile" sur laquelle on dessine le ciel puis Saturne.
 * Paramètres :
 *   - largeur : nombre de colonnes (au moins 1)
 *   - hauteur : nombre de lignes (au moins 1)
 * Retour : l'image créée, ou NULL si les dimensions sont invalides ou si la
 *          mémoire manque. Elle doit être libérée avec detruireImage.
 */
TImage *creerImage(int largeur, int hauteur);

/**
 * Rôle : libère la mémoire occupée par une image.
 * But : rendre au système la mémoire réservée par creerImage.
 * Paramètres :
 *   - image : l'image à libérer (rien ne se passe si elle vaut NULL)
 */
void detruireImage(TImage *image);

/**
 * Rôle : lit la couleur d'un pixel.
 * But : récupérer le fond (ciel étoilé) avant de dessiner la planète par-dessus.
 * Paramètres :
 *   - image   : l'image
 *   - ligne   : numéro de ligne, 0 en haut
 *   - colonne : numéro de colonne, 0 à gauche
 * Retour : la couleur du pixel, ou du noir si le pixel est hors de l'image
 */
TCouleur lirePixel(const TImage *image, int ligne, int colonne);

/**
 * Rôle : donne une couleur à un pixel (composantes arrondies et ramenées
 *        entre 0 et 255).
 * But : dessiner dans l'image. Un pixel hors de l'image est ignoré, ce qui
 *       permet de dessiner une étoile au bord sans précaution particulière.
 * Paramètres :
 *   - image   : l'image à modifier
 *   - ligne   : numéro de ligne, 0 en haut
 *   - colonne : numéro de colonne, 0 à gauche
 *   - couleur : la nouvelle couleur du pixel
 */
void ecrirePixel(TImage *image, int ligne, int colonne, TCouleur couleur);

/**
 * Rôle : enregistre l'image dans un fichier BMP 24 bits non compressé.
 * But : produire un fichier que n'importe quelle visionneuse d'images sait
 *       ouvrir. Le format est écrit octet par octet : deux en-têtes (54 octets
 *       au total) décrivent l'image, puis viennent les pixels.
 * Paramètres :
 *   - image      : l'image à enregistrer
 *   - nomFichier : le chemin du fichier à créer (il est remplacé s'il existe)
 * Retour : 1 si le fichier a bien été écrit, 0 en cas d'erreur
 */
int enregistrerBmp(const TImage *image, const char *nomFichier);

#endif
