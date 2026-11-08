// ============================================================================
//  Tests automatiques de Saturne.
//
//  Chaque fonction "tester..." vérifie un module. Le programme affiche [OK] ou
//  [ECHEC] pour chaque vérification, puis renvoie 0 si tout est correct et 1
//  sinon (c'est ce code de retour que lit la commande ctest).
// ============================================================================

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "couleur.h"
#include "vecteur.h"
#include "image.h"
#include "etoiles.h"
#include "scene.h"
#include "rendu.h"

#define FICHIER_TEST "test_saturne.bmp"

static const double PI = 3.14159265358979323846;

static int nbTests = 0;
static int nbEchecs = 0;

/**
 * Rôle : enregistre le résultat d'une vérification et l'affiche.
 * But : écrire chaque test en une ligne lisible.
 * Paramètres :
 *   - condition   : vraie (non nulle) si le comportement attendu est observé
 *   - description : ce qui est vérifié, en français
 */
static void verifier(int condition, const char *description) {
    nbTests++;
    if (condition) {
        printf("[OK]     %s\n", description);
    } else {
        nbEchecs++;
        printf("[ECHEC]  %s\n", description);
    }
}

/**
 * Rôle : compare deux réels en tolérant une minuscule différence.
 * But : les calculs sur les réels comportent des erreurs d'arrondi
 *       (cos(PI / 2) ne vaut pas exactement 0), on ne peut donc pas utiliser ==.
 * Paramètres :
 *   - a, b : les deux réels
 * Retour : 1 s'ils diffèrent de moins d'un millionième, 0 sinon
 */
static int presqueEgal(double a, double b) {
    return fabs(a - b) < 1e-6;
}

/**
 * Rôle : indique si deux couleurs sont identiques.
 * But : comparer le résultat d'un calcul de couleur à la valeur attendue.
 * Paramètres :
 *   - a, b : les deux couleurs
 * Retour : 1 si leurs trois composantes sont égales (à un millionième près), 0 sinon
 */
static int memesCouleurs(TCouleur a, TCouleur b) {
    return presqueEgal(a.rouge, b.rouge) && presqueEgal(a.vert, b.vert) && presqueEgal(a.bleu, b.bleu);
}

/**
 * Rôle : lit un entier de 4 octets rangé en "little endian" dans un tableau.
 * But : relire les champs de l'en-tête d'un fichier BMP.
 * Paramètres :
 *   - octets   : le contenu du fichier
 *   - position : la position du premier octet de l'entier
 * Retour : l'entier lu
 */
static unsigned long lireEntier(const unsigned char octets[], int position) {
    return octets[position]
           | (unsigned long) octets[position + 1] << 8
           | (unsigned long) octets[position + 2] << 16
           | (unsigned long) octets[position + 3] << 24;
}

/**
 * Rôle : vérifie le mélange, l'éclaircissement et la conversion des couleurs.
 * But : ces opérations servent à chaque pixel de l'image.
 */
static void testerCouleurs() {
    TCouleur noir = creerCouleur(0, 0, 0);
    TCouleur blanc = creerCouleur(255, 255, 255);

    printf("\n-- couleur --\n");
    verifier(memesCouleurs(melangerCouleurs(noir, blanc, 0), noir), "un mélange avec t = 0 donne la première couleur");
    verifier(memesCouleurs(melangerCouleurs(noir, blanc, 1), blanc), "un mélange avec t = 1 donne la seconde couleur");
    verifier(memesCouleurs(melangerCouleurs(noir, blanc, 0.5), creerCouleur(127.5, 127.5, 127.5)),
             "un mélange avec t = 0,5 donne la couleur du milieu");
    verifier(memesCouleurs(multiplierCouleur(creerCouleur(100, 50, 20), 0.5), creerCouleur(50, 25, 10)),
             "multiplier par 0,5 divise chaque composante par deux");
    verifier(composanteVersOctet(-12) == 0 && composanteVersOctet(300) == 255,
             "une composante hors de [0, 255] est ramenée à 0 ou 255");
    verifier(composanteVersOctet(127.4) == 127 && composanteVersOctet(127.6) == 128,
             "une composante est arrondie à l'entier le plus proche");
}

