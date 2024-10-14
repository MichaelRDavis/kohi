#pragma once

#include <containers/array.h>
#include <math/math_types.h>
#include <strings/kname.h>

#include "assets/kasset_types.h"
#include "identifiers/khandle.h"

/** @brief Pre-defined resource types. */
typedef enum kresource_type {
    /** @brief Unassigned resource type */
    KRESOURCE_TYPE_UNKNOWN,
    /** @brief Texture resource type. */
    KRESOURCE_TYPE_TEXTURE,
    /** @brief Material resource type. */
    KRESOURCE_TYPE_MATERIAL,
    /** @brief Shader resource type. */
    KRESOURCE_TYPE_SHADER,
    /** @brief Static Mesh resource type (collection of geometries). */
    KRESOURCE_TYPE_STATIC_MESH,
    /** @brief Skeletal Mesh resource type (collection of geometries). */
    KRESOURCE_TYPE_SKELETAL_MESH,
    /** @brief Bitmap font resource type. */
    KRESOURCE_TYPE_BITMAP_FONT,
    /** @brief System font resource type. */
    KRESOURCE_TYPE_SYSTEM_FONT,
    /** @brief Scene resource type. */
    KRESOURCE_TYPE_SCENE,
    /** @brief Heightmap-based terrain resource type. */
    KRESOURCE_TYPE_HEIGHTMAP_TERRAIN,
    /** @brief Voxel-based terrain resource type. */
    KRESOURCE_TYPE_VOXEL_TERRAIN,
    /** @brief Sound effect resource type. */
    KRESOURCE_TYPE_SOUND_EFFECT,
    /** @brief Music resource type. */
    KRESOURCE_TYPE_MUSIC,
    KRESOURCE_TYPE_COUNT,
    // Anything beyond 128 is user-defined types.
    KRESOURCE_KNOWN_TYPE_MAX = 128
} kresource_type;

/** @brief Indicates where a resource is in its lifecycle. */
typedef enum kresource_state {
    /**
     * @brief No load operations have happened whatsoever
     * for the resource.
     * The resource is NOT in a drawable state.
     */
    KRESOURCE_STATE_UNINITIALIZED,
    /**
     * @brief The CPU-side of the resources have been
     * loaded, but no GPU uploads have happened.
     * The resource is NOT in a drawable state.
     */
    KRESOURCE_STATE_INITIALIZED,
    /**
     * @brief The GPU-side of the resources are in the
     * process of being uploaded, but the upload is not yet complete.
     * The resource is NOT in a drawable state.
     */
    KRESOURCE_STATE_LOADING,
    /**
     * @brief The GPU-side of the resources are finished
     * with the process of being uploaded.
     * The resource IS in a drawable state.
     */
    KRESOURCE_STATE_LOADED
} kresource_state;

typedef struct kresource {
    kname name;
    kresource_type type;
    kresource_state state;
    u32 generation;

    /** @brief The number of tags. */
    u32 tag_count;

    /** @brief An array of tags. */
    kname* tags;
} kresource;

typedef struct kresource_asset_info {
    kname asset_name;
    kname package_name;
    kasset_type type;
} kresource_asset_info;

ARRAY_TYPE(kresource_asset_info);

typedef void (*PFN_resource_loaded_user_callback)(kresource* resource, void* listener);

typedef struct kresource_request_info {
    kresource_type type;
    // The list of assets to be loaded.
    array_kresource_asset_info assets;
    // The callback made whenever all listed assets are loaded.
    PFN_resource_loaded_user_callback user_callback;
    // Listener user data.
    void* listener_inst;
} kresource_request_info;

/**
 * @brief Represents various types of textures.
 */
typedef enum kresource_texture_type {
    /** @brief A standard two-dimensional texture. */
    KRESOURCE_TEXTURE_TYPE_2D,
    /** @brief A 2d array texture. */
    KRESOURCE_TEXTURE_TYPE_2D_ARRAY,
    /** @brief A cube texture, used for cubemaps. */
    KRESOURCE_TEXTURE_TYPE_CUBE,
    /** @brief A cube array texture, used for arrays of cubemaps. */
    KRESOURCE_TEXTURE_TYPE_CUBE_ARRAY,
    KRESOURCE_TEXTURE_TYPE_COUNT
} kresource_texture_type;

typedef enum kresource_texture_format {
    KRESOURCE_TEXTURE_FORMAT_UNKNOWN,
    KRESOURCE_TEXTURE_FORMAT_RGBA8,
    KRESOURCE_TEXTURE_FORMAT_RGB8,
} kresource_texture_format;

typedef enum kresource_texture_flag {
    /** @brief Indicates if the texture has transparency. */
    KRESOURCE_TEXTURE_FLAG_HAS_TRANSPARENCY = 0x01,
    /** @brief Indicates if the texture can be written (rendered) to. */
    KRESOURCE_TEXTURE_FLAG_IS_WRITEABLE = 0x02,
    /** @brief Indicates if the texture was created via wrapping vs traditional
       creation. */
    KRESOURCE_TEXTURE_FLAG_IS_WRAPPED = 0x04,
    /** @brief Indicates the texture is a depth texture. */
    KRESOURCE_TEXTURE_FLAG_DEPTH = 0x08,
    /** @brief Indicates that this texture should account for renderer buffering (i.e. double/triple buffering) */
    KRESOURCE_TEXTURE_FLAG_RENDERER_BUFFERING = 0x10,
} kresource_texture_flag;

