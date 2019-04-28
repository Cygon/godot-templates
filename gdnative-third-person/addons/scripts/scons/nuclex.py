#!/usr/bin/env python

import os
import importlib
import platform
import types

from SCons.Environment import Environment
from SCons.Variables import Variables
from SCons.Variables import EnumVariable
from SCons.Variables import PathVariable
from SCons.Variables import BoolVariable
from SCons.Script import ARGUMENTS
from SCons.Script import Dir
from SCons.Util import WhereIs

# Nuclex SCons libraries
shared = importlib.import_module('shared')
cplusplus = importlib.import_module('cplusplus')
dotnet = importlib.import_module('dotnet')
blender = importlib.import_module('blender')

# Plan:
#   - if TARGET_ARCH is set, use it. For multi-builds,
#     this may result in failure, but that's okay
#   - multi-builds (typical case x86/x64 native + .net anycpu)
#     must control target arch manually anyway.

# ----------------------------------------------------------------------------------------------- #

def create_generic_environment():
    """Creates an general-purpose environment without specific support for any
    programming language or resource/asset system

    @return A new SCons environment without support for specific builds"""

    environment = Environment(
        variables = _parse_default_command_line_options()
    )

    _register_generic_extension_methods(environment)

    return environment

# ----------------------------------------------------------------------------------------------- #

def create_cplusplus_environment():
    """Creates a new environment with the required variables for building C/C++ projects

    @returns A new SCons environment set up for C/C++ builds"""

    environment = Environment(
        variables = _parse_default_command_line_options(),
        SOURCE_DIRECTORY = 'Source',
        HEADER_DIRECTORY = 'Include',
        TESTS_DIRECTORY = 'Tests',
        TESTS_RESULT_FILE = "gtest-results.xml",
        REFERENCES_DIRECTORY = 'References'
    )

    # Extension methods from the C/C== module
    cplusplus.setup(environment)

    # Nuclex standard build settings and extensions
    _set_standard_cplusplus_compiler_flags(environment)
    _set_standard_cplusplus_linker_flags(environment)
    _register_generic_extension_methods(environment)
    _register_cplusplus_extension_methods(environment)

    return environment

# ----------------------------------------------------------------------------------------------- #

def create_dotnet_environment():
    """Creates a new environment with the required variables for building .NET projects

    @returns A new scons environment set up for .NET builds"""

    environment = Environment(
        variables = _parse_default_command_line_options(),
        SOURCE_DIRECTORY = 'Source',
        TESTS_DIRECTORY = 'Tests',
        TESTS_RESULT_FILE = "nunit-results.xml",
        REFERENCES_DIRECTORY = 'References'
    )

    # Register extension methods and additional variables
    dotnet.setup(environment)

    _register_generic_extension_methods(environment)
    _register_dotnet_extension_methods(environment)

    return environment

# ----------------------------------------------------------------------------------------------- #

def create_blender_environment():
    """Creates a new environment with the required variables for export Blender models

    @returns A new scons environment set up for Blender exports"""

    environment = Environment(
        variables = _parse_default_command_line_options()
    )

    _register_generic_extension_methods(environment)
    _register_blender_extension_methods(environment)

    return environment

# ----------------------------------------------------------------------------------------------- #

def build_all(environment, root_directory):
    """Compiles all SCons build scripts below the specified directory

    @param  environment     SCons environment that will be starting the nested builds
    @param  root_directory  Directory below which all SCons build scripts will be executed"""

    build_scripts = _get_all_build_scripts(root_directory)
    for build_script in build_scripts:
        environment.SConscript(build_script)

# ----------------------------------------------------------------------------------------------- #

def _get_all_build_scripts(root_directory):
    """Locates SCons build scripts in all directories below the specified root

    @param  root_directory  Directory below which SCons scripts will be collected
    @returns All SCons scripts below the specified root directory"""

    scripts = []

    for entry in os.listdir(root_directory):
        path = os.path.join(root_directory, entry)
        if os.path.isdir(path):
            _recursively_collect_build_scripts(scripts, path)

    #scripts.reverse()

    return scripts

# ----------------------------------------------------------------------------------------------- #

