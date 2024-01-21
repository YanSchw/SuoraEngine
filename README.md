
<p align="center">
  <a href="https://github.com/YanSchw/SuoraEngine">
    <img src="Content/EngineContent/SuoraLogo.png" width="400" alt="Suora Engine logo">
  </a>
</p>

SuoraEngine is an open-source C++ Engine for building 3D real-time rendering and interactive Applications or Content, especially Video Games.

![Screenshot of the Suora Engine editor](Docs/Images/SuoraInEditorScreenshot.png)

## Current Features
In its current state, Suora's most remarkable features are:

#### Gameplay Programming
![Gameplay Programming](Docs/Images/SuoraCPP_Programming.png)
Gameplay Programming using C++ and a custom C++ Reflection System using SuoraHeaderTool and SuoraBuildTool. Everything is devided into C++ Modules, which can be compiled incrementally.

#### Visual Scripting (Experimental)
![Visual Scripting](Docs/Images/VisualScripting.png)
Create custom Gameplay Logic using Visual Scripting. Easy interoperability with C++ using SuoraHeaderTool.

**Other Features Suora offers:**

| Feature                               | Description                                                                                                          |
| -------                               | -----------                                                                                                          |
| **Custom SuoraBuildTool**             | To extend the Engine using Modules as Plugins and to allow Reflection and Introspection                              |
| **Custom Visual Scripting Tools**     | Node-based Visual Scripting to enable easy implementation of custom logic                                            |
| **Physics**                           | 3D Physics using JoltPhysics                                                                                         |
| **GameFramework**                     | Flexible Node-based GameFramework                                                                                    |
| **Custom EditorUI**                   | using the RenderingAPI as a Backend (**Immediate**)                                                                  |
| **InGameUI**                          | using the GameFramework and UINodes (**Retained**)                                                                   |
| **Full Visual Editor**                | Feature complete Level- and Asset-Editor to enable authoring game content                                            |
| **Deferred Rendering**                | Deferred + Forward Rendering using scriptable Shaders                                                                |
| **Particle Systems**                  | using Instanced Rendering                                                                                            |

## Getting Started [Windows]
- Install Visual Studio 2022 and also install **Desktop Development with C++** using the Visual Studio Installer
![Screenshot of VS2022-Installer-Workloads](Docs/Images/VS2022-Installer-Workloads.png)
- To create a new or open an existing Suora Project, start the Launcher in /Binaries/
- Choose a Project Name and Location and select **Create Project**
![CreateProject](Docs/Images/CreateProject.png)
- Visual Studio should open by default
  - If not, open the Location in your File Explorer and use the Buildscripts in {ProjectLocation}/Scripts/
  - Open the .sln manually
- Choose **Editor** as your Startup Project and **Release** as your Configuration
- Compile and Run

## Build the Engine Yourself
- Clone the repository: git clone --recursive https://github.com/YanSchw/SuoraEngine
- Execute the Buildscript in /Binaries/Scripts/{...}
  - Or build SuoraBuildTool yourself in /Code/SuoraBuildTool
- Generate the Solution using /GenerateSolution.bat
- Open the Solution using Visual Studio 2022
- Build using VS2022

## Build Requirements
- C++20 Compiler
- [Windows] Visual Studio 2022 Toolset (**MSVC**)

## Supported Platforms
- Windows (64bit)
