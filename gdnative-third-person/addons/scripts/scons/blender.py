#!/usr/bin/env python

import os
import shutil
import platform
import re

if platform.system() == 'Windows':
    import winreg

# ----------------------------------------------------------------------------------------------- #

# Paths in which the Blender executables can be found on Windows systems
_blender_windows_path = {
    '2.7': [
        '%ProgramFiles%\\Blender Foundation\\Blender\\blender.exe'
    ],
    '2.8': [
        '%ProgramFiles%\\Blender Foundation\\Blender\\blender.exe'
    ]
}

# Default version of Blender we will use
_default_blender_version = '2.7'

# ----------------------------------------------------------------------------------------------- #

def setup(environment):
    """Registers extension methods for Blender exporting into a SCons environment

    @param  environment  Environment the extension methods will be registered to"""

    environment.AddMethod(_export_fbx_or_collada, "export_fbx")
    environment.AddMethod(_export_fbx_or_collada, "export_collada")
    environment.AddMethod(_export_gltf, "export_gltf")
    environment.AddMethod(_export_animations_fbx_or_collada, "export_animations_fbx")
    environment.AddMethod(_export_animations_fbx_or_collada, "export_animations_collada")
    environment.AddMethod(_export_animations_gltf, "export_animations_gltf")

# ----------------------------------------------------------------------------------------------- #

def enumerate_blendfiles(root_directory, variant_directory = None):
    """Forms a list of all Blender files in a directory

    @param  root_directory     Directory containing the Blender files
    @param  variant_directory  Variant directory to which source paths will be rewritten"""

    blendfile_extensions = [
        '.blend',
    ]

    assets = []

    # Form a list of all files in the input directories recursively.
    for root, directory_names, file_names in os.walk(project_directory):
        for file_name in file_names:
            file_title, file_extension = os.path.splitext(file_name)
            if file_extension and any(file_extension in s for s in blendfile_extensions):
                if variant_directory is None:
                    headers.append(os.path.join(root, file_name))
                else:
                    headers.append(os.path.join(variant_directory, os.path.join(root, file_name)))

    return assets

# ----------------------------------------------------------------------------------------------- #

def _find_blender_executable(environment, blender_version):
    """Locates a suitable Blender executable on the current system.

    @param  blender_version  Version number of Blender that is requested (i.e. '2.7'(
    @returns The absolute path of a Blendr executable."""

    candidate_directories = []

    if platform.system() == 'Windows':
        blender_path = _find_blender_in_windows_registry()
        if not (blender_path is None):
            return blender_path
    else:
        blender_path = environment.WhereIs('blender')
        if blender_path:
            return blender_path

    # Build a list of directories in which Blender might be installed on the system
    if platform.system() == 'Windows':
        base_directory = os.environ['ProgramFiles']
        for entry in os.listdir(base_directory):
            if 'blender' in entry.lower():
                candidate_directories.append(os.path.join(base_directory, entry))

        base_directory = os.environ['ProgramFiles(x86)']
        for entry in os.listdir(base_directory):
            if 'blender' in entry.lower():
                candidate_directories.append(os.path.join(base_directory, entry))
    else:
        for entry in os.listdir('/opt'):
            if 'blender' in entry.lower():
                candidate_directories.append(os.path.join('/opt', entry))

    if platform.system() == 'Windows':
        executable_name = 'blender.exe'
    else:
        executable_name = 'blender'

    # Now check all potential install locations for the Blender executable
    for directory in candidate_directories:

        # Check within the 'bin' directory if one exists
        blender_directory = os.path.join(directory, 'Blender')
        if os.path.isdir(blender_directory):
            candidate_path = os.path.join(blender_directory, executable_name)
            if os.path.isfile(candidate_path):
                return candidate_path

        # Check the candidate directory itself
        candidate_path = os.path.join(directory, executable_name)
        if os.path.isfile(candidate_path):
            return candidate_path

    return None

# ----------------------------------------------------------------------------------------------- #