def _recursively_collect_build_scripts(scripts, directory):
    """Recursively searches for SCons build scripts and adds them to the provided list

    @param  scripts    List to which any discovered build scripts will be added
    @param  directory  Directory from which on the method will recursively search"""

    for entry in os.listdir(directory):
        path = os.path.join(directory, entry)
        if os.path.isdir(path):
            _recursively_collect_build_scripts(scripts, path)
        elif os.path.isfile(path):
            if ('SConstruct' in entry) or ('SConscript' in entry):
                scripts.append(path)


# ----------------------------------------------------------------------------------------------- #

def _parse_default_command_line_options():
    """Parses the command line options controlling various build settings

    @remarks
        This contains variables that work across all builds. Build-specific variables
        are discouraged, but would be irgnored by SCons' Variables class."""

    command_line_variables = Variables(None, ARGUMENTS)

    # Build configuration (also called build type in many SCons examples)
    command_line_variables.Add(
        BoolVariable(
            'DEBUG',
            'Whether to do an unoptimized debug build',
            False
        )
    )

    # Default architecture for the binaries. We follow the Debian practices,
    # which, while clueless and chaotic, are at least widely used.
    default_arch = 'amd64'
    if 'armv7' in platform.uname()[4]:
        default_arch = 'armhf'
    if 'aarch64' in platform.uname()[4]:
        default_arch = 'arm64'

    # CPU architecture to target
    command_line_variables.Add(
        EnumVariable(
            'TARGET_ARCH',
            'CPU architecture the binary will run on',
            default_arch,
            allowed_values=('armhf', 'arm64', 'x86', 'amd64', 'any')
        )
    )

    # Directory for intermediate files
    command_line_variables.Add(
        PathVariable(
            'INTERMEDIATE_DIRECTORY',
            'Directory in which intermediate build files will be stored',
            'obj',
            PathVariable.PathIsDirCreate
        )
    )

    # Directory for intermediate files
    command_line_variables.Add(
        PathVariable(
            'ARTIFACT_DIRECTORY',
            'Directory in which build artifacts (outputs) will be stored',
            'bin',
            PathVariable.PathIsDirCreate
        )
    )

    return command_line_variables

# ----------------------------------------------------------------------------------------------- #

def _register_generic_extension_methods(environment):
    """Registers general-purpose extension methodsinto a SCons environment

    @param  environment  Environment the extension methods will be registered to"""

    environment.AddMethod(_build_scons, "build_scons")
    environment.AddMethod(_is_debug_build, "is_debug_build")

# ----------------------------------------------------------------------------------------------- #

def _register_cplusplus_extension_methods(environment):
    """Registers extension methods for C/C++ builds into a SCons environment

    @param  environment  Environment the extension methods will be registered to"""

    environment.AddMethod(_add_cplusplus_package, "add_package")
    environment.AddMethod(_build_cplusplus_library, "build_library")
    environment.AddMethod(_build_cplusplus_library_with_tests, "build_library_with_tests")
    environment.AddMethod(_build_cplusplus_executable, "build_executable")
    environment.AddMethod(_run_cplusplus_unit_tests, "run_unit_tests")

# ----------------------------------------------------------------------------------------------- #

def _register_dotnet_extension_methods(environment):
    """Registers extension methods for .NET builds into a SCons environment

    @param  environment  Environment the extension methods will be registered to"""

    environment.AddMethod(_build_msbuild_project, "build_project")
    environment.AddMethod(_build_msbuild_project_with_tests, "build_project_with_tests")

# ----------------------------------------------------------------------------------------------- #

def _register_blender_extension_methods(environment):
    """Registers extension methods for Blender exports into a SCons environment

    @param  environment  Environment the extension methods will be registered to"""

    blender.register_extension_methods(environment)

# ----------------------------------------------------------------------------------------------- #

def _is_debug_build(environment):
    """Checks whether a debug build has been requested

    @param  environment  Environment whose settings will be checked for a debug build
    @returns True if a debug build has been requested, otherwise False"""

    if 'DEBUG' in environment:
        return environment['DEBUG']
    else:
        return False

# ----------------------------------------------------------------------------------------------- #

