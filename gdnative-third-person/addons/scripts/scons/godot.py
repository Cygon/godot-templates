#!/usr/bin/env python

import os
import shutil
import platform

# ----------------------------------------------------------------------------------------------- #

# Official download server for the Godot binaries
_download_server = 'https://downloads.tuxfamily.org'

# URLs of the headless server executables
_godot_headless_urls = {
    '3.0': _download_server + '/godotengine/3.0.6/Godot_v3.0.6-stable_linux_headless.64.zip',
    '3.1': _download_server + '/godotengine/3.1/Godot_v3.1-stable_linux_headless.64.zip'
}

# URLs of the export templates
_godot_template_urls = {
    '3.0': _download_server + '/godotengine/3.0.6/Godot_v3.0.6-stable_export_templates.tpz',
    '3.1': _download_server + '/godotengine/3.1/Godot_v3.1-stable_export_templates.tpz'
}

# Name of the Godot executables when using an official build or git build
_godot_executables = {
    '3.0': [
        'Godot_v3.0.6-stable_linux_headless.64',
        'Godot_v3.0.6-stable_x11.64',
        'Godot_v3.0.6-stable_win64.exe'
    ],
    '3.1': [
        'Godot_v3.1-stable_linux_headless.64',
        'Godot_v3.1-stable_x11.64',
        'Godot_v3.1-stable_win64.exe'
    ],
    'git': [
        'godot_headless.x11.opt.tools.64'
    ]
}

# Default version of Godot we will use (good idea? liability for future compatbility?)
_default_godot_version = '3.1'

# ----------------------------------------------------------------------------------------------- #

def register_extension_methods(environment):
    """Registers extensions methods for Godot builds into a SCons environment

    @param  environment  Environment the extension methods will be registered to"""

    environment.AddMethod(_export_project, "export_project")

# ----------------------------------------------------------------------------------------------- #

def _enumerate_subdirectories(root_directory, ignored_directories=['bin', 'obj']):
    """Enumerates the subdirectories inside a directory

    @param  root_directory       Directory whose direct subdirectories will be enumerated
    @param  ignored_directories  Directories to ignore if encountered"""

    directories = []

    for entry in os.listdir(root_directory):
        if os.path.isdir(entry):
            if not any(entry in s for s in ignored_directories):
                directories.append(entry)

    return directories

# ----------------------------------------------------------------------------------------------- #

def enumerate_assets(project_directory, variant_directory = None):
    """Forms a list of all Godot assets in a directory

    @param  project_directory     Directory containing the headers
    @param  variant_directory    Variant directory to which source paths will be rewritten"""

    # TODO: manually traverse directory levels and honor .gdignore files?
    # TODO: don't filter by file extensions, everything in a project directory is an asset?

    asset_file_extensions = [
        '.tscn',
        '.escn',
        '.scn',
        '.tres',
        '.res',
        '.dae',
        '.obj',
        '.wav',
        '.ogg',
        '.png',
        '.tga',
        '.tif',
        '.jpg',
        '.ttf',
        '.font',
        '.import'
    ]

    assets = []

    # Form a list of all files in the input directories recursively.
    for root, directory_names, file_names in os.walk(project_directory):
        for file_name in file_names:
            file_title, file_extension = os.path.splitext(file_name)
            if file_extension and any(file_extension in s for s in asset_file_extensions):
                if variant_directory is None:
                    headers.append(os.path.join(root, file_name))
                else:
                    headers.append(os.path.join(variant_directory, os.path.join(root, file_name)))

    return assets

# ----------------------------------------------------------------------------------------------- #

def _find_godot_executable(godot_version):
    """Locates a suitable Godot executable on the current system.

    @param  godot_version  Version number of Godot that is requested (i.e. '3.0' or '3.1')
    @returns The absolute path of a Godot executable."""

    candidate_directories = []

    # Build a list of directories in which Godot might be installed on
    # the current platform
    if platform.system() == 'Windows':
        base_directory = os.environ['ProgramFiles']
        for entry in os.listdir(base_directory):
            if 'godot' in entry.lower():
                candidate_directories.append(os.path.join(base_directory, entry))

        base_directory = os.environ['ProgramFiles(x86)']
        for entry in os.listdir(base_directory):
            if 'godot' in entry.lower():
                candidate_directories.append(os.path.join(base_directory, entry))
    else:
        for entry in os.listdir('/opt'):
            if 'godot' in entry.lower():
                candidate_directories.append(os.path.join('/opt', entry))

    # Now check all potential install locations for the Godot executable
    # in the version wanted by the user
    for directory in candidate_directories:

        # Check within the 'bin' directory if one exists
        bin_directory = os.path.join(directory, 'bin')
        if os.path.isdir(bin_directory):
            for executable_name in _godot_executables[godot_version]:
                candidate_path = os.path.join(bin_directory, executable_name)
                if os.path.isfile(candidate_path):
                    return candidate_path

        # Check the candidate directory itself
        for executable_name in _godot_executables[godot_version]:
            candidate_path = os.path.join(directory, executable_name)
            if os.path.isfile(candidate_path):
                return candidate_path

    return None

# ----------------------------------------------------------------------------------------------- #

def _export_project(environment):
    if 'GODOT_VERSION' in environment:
        godot_version = environment['GODOT_VERSION']
    else:
        godot_version = _default_godot_version

    godot_executable_path = _find_godot_executable(godot_version)

    print(godot_executable_path)
