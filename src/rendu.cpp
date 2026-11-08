// ============================================================================
//  Module rendu : parcours de l'image et sur-échantillonnage.
//  La documentation de la fonction publique se trouve dans include/rendu.h.
// ============================================================================

#include <stdio.h>
#include "couleur.h"
#include "image.h"
#include "scene.h"
#include "rendu.h"

void rendreScene(TImage *image, const TScene *scene, int afficherProgression) {
    const int nbEchantillons = SUR_ECHANTILLONNAGE * SUR_ECHANTILLONNAGE;
    int derniereDizaine = 0;

    for (int ligne = 0; ligne < image->hauteur; ligne++) {
        for (int colonne = 0; colonne < image->largeur; colonne++) {
            TCouleur fond = lirePixel(image, ligne, colonne);
            TCouleur somme = creerCouleur(0, 0, 0);

            // Centre de chaque sous-pixel : par exemple, pour 3 x 3 sous-pixels,
            // aux positions 1/6, 3/6 et 5/6 du pixel, dans les deux directions
            for (int i = 0; i < SUR_ECHANTILLONNAGE; i++) {
                for (int j = 0; j < SUR_ECHANTILLONNAGE; j++) {
                    double x = colonne + (j + 0.5) / SUR_ECHANTILLONNAGE;
                    double y = ligne + (i + 0.5) / SUR_ECHANTILLONNAGE;
                    somme = ajouterCouleurs(somme, couleurPoint(scene, x, y, fond));
                }
            }

            ecrirePixel(image, ligne, colonne, multiplierCouleur(somme, 1.0 / nbEchantillons));
        }

        // Une ligne de plus est terminée : on affiche l'avancement par tranches de 10 %
        if (afficherProgression) {
            int dizaine = 10 * (ligne + 1) / image->hauteur;
            if (dizaine > derniereDizaine) {
                printf("  %d %%\n", 10 * dizaine);
                derniereDizaine = dizaine;
            }
        }
    }
}