def _set_standard_cplusplus_compiler_flags(environment):
    """Sets up standard flags for the compiler

    @param  environment  Environment in which the C++ compiler flags wlll be set."""

    if platform.system() == 'Windows':
        environment.Append(CXXFLAGS='/EHsc') # Only C++ exceptions, no Microsoft exceptions
        environment.Append(CXXFLAGS='/GF') # String pooling in debug and release
        #environment.Append(CXXFLAGS='/Gv') # Vectorcall for speed
        environment.Append(CXXFLAGS='/utf-8') # Source code and outputs are UTF-8 encoded
        environment.Append(CXXFLAGS='/std:c++14') # Use a widely supported but current C++
        environment.Append(CXXFLAGS='/W4') # Show all warnings
        environment.Append(CXXFLAGS='/GS-') # No buffer security checks (we make games!)
        environment.Append(CXXFLAGS='/GR') # Generate RTTI for dynamic_cast and type_info

        environment.Append(CFLAGS='/GF') # String pooling in debug and release
        #environment.Append(CFLAGS='/Gv') # Vectorcall for speed
        environment.Append(CFLAGS='/utf-8') # Source code and outputs are UTF-8 encoded
        environment.Append(CFLAGS='/W4') # Show all warnings
        environment.Append(CFLAGS='/GS-') # No buffer security checks (we make games!)

        if _is_debug_build(environment):
            environment.Append(CXXFLAGS='/Od') # No optimization for debugging
            environment.Append(CXXFLAGS='/MDd') # Link shared multithreaded debug runtime
            environment.Append(CXXFLAGS='/Zi') # Generate complete debugging information
            environment.Append(CXXFLAGS='/FS') # Support shared writing to the PDB file

            environment.Append(CFLAGS='/Od') # No optimization for debugging
            environment.Append(CFLAGS='/MDd') # Link shared multithreaded debug runtime
            environment.Append(CFLAGS='/Zi') # Generate complete debugging information
            environment.Append(CFLAGS='/FS') # Support shared writing to the PDB file
        else:
            environment.Append(CXXFLAGS='/O2') # Optimize for speed
            environment.Append(CXXFLAGS='/Gy') # Function-level linking for better trimming
            environment.Append(CXXFLAGS='/GL') # Whole program optimizaton (merged build)
            environment.Append(CXXFLAGS='/MD') # Link shared multithreaded release runtime
            environment.Append(CXXFLAGS='/Gw') # Enable whole-program *data* optimization

            environment.Append(CFLAGS='/O2') # Optimize for speed
            environment.Append(CFLAGS='/Gy') # Function-level linking for better trimming
            environment.Append(CFLAGS='/GL') # Whole program optimizaton (merged build)
            environment.Append(CFLAGS='/MD') # Link shared multithreaded release runtime
            environment.Append(CFLAGS='/Gw') # Enable whole-program *data* optimization

    else:
        environment.Append(CXXFLAGS='-std=c++14') # Use a widely supported but current C++
        environment.Append(CXXFLAGS='-fvisibility=hidden') # Default visibility: don't export
        environment.Append(CXXFLAGS='-Wpedantic') # Enable all ISO C++ deviation warnings
        environment.Append(CXXFLAGS='-Wall') # Show all warnings
        environment.Append(CXXFLAGS='-Wno-unknown-pragmas') # Don't warn about #pragma region
        #environment.Append(CXXFLAGS=['-flinker-output=pie']) # Position-independent executable
        environment.Append(CXXFLAGS='-shared-libgcc') # Show all warnings

        environment.Append(CFLAGS='-fvisibility=hidden') # Default visibility: don't export
        environment.Append(CFLAGS='-Wpedantic') # Enable all ISO C++ deviation warnings
        environment.Append(CFLAGS='-Wall') # Show all warnings
        environment.Append(CFLAGS='-Wno-unknown-pragmas') # Don't warn about #pragma region
        #environment.Append(CFLAGS=['-flinker-output=pie']) # Position-independent executable
        environment.Append(CFLAGS='-shared-libgcc') # Show all warnings

        if _is_debug_build(environment):
            environment.Append(CXXFLAGS='-Og') # Tailor code for optimal debugging
            environment.Append(CXXFLAGS='-g') # Generate debugging information

            environment.Append(CFLAGS='-Og') # Tailor code for optimal debugging
            environment.Append(CFLAGS='-g') # Generate debugging information
        else:
            environment.Append(CXXFLAGS='-O3') # Optimize for speed
            environment.Append(CXXFLAGS='-flto') # Merge all code before compiling

            environment.Append(CFLAGS='-O3') # Optimize for speed
            environment.Append(CFLAGS='-flto') # Merge all code before compiling

