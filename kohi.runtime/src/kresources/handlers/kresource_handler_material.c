

#include "assets/kasset_types.h"
#include "defines.h"
#include "kasset_kresource_utils.h"
#include "kresources/kresource_types.h"
#include "logger.h"
#include "memory/kmemory.h"
#include "serializers/kasset_material_serializer.h"
#include "strings/kname.h"
#include "systems/asset_system.h"
#include "systems/kresource_system.h"

typedef struct material_resource_handler_info {
    kresource_material* typed_resource;
    kresource_handler* handler;
    kresource_material_request_info* request_info;
    kasset_material* asset;
} material_resource_handler_info;

static void material_kasset_on_result(asset_request_result result, const struct kasset* asset, void* listener_inst);
static void asset_to_resource(const kasset_material* asset, kresource_material* out_material);

kresource* kresource_handler_material_allocate(void) {
    return (kresource*)KALLOC_TYPE(kresource_material, MEMORY_TAG_RESOURCE);
}

b8 kresource_handler_material_request(kresource_handler* self, kresource* resource, const struct kresource_request_info* info) {
    if (!self || !resource) {
        KERROR("kresource_handler_material_request requires valid pointers to self and resource.");
        return false;
    }

    kresource_material* typed_resource = (kresource_material*)resource;
    kresource_material_request_info* typed_request = (kresource_material_request_info*)info;
    typed_resource->base.state = KRESOURCE_STATE_UNINITIALIZED;

    if (info->assets.base.length != 1) {
        if (info->assets.base.length == 0 && typed_request->material_source_text) {
            // Deserialize material asset from provided source.
            kasset material_from_source = {0};
            if (!kasset_material_deserialize(typed_request->material_source_text, &material_from_source)) {
                KERROR("Failed to deserialize material from direct source upon resource request.");
                return false;
            }

            asset_to_resource((kasset_material*)&material_from_source, typed_resource);
            return true;
        } else {
            KERROR("kresource_handler_material_request requires exactly one asset OR zero assets and material source text.");
            return false;
        }
    }

    // NOTE: dynamically allocating this so lifetime isn't a concern.
    material_resource_handler_info* listener_inst = kallocate(sizeof(material_resource_handler_info), MEMORY_TAG_RESOURCE);
    // Take a copy of the typed request info.
    listener_inst->request_info = kallocate(sizeof(kresource_material_request_info), MEMORY_TAG_RESOURCE);
    kcopy_memory(listener_inst->request_info, typed_request, sizeof(kresource_material_request_info));
    listener_inst->typed_resource = typed_resource;
    listener_inst->handler = self;
    listener_inst->asset = 0;

    typed_resource->base.state = KRESOURCE_STATE_INITIALIZED;

    typed_resource->base.state = KRESOURCE_STATE_LOADING;

    kresource_asset_info* asset_info = &info->assets.data[0];
    asset_system_request(
        self->asset_system,
        asset_info->type,
        asset_info->package_name,
        asset_info->asset_name,
        true,
        listener_inst,
        material_kasset_on_result,
        0,
        0);

    return true;
}

void kresource_handler_material_release(kresource_handler* self, kresource* resource) {
    if (resource) {
        kresource_material* typed_resource = (kresource_material*)resource;

        if (typed_resource->custom_sampler_count && typed_resource->custom_samplers) {
            KFREE_TYPE_CARRAY(typed_resource->custom_samplers, kasset_material_sampler, typed_resource->custom_sampler_count);
        }

        KFREE_TYPE(typed_resource, kresource_material, MEMORY_TAG_RESOURCE);
    }
}

static void material_kasset_on_result(asset_request_result result, const struct kasset* asset, void* listener_inst) {
    material_resource_handler_info* listener = (material_resource_handler_info*)listener_inst;
    if (result == ASSET_REQUEST_RESULT_SUCCESS) {
        // Save off the asset pointer to the array.
        listener->asset = (kasset_material*)asset;

        asset_to_resource(listener->asset, listener->typed_resource);
    } else {
        KERROR("Failed to load a required asset for material resource '%s'. Resource may not appear correctly when rendered.", kname_string_get(listener->typed_resource->base.name));
    }

    // Destroy the request.
    array_kresource_asset_info_destroy(&listener->request_info->base.assets);
    kfree(listener->request_info, sizeof(kresource_material_request_info), MEMORY_TAG_RESOURCE);
    // Free the listener itself.
    kfree(listener, sizeof(material_resource_handler_info), MEMORY_TAG_RESOURCE);
}

static void asset_to_resource(const kasset_material* asset, kresource_material* out_material) {
    // Take a copy of all of the asset properties.

    out_material->type = kasset_material_type_to_kresource(asset->type);
    out_material->model = kasset_material_model_to_kresource(asset->model);

    out_material->has_transparency = asset->has_transparency;
    out_material->double_sided = asset->double_sided;
    out_material->recieves_shadow = asset->recieves_shadow;
    out_material->casts_shadow = asset->casts_shadow;
    out_material->use_vertex_colour_as_base_colour = asset->use_vertex_colour_as_base_colour;

    out_material->custom_shader_name = asset->custom_shader_name;

    out_material->base_colour = asset->base_colour;
    out_material->base_colour_map = kasset_material_texture_to_kresource(asset->base_colour_map);

    out_material->normal_enabled = asset->normal_enabled;
    out_material->normal = asset->normal;
    out_material->normal_map = kasset_material_texture_to_kresource(asset->normal_map);

    out_material->metallic = asset->metallic;
    out_material->metallic_map = kasset_material_texture_to_kresource(asset->metallic_map);
    out_material->metallic_map_source_channel = kasset_material_tex_map_channel_to_kresource(asset->metallic_map_source_channel);

    out_material->roughness = asset->roughness;
    out_material->roughness_map = kasset_material_texture_to_kresource(asset->roughness_map);
    out_material->roughness_map_source_channel = kasset_material_tex_map_channel_to_kresource(asset->roughness_map_source_channel);

    out_material->ambient_occlusion_enabled = asset->ambient_occlusion_enabled;
    out_material->ambient_occlusion = asset->ambient_occlusion;
    out_material->ambient_occlusion_map = kasset_material_texture_to_kresource(asset->ambient_occlusion_map);
    out_material->ambient_occlusion_map_source_channel = kasset_material_tex_map_channel_to_kresource(asset->ambient_occlusion_map_source_channel);

    out_material->mra = asset->mra;
    out_material->mra_map = kasset_material_texture_to_kresource(asset->mra_map);
    out_material->use_mra = asset->use_mra;

    out_material->emissive_enabled = asset->emissive_enabled;
    out_material->emissive = asset->emissive;
    out_material->emissive_map = kasset_material_texture_to_kresource(asset->emissive_map);

    out_material->custom_sampler_count = asset->custom_sampler_count;
    KALLOC_TYPE_CARRAY(kasset_material_sampler, out_material->custom_sampler_count);
    KCOPY_TYPE_CARRAY(
        out_material->custom_samplers,
        asset->custom_samplers,
        kasset_material_sampler,
        out_material->custom_sampler_count);

    out_material->base.state = KRESOURCE_STATE_LOADED;
}
