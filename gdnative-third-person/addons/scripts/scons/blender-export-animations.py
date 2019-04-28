#!/usr/bin/env python

# Purpose:
#   When dealing with larger animation projects such as characters for
#   video games or machinima, a lot of Actions (Blender animation clips)
#   will result. It is advantageous to store these in separate files.
#
#   Blender supports this scenario by allowing .blend files to "link"
#   in objects from each other. However, as of now, the FBX exporter
#   will not correctly export animation tracks in separate .blend files.
#
#   up to 2.72:   works if not using a proxy for the armature
#   2.73:         nothing works at all
#   2.74 to 2.77: only works with proxy but animations are detached
#   2.78:         only works with proxy, animations attached to wrong object
#
#   In short, until the rewrite of the proxy system (may happen in 2.8),
#   exporting animations on linked Armatures only works in Blender 2.72
#   and without proxies, but the old FBX exporter has its own problems.
#
#   The only viable solution, then, is to:
#     1) open the master file
#     2) append all animation tracks from a separate .blend file to it
#     3) export the master file
#     4) exit without saving
#
#   This script will do just that. Since exporting the master meshes
#   multiple times would be a giant waste of space, it only exports
#   a single mesh (for this purpose, this script creates a simple skinned
#   5 vertex pyramid on the fly that it exports - if you see a pyramid
#   in your FBX file, everything is working correctly!)
#
#   It is also possible to output Collada (.dae) files simply by specifying
#   the appropriate file extension. Usage of Collada is discouraged because
#   there are no two importers or exporters who interpret the format the same.
#
# Usage:
#   Invoke Blender like this:
#
#   blender master.blend --python animation-export.py -- \
#           target.fbx animations.blend wildcard1* wildcard2*
#
#   - The first argument (master.blend) is the .blend file containing
#     your character without animations
#
#   - The second argument (--python animation-export.py) runs this script
#
#   - The third argument (--) is a separator behind which the script
#     arguments begin. You can add other Blender arguments before this.
#
#   - The fourth argument (target.fbx) is the output file into which
#     the animations will be exported
#
#   - The fifth argument (animations.blend) specifies the .blend file
#     containing the animation tracks you wish to export
#
#   - The sixth and following arguments (wildcard*) are wildcard strings
#     specifying which animations to append (this is useful if you have
#     animations for multiple characters in animations.blend and want to
#     append only the animations for the character you're exporting).
#
#
# Conventions:
#   If you have multiple Armatures in your .blend file (very common if you
#   use Rigify and kept the metarig), turn off interaction on them to make
#   this script ignore them. Interaction can be turned off by disabling the
#   little mouse cursor-like symbol in the outliner.
#
import bpy
import sys
import fnmatch
import os

# ----------------------------------------------------------------------------------------------- #

def _main():
    """Performs the animations export in Blender"""

    print('Enabling required add-ons (better collada, fbx, rigify)')
    _enable_required_plugins()

    cwd = os.getcwd()
    print('Base path: \033[94m' + cwd + '\033[0m')

    argv = sys.argv
    argv = argv[argv.index("--") + 1:]  # get all args after "--"

    # Target path
    outpath = argv[0]
    print('Output path: \033[94m' + outpath + '\033[0m')

    # Animation library (.blend file) we want to append from
    animlib = argv[1]
    animlib = os.path.join(cwd, animlib)
    print('Animation library: \033[94m' + animlib + '\033[0m')

    # Wildcard of the actions we need to append
    action_masks = argv[2:]

    # Append the actions from the animation library
    _disable_all_actions()
    _append_actions_from_file(animlib, action_masks)

    # Select the first Armature and the export dummy mesh in the scene
    _make_all_layers_visible() # dummy objects might be on different layers
    _clear_selection() # don't rely on what was selected when the user saved
    armature = _select_first_armature() # we want to export the Armature

    _create_and_select_dummy_mesh(armature) # create dummy geometry to export
    _select_first_wildcard_action(armature, action_masks[0]);

    # Figure out which export format the user wants to use
    filename, file_extension = os.path.splitext(outpath)
    file_extension = file_extension.lower()

    # Export the scene
    if file_extension == ".fbx":
        _export_to_fbx(outpath)
    elif file_extension == ".dae":
        _export_to_collada(outpath)
    else:
        die("Only FBX and Collada (.dae) are supported at this time")

    print('\033[92mCompleted!\033[0m')

