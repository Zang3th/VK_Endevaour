# VK_Endevaour

## ProfilerPanel

#Version
- 0.2.2

#Date
- 22.07.26 - 23.07.26

### Improvements

- [x] ~Szene mit .obj und Debugdreieck bauen~
- [x] ~Konstanten = 1 aus VulkanGlobalUniforms in VulkanGlobal auslagern~
- [x] ~ObjLoader ein RandomizeColor-Flag für die Vertices mitgeben~

### ProfilerPanel

- [x] ~Erste einfache API überlegen~
- [x] ~Implementieren~
- [x] ~DrawStats pflegen~
- [x] ~UI für das Panel finalisieren~

## RenderTargets

#Version
- 0.2.3

#Date
- 21.08.26 - 31.08.26

### Allgemeines

- [x] ~AGENTS.md anpassen~
- [x] ~GLFW auf v3.5.1 upgraden~

### Improvements

- [x] ~Math + alte Tests rauswerfen~
- [x] ~Neue Tests hinzufügen~
- [x] ~Tooling-Verbesserungen~
- [x] ~Includes überarbeiten~
- [x] ~Logging von Allocations nochmal verbessern~
- [x] ~Mehr Forward-Declarations einbauen~

### Bugs

- [x] ~Tests für den ObjLoader fixen~
- [x] ~Profiling für Move/Resize vom Window fixen~

### VulkanRenderTargets

#Info
- Szene in eigene Color- und Depth-Targets rendern.
- Das multisampled ColorTarget wird am Ende des Renderings in das einfach gesampelte Swapchain-Image aufgelöst.
- Das aufgelöste Swapchain-Image wird präsentiert.

- [x] ~Image, ImageView und Allocation~
- [x] ~ColorTarget + DepthTarget pro Swapchain-Image initialisieren~
- [x] ~SampleCount explizit an RenderTargets übergeben~

### MSAA + Depth-Buffering

#### Render-Konfiguration
- [x] ~DepthFormat bestimmen und speichern~
- [x] ~Gemeinsame Color-/Depth-Sample-Counts abfragen~
- [x] ~Vulkan-Depth-Range über perspectiveRH_ZO anpassen~
- [x] ~Pipeline-RasterizationSamples setzen~
- [x] ~Depth-Test und Depth-Write aktivieren~
- [x] ~PipelineRenderingCreateInfo::depthAttachmentFormat setzen~

#### Targets und Transitions
- [x] ~ColorAttachment auf MSAA-ColorTarget umstellen~
- [x] ~Im ColorAttachment alles für `resolve` ergänzen~
- [x] ~TransitionImageLayout um AspectFlags erweitern~
- [x] ~MSAA-ColorTarget nach ColorAttachmentOptimal transitionieren~
- [x] ~DepthAttachment ergänzen~
- [x] ~DepthTarget nach DepthAttachmentOptimal transitionieren~
- [x] ~RenderingInfo um pDepthAttachment ergänzen~

#### ImGui
- [x] ~`PipelineInfoMain.MSAASamples` auf den aktiven Color-/Depth-SampleCount setzen~

#Pin
## Camera

#Version
- 0.2.4

#Date
- TBD

#Info
Die Kamera wird von der Anwendung verwaltet, dort mittels Input aktualisiert und pro Frame an den Renderer übergeben.

Zusätzlich werden die Kameraparameter im ProfilerPanel dargestellt.

Die Projektion soll rechtshändig mit Z-up sein (entspricht Blender):

- +X = rechts
- -Y = vorwärts
- +Z = oben

### Improvements

- [ ] Tests für Core::Utility ergänzen
- [ ] RenderFrame, RenderItem und RenderRequest einführen

### Bugs

- [ ] Lag-Spikes inspizieren

### Camera

- [ ] Minimale API überlegen
- [ ] Implementieren

### CameraController

- [ ] Minimale API überlegen
- [ ] Implementieren

### RenderControls

#Info
Optionen via UI steuern:

- [ ] Polygon Mode
- [ ] Cull Mode

#### ToDo
- [ ] Interaktion / Schnittstelle mit dem Renderer überlegen
- [ ] UI implementieren

### Kofi Game Engine Series

- [ ] Video 31

## Model

#Version
- 0.2.5

#Date
- TBD

### VulkanMesh

