// ============================================================================
//  Module scene : géométrie, éclairage et ombres de Saturne.
//  La documentation des fonctions publiques se trouve dans include/scene.h.
// ============================================================================

#include <math.h>
#include "couleur.h"
#include "vecteur.h"
#include "scene.h"

static const double PI = 3.14159265358979323846;

// Position des éléments dans l'image
#define ROTATION_DEGRES       -10.0  // pente des anneaux dans l'image
#define INCLINAISON_DEGRES     25.0  // angle sous lequel on voit le plan des anneaux

// Position du Soleil
#define HAUTEUR_SOLEIL_DEGRES  25.0  // hauteur au-dessus du plan des anneaux
#define AZIMUT_SOLEIL_DEGRES  140.0  // direction dans ce plan : 180 = à gauche, 90 = vers nous

// Éclairage
#define LUMIERE_AMBIANTE    0.04  // lumière qui atteint quand même le côté nuit
#define INTENSITE_REFLET    0.18  // force du reflet du Soleil sur la planète
#define FINESSE_REFLET     20.0   // plus cette valeur est grande, plus le reflet est petit
#define LUMIERE_DANS_OMBRE  0.08  // part de lumière qui reste dans l'ombre de la planète

// Couleurs des bandes nuageuses de la planète
static const TCouleur BANDE_CLAIRE = {250, 226, 170};
static const TCouleur BANDE_FONCEE = {226, 184, 122};

/*
 * Les trois anneaux principaux de Saturne, avec leurs dimensions réelles en
 * rayons de la planète (1 rayon = 60 268 km). Entre l'anneau B et l'anneau A
 * se trouve un espace presque vide : la division de Cassini.
 */
static const TAnneau ANNEAUX_DE_SATURNE[NB_ANNEAUX] = {
    {"C", 1.239, 1.527, {165, 148, 122}, 0.30},  // sombre et peu dense
    {"B", 1.527, 1.951, {238, 218, 175}, 0.92},  // le plus brillant et le plus dense
    {"A", 2.027, 2.269, {212, 192, 152}, 0.75},  // au-delà de la division de Cassini
};

/**
 * Rôle : convertit un angle de degrés en radians.
 * But : les paramètres sont plus parlants en degrés, mais sin et cos
 *       attendent des radians.
 * Paramètres :
 *   - degres : l'angle en degrés
 * Retour : l'angle en radians
 */
static double degresVersRadians(double degres) {
    return degres * PI / 180.0;
}

/**
 * Rôle : calcule la couleur et l'opacité d'un anneau à une distance donnée.
 *        Comme le vrai Saturne, chaque anneau est fait de nombreuses bandes
 *        fines : on fait varier la luminosité avec deux sinusoïdes de
 *        fréquences différentes, et le milieu de l'anneau est un peu plus clair.
 * But : donner du relief aux anneaux au lieu d'une couleur uniforme.
 * Paramètres :
 *   - anneau   : l'anneau concerné
 *   - distance : distance au centre de la planète, en rayons de la planète
 *   - opacite  : reçoit l'opacité de l'anneau à cette distance, entre 0 et 1
 * Retour : la couleur de l'anneau à cette distance
 */
static TCouleur couleurAnneau(const TAnneau *anneau, double distance, double *opacite) {
    // Position dans l'anneau : 0 au bord intérieur, 1 au bord extérieur
    double t = (distance - anneau->rayonInterieur) / (anneau->rayonExterieur - anneau->rayonInterieur);

    // Fines bandes : valeur entre 0 et 1 qui varie rapidement avec la distance
    double stries = 0.5 + 0.5 * sin(distance * 157.0) * sin(distance * 41.0 + 1.0);

    // sin(PI t) vaut 0 aux bords et 1 au milieu : le centre de l'anneau est plus lumineux
    double luminosite = (0.80 + 0.20 * sin(PI * t)) * (0.88 + 0.12 * stries);

    *opacite = anneau->opacite * (0.75 + 0.25 * stries);
    return multiplierCouleur(anneau->couleur, luminosite);
}

/**
 * Rôle : calcule quelle part de la lumière du Soleil est arrêtée par les
 *        anneaux avant d'atteindre un point de la planète. On suit le rayon
 *        lumineux point + t × lumiere jusqu'au plan des anneaux ; s'il le
 *        traverse dans un anneau, ce point est dans l'ombre de cet anneau.
 * But : dessiner l'ombre des anneaux sur la planète.
 * Paramètres :
 *   - scene : la scène
 *   - point : un point de la surface de la planète
 * Retour : l'opacité de l'anneau traversé (0 si aucun anneau n'est traversé)
 */
