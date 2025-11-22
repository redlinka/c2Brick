# img2brick

Programme C permettant de générer un plan de mosaïque LEGO à partir d’un fichier texte contenant les couleurs d’une image.
Chaque pixel de l’image est associé à la brique dont la couleur RGB est la plus proche selon un catalogue défini dans le code (pour l'instant).

## Entrées

1. `image.txt`
   Contient une grille de couleurs hexadécimales (ex. `FFFFFF 000000 FF0000\n FFFFFF 000000 FF0000\n FFFFFF 000000 FF0000\n`).

2. Catalogue de briques
   Défini dans `main()` avec nom, couleur RGB, prix, et quantité disponible. le catalogue sera plus tard implémenté de façon plus concrète dans une BDD au fil de l'avancement du projet.

## Sorties

1. Console
   • Liste des pixels en RGB
   • Qualité de correspondance de la mosaïque (en %)
   • Prix total des briques nécessaires
   • Bilan de stock et briques manquantes

2. `tiled_image.txt`
   Plan LEGO final sous forme de grille avec les noms des briques sélectionnées.


## Structures

*ColorValues*
Représente la couleur d’un pixel ou d’une brique en format RGB.
| Champ | Type  | Description              |
| ----- | ----- | ------------------------ |
| `r`   | `int` | Composante rouge (0–255) |
| `g`   | `int` | Composante verte (0–255) |
| `b`   | `int` | Composante bleue (0–255) |

*Brick*
Représente une brique LEGO dans le catalogue avec ses caractéristiques physiques, sa couleur et son stock.
| Champ    | Type          | Description                          |
| -------- | ------------- | ------------------------------------ |
| `name`   | `char[32]`    | Nom de la brique (ex. "1x1 Rouge")   |
| `width`  | `int`         | Largeur de la brique en unités LEGO  |
| `height` | `int`         | Hauteur de la brique en unités LEGO  |
| `holes`  | `int`         | Nombre de picots/creux sur la brique |
| `color`  | `ColorValues` | Couleur de la brique en RGB          |
| `price`  | `float`       | Prix unitaire de la brique en euros  |
| `number` | `int`         | Quantité disponible en stock         |

*BestMatch*
Représente le résultat de la comparaison d’un pixel avec toutes les briques du catalogue.

| Champ   | Type  | Description                                                                         |
| ------- | ----- | ----------------------------------------------------------------------------------- |
| `index` | `int` | Index de la brique du catalogue la plus proche du pixel                             |
| `diff`  | `int` | Différence de couleur entre le pixel et la brique (somme des carrés des écarts RGB) |

## Fonctionnement

Pour chaque pixel :

* conversion du code hexadécimal en valeurs RGB --> `ColorValues* pixelify(const char* path)`
* calcul d’une différence de couleur --> `int compareColors(ColorValues p1, ColorValues p2)`
* sélection de la brique la plus proche -->`BestMatch findBestBrick(ColorValues pixel, Brick* catalog, int catSize)`
* orchestration et mise à jour du coût et du stock --> `void toBrick_1x1(ColorValues* pixels, Brick* catalog, int catSize)`

La qualité est basée sur la distance (différence) totale entre couleurs d’origine et briques.

## Compilation et exécution (Terminal)

Placer `img2brick.c` et `image.txt` dans le même dossier.
Se rendre dans ce dossier via un terminal :

```sh
cd /path/to/file
```

Compiler avec gcc :

```sh
gcc img2brick.c -o img2brick
```

Exécuter :

```sh
./img2brick
```

Le fichier `tiled_image.txt` sera généré dans le même dossier.

## Limitations

* Seulement des briques 1x1 ddans cette version.
