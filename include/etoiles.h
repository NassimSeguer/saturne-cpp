#ifndef SATURNE_ETOILES_H
#define SATURNE_ETOILES_H

// ============================================================================
//  Module etoiles : le fond de l'image, un ciel noir parsemé d'étoiles.
// ============================================================================

#include "image.h"

#define PIXELS_PAR_ETOILE 900  // une étoile pour 900 pixels en moyenne

/**
 * Rôle : remplit l'image avec un ciel sombre (léger dégradé du haut vers le
 *        bas) puis y dessine des étoiles placées au hasard. La plupart sont
 *        faibles et quelques-unes très brillantes, comme dans un vrai ciel.
 * But : servir de fond à la planète. Le ciel est entièrement calculé : aucune
 *       photo n'est utilisée.
 * Paramètres :
 *   - image  : l'image à remplir
 *   - graine : valeur d'initialisation du hasard (srand). Sur un même système,
 *              une même graine donne toujours le même ciel : l'image est
 *              donc reproductible.
 */
void dessinerFondEtoile(TImage *image, unsigned int graine);

#endif