# ----------------------------------------------------------------------------------------------- #

def _enable_required_plugins(require_collada = True):
    """Enables the plugins required to export scenes

    @param  require_collada  Whether the "Better Collada" exporter should be enabled"""

    bpy.ops.wm.addon_enable(module='rigify') # Or are Rigify rigs self-contained after generate?
    bpy.ops.wm.addon_enable(module='io_scene_fbx')

    # The collada exporter should be built-in, but we use the "Better Collada" exporter
    if require_collada:
        bpy.ops.wm.addon_enable(module='io_scene_dae') # Better Collada

# ----------------------------------------------------------------------------------------------- #

def _disable_all_actions():
    """Disables all actions in the current scene from being exported

    @remarks
      This is required to prevent the FBX exporter from exporting everything
      and the kitchen sink (uses the fact that the FBX exporter skips actions
      with unmatchable curves by inserting invalid bones into unwanted curves)"""

    print('Disabling master rig actions')

    for action in bpy.data.actions:
        print('\t\033[94m' + action.name + '\033[0m')
        curve = action.fcurves.new(
            data_path="InvalidBoneToBlockActionFromFbxExport", index=0
        )

# ----------------------------------------------------------------------------------------------- #

def _append_actions_from_file(path, wildcards):
    """Appends all actions from another .blend file that match a given wildcard

    @param  path       Path of the blendfile from which animations will be appended
    @param  wildcards  Naems of the animations (wildcards allowed) that will be appended"""

    print('Merging actions from animation library:')
    actions = None

    # https://www.blender.org/api/blender_python_api_2_77_0/bpy.types.BlendDataLibraries.html
    with bpy.data.libraries.load(path) as (data_from, data_to):
        actions = [ name for name in data_from.actions if _matches_any_wildcard(name, wildcards) ]
        data_to.actions = actions

    # The FBX exporter doesn't care, but the Collada exporter steadfastly refuses to export
    # actions with zero users, so we need to add a fake user to export them
    for action in actions:
        print('\t\033[94m' + action.name + '\033[0m')
        action.use_fake_user = 1

    #for action in data_to.actions:

# ----------------------------------------------------------------------------------------------- #

def _matches_any_wildcard(name, wildcards):
    """Checks whether the specified name matches any of a series of wildcards

    @param  name       Name that will be checked against the wildcards
    @param  wildcards  Wildcards of which at least one has to match the name
    @returns True if at least one wildcard matched the name, false otherwise"""

    for wildcard in wildcards:
        if fnmatch.fnmatch(name, wildcard):
            return True

    return False

# ----------------------------------------------------------------------------------------------- #

def _make_all_layers_visible():
    """Makes all layers in the scene visible"""

    for i in range(len(bpy.context.scene.layers)):
        bpy.context.scene.layers[i] = True

# ----------------------------------------------------------------------------------------------- #

def _clear_selection():
    """Unselects all selected objects in the scene"""

    for ob in bpy.data.objects:
        ob.select = False

# ----------------------------------------------------------------------------------------------- #

def _select_first_armature():
    """Selects the first object of type Armature found in the scene"""

    for ob in bpy.data.objects:
        if ob.type == 'ARMATURE':
            if ob.hide_select == False:
                print('Selected Armature for exporting: \033[94m' + ob.name + '\033[0m')
                ob.select = True
                return ob

# ----------------------------------------------------------------------------------------------- #