# ----------------------------------------------------------------------------------------------- #

def _set_standard_cplusplus_linker_flags(environment):
    """Sets up standard flags for the linker

    @param  environment  Environment in which the C++ compiler linker wlll be set."""

    if platform.system() == 'Windows':
        if _is_debug_build(environment):
            pass
        else:
            environment.Append(LINKFLAGS='/LTCG') # Merge all code before compiling
            environment.Append(LIBFLAGS='/LTCG') # Merge all code before compiling

    else:
        environment.Append(LINKFLAGS='-z defs') # Detect unresolved symbols in shared object
        environment.Append(LINKFLAGS='-Bsymbolic') # Prevent replacement on shared object syms

# ----------------------------------------------------------------------------------------------- #

def _build_scons(environment, source, arguments, target):
    """Builds another SCons script.

    @param  environment  Environment that will be used to search for the SCons executable
    @param  source       Input file(s) for the build
    @param  arguments    Arguments that will be passed to SCons
    @param  target       Output file(s) produced by the build
    @returns A scons build action producing the target file"""

    # Clone the environment and use the real search PATH. This will not pollute
    # the environment in which the SCons subprocess runs, but is the only way
    # to invoke SCons in Windows because it's a batch file that exepcts Python
    # to also be in the system search PATH.
    cloned_environment = environment.Clone(ENV=os.environ)

    # Try to locate SCons with the environment's new search path
    scons_path = cloned_environment.WhereIs('scons')

    # If not found, try with SCons.Util.WhereIs() which actually works differently
    # from environment.WhereIs()...
    if scons_path is None:
        scons_path = WhereIs('scons')

    # Still not found? Just blindly shout 'scons' and if it fails that will
    # at least produce a meaningful error message that might make the user add
    # SCons to the system search PATH.
    if scons_path is None:
        scons_path = 'scons'

    if platform.system() == 'Windows':
        return cloned_environment.Command(
            source = source,
            action = '"' + scons_path + '" ' + arguments,
            target = target
        )
    else:
        return cloned_environment.Command(
            source = source,
            action = scons_path + ' ' + arguments,
            target = target
        )

# ----------------------------------------------------------------------------------------------- #

def _add_cplusplus_package(environment, universal_package_name, universal_library_names = None):
    """Adds a precompiled package consisting of some header files and a code library
    to the current build.

    @param  environment              Environment to which a package will be added
    @param  universal_package_name   Name of the package that will be added to the build
    @param  universal_library_names  Names of libraries (inside the package) that need to
                                     be linked.
    @remarks
        If no universal_library_names are given, a library with the same name as
        the package is assumed. The universal_library_name can be used if a package
        offers multiple linkable library (i.e. boost modules, gtest + gtest_main)"""

    references_directory = os.path.join('..', environment['REFERENCES_DIRECTORY'])
    package_directory = os.path.join(references_directory, universal_package_name)

    # Path for the package's headers
    include_directory = cplusplus.find_or_guess_include_directory(package_directory)
    if include_directory is None:
        raise FileNotFoundError('Could not find include directory for package')

    environment.add_include_directory(include_directory)

    # Path for the package's libraries
    library_directory = cplusplus.find_or_guess_library_directory(environment, package_directory)
    if library_directory is None:
        raise FileNotFoundError('Could not find library directory for package')

    environment.add_library_directory(library_directory)

    # Library that needs to be linked
    if universal_library_names is None:
        environment.add_library(universal_package_name)
    elif isinstance(universal_library_names, list):
        for universal_library_name in universal_library_names:
            environment.add_library(universal_library_name)
    else:
        environment.add_library(universal_library_names)

# ----------------------------------------------------------------------------------------------- #

