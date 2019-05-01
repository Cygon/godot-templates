#ifndef NUCLEX_CPPEXAMPLE_MAIN_H
#define NUCLEX_CPPEXAMPLE_MAIN_H

#include "Config.h"

#include <Godot.hpp>

// --------------------------------------------------------------------------------------------- //

/// <summary>Initializes the GDNative module when it is loaded</summary>
/// <param name='o'>Provides version information and connection points to the engine</param>
extern "C" void NUCLEX_CPPEXAMPLE_API godot_gdnative_init(godot_gdnative_init_options *o);

/// <summary>Cleans up all resource used by the GDNative module before it unloads</summary>
/// <param name='o'>Provides additional informations about the shutdown</param>
extern "C" void NUCLEX_CPPEXAMPLE_API godot_gdnative_terminate(
  godot_gdnative_terminate_options *o
);

/// <summary>Registers the module's classes and methods to the engine</summary>
extern "C" void NUCLEX_CPPEXAMPLE_API godot_nativescript_init(void *handle);

// --------------------------------------------------------------------------------------------- //

#endif // NUCLEX_CPPEXAMPLE_MAIN_H
