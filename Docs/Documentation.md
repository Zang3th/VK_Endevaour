# Dokumentation

## Ressourcen

- **Blogpost von Elias Dalter** (Writing an Engine with Vulkan): [https://edw.is/learning-vulkan/]
- **Vulkan Tutorials der TU Wien** (Deckt die Grundlagen ab): [https://www.youtube.com/playlist?list=PLmIqTlJ6KsE1Jx5HV4sd2jOe3V1KMHHgn]
- **VKGuide** (Beste Lernressource): [https://vkguide.dev/]
- **Tutorials von Thorsten Thormälen** (Lektionen zu Shadern): [https://www.youtube.com/playlist?list=PL8vNj3osX2PzZ-cNSqhA8G6C1-Li5-Ck8]
- **Mastering Graphics Programming with Vulkan by Marco Castorina and Gabriel Sassone** (Fortgeschrittene Graphikprogrammierungs Konzepte): Buch

## Fragen

## Einleitung

### Tips für Hobbyisten

- Vermeide Overengineering, übermäßige Generalisierung und baue alle Systeme nur so komplex wie sie auch sein müssen
- Vermeide das frühzeitige Schreiben von großen Wrappern rund um Vulkanfunktionalitäten, besonders während des Lernprozesses

### Bibliotheken

**vk-bootstrap:**

- Vereinfacht eine Menge von Vulkan-Boilerplatecode
- Betrifft hauptsächlich den Initialisierungsprozess

**Vulkan Memory Allocator:**

- Vereinfacht die Arbeit mit Vulkans Memory Allocators

**volk:**

- Vereinfacht das Laden von Funktionserweiterungen

### Shader

- In Vulkan können alle Shadersprachen verwendet werden welche sich zu *SPIR-V* kompilieren lassen
- Shader können beim Building vorkompiliert werden
  - GLSLC-Compiler ist in der Vulkan SDK bereits integriert
  - Durch die Definition einer *DEPFILE* ist es möglich, nur geänderte Dateien neu kompilieren zu müssen
  - Diese Schritte können einfach in der CMakeLists.txt ergänzt werden

### Vorteile von Vulkan

- Keine globalen States mehr
  - OpenGL ist riesiger Zustandsautomat
- Moderne API
  - Bessere Validierungsfehler
  - Besseres Debugging + die Möglichkeit von Shaderdebugging
- Nativer Multithreadingsupport
  - Keine Hacks mehr nötig

## Vulkan

### Erste Schritte
