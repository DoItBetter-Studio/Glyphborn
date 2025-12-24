bl_info = {
    "name": "Glyphborn Skeleton Exporter",
    "author": "Tigga",
    "version": (0, 1),
    "blender": (4, 0, 0),
    "location": "File > Export > Glyphborn Skeleton (.gbsk)",
    "description": "Exports armature skeleton to Glyphborn .gbsk format",
    "category": "Import-Export",
}

import bpy
import struct

# --------------------------------------------
# Skeleton Export Logic
# --------------------------------------------

def export_skeleton(armature_obj, filepath):
    armature = armature_obj
    bones = armature.bones
    
    # Build bone name -> index table
    bone_indices = {}
    for i, bone in enumerate(bones):
        bone_indices[bone.name] = i
    
    with open(filepath, "wb") as f:
        
        # ---- Header ----
        f.write(b"GBSK")
        f.write(struct.pack("<H", 1))
        f.write(struct.pack("<H", len(bones)))
        
        # ---- Bone Data ----
        for bone in bones:
            
            # Parent Index
            if bone.parent:
                parent_index = bone_indices[bone.parent.name]
            else:
                parent_index = 0xFFFF   # No parent
            
            f.write(struct.pack("<H", parent_index))
            
            # Bone name (32 bytes, null-padded)
            name_bytes = bone.name.encode("utf-8")
            name_bytes = name_bytes[:31]
            name_bytes += b"\0" * (32 - len(name_bytes))
            f.write(name_bytes)
            
            # Bind pose transform
            mat = bone.matrix_local
            
            loc = mat.to_translation()
            rot = mat.to_quaternion()
            scl = mat.to_scale()
            
            # Write transform
            f.write(struct.pack("<3f", loc.x, loc.y, loc.z))
            f.write(struct.pack("<4f", rot.x, rot.y, rot.z, rot.w))
            f.write(struct.pack("<3f", scl.x, scl.y, scl.z))
    
    print("Glyphborn skeleton exported:", filepath)

class EXPORT_OT_glyphborn(bpy.types.Operator):
    bl_idname = "export_scene.gbsk"
    bl_label = "Export Glyphborn Model"
    
    filepath: bpy.props.StringProperty(
        subtype="FILE_PATH",
        default="skeleton.gbsk"
    )
    
    def execute(self, context):
        obj = context.active_object
        
        if obj is None or obj.type != "ARMATURE":
            self.report({'ERROR'}, "Select an Armature to export")
            return {'CANCELLED'}

        export_skeleton(obj, self.filepath)
        return {'FINISHED'}
        
    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}
    
def menu_func_export(self, context):
    self.layout.operator(
        EXPORT_OT_glyphborn.bl_idname,
        text="Glyphborn Skeleton (.gbsk)"
    )

def register():
    bpy.utils.register_class(EXPORT_OT_glyphborn)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)

def unregister():
    bpy.utils.unregister_class(EXPORT_OT_glyphborn)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)

if __name__ == "__main__":
    register()