def _build_cplusplus_library(
  environment, universal_library_name, static = False, sources = None
):
    """Creates a shared C/C++ library

    @param  environment             Environment controlling the build settings
    @param  universal_library_name  Name of the library in universal format
                                    (i.e. 'My.Awesome.Stuff')
    @param  static                  Whether to build a static library (default: no)
    @param  sources                 Source files to use (None = auto)
    @remarks
        Assumes the default conventions, i.e. all source code is contained in a directory
        named 'Source' and all headers in a directory named 'Include'.

        See get_platform_specific_library_name() for how the universal_library_name parameter
        is used to produce the output filename on different platforms."""

    environment = environment.Clone()

    # Include directories
    # These will automatically be scanned by SCons for changes
    environment.add_include_directory(environment['HEADER_DIRECTORY'])

    # Recursively search for the source code files or transform the existing file list
    sources = _add_variantdir_and_enumerate_cplusplus_sources(
        environment, environment['SOURCE_DIRECTORY'], sources
    )
    library_path = _put_in_intermediate_path(
        environment, cplusplus.get_platform_specific_library_name(universal_library_name, static)
    )

    if platform.system() == 'Windows':
        if _is_debug_build(environment):
            pdb_file_path = os.path.splitext(library_path)[0] + '.pdb'
            pdb_file_absolute_path = environment.File(pdb_file_path).srcnode().abspath
            environment.Append(CXXFLAGS='/Fd"' + pdb_file_absolute_path + '"')
            environment.Append(CFLAGS='/Fd"' + pdb_file_absolute_path + '"')
    else:
        environment.Append(CXXFLAGS='-fpic') # Use position-independent code
        environment.Append(CFLAGS='-fpic') # Use position-independent code

    # Build either a static or a shared library
    build_library = None
    if static:
        build_library = environment.StaticLibrary(library_path, sources)
    else:
        build_library = environment.SharedLibrary(library_path, sources)

    # If we're on Windows, a side effect of building a library in debug mode is
    # that a PDB file will be generated. Deal with that.
    if (platform.system() == 'Windows') and _is_debug_build(environment):
        build_debug_database = environment.SideEffect(pdb_file_absolute_path, build_library)
        return build_library + build_debug_database
    else:
        return build_library

# ----------------------------------------------------------------------------------------------- #

def _build_cplusplus_executable(
    environment, universal_executable_name, console = False, sources = None
):
    """Creates a vanilla C/C++ executable

    @param  environment                Environment controlling the build settings
    @param  universal_executable_name  Name of the executable in universal format
                                       (i.e. 'My.Awesome.App')
    @param  console                    Whether to build a shell/command line executable
    @param  sources                    Source files to use (None = auto)
    @remarks
        Assumes the default conventions, i.e. all source code is contained in a directory
        named 'Source' and all headers in a directory named 'Include'.

        See get_platform_specific_executable_name() for how the universal_library_name
        parameter is used to produce the output filename on different platforms."""

    environment = environment.Clone()

    # Include directories
    # These will automatically be scanned by SCons for changes
    environment.add_include_directory(environment['HEADER_DIRECTORY'])

    # Recursively search for the source code files or transform the existing file list
    sources = _add_variantdir_and_enumerate_cplusplus_sources(
        environment, environment['SOURCE_DIRECTORY'], sources
    )
    executable_path = _put_in_intermediate_path(
        environment, cplusplus.get_platform_specific_executable_name(universal_executable_name)
    )

    # On Windows, there is a distinguishment between console (shell) applications
    # and GUI applications. Add the appropriate flag if needed.
    if platform.system() == 'Windows':
        if console:
            environment.Append(LINKFLAGS='/SUBSYSTEM:CONSOLE')

        if _is_debug_build(environment):
            pdb_file_path = os.path.splitext(executable_path)[0] + '.pdb'
            pdb_file_absolute_path = environment.File(pdb_file_path).srcnode().abspath
            environment.Append(CXXFLAGS='/Fd"' + pdb_file_absolute_path + '"')
            environment.Append(CFLAGS='/Fd"' + pdb_file_absolute_path + '"')
    else:
        environment.Append(CXXFLAGS='-fpic') # Use position-independent code
        environment.Append(CXXFLAGS='-fpie') # Use position-independent code
        environment.Append(CFLAGS='-fpic') # Use position-independent code
        environment.Append(CFLAGS='-fpie') # Use position-independent code

    # Build the executable
    build_executable = environment.Program(executable_path, sources)
    if (platform.system() == 'Windows') and _is_debug_build(environment):
        build_debug_database = environment.SideEffect(pdb_file_absolute_path, build_executable)
        return build_executable + build_debug_database
    else:
        return build_executable

