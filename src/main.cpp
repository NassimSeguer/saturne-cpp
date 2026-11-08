// ============================================================================
//  Saturne : une image de la planète et de ses anneaux, calculée pixel par
//  pixel, sans bibliothèque graphique.
//
//  Utilisation : saturne [fichier.bmp] [largeur hauteur]
//  Par défaut, l'image fait 1280 x 720 pixels et s'appelle saturne.bmp.
//
//  Organisation du code :
//    couleur : représentation et mélange des couleurs
//    vecteur : calculs sur les vecteurs de l'espace
//    image   : tableau de pixels et écriture du format BMP
//    etoiles : fond de ciel étoilé
//    scene   : géométrie, éclairage et ombres de Saturne
//    rendu   : calcul de l'image complète, avec anticrénelage
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "etoiles.h"
#include "scene.h"
#include "rendu.h"

#define FICHIER_PAR_DEFAUT "saturne.bmp"
#define LARGEUR_PAR_DEFAUT 1280
#define HAUTEUR_PAR_DEFAUT 720
#define TAILLE_MIN         100   // en dessous, la planète serait trop petite pour être lisible
#define TAILLE_MAX         8000  // au-delà, l'image occuperait trop de mémoire
#define GRAINE_ETOILES     2026  // changer cette valeur donne un autre ciel étoilé

/**
 * Rôle : passe la console Windows en UTF-8 (page de code 65001).
 * But : afficher correctement les accents des messages. Sous Linux et macOS,
 *       la console est déjà en UTF-8 : la fonction ne fait rien.
 */
static void activerUtf8Console() {
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif
}

/**
 * Rôle : convertit un argument de la ligne de commande en dimension d'image.
 * But : vérifier la largeur et la hauteur demandées avant de créer l'image.
 * Paramètres :
 *   - texte : l'argument, par exemple "1920"
 * Retour : la valeur si c'est un entier entre TAILLE_MIN et TAILLE_MAX, -1 sinon
 */
static int lireDimension(const char *texte) {
    char *fin;
    long valeur = strtol(texte, &fin, 10);

    // strtol s'arrête au premier caractère qui n'est pas un chiffre : si ce n'est
    // pas la fin du texte, l'argument n'était pas un nombre entier ("12a", "abc")
    if (fin == texte || *fin != '\0' || valeur < TAILLE_MIN || valeur > TAILLE_MAX) {
        return -1;
    }
    return (int) valeur;
}

/**
 * Rôle : point d'entrée du programme. Lit les arguments, dessine le ciel, calcule
 *        l'image de Saturne puis l'enregistre au format BMP.
 * But : relier les modules entre eux ; main ne contient aucun calcul d'image.
 * Paramètres :
 *   - argc : nombre d'arguments (le nom du programme compris)
 *   - argv : les arguments : [fichier.bmp] [largeur hauteur]
 * Retour : 0 si l'image a été créée, 1 en cas d'erreur
 */
int main(int argc, char *argv[]) {
    const char *nomFichier = FICHIER_PAR_DEFAUT;
    int largeur = LARGEUR_PAR_DEFAUT;
    int hauteur = HAUTEUR_PAR_DEFAUT;

    activerUtf8Console();

    if (argc >= 2) {
        nomFichier = argv[1];
    }
    if (argc == 4) {
        largeur = lireDimension(argv[2]);
        hauteur = lireDimension(argv[3]);
    }
    if (argc == 3 || argc > 4 || largeur < 0 || hauteur < 0) {
        printf("Utilisation : saturne [fichier.bmp] [largeur hauteur]\n");
        printf("  largeur et hauteur : entre %d et %d pixels (par défaut %d x %d)\n",
               TAILLE_MIN, TAILLE_MAX, LARGEUR_PAR_DEFAUT, HAUTEUR_PAR_DEFAUT);
        return 1;
    }

    TImage *image = creerImage(largeur, hauteur);
    if (image == NULL) {
        fprintf(stderr, "Erreur : mémoire insuffisante pour une image de %d x %d pixels.\n", largeur, hauteur);
        return 1;
    }

    TScene scene;
    initialiserScene(&scene, largeur, hauteur);

    printf("Ciel étoilé...\n");
    dessinerFondEtoile(image, GRAINE_ETOILES);

    printf("Rendu de Saturne : %d x %d pixels, %d échantillons par pixel\n",
           largeur, hauteur, SUR_ECHANTILLONNAGE * SUR_ECHANTILLONNAGE);
    rendreScene(image, &scene, 1);

    int reussite = enregistrerBmp(image, nomFichier);
    detruireImage(image);

    if (!reussite) {
        fprintf(stderr, "Erreur : impossible d'écrire le fichier %s\n", nomFichier);
        return 1;
    }
    printf("Image enregistrée : %s\n", nomFichier);
    return 0;
}
