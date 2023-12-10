This covers the GameFramework and commonly used terms when working with Suora.

## Project

A Suora Engine Project contains all the contents of your game. 

The project directory contains the following folders and files:


| File or Folder          | Description                                                       |
| -------                 | -----------                                                       |
| **/Build**              | (C++ only) contains the latest build, when building from C++     |
| **/Content**            | Contains all the projects **Assets**                              |
| **/Code**     | Contains all the Projects Code Modules. A C++ Module is located in /Code/Modules/**{MODULE_NAME}** |

The **Content Drawer** inside the Editor mirrors the same directory structure found inside the Project folder on your disk.

All Suora Projects have a **.suora** Project file associated with them. This file can be found in the **/Content** folder, as the Projectsettings are an **Asset** by themselves.

## Object

The **Object** class is the Baseclass of all **SuoraClasses**, that the **Engine** knows of through **Reflection** (Introspection).
You usually derive from **Object** or one of its derivatives to expose your **Class** to the **Engine**.

~~~c++
#include "Suora/Core/Object/Object.h"
#include "MyObjectDerivative.generated.h"

class MyObjectDerivative : public Object
{
    SUORA_CLASS(5789545);
public:
};
~~~
*This code exposes the **Class** 'MyObjectDerivative'*

## Class

A **Class** is a handle, that the **Engine** can use, to allocate, introspect, serialize or deserialize an **Object**.
For every C++ **SuoraClass** you can use
~~~c++
Class cls = OBJECT_CLASS_NAME::StaticClass();
~~~
to obtain the native **Class** handle.

However, a **Class**, must not be a **Native Class**.
All **Blueprints** are also wrapped by **Class**.

For **Native Classes** the handle is the **NativeClassID**.
For **Blueprint Classes** the handle is the Blueprint Assets **UUID**.

# GameFramework

## Node

## GameInstance