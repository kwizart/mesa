#include <stdio.h>

#include "util/u_memory.h"

#include "tegra_common.h"
#include "tegra_context.h"
#include "tegra_resource.h"
#include "tegra_screen.h"

static const struct debug_named_value debug_options[] = {
   { "unimplemented", TEGRA_DEBUG_UNIMPLEMENTED,
     "Print unimplemented functions" },
   { "tgsi", TEGRA_DEBUG_TGSI,
     "Dump TGSI during program compile" },
   { NULL }
};

DEBUG_GET_ONCE_FLAGS_OPTION(tegra_debug, "TEGRA_DEBUG", debug_options, 0)
uint32_t tegra_debug;

static void
tegra_screen_destroy(struct pipe_screen *pscreen)
{
   struct tegra_screen *screen = tegra_screen(pscreen);

   slab_destroy_parent(&screen->transfer_pool);

   drm_tegra_close(screen->drm);
   FREE(screen);
}

static const char *
tegra_screen_get_name(struct pipe_screen *pscreen)
{
   return "Tegra";
}

static const char *
tegra_screen_get_vendor(struct pipe_screen *pscreen)
{
   return "Grate";
}

static const char *
tegra_screen_get_device_vendor(struct pipe_screen *pscreen)
{
   return "NVIDIA";
}