# ----------------------------------------------------------------------------------------------- #

def _build_cplusplus_library_with_tests(
    environment, universal_library_name, universal_test_executable_name,
    sources = None, test_sources = None
):
    """Creates a C/C++ shared library and also builds a unit test executable or it

    @param  environment                Environment controlling the build settings
    @param  universal_executable_name  Name of the library in universal format
                                       (i.e. 'My.Awesome.Stuff')
    @param  sources                    Source files to use (None = auto)
    @param  test_sources               Source files to use for the unit tests (None = auto)
    @remarks
        Assumes the default conventions, i.e. all source code is contained in a directory
        named 'Source' and all headers in a directory named 'Include'.

        In addition to the convenience factor, this method also avoids SCons warnings about
        two environments producing the same intermediate files (or having to compile everything
        twice) by first building a static library, then producing a shared library from it
        and producing the unit test executabel from it together with the unit test sources.

        See get_platform_specific_executable_name() for how the universal_library_name
        parameter is used to produce the output filename on different platforms."""

    # Recursively search for the source code files or transform the existing file list
    sources = _add_variantdir_and_enumerate_cplusplus_sources(
        environment, environment['SOURCE_DIRECTORY'], sources
    )
    test_sources = _add_variantdir_and_enumerate_cplusplus_sources(
        environment, environment['TESTS_DIRECTORY'], test_sources
    )

    intermediate_directory = os.path.join(
        environment['INTERMEDIATE_DIRECTORY'],
        environment.get_build_directory_name()
    )

    base_directory = environment.Dir('.').abspath

    # Build a static library that we can reuse for the shared library and test executable
    if True:
        intermediate_library_name = cplusplus.get_platform_specific_library_name(
            universal_library_name + ".Static", static = True
        )
        intermediate_library_path = _put_in_intermediate_path(
            environment, intermediate_library_name
        )

        staticlib_environment = environment.Clone();
        staticlib_environment.add_include_directory(environment['HEADER_DIRECTORY'])
        #staticlib_environment['PDB'] = os.path.splitext(intermediate_library_path)[0] + '.pdb"'

        if platform.system() == 'Windows':
            if _is_debug_build(environment):
                pdb_file_path = os.path.splitext(intermediate_library_path)[0] + '.pdb'
                pdb_file_absolute_path = environment.File(pdb_file_path).srcnode().abspath
                staticlib_environment.Append(CXXFLAGS='/Fd"' + pdb_file_absolute_path + '"')
                staticlib_environment.Append(CFLAGS='/Fd"' + pdb_file_absolute_path + '"')
        else:
            staticlib_environment.Append(CXXFLAGS='-fpic') # Use position-independent code
            staticlib_environment.Append(CFLAGS='-fpic') # Use position-independent code

        compile_static_library = staticlib_environment.StaticLibrary(
            intermediate_library_path, sources
        )
        if (platform.system() == 'Windows') and _is_debug_build(environment):
            staticlib_environment.SideEffect(pdb_file_absolute_path, compile_static_library)

    # Build a shared library using nothing but the static library for sources
    if True:
        sources = [] # We don't use any sources but the static library from above

        library_name = cplusplus.get_platform_specific_library_name(universal_library_name)
        library_path = _put_in_intermediate_path(environment, library_name)

        sharedlib_environment = environment.Clone();
        sharedlib_environment.add_include_directory(environment['HEADER_DIRECTORY'])
        #sharedlib_environment['PDB'] = os.path.splitext(library_path)[0] + '.pdb"'

        sharedlib_environment.add_library_directory(intermediate_directory)
        sharedlib_environment.add_library(intermediate_library_name)

        if platform.system() == 'Windows':
            if _is_debug_build(environment):
                pdb_file_path = os.path.splitext(library_path)[0] + '.pdb'
                pdb_file_absolute_path = environment.File(pdb_file_path).srcnode().abspath
                sharedlib_environment.Append(CXXFLAGS='/Fd"' + pdb_file_absolute_path + '"')
                sharedlib_environment.Append(CFLAGS='/Fd"' + pdb_file_absolute_path + '"')

            dummy_path = _put_in_intermediate_path(environment, 'msvc-dllmain-dummy.cpp')
            sources.append(dummy_path)
            create_dummy_file = sharedlib_environment.Command(
                source = [], action = 'echo // > $TARGET', target = dummy_path
            )

            compile_shared_library = sharedlib_environment.SharedLibrary(library_path, sources)
            sharedlib_environment.Depends(compile_shared_library, create_dummy_file)

            # On Windows, a .PDB file is produced when doing a debug build
            if _is_debug_build(environment):
                build_debug_database = environment.SideEffect(
                    pdb_file_absolute_path, compile_shared_library
                )

        else:
            sharedlib_environment.Append(CXXFLAGS='-fpic') # Use position-independent code
            sharedlib_environment.Append(CFLAGS='-fpic') # Use position-independent code
            compile_shared_library = sharedlib_environment.SharedLibrary(library_path, sources)

    if True:
        executable_name = cplusplus.get_platform_specific_executable_name(
            universal_test_executable_name
        )
        executable_path = _put_in_intermediate_path(environment, executable_name)

        executable_environment = environment.Clone()
        executable_environment.add_include_directory(environment['HEADER_DIRECTORY'])
        #executable_environment['PDB'] = os.path.splitext(executable_path)[0] + '.pdb"'

        executable_environment.add_library_directory(intermediate_directory)
        executable_environment.add_library(intermediate_library_name)

        executable_environment.add_package('gtest', [ 'gtest', 'gtest_main' ])

        if platform.system() == 'Windows':
            executable_environment.Append(LINKFLAGS="/SUBSYSTEM:CONSOLE")

            if _is_debug_build(environment):
                pdb_file_path = os.path.splitext(executable_path)[0] + '.pdb'
                pdb_file_absolute_path = environment.File(pdb_file_path).srcnode().abspath
                executable_environment.Append(CXXFLAGS='/Fd"' + pdb_file_absolute_path + '"')
                executable_environment.Append(CFLAGS='/Fd"' + pdb_file_absolute_path + '"')
                # os.path.join(base_directory, os.path.splitext(executable_path)[0] + '.pdb'

            compile_unit_tests = executable_environment.Program(executable_path, test_sources)

            # On Windows, a .PDB file is produced when doing a debug build
            if _is_debug_build(environment):
                build_test_debug_database = environment.SideEffect(
                    pdb_file_absolute_path, compile_unit_tests
                )

        else: # Default path: everything but Windows
            executable_environment.add_library('pthread') # Needed by googletest
            executable_environment.Append(CXXFLAGS='-fpic') # Use position-independent code
            executable_environment.Append(CXXFLAGS='-fpie') # Use position-independent code
            executable_environment.Append(CFLAGS='-fpic') # Use position-independent code
            executable_environment.Append(CFLAGS='-fpie') # Use position-independent code

            compile_unit_tests = executable_environment.Program(executable_path, test_sources)

    environment.Depends(compile_shared_library, compile_static_library)
    environment.Depends(compile_unit_tests, compile_static_library)

    if (platform.system() == 'Windows') and _is_debug_build(environment):
        return (
            compile_shared_library + build_debug_database +
            compile_unit_tests + build_test_debug_database
        )
    else:
        return compile_shared_library + compile_unit_tests