static double opaciteOmbreDesAnneaux(const TScene *scene, TVecteur point) {
    // Le point du rayon est dans le plan quand normale · (point + t × lumiere) = 0
    double rapprochement = produitScalaire(scene->normaleAnneaux, scene->lumiere);
    if (fabs(rapprochement) < 1e-9) {
        return 0;  // Soleil exactement dans le plan des anneaux : le rayon ne le traverse jamais
    }

    double t = -produitScalaire(scene->normaleAnneaux, point) / rapprochement;
    if (t <= 0) {
        return 0;  // le plan est de l'autre côté : le rayon ne le croise pas en allant vers le Soleil
    }

    TVecteur traversee = additionnerVecteurs(point, multiplierVecteur(scene->lumiere, t));
    double distance = norme(traversee) / scene->rayon;
    int indice = indiceAnneau(scene, distance);
    if (indice < 0) {
        return 0;  // le rayon passe entre les anneaux
    }

    double opacite;
    couleurAnneau(&scene->anneaux[indice], distance, &opacite);
    return opacite;
}

/**
 * Rôle : calcule la couleur de la planète au point (u, v) de son disque, en
 *        la traitant comme une sphère : bandes nuageuses selon la latitude,
 *        éclairage diffus, reflet, ombre des anneaux et bord assombri.
 * But : donner du volume à la planète au lieu d'un disque plat.
 * Paramètres :
 *   - scene : la scène
 *   - u, v  : un point du disque de la planète, dans le repère de la scène
 * Retour : la couleur de la planète en ce point
 */
static TCouleur couleurPlanete(const TScene *scene, double u, double v) {
    double r = scene->rayon;

    // Point de la sphère vu en (u, v) : sur la face visible, u² + v² + z² = r²
    double z = sqrt(fmax(0.0, r * r - u * u - v * v));
    TVecteur point = creerVecteur(u, v, z);

    // Sur une sphère, la normale (direction perpendiculaire à la surface) part du centre
    TVecteur normale = multiplierVecteur(point, 1.0 / r);

    // 1. Bandes nuageuses : elles suivent les latitudes. L'équateur est dans le
    //    plan des anneaux, donc sin(latitude) = normale · normaleAnneaux.
    double sinLatitude = produitScalaire(normale, scene->normaleAnneaux);
    if (sinLatitude > 1) {
        sinLatitude = 1;  // protection contre les erreurs d'arrondi, asin n'accepte que [-1, 1]
    }
    if (sinLatitude < -1) {
        sinLatitude = -1;
    }
    double latitude = asin(sinLatitude);
    double motif = 0.5 + 0.30 * sin(latitude * 18.0) + 0.20 * sin(latitude * 43.0 + 1.3);
    TCouleur couleur = melangerCouleurs(BANDE_CLAIRE, BANDE_FONCEE, motif);

    // 2. Éclairage diffus (loi de Lambert) : une surface reçoit d'autant plus de
    //    lumière qu'elle fait face au Soleil. Négatif = côté nuit, donc 0.
    double diffus = produitScalaire(normale, scene->lumiere);
    if (diffus < 0) {
        diffus = 0;
    }

    // 3. Reflet (modèle de Blinn-Phong) : il est maximal là où la normale pointe
    //    à mi-chemin entre la direction du Soleil et celle de l'observateur.
    TVecteur versObservateur = creerVecteur(0, 0, 1);
    TVecteur miChemin = normaliser(additionnerVecteurs(scene->lumiere, versObservateur));
    double alignement = produitScalaire(normale, miChemin);
    double reflet = 0;
    if (alignement > 0 && diffus > 0) {
        reflet = INTENSITE_REFLET * pow(alignement, FINESSE_REFLET);
    }

    // 4. Ombre des anneaux : la lumière directe est diminuée de l'opacité de
    //    l'anneau qu'elle traverse. La lumière ambiante, elle, n'est pas concernée.
    double transmission = 1.0 - opaciteOmbreDesAnneaux(scene, point);
    diffus *= transmission;
    reflet *= transmission;

    // 5. Assombrissement du bord : vu en biais, le bord de la planète paraît plus sombre
    double bord = 0.65 + 0.35 * normale.z;

    double eclairage = (LUMIERE_AMBIANTE + (1 - LUMIERE_AMBIANTE) * diffus) * bord;
    couleur = multiplierCouleur(couleur, eclairage);
    return ajouterCouleurs(couleur, creerCouleur(255 * reflet, 255 * reflet, 255 * reflet));
}

