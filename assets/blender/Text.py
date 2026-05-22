import bpy

armature = bpy.data.armatures[0]
bpy.ops.object.mode_set(mode='EDIT')

for bone in armature.edit_bones:
    print(bone.name, bone.parent.name if bone.parent else "ROOT")

bpy.ops.object.mode_set(mode='OBJECT')