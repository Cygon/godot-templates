#!/bin/sh

# Delete old copy if present
if [ -d "godot-cpp" ]; then
	rm -r "godot-cpp"
fi

# Make a new clone of godot-cpp
git clone \
	--recursive \
	--branch master \
	--single-branch \
	--depth 1 \
	https://github.com/GodotNativeTools/godot-cpp


# This is now done by the master SConstruct file in the project root

# Generate headers for the installed godot version
#/opt/godot-git/bin/godot.x11.opt.tools.64 \
#	--gdnative-generate-json-api godot-cpp/godot_headers/api.json
#
# Compile godot-cpp
#pushd .
#cd godot-cpp
#
#scons -j15 platform=linux bits=64 generate_bindings=yes
#
# This is now done by the main build script
#scons -j15 \
#	godotbinpath="/opt/godot-git/bin/godot.x11.opt.tools.64" \
#	platform=linux \
#	bits=64
#
#
#popd
