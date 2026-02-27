## 1. Core Overview

Rôle du core

Frontière claire avec :

embedded runtime

API future

bindings

## 2. Core Pipeline

Pipeline décisionnel frame par frame :

Acquisition

Light vision (motion)

State machine

Heavy estimator (si nécessaire)

Rendering

## 3. Parking Place Model

Représentation géométrique

États (PlaceState)

Transitions autorisées

Hypothèses temporelles

👉 Ici tu relies directement à ton enum + logique.

## 4. Light Vision Module

Objectif

Hypothèses

Algorithme (KNN, ROI, masque)

Signal produit

## 5. Heavy Estimation Module

Quand il est déclenché

Pourquoi il est coûteux

Garanties attendues

## 6. Temporal Logic & State Machine

Pourquoi pas du “frame-wise only”

Gestion du bruit

Anti-flapping

## 7. Rendering & Debugging

Rôle du renderer

États → couleurs

FPS & overlay

## 8. Embedded Runtime

(c’est ici que va ta partie Docker actuelle)

Pourquoi Docker

Contraintes embarquées

Mode headless

Enregistrement

👉 Tu peux garder 90% de ton texte actuel ici

## 9. Build & Execution (Embedded)

Docker build

Docker run

CLI flags

## 10. Testing

Tests unitaires

Tests d’intégration

Ce qui est couvert / non couvert

## 11. Known Limitations (Core-level)

Sensibilité caméra

Illumination extrême

Hypothèses de stabilité