- [ ] VulkanModel in VulkanMesh umbenennen
- [ ] VulkanContext muss da raus
- [ ] VulkanContext::CopyBuffer muss auch woanders hin

### Model

#Info
Besteht aus einem Mesh-Handle (u32) und einem Transform-Struct

#### ToDo
- [ ] Implementieren

### VulkanRenderer

#Info
- Bekommt Models übergeben und pflegt damit eine RenderQueue, bestehend aus RenderCommands
- Erzeugt und speichert VulkanMeshes, aus denen mittels "CPU-Meshes" Non-owning Handles zurückgegeben werden

#### ToDo
- [ ] Implementieren

## WorldGrid

#Version
- 0.2.6

#Date
- TBD

### WorldGrid

### WorldAxes

#Backlog
## Backlog

### StorageBuffer

### StorageImage

### SampledImage

### TileManager

### TileGenerator

### TileSelector

### PixelRenderer

### Ray-Tracing Projekt

#Info
Raytracer mit Vulkan

#### Vorgehensweise
- [ ] Raytracing-Buch 1 durcharbeiten
- [ ] Raytracing-Buch 2 durcharbeiten
- [ ] Raytracing-Buch 3 durcharbeiten
- [ ] Vulkan-Basisapplikation erstellen
- [ ] Schnittstellen zwischen Raytracing und Vulkan herausarbeiten und Projektplan erstellen
- [ ] Build und Static Analysis via GitHub Actions aktivieren
- [ ] Code-Badges hinzufügen (neue Farben)
- [ ] Projektstruktur innerhalb der README ergänzen

#### Book 1: Ray Tracing in One Weekend
- [ ] 1. Overview
- [ ] 2. Output an Image
- [ ] 3. The vec3 Class
- [ ] 4. Rays, a simple Camera, and Background
- [ ] 5. Adding a sphere
- [ ] 6. Surface Normals and Multiple Objects
- [ ] 7. Moving Camera Code Into Its Own Class
- [ ] 8. Antialiasing
- [ ] 9. Diffuse Materials
- [ ] 10. Metal
- [ ] 11. Dielectrics
- [ ] 12. Positionable Camera
- [ ] 13. Defocus Blur
- [ ] 14. Where Next

#Archive
# Archive

## Learning der Basics

#Version
- 0.0.0

#Date
- 23.08.24 - 29.08.24
- 25.09.24 - 15.11.24

#Info
- Learning-Resources durcharbeiten
  - Begleitendes Git-Repo
  - Nebenbei Dokument mit Learnings pflegen
- Library mit Code aufbauen
  - Arbeite mit Asserts
- Arbeite am Anfang nur mit den allernotwendigsten Libraries
- Schaue auch kontinuierlich bei Hazel und EDBR rein, um dir Orientierung zu verschaffen
- Ergänze dann Libraries nach und nach
- Projekt 1: HelloCube (drehendes 3D-Objekt mit ImGui-Anzeige)

### Allgemeines

- [x] ~SalinityGL README anpassen: Projekt wird erstmal nicht weiterentwickelt~
- [x] ~Neues Git-Repo "VK Endevaour" aufmachen (public)~
- [x] ~Passe die README an Linux allgemein an~

### Blogpost: Elias Daler

- [x] ~Durcharbeiten (100%)~
- [x] ~Dokument pflegen, in dem alle Erkenntnisse und offenen Fragen hinterlegt werden~

### Tutorial: Vulkan-Tutorial.com

#Warn
- Wayland: Window wird erst nach Commit eines Buffers sichtbar