static int
tegra_screen_get_param(struct pipe_screen *pscreen, enum pipe_cap param)
{
   switch (param) {
   case PIPE_CAP_NPOT_TEXTURES:
      return 1; /* not really, but mesa requires it for now! */

   case PIPE_CAP_TWO_SIDED_STENCIL:
      return 0;

   case PIPE_CAP_MAX_DUAL_SOURCE_RENDER_TARGETS:
      return 0; /* ??? */

   case PIPE_CAP_ANISOTROPIC_FILTER:
      return 0;

   case PIPE_CAP_POINT_SPRITE:
      return 1;

   case PIPE_CAP_MAX_RENDER_TARGETS:
      return 8; /* ??? */

   case PIPE_CAP_OCCLUSION_QUERY:
      return 0; /* ??? */

   case PIPE_CAP_QUERY_TIME_ELAPSED:
      return 0; /* ??? - can we use syncpts for this? */

   case PIPE_CAP_TEXTURE_SHADOW_MAP:
      return 0;

   case PIPE_CAP_TEXTURE_SWIZZLE:
      return 0;

   case PIPE_CAP_MAX_TEXTURE_2D_LEVELS:
      return 16;

   case PIPE_CAP_MAX_TEXTURE_3D_LEVELS:
      return 0;

   case PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS:
      return 16; /* ??? */

   case PIPE_CAP_TEXTURE_MIRROR_CLAMP:
      return 0;

   case PIPE_CAP_BLEND_EQUATION_SEPARATE:
      return 1;

   case PIPE_CAP_SM3:
      return 1; /* well, not quite. but perhaps close enough? */

   case PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS:
      return 0;

   case PIPE_CAP_PRIMITIVE_RESTART:
      return 0; /* probably possible to do by splitting draws, but not sure */

   case PIPE_CAP_INDEP_BLEND_ENABLE:
      return 0; /* ??? */

   case PIPE_CAP_INDEP_BLEND_FUNC:
      return 0; /* ??? */

   case PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS:
      return 0;

   case PIPE_CAP_DEPTH_CLIP_DISABLE:
      return 0; /* ??? */

   case PIPE_CAP_SHADER_STENCIL_EXPORT:
      return 0; /* ??? */

   case PIPE_CAP_TGSI_INSTANCEID:
   case PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR:
      return 0;

   case PIPE_CAP_FRAGMENT_COLOR_CLAMPED:
      return 0; /* probably not */

   case PIPE_CAP_SEAMLESS_CUBE_MAP:
   case PIPE_CAP_SEAMLESS_CUBE_MAP_PER_TEXTURE:
      return 0; /* probably not */

   case PIPE_CAP_MIN_TEXEL_OFFSET:
   case PIPE_CAP_MAX_TEXEL_OFFSET:
      return 0;

   case PIPE_CAP_CONDITIONAL_RENDER:
      return 0; /* probably not */

   case PIPE_CAP_TEXTURE_BARRIER:
      return 0; /* no clue */

   case PIPE_CAP_MAX_STREAM_OUTPUT_SEPARATE_COMPONENTS:
   case PIPE_CAP_MAX_STREAM_OUTPUT_INTERLEAVED_COMPONENTS:
   case PIPE_CAP_STREAM_OUTPUT_PAUSE_RESUME:
      return 0;

   case PIPE_CAP_TGSI_CAN_COMPACT_CONSTANTS:
      return 0; /* probably */

   case PIPE_CAP_VERTEX_COLOR_UNCLAMPED:
      return 1; /* probably irrelevant for GLES2 */

   case PIPE_CAP_VERTEX_COLOR_CLAMPED:
      return 0; /* probably irrelevant for GLES2 */

   case PIPE_CAP_GLSL_FEATURE_LEVEL:
      return 120; /* no clue */

   case PIPE_CAP_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION:
      return 0; /* no idea, need to test */

   case PIPE_CAP_USER_VERTEX_BUFFERS:
      return 0; /* probably possible, but nasty for kernel */

   case PIPE_CAP_USER_CONSTANT_BUFFERS:
      return 1;

   case PIPE_CAP_VERTEX_BUFFER_OFFSET_4BYTE_ALIGNED_ONLY:
   case PIPE_CAP_VERTEX_BUFFER_STRIDE_4BYTE_ALIGNED_ONLY:
   case PIPE_CAP_VERTEX_ELEMENT_SRC_OFFSET_4BYTE_ALIGNED_ONLY:
      return 0;

   case PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT:
      return 4; /* DWORD aligned, can do pure data GATHER */

   case PIPE_CAP_START_INSTANCE:
      return 0;

   case PIPE_CAP_QUERY_TIMESTAMP:
      return 0; /* dunno */

   case PIPE_CAP_BUFFER_SAMPLER_VIEW_RGBA_ONLY:
      return 0;

   case PIPE_CAP_MIN_MAP_BUFFER_ALIGNMENT:
      return 0;

   case PIPE_CAP_CUBE_MAP_ARRAY:
      return 0;

   case PIPE_CAP_TEXTURE_BUFFER_OBJECTS:
   case PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
   case PIPE_CAP_MAX_TEXTURE_BUFFER_SIZE:
      return 0;

   case PIPE_CAP_TGSI_TEXCOORD:
      return 0;

   case PIPE_CAP_PREFER_BLIT_BASED_TEXTURE_TRANSFER:
      return 1;

   case PIPE_CAP_COMPUTE:
   case PIPE_CAP_TEXTURE_MULTISAMPLE:
   case PIPE_CAP_QUERY_PIPELINE_STATISTICS:
   case PIPE_CAP_TEXTURE_BORDER_COLOR_QUIRK:
      return 0;

   case PIPE_CAP_MAX_VIEWPORTS:
      return 1;

   case PIPE_CAP_MIXED_FRAMEBUFFER_SIZES:
      return 1;

   case PIPE_CAP_MAX_GEOMETRY_OUTPUT_VERTICES:
   case PIPE_CAP_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS:
   case PIPE_CAP_MAX_VERTEX_STREAMS:
      return 0;

   case PIPE_CAP_MAX_TEXTURE_GATHER_COMPONENTS:
   case PIPE_CAP_MIN_TEXTURE_GATHER_OFFSET:
   case PIPE_CAP_MAX_TEXTURE_GATHER_OFFSET:
   case PIPE_CAP_BUFFER_MAP_PERSISTENT_COHERENT: /* dunno */
   case PIPE_CAP_TEXTURE_QUERY_LOD:
   case PIPE_CAP_SAMPLE_SHADING:
      return 0;

   case PIPE_CAP_DRAW_INDIRECT:
   case PIPE_CAP_TGSI_FS_FINE_DERIVATIVE:
      return 0;

   case PIPE_CAP_VENDOR_ID:
      return 0x10de;

   case PIPE_CAP_DEVICE_ID:
      return 0xFFFFFFFF;

   case PIPE_CAP_ACCELERATED:
      return 1;

   case PIPE_CAP_VIDEO_MEMORY:
      return 0;

   case PIPE_CAP_UMA:
      return 1;

   case PIPE_CAP_CONDITIONAL_RENDER_INVERTED:
      return 0; /* no support, shouldn't matter */

   case PIPE_CAP_MAX_VERTEX_ATTRIB_STRIDE:
      return (1 << 24) - 1;

   case PIPE_CAP_SAMPLER_VIEW_TARGET:
   case PIPE_CAP_CLIP_HALFZ:
   case PIPE_CAP_POLYGON_OFFSET_CLAMP:
   case PIPE_CAP_RESOURCE_FROM_USER_MEMORY:
   case PIPE_CAP_DEVICE_RESET_STATUS_QUERY:
      return 0;

   case PIPE_CAP_MAX_SHADER_PATCH_VARYINGS:
      return 0;

   case PIPE_CAP_TEXTURE_FLOAT_LINEAR:
   case PIPE_CAP_TEXTURE_HALF_FLOAT_LINEAR:
   case PIPE_CAP_DEPTH_BOUNDS_TEST:
   case PIPE_CAP_TGSI_TXQS:
      return 0;

   case PIPE_CAP_SHAREABLE_SHADERS:
      return 1;

   case PIPE_CAP_COPY_BETWEEN_COMPRESSED_AND_PLAIN_FORMATS:
   case PIPE_CAP_CLEAR_TEXTURE: /* might be possible */
   case PIPE_CAP_DRAW_PARAMETERS:
   case PIPE_CAP_TGSI_PACK_HALF_FLOAT:
   case PIPE_CAP_MULTI_DRAW_INDIRECT:
   case PIPE_CAP_MULTI_DRAW_INDIRECT_PARAMS:
      return 0;

   case PIPE_CAP_TGSI_FS_POSITION_IS_SYSVAL:
   case PIPE_CAP_TGSI_FS_FACE_IS_INTEGER_SYSVAL:
      return 0; /* not really sure about these */

   case PIPE_CAP_SHADER_BUFFER_OFFSET_ALIGNMENT:
   case PIPE_CAP_INVALIDATE_BUFFER: /* not sure */
   case PIPE_CAP_STRING_MARKER:
   case PIPE_CAP_SURFACE_REINTERPRET_BLOCKS: /* can probably be supported */
   case PIPE_CAP_QUERY_BUFFER_OBJECT:
   case PIPE_CAP_QUERY_MEMORY_INFO:
   case PIPE_CAP_FRAMEBUFFER_NO_ATTACHMENT: /* not sure */
   case PIPE_CAP_ROBUST_BUFFER_ACCESS_BEHAVIOR: /* probably not */
   case PIPE_CAP_CULL_DISTANCE: /* don't know */
   case PIPE_CAP_PRIMITIVE_RESTART_FOR_PATCHES:
   case PIPE_CAP_TGSI_VOTE:
   case PIPE_CAP_MAX_WINDOW_RECTANGLES:
      return 0;

   case PIPE_CAP_VIEWPORT_SUBPIXEL_BITS:
      return 4; /* minimum for GLES 2.0, might be more */

   case PIPE_CAP_MIXED_COLOR_DEPTH_BITS:
      return 1; /* probably true ? */

   case PIPE_CAP_MIXED_COLORBUFFER_FORMATS:
      return 1; /* can't see why not... */

   case PIPE_CAP_STREAM_OUTPUT_INTERLEAVE_BUFFERS:
   case PIPE_CAP_GLSL_OPTIMIZE_CONSERVATIVELY:
   case PIPE_CAP_TGSI_FS_FBFETCH: /* supported, but let's enable later */
   case PIPE_CAP_TEXTURE_GATHER_OFFSETS:
   case PIPE_CAP_DOUBLES:
   case PIPE_CAP_INT64:
   case PIPE_CAP_INT64_DIVMOD:
   case PIPE_CAP_TGSI_CAN_READ_OUTPUTS:
   case PIPE_CAP_TGSI_TEX_TXF_LZ:
   case PIPE_CAP_TGSI_CLOCK:
   case PIPE_CAP_POLYGON_MODE_FILL_RECTANGLE:
   case PIPE_CAP_SPARSE_BUFFER_PAGE_SIZE:
   case PIPE_CAP_TGSI_BALLOT:
   case PIPE_CAP_NATIVE_FENCE_FD:
   case PIPE_CAP_CAN_BIND_CONST_BUFFER_AS_VERTEX:
   case PIPE_CAP_ALLOW_MAPPED_BUFFERS_DURING_EXECUTION:
   case PIPE_CAP_POST_DEPTH_COVERAGE:
   case PIPE_CAP_BINDLESS_TEXTURE:
   case PIPE_CAP_QUERY_SO_OVERFLOW:
   case PIPE_CAP_MEMOBJ:
   case PIPE_CAP_LOAD_CONSTBUF:
   case PIPE_CAP_TGSI_ANY_REG_AS_ADDRESS:
   case PIPE_CAP_TILE_RASTER_ORDER:
   case PIPE_CAP_MAX_COMBINED_SHADER_OUTPUT_RESOURCES:
   case PIPE_CAP_SIGNED_VERTEX_BUFFER_OFFSET:
      return 0;

   default:
      fprintf(stdout, "%s: unsupported parameter: %d\n", __func__, param);
      return 0;
   }
}

