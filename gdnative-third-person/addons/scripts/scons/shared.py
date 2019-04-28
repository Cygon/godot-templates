#!/usr/bin/env python

import os
import shutil

"""
Shared code for SCons projects

This file provides common helper methods for the other SCons modules.
"""

# ----------------------------------------------------------------------------------------------- #

def enumerate_subdirectories(root_directory, ignored_directories=[]):
    """Enumerates the direct subdirectories inside a directory

    @param  root_directory       Directory whose direct subdirectories will be enumerated
    @param  ignored_directories  Directories to ignore if encountered"""

    directories = []

    for entry in os.listdir(root_directory):
        if os.path.isdir(entry):
            if not any(entry in s for s in ignored_directories):
                directories.append(entry)

    return directories

# ----------------------------------------------------------------------------------------------- #

def enumerate_any_files(source_directory, variant_directory = ""):
    """Forms a list of all files in a project directory

    @param  source_directory     Directory containing all of the source files
    @param  variant_directory    Variant directory to which source paths will be rewritten"""

    files = []

    # Form a list of all files in the input directories recursively.
    for root, directory_names, file_names in os.walk(subdirectory):
        for file_name in file_names:
            if variant_directory:
                files.append(os.path.join(variant_directory, os.path.join(root, file_name)))
            else:
                files.append(os.path.join(root, file_name))

    return files

# ----------------------------------------------------------------------------------------------- #

def setup_variant_dirs(
    scons_environment, project_directory, intermediate_directory, ignored_directories
):
    """Sets up SCons variant directories for all direct subdirectories in a path

    @param  scons_environment       The SCons environment in which variant dirs will be set up
    @param  project_directory       Directory that contains the project's files
    @param  intermediate_directory  Directory into which intermediate files will be placed
    @param  ignored_directories     Directories that will be ignored if encountered"""

    # Set up VariantDirs for all direct subdirectories
    subdirectories = _enumerate_subdirectories(project_directory, ignored_directories)
    for subdirectory in subdirectories:
        build_directory = os.path.join(intermediate_directory, subdirectory)
        scons_environment.VariantDir(build_directory, subdirectory, duplicate = 0)

# ----------------------------------------------------------------------------------------------- #
