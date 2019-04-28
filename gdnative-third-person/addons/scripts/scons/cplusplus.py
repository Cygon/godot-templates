#!/usr/bin/env python

import os
import shutil
import platform
import subprocess
import re
import types

"""
Helpers for building C/C++ projects with SCons
"""

# ----------------------------------------------------------------------------------------------- #

def setup(environment):
    """Registers extension methods for C/C++ builds into a SCons environment

    @param  environment  Environment the extension methods will be registered to"""

    environment.AddMethod(_add_include_directory, "add_include_directory")
    environment.AddMethod(_add_library_directory, "add_library_directory")
    environment.AddMethod(_add_library, "add_library")
    environment.AddMethod(_get_build_directory_name, "get_build_directory_name")

# ----------------------------------------------------------------------------------------------- #

def enumerate_headers(header_directory, variant_directory = None):
    """Forms a list of all C/C++ header files in an include directory

    @param  header_directory     Directory containing the headers
    @param  variant_directory    Variant directory to which source paths will be rewritten"""

    source_file_extensions = [
        '.h',
        '.H',
        '.hpp',
        '.hh',
        '.hxx',
        '.inl',
        '.inc'
    ]

    headers = []

    # Form a list of all files in the input directories recursively.
    for root, directory_names, file_names in os.walk(header_directory):
        for file_name in file_names:
            file_title, file_extension = os.path.splitext(file_name)
            if file_extension and any(file_extension in s for s in source_file_extensions):
                if variant_directory is None:
                    headers.append(os.path.join(root, file_name))
                else:
                    headers.append(os.path.join(variant_directory, os.path.join(root, file_name)))

    return headers

# ----------------------------------------------------------------------------------------------- #

def enumerate_sources(source_directory, variant_directory = None):
    """Forms a list of all C/C++ source code files in a source directory

    @param  source_directory     Directory containing the C/C++ source code files
    @param  variant_directory    Variant directory to which source paths will be rewritten"""

    source_file_extensions = [
        '.c',
        '.C',
        '.cpp',
        '.cc',
        '.cxx',
        '.inl',
        '.inc'
    ]

    sources = []

    # Form a list of all files in the input directories recursively.
    for root, directory_names, file_names in os.walk(source_directory):
        for file_name in file_names:
            file_title, file_extension = os.path.splitext(file_name)
            if file_extension and any(file_extension in s for s in source_file_extensions):
                if variant_directory is None:
                    sources.append(os.path.join(root, file_name))
                else:
                    sources.append(os.path.join(variant_directory, os.path.join(root, file_name)))

    return sources

# ----------------------------------------------------------------------------------------------- #

def find_or_guess_include_directory(package_path):
    """Tries to locate the include directory for a package

    @param  self          The instance this method should work on
    @param  package_path  Path to the package"""

    candidate = os.path.join(package_path, 'include')
    if os.path.isdir(candidate):
        return candidate

    candidate = os.path.join(package_path, 'Include')
    if os.path.isdir(candidate):
        return candidate

    package_name = os.path.basename(os.path.normpath(package_path))
    candidate = os.path.join(package_path, package_name)
    if os.path.isdir(candidate):
        return candidate

    return None

# ------------------------------------------------------------------------------------------- #

def find_or_guess_library_directory(environment, library_builds_path):
    """Tries to locate the library directory for a package

    @param  environment          Environment used to look up architecture and compiler
    @param  library_builds_path  Path to the directory containing the library builds
                                 Each build is expected to be in a directory matching
                                 the build directory name (_get_build_directory_name())
    @remarks
        This can be used to automatically find the directory in which precompiled
        library binaries are stored."""

    compiler_name = get_compiler_name(environment)
    if compiler_name is None:
        raise FileNotFound('C/C++ compiler could not be found')

    compiler_version = get_compiler_version(environment)
    if compiler_version is None:
        raise FileNotFound('C/C++ compiler could not be found')

    major_compiler_version = int(compiler_version[0])
    while major_compiler_version > 6: # We don't serve compilers earlier than this :-)

        # New naming scheme, using only major compiler/toolset version
        library_build_name = _make_build_directory_name(
            environment, compiler_name, str(major_compiler_version)
        )
        candidate = os.path.join(library_builds_path, library_build_name)

        if os.path.isdir(candidate):
            return candidate

        # Compatibility with existing naming scheme in externals repository
        library_build_name = _make_build_directory_name(
            environment, compiler_name, str(major_compiler_version) + '.0'
        )
        candidate = os.path.join(library_builds_path, library_build_name)

        if os.path.isdir(candidate):
            return candidate

        # Also try builds for previous compiler versions
        major_compiler_version -= 1

    # No compiler-specified binaries, give the 'lib' dir a final try
    candidate = os.path.join(library_builds_path, 'lib')
    if os.path.isdir(candidate):
        return candidate

    return None