#### ToDo
- [x] ~0. Introduction~
- [x] ~1. Overview~
- [x] ~2. Development environment~
- [x] ~3.1.0 Drawing a Triangle / Setup / Base code~
- [x] ~3.1.1 Drawing a Triangle / Setup / Instance~
- [x] ~3.1.2 Drawing a Triangle / Setup / Validation layers~
- [x] ~Problem: Validation Layer: loader_add_layer_properties: 'layers' tag not supported until file version 1.0.1 => .json manuell editiert~
- [x] ~Für die restlichen Probleme habe ich keinen Fix ... Ich habe alles probiert: GLFW 3.4, Vulkan SDK 1.3.290, SDL2, Nvidia-Treiber neu installiert, VkBootstrap – hat alles nichts gebracht ...~
- [x] ~Hole dir Access zu Hazel~
- [x] ~Füge SEVERITY_INFO_BIT_EXT hinzu~
- [x] ~Passe Logging entsprechend des Severity-Levels an~
- [x] ~Schaue dir Asserts an (Best Practices bspw. in Hazel), ergänze Assert.hpp und passe die Applikation entsprechend an~
- [x] ~3.1.3 Drawing a Triangle / Setup / Physical devices and queue families~
- [x] ~Lookup-Code aus dem Header raushauen~
- [x] ~3.1.4 Drawing a Triangle / Setup / Logical device and queues~
- [x] ~3.2.0 Drawing a Triangle / Presentation / Window surface~
- [x] ~3.2.1 Drawing a Triangle / Presentation / Swap chain~
- [x] ~3.2.2 Drawing a Triangle / Presentation / Image views~
- [x] ~3.3.0 Drawing a Triangle / Graphics Pipeline Basics / Introduction~
- [x] ~3.3.1 Drawing a Triangle / Graphics Pipeline Basics / Shader modules~
- [x] ~3.3.2 Drawing a Triangle / Graphics Pipeline Basics / Fixed functions~
- [x] ~3.3.3 Drawing a Triangle / Graphics Pipeline Basics / Render passes~
- [x] ~3.3.4 Drawing a Triangle / Graphics Pipeline Basics / Conclusion~
- [x] ~3.4.0 Drawing a Triangle / Drawing / Framebuffers~
- [x] ~3.4.1 Drawing a Triangle / Drawing / Command buffers~
- [x] ~3.4.2 Drawing a Triangle / Drawing / Rendering and presentation~
- [x] ~3.4.3 Drawing a Triangle / Drawing / Frames in flight~
- [x] ~3.5 Drawing a Triangle / Swap chain recreation~
- [x] ~4.1 Vertex buffers / Vertex input description~
- [x] ~4.2 Vertex buffers / Vertex buffer creation~
- [x] ~4.3. Vertex buffers / Staging buffer~
- [x] ~4.4 Vertex buffers / Index buffer~
- [x] ~5.1 Uniform buffers / Descriptor layout and buffer~
- [x] ~5.2 Uniform buffers / Descriptor pool and sets~
- [x] ~Diesen seltsamen Swapchain-Recreation-Bug fixen => Lag daran, dass ich nicht immer die neuesten Swapchain-Properties abgerufen habe~
- [x] ~6.1 Texture mapping / Images~
- [x] ~6.2 Texture mapping / Image view and sampler~
- [x] ~6.3 Texture mapping / Combined image sampler~
- [x] ~7. Depth buffering~
- [x] ~8. Loading models~
- [x] ~9. Generating Mipmaps~
- [x] ~10. Multisampling~
- [x] ~ImGui integrieren (das ist die Baseline für das Projekt)~
- [x] ~Static Analysis über das Projekt laufen lassen~
- [x] ~Ausdruck in den Asserts in Klammern setzen (Auswertungsreihenfolge!)~

### Tutorial: TU Wien

- [x] ~0. First steps~
- [x] ~1. Swap Chain~
- [x] ~2. Resources and Descriptors~
- [x] ~3. Commands and Command Buffers~
- [x] ~4. Pipelines and Stages~

## Refactoring der Basics

#Version
- 0.0.1

#Date
- 23.04.25 - 19.05.25

### Allgemeines

- [x] ~Projekt erstellen (initiales Git-Repo)~
- [x] ~README.md schreiben~
- [x] ~CMake-Konfiguration aufsetzen (3 separate Files, Engine als Static Library etc.)~
- [x] ~fmt inkludieren (als CMake-Subdirectory)~
- [x] ~Dünnen Logging-Wrapper mittels Defines bauen (Ergänzung von Levels via Farben und Uhrzeit)~
- [x] ~LSP funktioniert (CMake_EXPORT_COMPILE_COMMANDS)~
- [x] ~Vulkan SDK zum Laufen bringen~
- [x] ~GLFW zum Laufen bringen~
- [x] ~ChatGPT-Projekt aufsetzen~
- [x] ~Altes Projekt zum Laufen bringen~
- [x] ~Projekt aufräumen~
- [x] ~Vulkan-Basisapplikation erstellen (ordentlich, modular, verständlich) => API: https://x.com/SebAaltonen/status/1848311998376738892~
- [x] ~Designated struct initializers + structured bindings verwenden~
- [x] ~ImGui updaten~
- [x] ~VulkanMemoryAllocator integrieren~
- [x] ~VulkanSamples klonen~

