# STATE.md — Project Memory

> **Last Updated**: 2026-03-03
> **Current Phase**: Not started
> **Session**: 1

## Current Position
- **Milestone**: v1.0 — Autonomous Explorer
- **Phase**: Not started (Phase 1 next)
- **Task**: N/A

## Context
- Project initialized with /new-project
- SPEC.md finalized
- Hardware research completed
- Roadmap created with 6 phases
- Waiting for /plan 1 to begin Phase 1

## Key Decisions
1. **ESP-NOW** for AtomS3R-CAM ↔ M5StickC Plus 2 communication (not UART/I2C)
2. **Arduino IDE (C++)** as primary development environment
3. **TFLite Micro** for on-device AI on AtomS3R-CAM
4. **Two firmware projects**: main (M5StickC Plus 2) + camera (AtomS3R-CAM)
5. **FreeRTOS** tasks for parallel sensor reading, motor control, communication

## Blockers
- None

## Session Log
- 2026-03-03: Project initialized, SPEC finalized, ROADMAP created