/**
 * Rôle : vérifie les opérations sur les vecteurs avec des cas faciles à calculer de tête.
 * But : l'éclairage et les ombres reposent entièrement sur ces opérations.
 */
static void testerVecteurs() {
    TVecteur a = creerVecteur(3, 4, 0);
    TVecteur tourne = tournerAutourDeZ(creerVecteur(1, 0, 0), PI / 2);

    printf("\n-- vecteur --\n");
    verifier(presqueEgal(norme(a), 5), "la norme de (3, 4, 0) vaut 5");
    verifier(presqueEgal(norme(normaliser(a)), 1), "un vecteur normalisé a une longueur de 1");
    verifier(presqueEgal(produitScalaire(creerVecteur(1, 0, 0), creerVecteur(0, 1, 0)), 0),
             "deux directions perpendiculaires ont un produit scalaire nul");
    verifier(presqueEgal(produitScalaire(creerVecteur(1, 2, 3), creerVecteur(4, 5, 6)), 32),
             "(1, 2, 3) · (4, 5, 6) = 4 + 10 + 18 = 32");
    verifier(presqueEgal(tourne.x, 0) && presqueEgal(tourne.y, 1) && presqueEgal(tourne.z, 0),
             "un quart de tour autour de z transforme (1, 0, 0) en (0, 1, 0)");
}

/**
 * Rôle : enregistre une petite image de 3 x 2 pixels puis relit le fichier
 *        octet par octet pour contrôler l'en-tête, l'ordre des pixels et le bourrage.
 * But : garantir que le fichier respecte le format BMP.
 */
static void testerImageBmp() {
    unsigned char octets[100];
    size_t taille = 0;

    printf("\n-- image et format BMP --\n");
    TImage *image = creerImage(3, 2);
    verifier(image != NULL, "création d'une image de 3 x 2 pixels");
    if (image == NULL) {
        return;
    }

    verifier(memesCouleurs(lirePixel(image, 0, 0), creerCouleur(0, 0, 0)), "une nouvelle image est noire");
    ecrirePixel(image, 1, 0, creerCouleur(10, 20, 30));    // en bas à gauche
    ecrirePixel(image, 0, 2, creerCouleur(200, 100, 50));  // en haut à droite
    ecrirePixel(image, 5, 5, creerCouleur(255, 255, 255)); // hors de l'image : ignoré
    verifier(memesCouleurs(lirePixel(image, 1, 0), creerCouleur(10, 20, 30)), "un pixel écrit puis relu garde sa couleur");
    verifier(enregistrerBmp(image, FICHIER_TEST), "le fichier BMP est enregistré");
    detruireImage(image);

    FILE *fichier = fopen(FICHIER_TEST, "rb");
    if (fichier != NULL) {
        taille = fread(octets, 1, sizeof(octets), fichier);
        fclose(fichier);
    }
    remove(FICHIER_TEST);

    // Une ligne fait 3 pixels x 3 octets = 9 octets, complétés à 12 (multiple de 4)
    verifier(taille == 78, "taille du fichier : 54 octets d'en-tête + 2 lignes de 12 octets = 78");
    if (taille != 78) {
        return;
    }
    verifier(octets[0] == 'B' && octets[1] == 'M', "le fichier commence par la signature BM");
    verifier(lireEntier(octets, 2) == 78 && lireEntier(octets, 10) == 54,
             "l'en-tête indique la taille du fichier et la position des pixels");
    verifier(lireEntier(octets, 18) == 3 && lireEntier(octets, 22) == 2 && octets[28] == 24,
             "l'en-tête indique une largeur de 3, une hauteur de 2 et 24 bits par pixel");
    verifier(octets[54] == 30 && octets[55] == 20 && octets[56] == 10,
             "la ligne du bas est écrite en premier, chaque pixel en bleu, vert, rouge");
    verifier(octets[63] == 0 && octets[64] == 0 && octets[65] == 0, "chaque ligne se termine par 3 octets de bourrage");
    verifier(octets[72] == 50 && octets[73] == 100 && octets[74] == 200, "le pixel en haut à droite est écrit en dernier");
}