### Improvements

- [x] ~Richtige Types in Utility.cpp verwenden~
- [x] ~Das Device an den Allocator durchreichen~
- [x] ~Surface per const& an die Swapchain übergeben~
- [x] ~Grep nach uint32_t, int32_t, float und double~

### Core

- [x] ~Log implementieren~
- [x] ~Types implementieren~
- [x] ~Memory implementieren~
- [x] ~Window implementieren~

### Vulkan

- [x] ~VulkanAssert~
- [x] ~VulkanContext~
- [x] ~VulkanDebug + DebugMessenger (=> VulkanContext)~
- [x] ~vk::Instance (=> VulkanContext)~
- [x] ~vk::SurfaceKHR (=> VulkanContext)~
- [x] ~VulkanPhysicalDevice implementieren~
- [x] ~VulkanDevice implementieren~
- [x] ~VulkanSwapchain~
- [x] ~vk::Image + vk::ImageView (=> VulkanSwapchain)~
- [x] ~VulkanShader~
- [x] ~VulkanModel~
- [x] ~vk::Buffer und VmaAllocation (VulkanModel)~
- [x] ~vk::CommandPool~
- [x] ~vk::CommandBuffer~
- [x] ~VulkanPipeline~
- [x] ~DrawFrame()~
- [x] ~Shutdown-Prozess ordentlich tracken/loggen + fixen~
- [x] ~Swapchain refactoren~
- [x] ~Resize fixen~
- [x] ~Engine crasht, wenn FRAMES_IN_FLIGHT != 3~
- [x] ~Engine allokiert mit jedem Frame mehr Speicher => Fixen (Passiert bei mir bei allen Vulkan-Programmen. Scheint am GLFW-Wayland-Layer zu liegen)~

### Graphics

- [x] ~Applikationsinterface definieren und erste API überlegen~
- [x] ~Ersten Rendererentwurf implementieren~

### Kofi Game Engine Series

- [x] ~Video 0~
- [x] ~Video 2~
- [x] ~Video 3~
- [x] ~Video 5~
- [x] ~Video 6~
- [x] ~Video 7~
- [x] ~Video 8~
- [x] ~Video 9~
- [x] ~Video 11~
- [x] ~Video 12~
- [x] ~Video 13~
- [x] ~Video 14~
- [x] ~Video 15~
- [x] ~Video 16~
- [x] ~Video 17~

## Windows-Port

#Version
- 0.1.0

#Date
- 09.12.25 - 06.01.26
- 21.05.26 - 05.06.26

### Allgemeines

#### Software & Tooling
- [x] ~RenderDoc~
- [x] ~Blender~
- [x] ~F3D~

#### Project
- [x] ~Vulkan SDK installieren (Winget)~
- [x] ~CheckDependencies.py schreiben (Vulkan SDK, GLSLC, CMake, Clang++/Linker, Ninja) + alle Tools, die andere Skripte verwenden~
- [x] ~Einmal pro Woche das Projekt auf dem Laptop bauen~
- [x] ~LogStats.py schreiben~
- [x] ~BuildEngine.py schreiben (--debug, --release, --all, --clean)~
- [x] ~AnalyzeSources.py schreiben (--tidy, --verify)~
- [x] ~FormatFiles.py schreiben~

#### GitHub
- [x] ~README überarbeiten (u. a. mit Projektstruktur). Orientierung an SalinityGL~
- [x] ~Minimale Build Instructions für Windows und Linux hinzufügen (Voraussetzungen erläutern)~
- [x] ~Tags zum Repo hinzufügen~

### Improvements

#Info
- Kann ich das Logging verbessern oder muss alles zwangsläufig durch stderr laufen? => Läuft eh unter Windows nicht
- Wie spiele ich clangd-Vorschläge ("fixes available") in Neovim ein? => code_action (ga)

- [x] ~Typo im Objloader.cpp~
- [x] ~Typo in README (Punkt fehlt bei vorletzter Zeile)~
- [x] ~std::source_location im Logging ergänzen~
- [x] ~Aufruf der Skripte ohne Parameter fixen~
- [x] ~Richtige Namespaces wie Engine::Core, Engine::Graphics etc. überall ergänzen~
- [x] ~Skript-Output besser formatieren~
- [x] ~RAD_Debugger Skript bauen~

