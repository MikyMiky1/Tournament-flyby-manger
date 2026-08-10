# Flapper Tournament: Ultimate Edition

**A fully browser-based tournament management platform driven by a real-time, head-to-head Flappy Bird-style arcade game.**  
Built entirely with vanilla HTML, CSS, and JavaScript – no frameworks, no libraries – this project demonstrates deep proficiency in complex state management, custom game engine development, and advanced UI/UX design.

[![Vanilla JS](https://img.shields.io/badge/vanilla-JS-yellow.svg)]() [![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)]()

---

## Table of Contents
- [Overview](#overview)
- [Live Demo](#live-demo)
- [Features](#features)
- [Screenshots](#screenshots)
- [Architecture & Key Decisions](#architecture--key-decisions)
- [Project Journey](#project-journey)
  - [Managing Complex State Without a Framework](#managing-complex-state-without-a-framework)
  - [Crafting a Smooth Game Engine & Physics](#crafting-a-smooth-game-engine--physics)
  - [Unifying Input Across Devices](#unifying-input-across-devices)
  - [Immersive UI/UX with CSS Mastery](#immersive-uiux-with-css-mastery)
- [Tech Stack](#tech-stack)
- [Getting Started](#getting-started)
- [Future Enhancements](#future-enhancements)
- [Acknowledgments](#acknowledgments)

---

## Overview
Flapper Tournament: Ultimate Edition is a dual-layered web application.  
**Layer one** is an immersive, arcade-style “Credit Screen” where players choose from six uniquely designed fighters – each with custom stats and special moves. Once the roster is locked in, the experience transitions to **layer two**: a full-featured Tournament Manager. Here, you create teams, manage players, and run single-elimination brackets. Every match is decided in real time by a two-player Flappy Bird mini-game, making the outcome entirely skill-based.

The entire app runs as a single-page application (SPA) with multiple modes, all data persisted client-side. It was conceived and built as a flagship portfolio piece to showcase full-stack front-end engineering, game development, and design.

---

## Live Demo
*https://mikymiky1.github.io/Tournament-flyby-manger/*

---

## Features
- **Dynamic Tournament Engine**  
  Manages single-elimination brackets, automatically handles byes, advances rounds, declares champions, and records full match history.

- **Arcade Fighter Selection Screen**  
  A stylized credit-screen with six original fighters, animated avatars, and editable names/descriptions.

- **Real-Time Flappy Bird Mini-Game**  
  Two players compete head-to-head. The game is the sole method of resolving matches, adding real skill to the tournament.

- **Custom Game Engine**  
  Object-oriented design with separate classes for `Bird`, `FlapperGame`, and `Item` (power-ups). Physics, collision detection, and 60fps rendering via `requestAnimationFrame`.

- **Four Dynamic Themes**  
  Classic, Night City, Lava Cavern, and Ice World – each changes backgrounds, pipes, and ground colors instantly.

- **Power-Up System**  
  Three procedurally spawned items – Shield, Magnet, Star – introduce strategic depth and replayability.

- **AI Opponent**  
  A predictive AI that analyses gap positions to time its flaps, offering a challenging 1v1 alternative to human play.

- **Match Replay**  
  Every frame is recorded; after a match, players can watch a full replay of the action.

- **Comprehensive Archive & History**  
  Completed tournaments are archived forever. Match logs display winners, scores, and timestamps.

- **Fully Customizable Settings**  
  Remap flap keys (W, ArrowUp, etc.), toggle tilt controls for mobile, and edit fighter details on the credit screen.

- **Cross-Platform Responsive Design**  
  Desktop, tablet, and mobile layouts with specific optimizations for the tournament bracket, game canvas, and credit screen. “Face-to-Face” mode rotates the second player’s canvas 180° for opposite-seated mobile play.

- **Persistent Local Storage**  
  All teams, players, brackets, and settings survive page refreshes and browser restarts.

---

## Screenshots
*[Screenshots showcasing the credit screen, tournament bracket, in-game action, and replay panel will be inserted here.]*

---

## Architecture & Key Decisions
The application is structured as a **modular single-page app**. The entire source avoids third-party frameworks; every line is hand-written vanilla JavaScript. This choice was deliberate – it forced a deep understanding of the platform and resulted in a lightweight, dependency-free codebase.

- **Global State Management**  
  A central data store holds all application state. A single `updateUI()` function acts as the render pipeline, re-painting only the parts of the DOM that changed. This unidirectional data flow keeps the UI predictable and performant.

- **Game Engine**  
  Built entirely on the HTML5 Canvas API using object-oriented patterns. The engine maintains its own game loop, handles sprite rendering, physics (gravity, velocity, jump impulse), and per-pixel collision detection. It is completely decoupled from the tournament logic, communicating only through a defined event interface.

- **Input Abstraction Layer**  
  A unified handler listens for `keydown`, `touchstart`, and `deviceorientation` events. It maps them to player actions based on the current settings and active game mode, seamlessly supporting keyboards, touchscreens, and device tilt.

- **UI/UX System**  
  The design follows a dark cyberpunk aesthetic with CSS custom properties for theming, `backdrop-filter` glassmorphism, CSS Grid/Flexbox for layout, and keyframe animations for elements like floating avatars and scanlines. The mini-game modal itself conditionally renders different sub-panels (difficulty selector, theme picker, game canvas, reward summary) while preserving the user’s flow state.

---

## Project Journey
This section tells the story of how the most challenging parts of the application were solved.

### Managing Complex State Without a Framework
The app must seamlessly switch between the arcade selection screen, the team/tournament management dashboard, and the live mini-game – all without a page reload.  
**My solution**: I implemented a reactive, centralized state object and a dedicated rendering pipeline. Whenever state changes (e.g., a team is added, a match is won, a bracket advances), the `updateUI()` function efficiently patches only the relevant DOM sections. State persistence is handled via `localStorage`, serializing and deserializing the entire data model so that the user’s progress is never lost. This architecture mimics the unidirectional data flow of modern frameworks, but stays completely framework-free.

### Crafting a Smooth Game Engine & Physics
A Flappy Bird clone that runs at a silky 60fps on both desktops and mobile phones, with accurate collision detection and support for themes and power-ups, demanded a proper engine.  
**My solution**: I designed an object-oriented game loop using `requestAnimationFrame`. The `FlapperGame` class orchestrates spawning, updates, and rendering. `Bird` objects independently handle their own physics (gravity accumulation, jump velocity) and visual states. `Item` power-ups are spawned at intervals, each with its own collision box and effect. Pixel-perfect bounding-box collision detection is used between birds, pipes, and items. The loop calculates delta time to ensure consistent speed regardless of frame rate. This architecture made it simple to add features like the replay system (recording frame states) and multiple themes (changing color parameters).

### Unifying Input Across Devices
Supporting keyboards, touchscreens, and device tilt – plus allowing users to remap keys – required a highly flexible input system.  
**My solution**: A single input module listens to all relevant browser events and normalizes them into simple “flap” and “menu” actions. A settings panel lets users bind any key to “flap”. On mobile, the screen is split into two halves for each player’s touch zones. For the “Face-to-Face” mode, the second player’s canvas is rotated 180° using a CSS transform, and the input handler correctly maps their touches regardless of orientation. Tilt-to-flap is implemented by monitoring `deviceorientation` and triggering a flap when a threshold is exceeded.

### Immersive UI/UX with CSS Mastery
Creating a visual language that feels like a real arcade cabinet – with glowing neon, animated character cards, and a 3D-style marquee – while staying fully responsive, was a significant challenge.  
**My solution**: I leaned heavily on modern CSS. Custom properties define a cohesive dark palette with accent colors that can be tweaked globally. `@keyframes` drive the continuous floating and scanning effects. `backdrop-filter: blur()` and semi-transparent backgrounds create the glass panels. Flexbox and Grid handle complex responsive layouts (the tournament bracket collapses elegantly on small screens). The credit screen’s fighter selection uses a custom canvas-rendered avatar animation for each character, tied to hover and selection states. The result is an interface that feels at home in an arcade, yet is completely functional and accessible.

---

## Tech Stack
- **HTML5** – Semantic structure, Canvas element for game rendering
- **CSS3** – Custom properties, Grid, Flexbox, keyframe animations, backdrop-filter, responsive design
- **JavaScript (ES6+)** – OOP, modules, game engine, state management, event handling
- **Web APIs** – Canvas 2D Context, Web Audio API (sound effects), LocalStorage API, DeviceOrientation API
- **No frameworks, no libraries** – Every line is original code

---

## Getting Started
To run the project locally:

1. **Clone the repository**
   ```bash
   git clone https://github.com/your-username/flapper-tournament-ultimate.git
   ```
2. **Open `index.html`** in any modern browser.
   - No build tools or server setup required. The app runs directly from the file system.
   - For the best experience, use Chrome, Firefox, or Edge.

*If using ES modules, serve via a local server (e.g., `npx serve .`)*

---

## Future Enhancements
- Online multiplayer via WebRTC for remote head-to-head matches
- Additional game modes (time attack, endless)
- More fighter selection animations and unlockable characters
- Sound effect & music toggles with more immersive audio
- Export/import tournament data to share brackets

---

## Acknowledgments
This project is the culmination of countless hours of love for both classic arcade games and clean code. It was designed and developed entirely as a way to learn and as a portfolio piece to demonstrate mastery of vanilla web technologies.  
If you have feedback or ideas, feel free to open an issue or reach out!

---
