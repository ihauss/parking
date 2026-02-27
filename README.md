# Smart Parking

### A constrained computer vision system for parking occupancy estimation

## 1. Problem Statement
Monitoring parking occupancy seems trivial when high-resolution cameras, stable viewpoints and GPU-powered inference are available.

In practice, many real-world deployments rely on low-cost hardware, fixed overhead cameras subject to slight motion, and limited computational resources.

The challenge addressed in this project is to reliably estimate parking occupancy from a semi-stable top-down view, under strict compute constraints (CPU-only, no GPU), while maintaining robustness to illumination changes and minor camera displacement.

## Status

Phase 0 – Consolidation.

Core architecture refactored.
API stabilization in progress.
Docker restructuring ongoing.

## 2. Design Philosophy & Constraints

(fusion de “Scope and Constraints” + vision)

Pourquoi CPU-only

Pourquoi robustesse > précision brute

Pourquoi séparation light / heavy

👉 Très important pour montrer ta maturité d’ingénieur.

## 3. System Overview

Pipeline global (haut niveau)

Vue boîte noire

Pas de classes, pas de fichiers

## 4. High-Level Architecture

Modules principaux

Flux de données

Décisions vs perception

👉 Aucun détail OpenCV / C++ ici

## 5. Demo & Visualization

GIF / vidéo

Screenshot annoté

Résultat final visible

## 6. Performance Summary

FPS cible

CPU utilisé

Résolution

Conditions de test

## 7. Limitations

Hypothèses caméra

Cas non gérés

Trade-offs assumés

## 8. Roadmap / Future Work

Séparation API / embedded

Bindings Python

Calibration auto

Tracking temporel

## 9. Repository Structure
smart_parking/
├── core/
├── embedded/
├── bindings/
├── api/        (future)
└── README.md


👉 Aide énormément à la lecture.

10. Conclusion

Ce que ce projet démontre

Pourquoi il est intéressant techniquement


## 2. Scope and Constraints
## 3. System Overview
## 4. Architecture
## 5. Technical Choices
## 6. Demo and Visualization
## 7. Performance
## 8. Limitations
## 9. Future Work


## 11. Conclusion