### Bugs

- [x] ~Validation Layer laufen auf 1.4.328, während das SDK schon auf 1.4.350 läuft => Alte SDKs müssen deinstalliert und immer der neueste Vulkan Configurator gestartet werden~
- [x] ~Minimieren reparieren~
- [x] ~vkQueueSubmit(): Swapchain image 0 was presented but was not re-acquired => Unterschiedliche Plattformen erstellen unterschiedliche Mengen an Bildern => WSI-Image-Acquisition von VulkanFrame::Sync trennen~

## ImGui-Integration

#Version
- 0.2.0

#Date
- 08.06.26 - 13.06.26

### Allgemeines

#### Software & Tooling
- [x] ~Codex installieren und AGENTS.md erstellen~

#### Project
- [x] ~ImGui auf v1.92.8 upgraden => README.md anpassen~
- [x] ~VMA auf v3.4.0 upgraden => README.md anpassen~
- [x] ~Nächste App auf prozedurale Terrain-Generierung auf der GPU ändern~

### Improvements

#Info
- [x] Wie kann das LSP alles in Vendor/ und in externem Vulkan-/STL-Code ignorieren? => .clangd anpassen ... Funktioniert mehr schlecht als recht
- [x] Wie kann ich dieses lästige Einsetzen von Funktionskörpern optimieren? Vim-Makro oder ein neues LSP-Binding? => Neues Keybinding auf <Tab> gelegt

- [x] ~Logging: Shader-Creation verbessern (Typ von Shader)~
- [x] ~Logging: Shader-Destruction verbessern (Typ von Shader)~
- [x] ~.clang-format noch stärker an die Firma anpassen~
- [x] ~Codex: VulkanRenderer::DrawFrame runterstampfen => Was kann man noch besser abstrahieren oder zusammenfassen?~
- [x] ~Codex: VulkanFrame anschauen => Ist das wirklich optimal so?~
- [x] ~Codex: Abschließende Code-Review~

### Bugs

- [x] ~Codex-Code-Review: In allen Klassen, die interne Vulkan-Handles (owning) halten, den Copy-Konstruktor und Assignment-Operator löschen, da ansonsten Double-Destroys passieren können~
- [x] ~Codex-Code-Review: Uneinheitliche Codeabschnitte (die stilistisch oder architektonisch nicht zum Rest passen)~
- [x] ~Codex-Code-Review: Riskante, unsichere Codeabschnitte und fehlendes const~
- [x] ~Fast-Compile-Option einfügen (ohne sauberen Full Rebuild)~
- [x] ~GLFW-Error-Callback anlegen~
- [x] ~ImGui-Error-Callback anlegen~
- [x] ~API_VERSION überall korrekt ersetzen~
- [x] ~GLFW nur noch in Window.cpp inkludieren (denk an das Define) und Window zur einzigen Schnittstelle umbauen (danach auch unter Linux testen) + Swapchain fragt Resize-/Minimize-State beim Window ab (Callback sollte auch dort liegen)~

### VulkanDescriptorPool

- [x] ~Minimale API überlegen~
- [x] ~Implementieren~

### ImGuiLayer

- [x] ~Grundlegende API und ImGui-Integration überlegen~
- [x] ~ImGui Backend integrieren~

## DescriptorSets

#Version
- 0.2.1

#Date
- 15.06.26 - 25.06.26

### Improvements

- [x] ~Logging: .obj-Loading verbessern~
- [x] ~Logging: Swapchain-Creation verbessern~
- [x] ~Logging: Memory-Allocations verbessern~
- [x] ~Alle Header-Includes von vulkan.hpp prüfen~
- [x] ~Neue .obj-Dateien besorgen~

### Bugs

- [x] ~Tinyrunner-Warnings fixen~
- [x] ~CullMode / Clockwise Vertex Drawing fixen + vereinheitlichen~

### VulkanDescriptorSetLayout

- [x] ~Minimale API überlegen~
- [x] ~Implementieren~

### VulkanGlobalUniforms

- [x] ~Minimale API überlegen~
- [x] ~Implementieren~

### Timer

- [x] ~Minimale API entwerfen~
- [x] ~Implementieren~

### Kofi Game Engine Series

- [x] ~Video 29~
- [x] ~Video 30~
