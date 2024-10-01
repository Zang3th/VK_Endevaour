# Intro-Dokumentation

## Ressourcen

### Tutorials

- **Vulkan-Tutorial** (Sehr gute Grundlage): [https://vulkan-tutorial.com/Introduction]
- **VKGuide** (Gute Lernressource, abstrahiert aber im Vorhinein): [https://vkguide.dev/]
- **Vulkan Tutorials der TU Wien** (Decken die Grundlagen ab): [https://www.youtube.com/playlist?list=PLmIqTlJ6KsE1Jx5HV4sd2jOe3V1KMHHgn]
- **Mastering Graphics Programming with Vulkan by Marco Castorina and Gabriel Sassone** (Fortgeschrittene Graphikprogrammierungs Konzepte): Buch

### Code

- **Vulkan-Guide GitHub Repository**: [https://github.com/KhronosGroup/Vulkan-Guide]
- **Vulkan-Samples GitHub Repository**: [https://github.com/khronosGroup/Vulkan-samples]

### Anderes

- **Blogpost von Elias Dalter** (Writing an Engine with Vulkan): [https://edw.is/learning-vulkan/]
- **Tutorials von Thorsten Thormälen** (Lektionen zu Shadern): [https://www.youtube.com/playlist?list=PL8vNj3osX2PzZ-cNSqhA8G6C1-Li5-Ck8]
- **Vulkan Specification**: [https://registry.khronos.org/vulkan/specs/1.2-extensions/pdf/vkspec.pdf]

## Einleitung

### Allgemeines zu Vulkan

- Graphics and Compute API
- High efficiency by being low-level
- Very explicit/verbose
- Cross-platform

### Vorteile von Vulkan gegenüber OpenGL

- Vulkan ist eine allgemeine GPU API
  - Compute ist automatisch und von vornherein integriert
- Keine globalen States mehr
  - OpenGL ist riesiger Zustandsautomat
- Moderne API
  - Bessere Validierungsfehler
  - Besseres Debugging + die Möglichkeit von Shaderdebugging
- Nativer Multithreadingsupport
  - Keine Hacks mehr nötig

### Shader

- In Vulkan können alle Shadersprachen verwendet werden welche sich zu *SPIR-V* kompilieren lassen
- Shader können beim Building vorkompiliert werden
  - GLSLC-Compiler ist in der Vulkan SDK bereits integriert
  - Durch die Definition einer *DEPFILE* ist es möglich, nur geänderte Dateien neu kompilieren zu müssen
  - Diese Schritte können einfach in der CMakeLists.txt ergänzt werden

### Bibliotheken

**vk-bootstrap:**

- Vereinfacht eine Menge von Vulkan-Boilerplatecode
- Betrifft hauptsächlich den Initialisierungsprozess

**Vulkan Memory Allocator:**

- Vereinfacht die Arbeit mit Vulkans Memory Allocators

**volk:**

- Vereinfacht das Laden von Funktionserweiterungen

### Tips für Hobbyisten

- Vermeide Overengineering, übermäßige Generalisierung und baue alle Systeme nur so komplex wie sie auch sein müssen
- Vermeide das frühzeitige Schreiben von großen Wrappern rund um Vulkanfunktionalitäten, besonders während des Lernprozesses
- Es existieren graphische Tools zur Konfiguration von Vulkan oder zum Einsehen von Hardwarekapazitäten
  - Alles natürlich auch aus Code heraus machbar oder im Web einsehbar: [https://vulkan.gpuinfo.org]

## Schritte zum ersten Triangle

- Create a VkInstance
- Select a supported graphics card (VkPhysicalDevice)
- Create a VkDevice and VkQueue for drawing and presentation
- Create a window, window surface and swap chain
- Wrap the swap chain images into VkImageView
- Create a render pass that specifies the render targets and usage
- Create framebuffers for the render pass
- Set up the graphics pipeline
- Allocate and record a command buffer with the draw commands for every possible swap chain image
- Draw frames by acquiring images, submitting the right draw command buffer and returning the images back to the swap chain