def _find_blender_in_windows_registry():
    """Tries to locate Blender using the Windows registry

    @returns The path to the Blender executable or None if not found"""
    commands = []

    # There is no clear install path in the registry (and the one there is has a GUID-like
    # number in front of it, so it's likely not future-proof). So we'll check the file
    # association for .blend files. If this fails, we can still scour the system's
    # "Program Files" directory.
    try:
        blendfile_key = winreg.OpenKey(
            winreg.HKEY_LOCAL_MACHINE,
            'SOFTWARE\\Classes\\blendfile\\shell\\open\\command', 0, winreg.KEY_READ
        )
        commands.append(winreg.QueryValue(blendfile_key, str()))
        winreg.CloseKey(blendfile_key)
    except FileNotFoundError:
        pass

    try:
        blendfile_key = winreg.OpenKey(
            winreg.HKEY_CLASSES_ROOT,
            'blendfile\\shell\\open\\command', 0, winreg.KEY_READ
        )
        commands.append(winreg.QueryValue(blendfile_key, str()))
        winreg.CloseKey(blendfile_key)
    except FileNotFoundError:
        pass

    candidates = []

    # Now we have a few "commands" which invoke a program when .blend files are
    # opened on the system. Extract the path of the program they're invoking.
    for command in commands:
        matches = re.findall(r'\"(.+?)\"', command)
        if not (matches is None):
            length = len(matches)
            if length > 0:
                candidates.append(matches[0])

    # Finally, the program may be something else, so look for a candidate that
    # points to an actually existing path to a file with the name "blender.exe" in it
    for candidate in candidates:
        if os.path.isfile(candidate):
            if 'blender.exe' in candidate.lower():
                return candidate

    return None

# ----------------------------------------------------------------------------------------------- #

def _export_fbx_or_collada(environment, target_path, blendfile_path, meshes = None):
    """Exports a blendfile to FBX or Collada

    @param  environment     SCons environment in which the export will be done
    @param  target_path     Path in which the target file will be saved
    @param  blendfile_path  Path of the source blendfile containing the meshes
    @param  meshes          List of meshes that will be exported"""

    blender_executable = _find_blender_executable(environment, _default_blender_version)
    if blender_executable is None:
        raise FileNotFoundError("Could not locate a Blender executable")

    own_directory = os.path.dirname(__file__)
    relative_script_path = os.path.join(own_directory, 'blender-export-meshes.py')
    absolute_script_path = environment.File('#' + relative_script_path).srcnode().abspath

    extra_arguments = str()
    if not (meshes is None):
        for mesh in meshes:
            extra_arguments += ' ' + mesh

    # Finally, invoke Blender to export the model using an export script that
    # gets executed by Blender itself
    return environment.Command(
        source = blendfile_path,
        action = (
            '"' + blender_executable + '" "$SOURCE"' +
            ' --enable-autoexec' +
            ' --python "' + absolute_script_path + '"' +
            ' --background' +
            ' --' +
            ' $TARGET ' +
            extra_arguments
        ),
        target = target_path
    )

# ----------------------------------------------------------------------------------------------- #

def _export_gltf(environment):
    pass

# ----------------------------------------------------------------------------------------------- #

def _export_animations_fbx_or_collada(
    environment, target_path, actor_blendfile_path, animation_blendfile_path, animations = None
):
    """Exports only the animations in a blendfile to FBX or Collada

    @param  environment               SCons environment in which the export will be done
    @param  target_path               Path in which the target file will be saved
    @param  actor_blendfile_path      Path of the blendfile containing the rigged actor
    @param  animation_blendfile_path  Path of the blendfile containing the animations
    @param  animations                List of animations that will be exported"""

    blender_executable = _find_blender_executable(environment, _default_blender_version)
    if blender_executable is None:
        raise FileNotFoundError("Could not locate a Blender executable")

    own_directory = os.path.dirname(__file__)
    relative_script_path = os.path.join(own_directory, 'blender-export-animations.py')
    absolute_script_path = environment.File('#' + relative_script_path).srcnode().abspath

    animation_blendfile_path = environment.File(animation_blendfile_path).srcnode().abspath

    extra_arguments = str()
    if not (animations is None):
        for animation in animations:
            extra_arguments += ' ' + animation

    # Finally, invoke Blender to export the model using an animation export script that
    # gets executed by Blender itself
    export_command = environment.Command(
        source = actor_blendfile_path,
        action = (
            '"' + blender_executable + '" "$SOURCES"' +
            ' --enable-autoexec' +
            ' --python "' + absolute_script_path + '"' +
            ' --background' +
            ' --' +
            ' $TARGET ' +
            ' ' + animation_blendfile_path +
            extra_arguments
        ),
        target = target_path
    )
    environment.Depends(export_command, animation_blendfile_path)

# ----------------------------------------------------------------------------------------------- #

def _export_animations_gltf(environment):
    pass

