/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreScriptHighlighter.pp
 *  @brief  Syntax highlighter for Ogre scripts.
 */


#include "StableHeaders.h"
#include "OgreScriptHighlighter.h"

// As copied from OgreMaterialSerializer.cpp.
const char *cMaterialKeywords[] = {
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

// As copied from http://www.ogre3d.org/tikiwiki/Syntax+highlighting+for+TextWrangler
const char *cParticleKeywords[] = {
    "particle_system", "quota", "material", "particle_width", "particle_height", "cull_each", "billboard_type",
    "billboard_origin", "billboard_rotation_type", "common_direction", "common_up_vector", "renderer", "sorted",
    "local_space", "point_rendering", "accurate_facing", "iteration_interval", "nonvisible_update_timeout", "emitter",
    "angle", "colour", "colour_range_start", "colour_range_end", "direction", "emission_rate", "position", "velocity",
    "velocity_min", "velocity_max", "time_to_live", "time_to_live_min", "time_to_live_max", "duration", "duration_min",
    "duration_max", "repeat_delay", "repeat_delay_min", "repeat_delay_max", "Point", "Box", "width", "height", "depth",
    "Cylinder", "Ellipsoid", "HollowEllipsoid", "inner_width", "inner_height", "inner_depth", "Ring", "inner_width",
    "inner_height", "name", "emit_emitter", "emit_emitter_quota", "affector", "LinearForce", "force_vector", "force_application",
    "ColourFader", "red", "green", "blue", "alpha", "ColourFader2", "red1", "green1", "blue1", "alpha1", "red2", "green2",
    "blue2", "alpha2", "state_change", "Scaler", "rate", "Rotator", "vrotation_speed_range_start", "rotation_speed_range_end",
    "rotation_range_start", "rotation_range_end", "ColourInterpolator", "time0", "colour0", "time1", "colour1", "time2", "colour2",
    "time3", "colour3", "time4", "colour4", "time5", "colour5", "ColourImage", "image", "DeflectorPlane", "plane_point", "plane_normal",
    "bounce", "DirectionRandomiser", "randomness", "scope", "keep_velocity"
};

bool IsKeyword(const QString &text, const char **keywords, size_t numKeywords)
{
    for(size_t i = 0; i < numKeywords; ++i)
        if (text == keywords[i])
            return true;

    return false;
}

void OgreScriptHighlighter::highlightBlock(const QString &text)
{
    int index = 0;
    foreach(const QString &word, text.split(' '))
    {
        index = text.indexOf(word, index);
        bool keyword = false;
        if (scriptType == "OgreMaterial")
            keyword = IsKeyword(word, cMaterialKeywords, NUMELEMS(cMaterialKeywords));
        else if (scriptType == "OgreParticle")
            keyword = IsKeyword(word, cParticleKeywords, NUMELEMS(cParticleKeywords));
        if (keyword)
            setFormat(index, word.length(), Qt::blue);
    }
}
