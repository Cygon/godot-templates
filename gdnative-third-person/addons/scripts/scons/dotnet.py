#!/usr/bin/env python

import os
import shutil
import platform
import xml.etree.ElementTree as ET
from SCons.Script import Scanner

# ----------------------------------------------------------------------------------------------- #

# Paths in which the 32 bit version of MSBuild can be found on Windows systems
_windows_x86_msbuild_paths = {
    '2.0': [
        '%WinDir%\\Microsoft.NET\\Framework\\v2.0.50727\\MSBuild.exe'
    ],
    '3.5': [
        '%WinDir%\\Microsoft.NET\\Framework\\v3.5\\MSBuild.exe'
    ],
    '4.0': [
        '%WinDir%\\Microsoft.NET\\Framework\\v4.0.30319\\MSBuild.exe'
    ],
    'latest': [
        '%ProgramFiles(x86)%\\Microsoft Visual Studio\\2017\\Community\\MSBuild\\15.0\\Bin\\MSBuild.exe',
        '%ProgramFiles(x86)%\\Microsoft Visual Studio\\2017\\Professional\\MSBuild\\15.0\\Bin\\MSBuild.exe',
        '%ProgramFiles(x86)%\\Microsoft Visual Studio\\2017\\Enterprise\\MSBuild\\15.0\\Bin\\MSBuild.exe',
        'C:\\Program Files (x86)\\MSBuild\\14.0\\Bin\\MSBuild.exe'
    ]
}

# Paths in which the 64 bit version of MSBuild can be found on Windows systems
_windows_amd64_msbuild_paths = {
    '2.0': [
        '%WinDir%\\Microsoft.NET\\Framework64\\v2.0.50727\\MSBuild.exe'
    ],
    '3.5': [
        '%WinDir%\\Microsoft.NET\\Framework64\\v3.5\\MSBuild.exe'
    ],
    '4.0': [
        '%WinDir%\\Microsoft.NET\\Framework64\\v4.0.30319\\MSBuild.exe'
    ],
    'latest': [
        '%ProgramFiles(x86)%\\Microsoft Visual Studio\\2017\\Community\\MSBuild\\15.0\\Bin\\amd64\\MSBuild.exe',
        '%ProgramFiles(x86)%\\Microsoft Visual Studio\\2017\\Professional\\MSBuild\\15.0\\Bin\\amd64\\MSBuild.exe',
        '%ProgramFiles(x86)%\\Microsoft Visual Studio\\2017\\Enterprise\\MSBuild\\15.0\\Bin\\amd64\\MSBuild.exe',
        'C:\\Program Files (x86)\\MSBuild\\14.0\\Bin\\amd64\\MSBuild.exe'
    ]
}

# Possible paths the Visual Studio environment setup batch file can be found in
_windows_visual_studio_batch_paths = [
  '%ProgramFiles(x86)%\\Microsoft Visual Studio\\2017\\Community\\VC\\Auxiliary\\Build\\vcvars32.bat'
]

# Default version of MSBuild we will use
_default_msbuild_version = 'system'

# ----------------------------------------------------------------------------------------------- #

def setup(environment):
    """Sets the .NET build system up for the specified environment

    @param  environment  Environment in which the .NET build system will be set up"""

    msbuild_scanner = Scanner(
        function = _scan_msbuild_project,
        skeys = [ '.csproj', '.vcxproj', 'proj' ]
    )
    environment.Append(SCANNERS = msbuild_scanner)

    #environment.AddMethod(_compile_cli_application, "CliProgram")
    #environment.AddMethod(_compile_cli_library, "CliLibrary")
    environment.AddMethod(_call_msbuild, "MSBuild")
    environment.AddMethod(_get_build_directory_name, "get_build_directory_name")

# ----------------------------------------------------------------------------------------------- #