/**
 * Rôle : vérifie le ciel étoilé : il est reproductible et contient des étoiles.
 * But : l'image doit être identique à chaque exécution du programme.
 */
static void testerEtoiles() {
    TImage *ciel1 = creerImage(200, 100);
    TImage *ciel2 = creerImage(200, 100);
    int identiques = 1;
    int pixelsLumineux = 0;

    printf("\n-- etoiles --\n");
    dessinerFondEtoile(ciel1, 42);
    dessinerFondEtoile(ciel2, 42);
    for (int ligne = 0; ligne < 100; ligne++) {
        for (int colonne = 0; colonne < 200; colonne++) {
            TCouleur c1 = lirePixel(ciel1, ligne, colonne);
            if (!memesCouleurs(c1, lirePixel(ciel2, ligne, colonne))) {
                identiques = 0;
            }
            if (c1.rouge > 60) {
                pixelsLumineux++;
            }
        }
    }
    detruireImage(ciel1);
    detruireImage(ciel2);

    verifier(identiques, "la même graine donne exactement le même ciel");
    verifier(pixelsLumineux > 0, "le ciel contient des étoiles");
}

/**
 * Rôle : vérifie la géométrie de la scène : anneaux, plan des anneaux et
 *        changement de repère.
 * But : s'assurer que les dimensions réelles des anneaux sont respectées.
 */
static void testerGeometrie() {
    TScene scene;
    double u, v, x, y;

    printf("\n-- scene : geometrie --\n");
    initialiserScene(&scene, 1280, 720);
    verifier(indiceAnneau(&scene, 1.0) == -1, "à 1 rayon du centre : la surface de la planète, pas d'anneau");
    verifier(indiceAnneau(&scene, 1.4) == 0, "à 1,4 rayon : anneau C");
    verifier(indiceAnneau(&scene, 1.8) == 1, "à 1,8 rayon : anneau B");
    verifier(indiceAnneau(&scene, 1.99) == -1, "à 1,99 rayon : division de Cassini, vide");
    verifier(indiceAnneau(&scene, 2.1) == 2, "à 2,1 rayons : anneau A");
    verifier(indiceAnneau(&scene, 2.5) == -1, "au-delà de 2,27 rayons : plus d'anneau");

    TVecteur point = pointSurAnneaux(&scene, 123.0, -45.0);
    verifier(presqueEgal(produitScalaire(point, scene.normaleAnneaux), 0), "le point calculé appartient au plan des anneaux");
    verifier(presqueEgal(norme(scene.lumiere), 1) && presqueEgal(norme(scene.normaleAnneaux), 1),
             "la direction du Soleil et la normale aux anneaux sont de longueur 1");

    imageVersScene(&scene, 700, 300, &u, &v);
    sceneVersImage(&scene, u, v, &x, &y);
    verifier(presqueEgal(x, 700) && presqueEgal(y, 300), "passer de l'image à la scène puis revenir redonne le même point");
}

/**
 * Rôle : vérifie l'ombre de la planète sur les anneaux, avec deux points du
 *        plan des anneaux : l'un du côté du Soleil, l'autre juste derrière la
 *        planète par rapport au Soleil.
 * But : contrôler le calcul de l'ombre par lancer de rayon.
 */
