#ifndef SATURNE_RENDU_H
#define SATURNE_RENDU_H

// ============================================================================
//  Module rendu : calcul de l'image complète, pixel par pixel.
// ============================================================================

#include "image.h"
#include "scene.h"

// Chaque pixel est découpé en SUR_ECHANTILLONNAGE x SUR_ECHANTILLONNAGE sous-pixels
#define SUR_ECHANTILLONNAGE 3

/**
 * Rôle : calcule la couleur de chaque pixel de l'image. Pour éviter les bords
 *        en escalier (crénelage), chaque pixel est découpé en 3 x 3 sous-pixels :
 *        on calcule la couleur au centre de chacun, puis on fait la moyenne.
 *        C'est le sur-échantillonnage (anticrénelage).
 * But : produire l'image finale. Le fond déjà présent dans l'image (le ciel)
 *       reste visible partout où il n'y a ni planète ni anneau.
 * Paramètres :
 *   - image               : l'image, qui contient déjà le fond
 *   - scene               : la scène à dessiner
 *   - afficherProgression : 1 pour afficher l'avancement (10 %, 20 %...), 0 sinon
 */
void rendreScene(TImage *image, const TScene *scene, int afficherProgression);

#endif