def enumerate_sources(source_directory, variant_directory = None):
    """Forms a list of all .NET source code files in a source directory

    @param  source_directory     Directory containing the C/C++ source code files
    @param  variant_directory    Variant directory to which source paths will be rewritten"""

    source_file_extensions = [
        '.cs',
        '.vb'
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

def _find_msbuild_executable(environment, msbuild_version):
    """Locates a suitable MSBuild executable on the current system.

    @param  environment      Environment that is used to look for MSBuild
    @param  msbuild_version  Version number of MSBuild that is requested (i.e. 'latest')
    @returns The absolute path of a MSBuild executable."""

    # If we're asked to find *any* MSBuild version, let's start by seeing if it is already
    # in the path. This is the case for Linux systems and may be the case for Windows
    # systems if the user has manually added the Visual Studio environment variables to
    # his standard environment setup (which is not unusual for CI build slaves)
    if (msbuild_version == 'latest') or (msbuild_version == 'system'):

        # Prefer MSBuild (as of 2018, Mono xbuild is being replaced by Open Sourced msbuild)
        msbuild = environment.WhereIs('msbuild')
        if msbuild:
            return msbuild

        # But if we can't get MSBuild, fall back to xbuild (only available on Mono systems)
        xbuild = environment.WhereIs('xbuild')
        if xbuild:
            return xbuild

    if platform.system() == 'Windows':

        # Only do a scan if we're asked for the latest version as we can't guarantee
        # what we'll find and figuring out which version we have is a problem of its own
        if (msbuild_version == 'latest') or (msbuild_version == 'system'):
            msbuild_directories = _find_msbuild_directories()
            for msbuild_directory in msbuild_directories:
                for version in os.listdir(msbuild_directory):
                    version_directory = os.path.join(msbuild_directory, version)
                    if os.path.isdir(version_directory):

                        executable_path = os.path.join(version_directory, 'Bin\\amd64\\MSBuild.exe')
                        if os.path.isfile(executable_path):
                            return executable_path

                        executable_path = os.path.join(version_directory, 'amd64\\MSBuild.exe')
                        if os.path.isfile(executable_path):
                            return executable_path

                        executable_path = os.path.join(version_directory, 'Bin\\MSBuild.exe')
                        if os.path.isfile(executable_path):
                            return executable_path

                        executable_path = os.path.join(version_directory, 'MSBuild.exe')
                        if os.path.isfile(executable_path):
                            return executable_path

        # If a specific msbuild version was requested, look for it in its known path
        for candidate in _windows_amd64_msbuild_paths[msbuild_version]:
            if os.path.isfile(candidate):
                return file

    else:

        # Linux systems where msbuild is not in the executable paths (we checked them above).
        # Try msbuild and xbuild in their standard locations, otherwise give up.
        # (we could look for Mono in /opt if we're hardcore...)
        if os.path.isfile('/usr/bin/msbuild'):
            return '/usr/bin/msbuild'
        elif os.path.isfile('/usr/bin/xbuild'):
            return '/usr/bin/xbuild'

    return None

# ----------------------------------------------------------------------------------------------- #

def _find_msbuild_directories():
    """Looks for MSBuild directories on the system

    @returns All MSBuild directories found on the system"""

    msbuild_directories = []

    # Look for directories containing 'Visual Studio' as candidates for
    # to use as the Visual Studio install directory
    visual_studio_directories = []

    # Visual Studio is still an x86 application, so search only here
    program_files_directory = os.environ['ProgramFiles(x86)']
    for directory in os.listdir(program_files_directory):
        if 'visual studio' in directory.lower():
            visual_studio_directories.append(os.path.join(program_files_directory, directory))
        if directory.lower() == 'msbuild':
            msbuild_directories.append(os.path.join(program_files_directory, directory))

    # Up until Visual Studio 2015, MSBuild was in its own directory
    program_files_directory = os.environ['ProgramFiles']
    for directory in os.listdir(program_files_directory):
        if directory.lower() == 'msbuild':
            msbuild_directories.append(os.path.join(program_files_directory, directory))

    # Look for a directory called 'MSBuild' exactly 3 levels deep
    # This looks a bit ugly, but we try to earl-out wherever possible and
    # a Visual Studio directory only has few files in these intermediate levels
    for directory in visual_studio_directories:
        for version in os.listdir(directory):
            version_directory = os.path.join(directory, version)
            if os.path.isdir(version_directory):
                if directory.lower() == 'msbuild':
                    msbuild_directories.add(version_directory)
                else:
                    for edition in os.listdir(version_directory):
                        edition_directory = os.path.join(version_directory, edition)
                        if os.path.isdir(edition_directory):
                            if edition.lower() == 'msbuild':
                                msbuild_direcories.add(edition_directory)
                            else:
                                for msbuild in os.listdir(edition_directory):
                                    msbuild_directory = os.path.join(edition_directory, msbuild)
                                    if os.path.isdir(msbuild_directory):
                                        if msbuild.lower() == 'msbuild':
                                            msbuild_directories.append(msbuild_directory)

    return msbuild_directories

# ----------------------------------------------------------------------------------------------- #

def _call_msbuild(environment, msbuild_project_path, output_directory):
    """Invokes MSBuild to compile an MSBuild-based project

    @param  environment           Environment on which MSBuild will be invoked
    @param  msbuild_project_path  Path to the MSBuild project (i.e. *.csproj, *.vcxproj)
    @param  output_directory      Direction into which the build output will be saved"""

    msbuild_version = _default_msbuild_version
    if 'MSBUILD_VERSION' in environment:
        msbuild_version = environment['MSBUILD_VERSION']

    msbuild_executable = _find_msbuild_executable(environment, msbuild_version)
    if msbuild_executable is None:
        raise FileNotFoundError('Could not find msbuild executable')

    output_directory_node = environment.Dir(output_directory)
    msbuild_project_file = environment.File(msbuild_project_path)

    # Extra arguments controlling the build
    absolute_output_directory = output_directory_node.srcnode().abspath
    absolute_msbuild_directory = os.path.dirname(msbuild_project_file.srcnode().abspath)
    relative_outdir = os.path.relpath(absolute_output_directory, absolute_msbuild_directory)
    extra_arguments = (
        ' /property:OutDir=' + os.path.join(relative_outdir, str())
        #' /p:OutputPath="' + output_path + '"'
    )

    # We don't do fancy build configurations, so it's either a debug build or
    # a release build. We need to translate this to MSBuild's "Configuration"
    is_debug_build = False
    if 'DEBUG' in environment:
        is_debug_build = environment['DEBUG']

    if is_debug_build:
        extra_arguments += ' /property:Configuration=Debug'
    else:
        extra_arguments += ' /property:Configuration=Release'

    # Target architecture.
    #if environment['TARGET_ARCH'] == 'any':
    #    extra_arguments += ' /property:Platform=AnyCPU'
    #else:
    #    extra_arguments += ' /property:Platform=' + environment['TARGET_ARCH']

    # Determine a list of (representative) output files. It would be awesome if
    # you could fully and accurately obtain the artifacts produced by MSBuild,
    # but it's the usual Microsoft mess, they can't even figure out their output
    # directory most of the time :-/
    outputs = []

    msbuild_project_file = environment.File(msbuild_project_path)
    output_filenames = _get_msbuild_output_filenames(msbuild_project_file)
    for output_filename in output_filenames:
        outputs.append(os.path.join(output_directory, output_filename))

    # Finally, invoke MSBuild, telling SCons about which files are its inputs
    # and which files will be produced to the best of our ability
    return environment.Command(
        source = msbuild_project_file,
        action = '"' + msbuild_executable + '" $SOURCE' + extra_arguments,
        target = outputs
    )

# ----------------------------------------------------------------------------------------------- #

def _scan_msbuild_project(node, environment, path):
    """Scans an MSBuild project for other files it is referencing. This is important
    for SCons to detect changes in files that are used within the build.

    @param  node         MSBuild project as a SCons File object
    @param  environment  Environment in which the project is being compiled
    @param  path         Who knows?"""

    sources = []

    xml_namespaces = {
        'msbuild': 'http://schemas.microsoft.com/developer/msbuild/2003'
    }

    contents = node.get_text_contents()
    project_node = ET.fromstring(contents)

    compile_nodes = project_node.findall('./msbuild:ItemGroup/msbuild:Compile', xml_namespaces)
    for compile_node in compile_nodes:
        windows_path = compile_node.attrib.get('Include')
        #print('Windows path: ' + windows_path)
        windows_path_elements = windows_path.split('\\')
        file_path = windows_path_elements[0]
        del windows_path_elements[0]
        for windows_path_element in windows_path_elements:
            file_path = os.path.join(file_path, windows_path_element)
        #print('Native path: ' + file_path)

        sources.append(file_path)

    return sources

# ----------------------------------------------------------------------------------------------- #

def _get_msbuild_output_filenames(node, is_debug_build = False):
    """Determines the output files generated when a MSBuild project is compiled.

    @param  node            File node for the msbuild project file
    @param  is_debug_build  Whether a debug build is being performed
    @returns A list of the output files generated when the project is built
    @remarks
        These outputs are not complete (the problem is far too complex for that),
        but sufficient for SCons to detect if a rebuild is required excluding cases
        where the user hand-deletes individual outputs.

        Only the file names are returned from this method as the build task
        will control the output directory via the OutDir property."""

    outputs = []

    xml_namespaces = {
        'msbuild': 'http://schemas.microsoft.com/developer/msbuild/2003'
    }

    contents = node.get_text_contents()
    project_node = ET.fromstring(contents)

    assembly_name = project_node.find(
        './msbuild:PropertyGroup/msbuild:AssemblyName', xml_namespaces
    )
    output_type = project_node.find(
        './msbuild:PropertyGroup/msbuild:OutputType', xml_namespaces
    )
    #documentation_file = project_node.find(
    #    './msbuild:PropertyGroup/msbuild:DocumentationFile', xml_namespaces
    #)

    # Main build output
    if output_type.text == 'Library':
        outputs.append(assembly_name.text + '.dll')
    elif (output_type.text == 'WinExe') or (output_type.text == 'Exe'):
        outputs.append(assembly_name.text + '.exe')

    # Debug database (contains informations to map code addresses to line numbers,
    # local variable names etc.)
    #if is_debug_build:
    #    outputs.append(assembly_name.text + '.pdb')

    #if not (documentation_file is None):
    #    outputs.append(os.path.basename(documentation_file.text))

    return outputs

# ----------------------------------------------------------------------------------------------- #

def detect_msbuild_target_framework(node):
    """Tries to figure out which .NET framework and version an msbuild project targets.

    @param  node  File node for the msbuild project file
    @returns The version tag of the .NET framework targeted by the msbuild project"""

    xml_namespaces = {
        'msbuild': 'http://schemas.microsoft.com/developer/msbuild/2003'
    }

    contents = node.get_text_contents()
    project_node = ET.fromstring(contents)

    target_framework_version = project_node.find(
        './msbuild:PropertyGroup/msbuild:TargetFrameworkVersion', xml_namespaces
    )
    if not (target_framework_version is None):
        # https://docs.microsoft.com/en-us/visualstudio/msbuild/msbuild-target-framework-and-target-platform?view=vs-2017
        if target_framework_version.text == 'v2.0':
            return 'net20'
        elif target_framework_version.text == 'v3.0':
            return 'net30'
        elif target_framework_version.text == 'v3.5':
            return 'net35'
        elif target_framework_version.text == 'v4.0':
            return 'net40'
        elif target_framework_version.text == 'v4.5.2':
            return 'net45'
        elif target_framework_version.text == 'v4.6':
            return 'net46'
        elif target_framework_version.text == 'v4.6.1':
            return 'net46'
        elif target_framework_version.text == 'v4.6.2':
            return 'net46'
        elif target_framework_version.text == 'v4.7':
            return 'net47'
        elif target_framework_version.text == 'v4.7.1':
            return 'net47'
        elif target_framework_version.text == 'v4.7.2': # Probably...
            return 'net47'

    target_framework = project_node.find(
        './msbuild:PropertyGroup/msbuild:TargetFramework', xml_namespaces
    )
    if not (target_framework is None):
        if target_framework.text == 'netstandard1.0':
            return 'netstandard10'
        elif target_framework.text == 'netstandard1.1':
            return 'netstandard11'
        elif target_framework.text == 'netstandard1.2':
            return 'netstandard12'
        elif target_framework.text == 'netstandard1.3':
            return 'netstandard13'
        elif target_framework.text == 'netstandard1.4':
            return 'netstandard14'
        elif target_framework.text == 'netstandard1.5':
            return 'netstandard15'
        elif target_framework.text == 'netstandard1.6':
            return 'netstandard16'
        elif target_framework.text == 'netstandard2.0':
            return 'netstandard20'
        elif target_framework.text == 'netstandard3.0': # Probably...
            return 'netstandard30'

    # No known target frameworks
    return None

# ----------------------------------------------------------------------------------------------- #

def _get_build_directory_name(environment, dotnet_version_tag = 'net40'):
    """Determines the name of the build directory with the current environment settings

    @param  environment         Environment to determine the build directory name from
    @param  dotnet_version_tag  Version tag of the .NET framework the project is targeting
    @returns A suitable and unique name for the selected build configuration and framework"""

    is_debug_build = False
    if 'DEBUG' in environment:
        is_debug_build = environment['DEBUG']

    if is_debug_build:
        configuration_name = 'debug'
    else:
        configuration_name = 'release'

    return (
        dotnet_version_tag + '-' +
        configuration_name
    )

#csccom = "$CSC $CSCFLAGS -out:${TARGET.abspath} $SOURCES"
#csclibcom = "$CSC -t:library $CSCLIBFLAGS $_CSCLIBPATH $_CSCLIBS -out:${TARGET.abspath} $SOURCES"
