
#pragma once

#include "defines.h"

struct kasset;
struct kasset_importer;

KAPI b8 kasset_importer_static_mesh_obj_import(const struct kasset_importer* self, u64 data_size, const void* data, void* params, struct kasset* out_asset);