/** @brief Holds bit flags for textures.. */
typedef u32 kresource_texture_flag_bits;

#define KRESOURCE_TYPE_NAME_TEXTURE "Texture"

typedef struct kresource_texture {
    kresource base;
    /** @brief The texture type. */
    kresource_texture_type type;
    /** @brief The texture width. */
    u32 width;
    /** @brief The texture height. */
    u32 height;
    /** @brief The format of the texture data. */
    kresource_texture_format format;
    /** @brief For arrayed textures, how many "layers" there are. Otherwise this is 1. */
    u16 array_size;
    /** @brief Holds various flags for this texture. */
    kresource_texture_flag_bits flags;
    /** @brief The number of mip maps the internal texture has. Must always be at least 1. */
    u8 mip_levels;
    /** @brief The the handle to renderer-specific texture data. */
    k_handle renderer_texture_handle;
} kresource_texture;

typedef struct kresource_texture_pixel_data {
    u8* pixels;
    u32 pixel_array_size;
    u32 width;
    u32 height;
    u32 channel_count;
    kresource_texture_format format;
    u8 mip_levels;
} kresource_texture_pixel_data;

ARRAY_TYPE(kresource_texture_pixel_data);

typedef struct kresource_texture_request_info {
    kresource_request_info base;

    kresource_texture_type texture_type;
    u8 array_size;
    kresource_texture_flag_bits flags;

    // Optionally provide pixel data per layer. Must match array_size in length.
    // Only used where asset at index has type of undefined.
    array_kresource_texture_pixel_data pixel_data;

    // Texture width in pixels. Ignored unless there are no assets or pixel data.
    u32 width;

    // Texture height in pixels. Ignored unless there are no assets or pixel data.
    u32 height;

    // Texture format. Ignored unless there are no assets or pixel data.
    kresource_texture_format format;

    // The number of mip levels. Ignored unless there are no assets or pixel data.
    u8 mip_levels;

    // Indicates if loaded image assets should be flipped on the y-axis when loaded. Ignored for non-asset-based textures.
    b8 flip_y;
} kresource_texture_request_info;

/**
 * @brief A structure which maps a texture, use and
 * other properties.
 */
typedef struct kresource_texture_map {
    /**
     * @brief The cached generation of the assigned texture.
     * Used to determine when to regenerate this texture map's
     * resources when a texture's generation changes (as this could
     * be required if, say, a texture's mip levels change).
     * */
    u32 generation;
    /**
     * @brief Cached mip map levels. Should match assigned
     * texture. Must always be at least 1.
     */
    u32 mip_levels;
    /** @brief A constant pointer to a texture resource. */
    const kresource_texture* texture;
    /** @brief Texture filtering mode for minification. */
    texture_filter filter_minify;
    /** @brief Texture filtering mode for magnification. */
    texture_filter filter_magnify;
    /** @brief The repeat mode on the U axis (or X, or S) */
    texture_repeat repeat_u;
    /** @brief The repeat mode on the V axis (or Y, or T) */
    texture_repeat repeat_v;
    /** @brief The repeat mode on the W axis (or Z, or U) */
    texture_repeat repeat_w;
    /** @brief An identifier used for internal resource lookups/management. */
    // TODO: handle?
    u32 internal_id;
} kresource_texture_map;

typedef enum kresource_material_type {
    KRESOURCE_MATERIAL_TYPE_UNKNOWN,
    KRESOURCE_MATERIAL_TYPE_UNLIT,
    KRESOURCE_MATERIAL_TYPE_PHONG,
    KRESOURCE_MATERIAL_TYPE_PBR,
    KRESOURCE_MATERIAL_TYPE_LAYERED_PBR
} kresource_material_type;

typedef struct kresource_material {
    kresource base;
    kresource_material_type type;

    /** @brief The diffuse colour. */
    vec4 diffuse_colour;
    kresource_texture_map albedo_diffuse_map;
    kresource_texture_map normal_map;
    kresource_texture_map specular_map;
    kresource_texture_map metallic_roughness_ao_map;
    kresource_texture_map emissive_map;

    /** @brief The number of material layers. Only used for layered materials. */
    u32 layer_count;
    /** @brief A map used for a texture array. Only used for layered materials. */
    kresource_texture_map layered_material_map;

    /** @brief (Phong-only) The material shininess, determines how concentrated the specular lighting is. */
    f32 specular_strength;

    u32 group_id;
} kresource_material;

typedef struct kresource_material_instance {
    kresource_material* material;

    u32 per_draw_id;
} kresource_material_instance;

typedef struct kresource_material_request_info {
    kresource_request_info base;
    // Optionally include source text to be used as if it resided in a .kmt file.
    const char* material_source_text;
} kresource_material_request_info;