static void testerOmbre() {
    TScene scene;

    printf("\n-- scene : ombre --\n");
    initialiserScene(&scene, 1280, 720);

    // Direction du Soleil dans le plan des anneaux : on retire de la direction
    // de la lumière sa composante perpendiculaire au plan
    TVecteur n = scene.normaleAnneaux;
    TVecteur versSoleil = normaliser(additionnerVecteurs(scene.lumiere,
                                     multiplierVecteur(n, -produitScalaire(scene.lumiere, n))));
    TVecteur coteSoleil = multiplierVecteur(versSoleil, 1.8 * scene.rayon);
    TVecteur derrierePlanete = multiplierVecteur(versSoleil, -1.8 * scene.rayon);

    verifier(!estDansOmbreDeLaPlanete(&scene, coteSoleil), "un morceau d'anneau du côté du Soleil est éclairé");
    verifier(estDansOmbreDeLaPlanete(&scene, derrierePlanete), "un morceau d'anneau juste derrière la planète est dans son ombre");
}

/**
 * Rôle : vérifie ce que l'on voit à différents endroits de l'image : le fond,
 *        la planète, et un anneau devant ou derrière la planète.
 * But : contrôler l'ordre d'affichage (ce qui passe devant quoi).
 */
static void testerCouleurPoint() {
    TScene scene;
    TScene sansAnneaux;
    TCouleur fond = creerCouleur(1, 2, 3);
    double x, y;

    printf("\n-- scene : couleur des points --\n");
    initialiserScene(&scene, 1280, 720);
    verifier(memesCouleurs(couleurPoint(&scene, 5, 5, fond), fond), "dans un coin de l'image, on voit le ciel");
    verifier(!memesCouleurs(couleurPoint(&scene, scene.centreX, scene.centreY, fond), fond),
             "au centre de l'image, on voit la planète");

    // Même scène avec des anneaux invisibles, pour comparer
    sansAnneaux = scene;
    for (int k = 0; k < NB_ANNEAUX; k++) {
        sansAnneaux.anneaux[k].opacite = 0;
    }

    // Point de l'anneau B à 1,8 rayon sur le petit axe de l'ellipse : il se
    // projette dans le disque de la planète, au-dessus (v < 0) ou en dessous (v > 0) du centre
    double ecart = 1.8 * scene.rayon * sin(scene.inclinaison);
    sceneVersImage(&scene, 0, -ecart, &x, &y);
    verifier(memesCouleurs(couleurPoint(&scene, x, y, fond), couleurPoint(&sansAnneaux, x, y, fond)),
             "la partie arrière des anneaux est cachée par la planète");
    sceneVersImage(&scene, 0, ecart, &x, &y);
    verifier(!memesCouleurs(couleurPoint(&scene, x, y, fond), couleurPoint(&sansAnneaux, x, y, fond)),
             "la partie avant des anneaux passe devant la planète");
}

/**
 * Rôle : calcule une petite image complète et contrôle quelques pixels.
 * But : vérifier que toutes les étapes du rendu s'enchaînent correctement.
 */
static void testerRendu() {
    TScene scene;
    TImage *image = creerImage(160, 90);

    printf("\n-- rendu --\n");
    initialiserScene(&scene, 160, 90);
    rendreScene(image, &scene, 0);

    TCouleur coin = lirePixel(image, 0, 0);
    TCouleur centre = lirePixel(image, 45, 80);
    detruireImage(image);

    verifier(memesCouleurs(coin, creerCouleur(0, 0, 0)), "le coin de l'image garde le fond (ici noir)");
    verifier(centre.rouge > 50 && centre.vert > 30, "le centre de l'image montre la planète éclairée");
}

/**
 * Rôle : lance tous les tests et affiche le bilan.
 * Retour : 0 si tous les tests réussissent, 1 sinon
 */
int main() {
#ifdef _WIN32
    system("chcp 65001 > nul");  // accents corrects dans la console Windows
#endif

    testerCouleurs();
    testerVecteurs();
    testerImageBmp();
    testerEtoiles();
    testerGeometrie();
    testerOmbre();
    testerCouleurPoint();
    testerRendu();

    printf("\n%d vérifications, %d échec(s)\n", nbTests, nbEchecs);
    if (nbEchecs > 0) {
        return 1;
    }
    return 0;
}