static float
tegra_screen_get_paramf(struct pipe_screen *pscreen,
                        enum pipe_capf param)
{
   switch (param) {
   case PIPE_CAPF_MAX_LINE_WIDTH:
   case PIPE_CAPF_MAX_LINE_WIDTH_AA:
   case PIPE_CAPF_MAX_POINT_WIDTH:
   case PIPE_CAPF_MAX_POINT_WIDTH_AA:
      return 8192.0f; /* no clue */

   case PIPE_CAPF_MAX_TEXTURE_ANISOTROPY:
      return 0.0f;

   case PIPE_CAPF_MAX_TEXTURE_LOD_BIAS:
      return 16.0f;

   default:
      fprintf(stdout, "%s: unsupported parameter: %d\n", __func__, param);
      return 0.0f;
   }
}

static int
tegra_screen_get_shader_param(struct pipe_screen *pscreen,
                              unsigned int shader,
                              enum pipe_shader_cap param)
{
   switch (shader) {
   case PIPE_SHADER_VERTEX:
      switch (param) {

      case PIPE_SHADER_CAP_MAX_INSTRUCTIONS:
      case PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS:
         return 1024;

      /* no vertex-texturing */
      case PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS:
      case PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS:
      case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
      case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
         return 0;

      case PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH:
      case PIPE_SHADER_CAP_TGSI_CONT_SUPPORTED:
      case PIPE_SHADER_CAP_SUBROUTINES:
         return 0;

      case PIPE_SHADER_CAP_MAX_INPUTS:
      case PIPE_SHADER_CAP_MAX_OUTPUTS:
         return 16;

      case PIPE_SHADER_CAP_MAX_CONST_BUFFER_SIZE:
      case PIPE_SHADER_CAP_MAX_CONST_BUFFERS:
         return 1024;

      case PIPE_SHADER_CAP_MAX_TEMPS:
         return 64 * 4; /* 64 vec4s */

      /* cannot index attributes, varyings nor GPRs */
      case PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
         return 0;

      case PIPE_SHADER_CAP_INDIRECT_CONST_ADDR:
         return 1; /* can index constant registers */

      case PIPE_SHADER_CAP_INTEGERS:
      case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
         return 0;

      case PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED:
         return 1;

      case PIPE_SHADER_CAP_TGSI_DROUND_SUPPORTED:
      case PIPE_SHADER_CAP_TGSI_DFRACEXP_DLDEXP_SUPPORTED:
      case PIPE_SHADER_CAP_TGSI_FMA_SUPPORTED:
      case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
         return 0;

      case PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE:
         return 0;

      case PIPE_SHADER_CAP_MAX_UNROLL_ITERATIONS_HINT:
         return INT_MAX;

      case PIPE_SHADER_CAP_SUPPORTED_IRS:
         return PIPE_SHADER_IR_TGSI;

      case PIPE_SHADER_CAP_PREFERRED_IR:
         return PIPE_SHADER_IR_TGSI;

      case PIPE_SHADER_CAP_LOWER_IF_THRESHOLD:
         return INT_MAX;

      case PIPE_SHADER_CAP_TGSI_SKIP_MERGE_REGISTERS:
          return 0;

      case PIPE_SHADER_CAP_TGSI_LDEXP_SUPPORTED:
          return 0;

      case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS:
      case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS:
          return 0;

      default:
         fprintf(stdout, "%s: unsupported vertex-shader parameter: %d\n", __func__, param);
         return 0;
      }

   case PIPE_SHADER_FRAGMENT:

      switch (param) {
      case PIPE_SHADER_CAP_MAX_INSTRUCTIONS:
         return 4 * 128;

      case PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS:
         return 4 * 128;

      case PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS:
         return 128;

      case PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS:
         return 128;

      /* no control flow */
      case PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH:
      case PIPE_SHADER_CAP_TGSI_CONT_SUPPORTED:
      case PIPE_SHADER_CAP_SUBROUTINES:
         return 0;

      case PIPE_SHADER_CAP_MAX_INPUTS:
      case PIPE_SHADER_CAP_MAX_OUTPUTS:
         return 16;

      case PIPE_SHADER_CAP_MAX_CONST_BUFFER_SIZE:
      case PIPE_SHADER_CAP_MAX_CONST_BUFFERS:
         return 32;

      case PIPE_SHADER_CAP_MAX_TEMPS:
         return 16; /* scalars */

      /* no indirection */
      case PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_CONST_ADDR:
         return 0;

      case PIPE_SHADER_CAP_INTEGERS:
      case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
         return 0;

      case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
      case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
         return 16;

      case PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED:
         return 1;

      case PIPE_SHADER_CAP_TGSI_DROUND_SUPPORTED:
      case PIPE_SHADER_CAP_TGSI_DFRACEXP_DLDEXP_SUPPORTED:
         return 0;

      case PIPE_SHADER_CAP_TGSI_FMA_SUPPORTED:
         return 0; /* might really be true, but need more testing to be sure */

      case PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE:
         return 0;

      case PIPE_SHADER_CAP_MAX_UNROLL_ITERATIONS_HINT:
         return INT_MAX;

      case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
         return 0;

      case PIPE_SHADER_CAP_SUPPORTED_IRS:
         return PIPE_SHADER_IR_TGSI;

      case PIPE_SHADER_CAP_PREFERRED_IR:
         return PIPE_SHADER_IR_TGSI;

      case PIPE_SHADER_CAP_LOWER_IF_THRESHOLD:
         return INT_MAX;

      case PIPE_SHADER_CAP_TGSI_SKIP_MERGE_REGISTERS:
          return 0;

      case PIPE_SHADER_CAP_TGSI_LDEXP_SUPPORTED:
          return 0;

      case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS:
      case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS:
          return 0;

      default:
         fprintf(stdout, "%s: unsupported fragment-shader parameter: %d\n", __func__, param);
         return 0;
      }
      break;

   case PIPE_SHADER_GEOMETRY:
   case PIPE_SHADER_TESS_CTRL:
   case PIPE_SHADER_TESS_EVAL:
   case PIPE_SHADER_COMPUTE:
      return 0;

   default:
      fprintf(stdout, "%s: unknown shader type: %u\n", __func__, shader);
      return 0;
   }
}