# ----------------------------------------------------------------------------------------------- #

def get_platform_specific_library_name(universal_library_name, static = False):
    """Forms the platform-specific library name from a universal library name

    @param  universal_library_name  Universal name of the library that will be converted
    @param  static                  Whether the name is for a static library
    @returns The platform-specific library name
    @remarks
      A universal library name is just the name of the library without extension,
      using dots to separate words - for example My.Awesome.Stuff. Depending on the platform,
      this might get turned into My.Awesome.Stuff.dll or libMyAwesomeStuff.so"""

    if platform.system() == 'Windows':

        if static:
            return universal_library_name + ".lib"
        else:
            return universal_library_name + ".dll"

    else:

        # Because Linux tools automatically add 'lib' and '.a'/'.so'
        return universal_library_name.replace('.', '')

        #if static:
        #    return 'lib' + universal_library_name.replace('.', '') + '.a'
        #else:
        #    return 'lib' + universal_library_name.replace('.', '') + '.so'

# ----------------------------------------------------------------------------------------------- #

def get_platform_specific_executable_name(universal_executable_name):
    """Forms the platform-specific executable name from a universal executable name

    @param  universal_executable_name  Universal name of the executable that will be converted
    @returns The platform-specific executable name
    @remarks
      A universal executable name is just the name of the executable without extension,
      using dots to separate words - for example My.Awesome.Program. Depending on the platform,
      this might get turned into My.Awesome.Program.exe or MyAwesomeProgram."""

    if platform.system() == 'Windows':
        return universal_executable_name + ".exe"
    else:
        return universal_executable_name.replace('.', '')

# ----------------------------------------------------------------------------------------------- #

def get_compiler_name(environment):
    """Returns a short string identifying the compiler (or compiler group) being used

    @param  environment  Environment from which the compiler will be looked up
    @returns The name of the compiler (or shared name of a group compilers) being used"""

    compiler_executable = None

    if 'CXX' in environment:
        compiler_executable = environment['CXX']
        if compiler_executable == "$CC":
            compiler_executable = environment['CC']
    elif 'CC' in environment:
        compiler_executable = environment['CC']
    else:
        raise FileNotFound('No C/C++ compiler found')

    if (compiler_executable == 'cl') or (compiler_executable == 'icc'):
        return 'msvc'
    elif (compiler_executable == 'gcc') or (compiler_executable == 'g++'):
        return 'gcc'
    else:
        return compiler_executable

# ----------------------------------------------------------------------------------------------- #

def get_compiler_version(environment):
    """Determines the version number of the C/C++ compiler being used

    @param  environment  Environment from which the C/C++ compiler executable will be looked up
    @returns The compiler version number, as an array of [Major, Minor, Revision]"""

    compiler_executable = None

    if 'CXX' in environment:
        compiler_executable = environment['CXX']
        if compiler_executable == "$CC":
	        compiler_executable = environment['CC']
    elif 'CC' in environment:
        compiler_executable = environment['CC']
    else:
        raise FileNotFound('No C/C++ compiler found')

    if (compiler_executable == 'cl') or (compiler_executable == 'icc'):
        if 'MSVC_VERSION' in environment:
            compiler_version = environment['MSVC_VERSION']
            return compiler_version.split('.')

        if 'MSVS' in environment:
            #print(environment['MSVS'])
            cl_install_directory = environment['MSVS']['VCINSTALLDIR']
            #print(cl_install_directory)

        msvc_process = subprocess.Popen(
            [compiler_executable], stdout=subprocess.PIPE
        )
        (stdout, stderr) = msvc_process.communicate()

        compiler_version = re.search('[0-9][0-9.]*', str(stdout))

    else:
        gcc_process = subprocess.Popen(
            [compiler_executable, '--version'], stdout=subprocess.PIPE
        )
        (stdout, stderr) = gcc_process.communicate()

        compiler_version = re.search('[0-9][0-9.]*', str(stdout))

    # If no match is found the compiler didn't proide the expected output
    # and we have no idea which version it might be
    if compiler_version is None:
        return None

    version = compiler_version.group().split('.')
    return version