void initialiserScene(TScene *scene, int largeur, int hauteur) {
    // Planète au centre ; son rayon est choisi pour que les anneaux (2,27 rayons)
    // tiennent en largeur et l'ensemble en hauteur, avec une marge.
    scene->centreX = largeur / 2.0;
    scene->centreY = hauteur / 2.0;
    scene->rayon = fmin(largeur / 5.0, hauteur / 2.6);
    scene->rotation = degresVersRadians(ROTATION_DEGRES);
    scene->inclinaison = degresVersRadians(INCLINAISON_DEGRES);

    // Trois directions perpendiculaires, de longueur 1, liées au plan des anneaux :
    //  - axe1 : le grand axe de l'ellipse que forment les anneaux à l'écran
    //  - axe2 : la direction du plan qui vient vers nous, vue en biais
    //  - la normale, perpendiculaire au plan, tournée du côté de l'observateur
    double i = scene->inclinaison;
    TVecteur axe1 = creerVecteur(1, 0, 0);
    TVecteur axe2 = creerVecteur(0, sin(i), cos(i));
    scene->normaleAnneaux = creerVecteur(0, -cos(i), sin(i));

    // Direction du Soleil : d'abord sa direction dans le plan des anneaux
    // (azimut), puis on la relève vers la normale de la hauteur voulue.
    // Le Soleil éclaire donc la face des anneaux que l'on voit.
    double azimut = degresVersRadians(AZIMUT_SOLEIL_DEGRES);
    double hauteurSoleil = degresVersRadians(HAUTEUR_SOLEIL_DEGRES);
    TVecteur dansLePlan = additionnerVecteurs(multiplierVecteur(axe1, cos(azimut)),
                                              multiplierVecteur(axe2, sin(azimut)));
    scene->lumiere = additionnerVecteurs(multiplierVecteur(dansLePlan, cos(hauteurSoleil)),
                                         multiplierVecteur(scene->normaleAnneaux, sin(hauteurSoleil)));

    for (int k = 0; k < NB_ANNEAUX; k++) {
        scene->anneaux[k] = ANNEAUX_DE_SATURNE[k];
    }
}

void imageVersScene(const TScene *scene, double x, double y, double *u, double *v) {
    // On se place au centre de la planète, puis on tourne de -rotation pour redresser les anneaux
    TVecteur p = tournerAutourDeZ(creerVecteur(x - scene->centreX, y - scene->centreY, 0), -scene->rotation);

    *u = p.x;
    *v = p.y;
}

void sceneVersImage(const TScene *scene, double u, double v, double *x, double *y) {
    TVecteur p = tournerAutourDeZ(creerVecteur(u, v, 0), scene->rotation);

    *x = scene->centreX + p.x;
    *y = scene->centreY + p.y;
}

TVecteur pointSurAnneaux(const TScene *scene, double u, double v) {
    // Le plan passe par le centre de la planète : ses points vérifient
    // normale · (u, v, z) = 0, ce qui donne z. Ici normale.z = sin(inclinaison),
    // non nul tant que les anneaux ne sont pas vus exactement par la tranche.
    TVecteur n = scene->normaleAnneaux;
    double z = -(n.x * u + n.y * v) / n.z;

    return creerVecteur(u, v, z);
}

int indiceAnneau(const TScene *scene, double distance) {
    for (int k = 0; k < NB_ANNEAUX; k++) {
        if (distance >= scene->anneaux[k].rayonInterieur && distance < scene->anneaux[k].rayonExterieur) {
            return k;
        }
    }
    return -1;
}

int estDansOmbreDeLaPlanete(const TScene *scene, TVecteur point) {
    // Le rayon lumineux est point + t × lumiere (t >= 0). Son point le plus
    // proche du centre de la planète est atteint pour t = -(point · lumiere).
    double projection = produitScalaire(point, scene->lumiere);
    if (projection >= 0) {
        return 0;  // en allant vers le Soleil, on s'éloigne de la planète : pas d'ombre
    }

    // Distance au carré entre le centre et le rayon (théorème de Pythagore)
    double distanceCarree = produitScalaire(point, point) - projection * projection;
    return distanceCarree < scene->rayon * scene->rayon;
}

TCouleur couleurPoint(const TScene *scene, double x, double y, TCouleur fond) {
    double u, v;
    imageVersScene(scene, x, y, &u, &v);

    // Raccourci : au-delà du bord extérieur du dernier anneau, il n'y a que le fond
    double limite = scene->anneaux[NB_ANNEAUX - 1].rayonExterieur * scene->rayon;
    if (u * u + v * v > limite * limite) {
        return fond;
    }

    // 1. Le morceau d'anneau éventuellement visible à cet endroit
    TVecteur pointAnneau = pointSurAnneaux(scene, u, v);
    double distance = norme(pointAnneau) / scene->rayon;
    int indice = indiceAnneau(scene, distance);
    TCouleur anneau = fond;
    double opacite = 0;
    if (indice >= 0) {
        anneau = couleurAnneau(&scene->anneaux[indice], distance, &opacite);
        if (estDansOmbreDeLaPlanete(scene, pointAnneau)) {
            anneau = multiplierCouleur(anneau, LUMIERE_DANS_OMBRE);
        }
    }

    // 2. La planète, si le point est dans son disque
    if (u * u + v * v < scene->rayon * scene->rayon) {
        TCouleur planete = couleurPlanete(scene, u, v);
        // Le morceau d'anneau situé devant la planète (z > 0) la recouvre en partie ;
        // celui situé derrière (z < 0) est caché par elle.
        if (indice >= 0 && pointAnneau.z > 0) {
            return melangerCouleurs(planete, anneau, opacite);
        }
        return planete;
    }

    // 3. Hors de la planète : l'anneau éventuel se superpose au ciel (opacite = 0 s'il n'y en a pas)
    return melangerCouleurs(fond, anneau, opacite);
}
