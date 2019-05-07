GDNative Third-Person Template
==============================

This is a template project for the Godot Game Engine that uses C++ to
implement the game logic. It demonstrates a handful of cool things:

- Writing your game code in C++ with GDnative

- Using SCons to export your models and compile your code
- Simplifying SCons builds using convention-over-configuration

- KinematicBody-based character controllers using real physics formulas

- Using Blender's Rigify system to animate a character
- Storing animations in .blend files separate from the master model
  (great for animations involving with 2 or more characters!)

I don't want to bore you, so this is a short, information-dense
overview to help you quickly find the things that interest you.


Requirements
------------

* SCons 3.0+
* Godot 3.1
* Blender 2.79
* "Better Collada" exporter plug-in for Blender
  (https://github.com/godotengine/collada-exporter)


Quickstart
----------

The `.7z` version of the template comes with everything already compiled,
so you can just open it in Godot and check it out.

To compile everything yourself, run the `SConstruct` file in the root
directory of the template. It will do all Blender exports, call Godot
to generate `api.json` and compile Godot-CPP and the GDNative code.

Hint: use `-j8` when invoking SCons, where 8 is the number of things
SCons should do in parallel (match it to your CPU core count)

    scons -j8


Blender Overview
----------------

**Rigify**

The character model used by the game is `Game/Actors/Dummy/Models/Dummy.Rigify.blend`.

It uses Blender's "Rigify", a modular rig system for humanoids, animals and
pretty much anything you can throw at it. Rigify includes an optional face rig,
which this character also uses.

[screenshot of rig + bone palette]

Do note the bone selection menu on the right. If you want to create some custom
animations, different sets of bones can be hidden/shown there.

The base mesh is a CC0-licensed model available from BlendSwap, with the original
rig discarded and replaced by a legacy Rigify rig with PitchiPoy face rig.


**Animations**

Animations are stored in a separate file at
`Game/Animations/BasicMovements/Models/Dummy-Locomotion.blend`

The animation `.blend` file links to the character model (rather than being
a copy of it).

[screenshot of the animations list]

Splitting a character's animations into multiple `.blend` files makes exports
(and therefore, the feedback cycle) faster and allows multiple artists to work
on animations in parallel.

Also, if you have animations where 2 or more characters interact, the animation
`.blend` file can link two master models, store animation clips or both
characters and play them sync when you scrub through the timeline!


SCons Build Script for Blender Export
-------------------------------------

Exporting from Blender to Godot can be tedious. A successful export of
a rigged model requires a lot of manual steps, including applying all
modifiers (the option for this in "Better Collada" would break things),
selecting all the objects you wish to export and ticking some check boxes.

This project automates all of the above with SCons.

- Example `SConstruct` file for model export:
  `Game/Actors/Dummy/SConstruct`

- Example `SConstruct` file for animation export:
  `Game/Animations/BasicMovements`

Furthermore, the a master `SConstruct` file in the project's root directory
recursively searches for `SConstruct` scripts in `Game` and all its
subdirectories, so adding more models with `SConstruct` scripts is painless.

Only models where the `.blend` file has changed since the last export will be
exported, so firing up the master `SConstruct` after any change is a nice way
to bring all exports up-to-date.


SCons Build Script for GDNative C++
-----------------------------------

Compilation of the GDNative C++ project also happens via SCons. My build
system uses a convention-over-configuration approach and makes the following
assumptions about a project (unless you override them):

- You want to compile for the current platform
- There's a folder `Source` containing all source files
- You want to do an optimized release build

The GDNative project's `SConstruct` file can be found in the following location:

`addons/example-cpp/SConstruct`

It, too, will be built when you build the master `SConstruct` file at the root
of the project, in parallel with the model exports even, should you use
the `-j8` option.

Note that Eexample-CPP depends on Godot-CPP (the GDNative C++ wrapper library),
and that Godot-CPP will only be compiled when you build the master `SConstruct`
file. Running the `SConstruct` of Example-CPP is fine, but 


GDNative Game Code
------------------

