#include "kresource_handler_audio.h"
#include "assets/kasset_types.h"
#include "containers/array.h"
#include "core/engine.h"
#include "debug/kassert.h"
#include "kresources/kresource_types.h"
#include "kresources/kresource_utils.h"
#include "logger.h"
#include "memory/kmemory.h"
#include "renderer/renderer_frontend.h"
#include "strings/kname.h"
#include "systems/asset_system.h"
#include "systems/kresource_system.h"

// TODO: move this to kasset_types?
ARRAY_TYPE_NAMED(const kasset_image*, kimage_ptr);

typedef struct audio_resource_handler_info {
    kresource_audio* typed_resource;
    kresource_handler* handler;
    kresource_audio_request_info* request_info;
    array_kimage_ptr assets;
    u32 loaded_count;
} audio_resource_handler_info;

static void audio_kasset_on_result(asset_request_result result, const struct kasset* asset, void* listener_inst);

kresource* kresource_handler_audio_allocate(void) {
    return (kresource*)kallocate(sizeof(kresource_audio), MEMORY_TAG_RESOURCE);
}

b8 kresource_handler_audio_request(struct kresource_handler* self, kresource* resource, const struct kresource_request_info* info) {
    if (!self || !resource) {
        KERROR("kresource_handler_audio_request requires valid pointers to self and resource.");
        return false;
    }

    kresource_audio* typed_resource = (kresource_audio*)resource;
    kresource_audio_request_info* typed_request = (kresource_audio_request_info*)info;
    struct renderer_system_state* renderer = engine_systems_get()->renderer_system;

    b8 assets_required = true;

    // NOTE: dynamically allocating this so lifetime isn't a concern.
    audio_resource_handler_info* listener_inst = kallocate(sizeof(audio_resource_handler_info), MEMORY_TAG_RESOURCE);
    // Take a copy of the typed request info.
    listener_inst->request_info = kallocate(sizeof(kresource_audio_request_info), MEMORY_TAG_RESOURCE);
    kcopy_memory(listener_inst->request_info, typed_request, sizeof(kresource_audio_request_info));
    listener_inst->typed_resource = typed_resource;
    listener_inst->handler = self;
    listener_inst->loaded_count = 0;
    if (assets_required) {
        listener_inst->assets = array_kimage_ptr_create(info->assets.base.length);
    }

    if (info->assets.base.length != 1) {
        KERROR("kresource_handler_audio requires exactly one asset. Request failed.");
        return false;
    }

    // Load the asset.
    kresource_asset_info* asset_info = &info->assets.data[0];
    if (asset_info->type == KASSET_TYPE_AUDIO) {
        asset_request_info request_info = {0};
        request_info.type = asset_info->type;
        request_info.asset_name = asset_info->asset_name;
        request_info.package_name = asset_info->package_name;
        request_info.auto_release = true;
        request_info.listener_inst = listener_inst;
        request_info.callback = audio_kasset_on_result;
        request_info.synchronous = false;
        request_info.hot_reload_callback = 0;
        request_info.hot_reload_context = 0;
        request_info.import_params_size = 0;
        request_info.import_params = 0;

        asset_system_request(self->asset_system, request_info);
    } else {
        KERROR("kresource_handler_audio asset must be of audio type");
        return false;
    }

    return true;
}

void kresource_handler_audio_release(struct kresource_handler* self, kresource* resource) {
    if (resource) {
        if (resource->type != KRESOURCE_TYPE_AUDIO) {
            KERROR("Attempted to release non-audio resource '%s' via audio resource handler. Resource not released.");
            return;
        }

        kresource_audio* t = (kresource_audio*)resource;
        if (t->pcm_data_size && t->pcm_data) {
            kfree(t->pcm_data, t->pcm_data_size, MEMORY_TAG_AUDIO);
        }

        // TODO: release backend data

        kfree(resource, sizeof(kresource_audio), MEMORY_TAG_RESOURCE);
    }
}

static void audio_kasset_on_result(asset_request_result result, const struct kasset* asset, void* listener_inst) {
    audio_resource_handler_info* listener = (audio_resource_handler_info*)listener_inst;
    if (result == ASSET_REQUEST_RESULT_SUCCESS) {
        // TODO: send to audio system to get resources, etc.

        // Release the asset reference as we are done with it.
        asset_system_release(engine_systems_get()->asset_state, asset->name, asset->package_name);
    } else {
        KERROR("Failed to load a required asset for audio resource '%s'. Resource may not work correctly when used.", kname_string_get(listener->typed_resource->base.name));
    }

destroy_request:
    // Destroy the request.
    array_kimage_ptr_destroy(&listener->assets);
    array_kresource_asset_info_destroy(&listener->request_info->base.assets);
    kfree(listener->request_info, sizeof(kresource_audio_request_info), MEMORY_TAG_RESOURCE);
    // Free the listener itself.
    kfree(listener, sizeof(audio_resource_handler_info), MEMORY_TAG_RESOURCE);
}
