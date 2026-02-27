# Smart Parking

### A constrained computer vision system for parking occupancy estimation

## 1. Problem Statement
Monitoring parking occupancy seems trivial when high-resolution cameras, stable viewpoints and GPU-powered inference are available.

In practice, many real-world deployments rely on low-cost hardware, fixed overhead cameras subject to slight motion, and limited computational resources.

The challenge addressed in this project is to reliably estimate parking occupancy from a semi-stable top-down view, under strict compute constraints (CPU-only, no GPU), while maintaining robustness to illumination changes and minor camera displacement.

## Status

V-1.0.0 complete – Core system stabilized
API production-ready
Deterministic state management
Dockerized deployment validated

## 2. Deployment

### API
docker build -t smart-parking-api --target api .
docker run -p 8000:8000 smart-parking-api

### Embedded
docker build -t smart-parking-embedded --target embedded .
docker run smart-parking-embedded

## 3. Design Philosophy & Constraints

### CPU-Only by Design

The system targets CPU-only execution to ensure low deployment cost, portability, and energy efficiency. 

Given the constrained top-down viewpoint and limited classification space (occupied vs. free), a carefully engineered vision pipeline provides sufficient reliability without GPU acceleration.

While this limits multi-camera scalability on a single device, it enforces computational discipline and edge compatibility from the ground up.

## 4. System Overview

<p align="center">
  <img src="docs/system_overview.png" width="750">
</p>

---

The system operates as a hierarchical evaluation pipeline designed for constrained environments.

An input image first passes through the **Image Processing** stage, where visual features relevant to parking occupancy are extracted. This layer focuses strictly on perception and remains independent from higher-level business logic.

The processed observations are then forwarded to the **Spot-Level Evaluation** layer. At this level, each parking space maintains its own evolving state, enabling temporal consistency and stable occupancy estimation even under illumination changes or minor camera displacement.

The **Parking-Level Aggregation** stage consolidates individual spot states into a coherent global representation of the parking area. This layer acts as the central orchestrator, exposing unified outputs to the external API.

Depending on the use case, the system provides:

- Operational metrics  
- Rendering information for visualization  
- Business-level occupancy data  

This multi-level structure ensures a clear separation between perception, local state evolution, and global aggregation while remaining lightweight and edge-compatible.

## 5. High-Level Architecture

## 6. Demo & Visualization

<p align="center">
  <img src="docs/demo.gif" width="800">
</p>

## 7. Performance Summary

**FPS** : 12 to 14 (Recording make it drop between 9 and 11)

**Résolution** : 1080p

**CPU** :

- Architecture: aarch64
- CPU(s): 8
- type: ARM
- Cortex-A55: 1.23 GHz to 2.31 GHz (x6)
- Cortex-A78: 2.4 GHz to 3.3 GHz (x2)

**inference** latency : (future)

**RAM** : 6GB

## 8. Limitations

The system is designed under specific operational assumptions and constraints.

### Camera Assumptions

- Semi-stable, fixed top-down viewpoint  
- Limited perspective distortion  
- Moderate and gradual illumination changes  
- No extreme weather or heavy occlusion conditions  

Significant camera displacement or drastic viewpoint changes may require recalibration.

---

### Unhandled Scenarios

- Severe occlusions (large vehicles overlapping multiple spots)  
- Rapid lighting transitions (e.g., strong reflections, headlights at night)  
- Highly dynamic environments with frequent camera movement  
- Non-standard parking layouts without predefined spot definitions  

---

### Architectural Trade-offs

- CPU-only execution limits the number of simultaneous camera streams per device  
- Prioritizing robustness over peak frame-level accuracy may introduce small, stable estimation bias  
- Manual configuration is currently required for parking layout definition  

These trade-offs are intentional to preserve computational efficiency, deterministic behavior, and architectural clarity.

## 9. Roadmap / Future Work

### V-1.0.1 – Robustness & Multi-Camera Foundations

- Crash recovery validation and restart testing  
- Strengthened state persistence guarantees  
- Introduction of a `CameraWorker` abstraction  
- Multi-camera orchestration layer  
- Clearer separation between API and embedded runtime  

### Upcoming Extensions

- Automatic camera calibration  
- Improved temporal tracking  
- Extended Python bindings  
- Optional cloud-based monitoring integration  

## 10. Repository Structure

## 11. Conclusion
