/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   MaterialScriptHighlighter.pp
 *  @brief  Syntax highlighter for Ogre material scripts.
 */


#include "StableHeaders.h"
#include "MaterialScriptHighlighter.h"

// As copied from OgreMaterialSerializer.cpp.
const char *cKeywords[] = {
    // Root attributes
    "material", "vertex_program", "geometry_program", "fragment_program",
    // Material attributes
    "lod_values", "lod_strategy", "lod_distances", "receive_shadows", "transparency_casts_shadows",
    "technique", "set_texture_alias",
    // Technique attributes
    "lod_index", "shadow_caster_material", "shadow_receiver_material", "scheme", "gpu_vendor_rule",
    "gpu_device_rule", "pass",
    // Pass attributes
    "ambient", "diffuse", "specular", "emissive", "scene_blend", "separate_scene_blend", "depth_check",
    "depth_write", "depth_func", "normalise_normals", "alpha_rejection", "alpha_to_coverage", "transparent_sorting",
    "colour_write", "light_scissor", "light_clip_planes", "cull_hardware", "cull_software", "lighting", "fog_override",
    "shading", "polygon_mode", "polygon_mode_overrideable", "depth_bias", "iteration_depth_bias", "texture_unit",
    "vertex_program_ref", "geometry_program_ref", "shadow_caster_vertex_program_ref", "shadow_receiver_vertex_program_ref",
    "shadow_receiver_fragment_program_ref", "fragment_program_ref", "max_lights", "start_light", "iteration", "point_size",
    "point_sprites", "point_size_attenuation", "point_size_min", "point_size_max", "illumination_stage",
    // Texture unit attributes
    "texture_source", "texture", "anim_texture", "cubic_texture", "binding_type", "tex_coord_set", "tex_address_mode",
    "tex_border_colour", "colour_op", "colour_op_ex", "colour_op_multipass_fallback", "alpha_op_ex", "env_map", "scroll",
    "scroll_anim", "rotate", "rotate_anim", "scale", "wave_xform", "transform", "filtering", "max_anisotropy", "texture_alias",
    "mipmap_bias", "content_type",
    // Program reference attributes
    "param_indexed", "param_indexed_auto", "param_named", "param_named_auto",
    // Program definition attributes
    "source", "syntax", "includes_skeletal_animation", "includes_morph_animation", "includes_pose_animation",
    "uses_vertex_texture_fetch", "default_params",
    // Program default param attributes
    "param_indexed", "param_indexed_auto", "param_named", "param_named_auto",
};

bool IsKeyword(const QString &text, const char **keywords, size_t numKeywords)
{
    for(size_t i = 0; i < numKeywords; ++i)
        if (text == keywords[i])
            return true;

    return false;
}

void MaterialScriptHighlighter::highlightBlock(const QString &text)
{
    int index = 0;
    foreach(const QString &word, text.split(' '))
    {
        index = text.indexOf(word, index);
        if (IsKeyword(word, cKeywords, NUMELEMS(cKeywords)))
            setFormat(index, word.length(), Qt::blue);
        //index = text.indexOf(expression, index + length);
    }
}