static boolean
tegra_screen_is_format_supported(struct pipe_screen *pscreen,
                                 enum pipe_format format,
                                 enum pipe_texture_target target,
                                 unsigned int sample_count,
                                 unsigned int usage)
{
   if (usage & (PIPE_BIND_RENDER_TARGET | PIPE_BIND_DEPTH_STENCIL)) {
      if (tegra_pixel_format(format) < 0)
         return false;
   }

   return true;
}

static void
tegra_screen_fence_reference(struct pipe_screen *pscreen,
                             struct pipe_fence_handle **ptr,
                             struct pipe_fence_handle *fence)
{
   unimplemented();
}

static boolean
tegra_screen_fence_finish(struct pipe_screen *screen,
                          struct pipe_context *ctx,
                          struct pipe_fence_handle *fence,
                          uint64_t timeout)
{
   unimplemented();
   return FALSE;
}

struct pipe_screen *
tegra_screen_create(struct drm_tegra *drm)
{
   struct tegra_screen *screen = CALLOC_STRUCT(tegra_screen);
   if (!screen)
      return NULL;

   screen->drm = drm;

   tegra_debug = debug_get_option_tegra_debug();

   screen->base.destroy = tegra_screen_destroy;
   screen->base.get_name = tegra_screen_get_name;
   screen->base.get_vendor = tegra_screen_get_vendor;
   screen->base.get_device_vendor = tegra_screen_get_device_vendor;
   screen->base.get_param = tegra_screen_get_param;
   screen->base.get_paramf = tegra_screen_get_paramf;
   screen->base.get_shader_param = tegra_screen_get_shader_param;
   screen->base.context_create = tegra_screen_context_create;
   screen->base.is_format_supported = tegra_screen_is_format_supported;

   /* fence functions */
   screen->base.fence_reference = tegra_screen_fence_reference;
   screen->base.fence_finish = tegra_screen_fence_finish;

   tegra_screen_resource_init(&screen->base);

   slab_create_parent(&screen->transfer_pool, sizeof(struct pipe_transfer), 16);

   return &screen->base;
}