def _export_to_fbx(outpath):
    """Exports the current Blender scene (with its assumedly carefully selected meshes
    and actions (= animation clips)) to an FBX file

    @param  outpath  Path under which the FBX file will be saved"""

    bpy.ops.export_scene.fbx(
        filepath=outpath,
        check_existing=False,
        axis_forward='-Z',
        axis_up='Y',
        version='BIN7400',
        use_selection=True,
        global_scale=1.0,
        apply_unit_scale=False,
        bake_space_transform=False,
        object_types={'ARMATURE', 'EMPTY', 'MESH'},
        use_mesh_modifiers=True,
        mesh_smooth_type='OFF',
        use_mesh_edges=False,
        use_tspace=True,
        use_custom_props=False,
        add_leaf_bones=False,
        primary_bone_axis='Y',
        secondary_bone_axis='X',
        use_armature_deform_only=True,
        bake_anim=True,
        bake_anim_use_all_bones=True,
        bake_anim_use_nla_strips=False,
        bake_anim_use_all_actions=True,
        bake_anim_force_startend_keying=True,
        bake_anim_step=1.0,
        bake_anim_simplify_factor=0.0,
        use_anim=True,
        use_anim_action_all=True,
        use_default_take=False,
        use_anim_optimize=False,
        path_mode='AUTO',
        embed_textures=False,
        batch_mode='OFF',
        use_metadata=True
    )

# ----------------------------------------------------------------------------------------------- #

def _export_to_collada(outpath):
    """Exports the current Blender scene (with its assumedly carefully selected meshes
    and actions (= animation clips)) to a Collada file

    @param  outpath  Path under which the Collada file will be saved"""

    bpy.ops.export_scene.dae(
        filepath=outpath,
        check_existing=False,
        use_export_selected=True,
        object_types={'ARMATURE', 'EMPTY', 'MESH'},
        use_mesh_modifiers=True,
        use_exclude_ctrl_bones=True,
        use_anim=True,
        use_anim_action_all=True,
        use_metadata=True,
        use_triangles=True
    )

# ----------------------------------------------------------------------------------------------- #

def _create_and_select_dummy_mesh(armature):
    """Creates a dummy mesh that is parented to the specified armature

    @param  armature  Armature to which the dummy mesh will be parented"""

    verts = [
        (-1.0, -1.0, 0.0),
        (+1.0, -1.0, 0.0),
        (+1.0, +1.0, 0.0),
        (-1.0, +1.0, 0.0),
        (0.0, 0.0, +1.0)
    ]
    faces = [
        (0, 1, 4),
        (1, 2, 4),
        (2, 3, 4),
        (3, 0, 4),
        (2, 1, 0),
        (0, 3, 2)
    ]
    mesh_data = bpy.data.meshes.new("AnimationDummy.Mesh")
    mesh_data.from_pydata(verts, [], faces)
    mesh_data.update()

    obj = bpy.data.objects.new("AnimationDummy", mesh_data)
    scene = bpy.context.scene
    scene.objects.link(obj)
    obj.parent = armature

    # The object needs to be select to be exported
    obj.select = True

    # Add the Armature modifier to the mesh
    mod = obj.modifiers.new('Armature', 'ARMATURE')
    mod.object = armature
    mod.use_bone_envelopes = False
    mod.use_vertex_groups = True

    # Create a vertex group matching the first bone of the Armature
    # (assumption: an Armature with less than 1 bone is impossible)
    # and assign all vertices of the mesh to this vertex group
    firstbone = armature.pose.bones[0]
    vgroup = obj.vertex_groups.new(firstbone.name)
    vgroup.add([0, 1, 2, 3, 4], 1.0, 'REPLACE')

# ----------------------------------------------------------------------------------------------- #

def _select_first_wildcard_action(armature, wildcard):
    """Selects the first action matching the specified wildcard into the Armature.

    @remarks
        The FBX exporter blindly exports the currently selected action, so we have to
        make sure the selected action is one of those we actually want to export."""

    action = next(action for action in bpy.data.actions if fnmatch.fnmatch(action.name, wildcard))
    armature.animation_data.action = action

# ----------------------------------------------------------------------------------------------- #

print(str())
print("blender-export-animations.py running...")
print('\033[95m===============================================================================\033[0m')

_main()

print('\033[95m===============================================================================\033[0m')
print(str())

# Quit. We do not want to risk keeping the window open,
# which might end up making the user save our patchwork file
#
quit()