# ----------------------------------------------------------------------------------------------- #

def _run_cplusplus_unit_tests(environment, universal_test_executable_name):
    """Runs the unit tests executable comiled from a build_unit_test_executable() call

    @param  environment                     Environment used to locate the unit test executable
    @param  universal_test_executable_name  Name of the unit test executable from the build step
    @remarks
        This executes the unit test executable and produces an XML file detailing
        the test results for CI servers and other processing."""

    environment = environment.Clone()

    # Figure out the path the unit tests executable would have been compiled to
    test_executable_name = cplusplus.get_platform_specific_executable_name(
        universal_test_executable_name
    )
    test_executable_path = _put_in_intermediate_path(
        environment, test_executable_name
    )

    test_results_path = _put_in_artifact_path(
        environment, environment['TESTS_RESULT_FILE']
    )

    return environment.Command(
        source = test_executable_path,
        action = '-$SOURCE --gtest_output=xml:$TARGET',
        target = test_results_path
    )

# ----------------------------------------------------------------------------------------------- #

def _build_msbuild_project(environment, msbuild_project_path):
    """Builds an MSBuild project

    @param  environment           Environment the MSBuild project will be compiled in
    @param  msbuild_project_path  Path to the MSBuild project file that will be built"""

    msbuild_project_file = environment.File(msbuild_project_path)
    dotnet_version_tag = dotnet.detect_msbuild_target_framework(msbuild_project_file)

    build_directory_name = environment.get_build_directory_name(dotnet_version_tag)

    intermediate_build_directory = os.path.join(
        environment['INTERMEDIATE_DIRECTORY'], build_directory_name
    )

    return environment.MSBuild(
        msbuild_project_file.srcnode().abspath,
        intermediate_build_directory
    )