# ----------------------------------------------------------------------------------------------- #

def _get_build_directory_name(environment):
    """Determines the name of the build directory for the current compiler version
    and output settings (such as platform and whether it's a debug or release build)

    @param  environment  Environment for which the build directory will be determined
    @returns The name the build directory should have
    @remarks
        The build directory is a directory whose name uniquely identifies the compiler,
        platform and build configuration used. When shipping cross-platform libraries,
        for example, a compiled binary of the library can be shipped for each compiler,
        architecture and build configuration supported, allowing developers to pick
        the right one to link depending on their system."""

    compiler_name = get_compiler_name(environment)
    if compiler_name is None:
        raise FileNotFound("C/C++ compiler could not be found")

    compiler_version = get_compiler_version(environment)
    if compiler_version is None:
        raise FileNotFound("C/C++ compiler could not be found")

    return _make_build_directory_name(environment, compiler_name, compiler_version[0])

# ----------------------------------------------------------------------------------------------- #

def _make_build_directory_name(environment, compiler_name, compiler_version):
    """Forms the build directory name given a compiler name, compiler version,
    target architecture and build configuration.

    @param  environment       Environment providing additional build settings
    @param  compiler_name     Name of the compiler that is being used
    @param  compiler_version  Major version number of the compiler that is being used
    @returns The build directory name for the specified compiler and architecture"""

    if platform.system() == 'Windows':
        platform_name = 'windows'
    else:
        platform_name = 'linux'

    architecture = _get_architecture_or_default(environment)

    is_debug_build = False
    if 'DEBUG' in environment:
        is_debug_build = environment['DEBUG']

    if is_debug_build:
        build_configuration = 'debug'
    else:
        build_configuration = 'release'

    return (
        platform_name + '-' +
        compiler_name + compiler_version + '-' +
        architecture + '-' +
        build_configuration
    )

# ----------------------------------------------------------------------------------------------- #

def _get_architecture_or_default(environment):
    """Returns the current target architecture or the default architecture if none
    has been explicitly set.

    @param  environment  Environment the target architecture will be looked up from
    @returns The name of the target architecture from the environment or a default"""

    architecture = environment['TARGET_ARCH']
    if architecture is None:
        return 'x64'
    else:
        return architecture

# ----------------------------------------------------------------------------------------------- #

def _add_include_directory(environment, include_directory, system = False):
    """Adds an C/C++ include directory to the build

    @param  environment        Environment the C/C++ include directory will be added to
    @param  include_directory  Include directory that will be added
    @param  system             Whether this is a system header directory
                               (suppresses warnings for headers you have no control over)
    @remarks
        Consider using add_package() instead to automatically set up all include and
        library directories as well as link the libraries themselves."""

    if system:
        if platform.system() == 'Windows':
            # Supported in recent update to Visual Studio 2017
            # https://devblogs.microsoft.com/cppblog/broken-warnings-theory/
            environment.Append(CCFLAGS=('/external:I', include_directory))
        else:
            environment.Append(CCFLAGS=('-isystem', include_directory))
    else:
        environment.Append(CPPPATH=[include_directory])

# ----------------------------------------------------------------------------------------------- #

def _add_library_directory(environment, library_directory):
    """Adds a C/C++ library directory to the build

    @param  environment        Environment the C/C++ library directory will be added to
    @param  library_directory  Library directory that will be added
    @remarks
        Consider using add_package() instead to automatically set up all include and
        library directories as well as link the libraries themselves."""

    environment.Append(LIBPATH=[library_directory])

# ----------------------------------------------------------------------------------------------- #

def _add_library(environment, library_name):
    """Adds a C/C++ library to the build

    @param  environment   Environment the C/C++ library will be linked to
    @param  library_name  Name of the library that will be linked
    @remarks
        Consider using add_package() instead to automatically set up all include and
        library directories as well as link the libraries themselves.

        The libary name is platform-specific. On Windows systems, the library is usually
        called MyAwesomeThing.lib while on GNU/Linux systems, the convention is
        libMyAwesomeThing.so (the toolchain will automatically try the lib prefix, though)"""

    environment.Append(LIBS=[library_name])
