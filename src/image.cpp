// ============================================================================
//  Module image : gestion des pixels et écriture du format BMP.
//  La documentation des fonctions publiques se trouve dans include/image.h.
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include "couleur.h"
#include "image.h"

#define TAILLE_ENTETE_FICHIER 14    // en-tête "BITMAPFILEHEADER"
#define TAILLE_ENTETE_DIB     40    // en-tête "BITMAPINFOHEADER", le plus répandu
#define BITS_PAR_PIXEL        24    // 3 octets par pixel : bleu, vert, rouge
#define PIXELS_PAR_METRE      2835  // 72 pixels par pouce, la résolution habituelle d'un écran

/**
 * Rôle : écrit un entier dans un fichier sur nbOctets octets, en commençant
 *        par l'octet de poids faible (ordre "little endian").
 * But : le format BMP impose cet ordre. On écrit les octets un par un, ce qui
 *       donne le même fichier quel que soit l'ordinateur utilisé.
 * Paramètres :
 *   - fichier  : le fichier, ouvert en écriture binaire
 *   - valeur   : l'entier à écrire (positif)
 *   - nbOctets : le nombre d'octets à écrire (2 ou 4)
 */
static void ecrireLittleEndian(FILE *fichier, unsigned long valeur, int nbOctets) {
    for (int i = 0; i < nbOctets; i++) {
        fputc((int) (valeur & 0xFF), fichier);  // & 0xFF garde l'octet de poids faible
        valeur >>= 8;                           // on décale pour passer à l'octet suivant
    }
}

TImage *creerImage(int largeur, int hauteur) {
    if (largeur <= 0 || hauteur <= 0) {
        return NULL;
    }

    TImage *image = (TImage *) malloc(sizeof(TImage));
    if (image == NULL) {
        return NULL;
    }

    // calloc met tous les octets à 0 : l'image commence entièrement noire
    image->pixels = (TPixel *) calloc((size_t) largeur * hauteur, sizeof(TPixel));
    if (image->pixels == NULL) {
        free(image);
        return NULL;
    }

    image->largeur = largeur;
    image->hauteur = hauteur;
    return image;
}

void detruireImage(TImage *image) {
    if (image != NULL) {
        free(image->pixels);
        free(image);
    }
}

TCouleur lirePixel(const TImage *image, int ligne, int colonne) {
    if (ligne < 0 || ligne >= image->hauteur || colonne < 0 || colonne >= image->largeur) {
        return creerCouleur(0, 0, 0);
    }

    const TPixel *pixel = &image->pixels[ligne * image->largeur + colonne];
    return creerCouleur(pixel->rouge, pixel->vert, pixel->bleu);
}

void ecrirePixel(TImage *image, int ligne, int colonne, TCouleur couleur) {
    if (ligne < 0 || ligne >= image->hauteur || colonne < 0 || colonne >= image->largeur) {
        return;
    }

    TPixel *pixel = &image->pixels[ligne * image->largeur + colonne];
    pixel->rouge = composanteVersOctet(couleur.rouge);
    pixel->vert = composanteVersOctet(couleur.vert);
    pixel->bleu = composanteVersOctet(couleur.bleu);
}

int enregistrerBmp(const TImage *image, const char *nomFichier) {
    FILE *fichier = fopen(nomFichier, "wb");  // "b" : mode binaire, les octets sont écrits tels quels
    if (fichier == NULL) {
        return 0;
    }

    // Chaque ligne de pixels doit occuper un multiple de 4 octets : on la
    // complète si besoin avec des octets à 0 (le "bourrage").
    int octetsParLigne = (3 * image->largeur + 3) / 4 * 4;
    int octetsDeBourrage = octetsParLigne - 3 * image->largeur;
    unsigned long tailleTableau = (unsigned long) octetsParLigne * image->hauteur;
    unsigned long debutPixels = TAILLE_ENTETE_FICHIER + TAILLE_ENTETE_DIB;

    // 1. En-tête de fichier (14 octets)
    fputc('B', fichier);                                      // signature "BM" des fichiers BMP
    fputc('M', fichier);
    ecrireLittleEndian(fichier, debutPixels + tailleTableau, 4);  // taille totale du fichier
    ecrireLittleEndian(fichier, 0, 4);                        // deux champs réservés, toujours à 0
    ecrireLittleEndian(fichier, debutPixels, 4);              // position du premier pixel

    // 2. En-tête d'image (40 octets)
    ecrireLittleEndian(fichier, TAILLE_ENTETE_DIB, 4);
    ecrireLittleEndian(fichier, image->largeur, 4);
    ecrireLittleEndian(fichier, image->hauteur, 4);          // positif : lignes rangées de bas en haut
    ecrireLittleEndian(fichier, 1, 2);                        // nombre de plans, toujours 1
    ecrireLittleEndian(fichier, BITS_PAR_PIXEL, 2);
    ecrireLittleEndian(fichier, 0, 4);                        // 0 = pas de compression
    ecrireLittleEndian(fichier, tailleTableau, 4);
    ecrireLittleEndian(fichier, PIXELS_PAR_METRE, 4);         // résolution horizontale
    ecrireLittleEndian(fichier, PIXELS_PAR_METRE, 4);         // résolution verticale
    ecrireLittleEndian(fichier, 0, 4);                        // pas de palette de couleurs
    ecrireLittleEndian(fichier, 0, 4);                        // toutes les couleurs sont importantes

    // 3. Les pixels : la ligne du bas en premier, et chaque pixel dans l'ordre bleu, vert, rouge
    for (int ligne = image->hauteur - 1; ligne >= 0; ligne--) {
        for (int colonne = 0; colonne < image->largeur; colonne++) {
            const TPixel *pixel = &image->pixels[ligne * image->largeur + colonne];
            fputc(pixel->bleu, fichier);
            fputc(pixel->vert, fichier);
            fputc(pixel->rouge, fichier);
        }
        for (int i = 0; i < octetsDeBourrage; i++) {
            fputc(0, fichier);
        }
    }

    // ferror signale une erreur d'écriture (disque plein...) ; fclose peut aussi échouer
    int erreur = ferror(fichier);
    if (fclose(fichier) != 0) {
        erreur = 1;
    }
    return !erreur;
}