# ----------------------------------------------------------------------------------------------- #

def _build_msbuild_project_with_tests(
    environment, msbuild_project_path, tests_msbuild_project_path
):
    """Builds an MSBuild project and its associated unit test project

    @param  environment                 Environment the MSBuild project will be compiled in
    @param  msbuild_project_path        Path to the MSBuild project file that will be built
    @param  tests_msbuild_project_path  Path of the MSBuild project for the unit tests"""

    build_main = _build_msbuild_project(environment, msbuild_project_path)
    build_tests = _build_msbuild_project(environment, tests_msbuild_project_path)

    return build_main + build_tests

# ----------------------------------------------------------------------------------------------- #

def _add_variantdir_and_enumerate_cplusplus_sources(environment, directory, sources = None):
    """Sets up a variant directory for a set of sources and enumerates the sources
    with their paths when compiled to the variant directory.

    @param  environment  Environment the variant directory will be set up in
    @param  directory    Directory containing the sources that will be enumerated
    @param  sources      User-provided list of source files to transform instead
                         If this is 'None', the directory will be enumerated.
    @returns The list of source files in their virtual variant dir locations"""

    # Append the build directory. This directory is unique per build setup,
    # so that debug/release and x86/amd64 builds can life side by side or happen
    # in parallel.
    intermediate_build_directory = os.path.join(
        environment['INTERMEDIATE_DIRECTORY'],
        environment.get_build_directory_name()
    )
    variant_directory = os.path.join(intermediate_build_directory, directory)

    # Set up the variant directory so that object files get stored separately
    environment.VariantDir(variant_directory, directory, duplicate = 0)

    if sources is None:

        # Recursively search for the source code files
        return cplusplus.enumerate_sources(directory, intermediate_build_directory)

    else:

        new_sources = []

        for file_path in sources:
            new_sources.append(
                os.path.join(intermediate_build_directory, file_path)
            )

        return new_sources

# ----------------------------------------------------------------------------------------------- #

def _put_in_intermediate_path(environment, filename):
    """Determines the intermediate path for a file with the specified name

    @param  environment  Environment for which the intermediate path will be determined
    @param  filename     Filename for which the intermediate path will be returned
    @returns The intermediate path for a file with the specified name"""

    intermediate_directory = os.path.join(
        environment['INTERMEDIATE_DIRECTORY'],
        environment.get_build_directory_name()
    )

    return os.path.join(intermediate_directory, filename)

# ----------------------------------------------------------------------------------------------- #

def _put_in_artifact_path(environment, filename):
    """Determines the artifact path for a file with the specified name

    @param  environment  Environment for which the artifact path will be determined
    @param  filename     Filename for which the artifact path will be returned
    @returns The artifact path for a file with the specified name"""

    artifact_directory = os.path.join(
        environment['ARTIFACT_DIRECTORY'],
        environment.get_build_directory_name()
    )

    return os.path.join(artifact_directory, filename)
