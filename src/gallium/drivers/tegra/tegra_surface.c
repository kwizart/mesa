#include <stdio.h>

#include "util/u_inlines.h"
#include "util/u_memory.h"

#include "tegra_surface.h"

static struct pipe_surface *
tegra_create_surface(struct pipe_context *context,
                     struct pipe_resource *resource,
                     const struct pipe_surface *template)
{
   unsigned int level = template->u.tex.level;
   struct tegra_surface *surface = CALLOC_STRUCT(tegra_surface);
   if (!surface)
      return NULL;

   pipe_resource_reference(&surface->base.texture, resource);
   pipe_reference_init(&surface->base.reference, 1);

   surface->base.context = context;
   surface->base.format = template->format;
   surface->base.width = u_minify(resource->width0, level);
   surface->base.height = u_minify(resource->height0, level);
   surface->base.u.tex.level = level;
   surface->base.u.tex.first_layer = template->u.tex.first_layer;
   surface->base.u.tex.last_layer = template->u.tex.last_layer;

   return &surface->base;
}

static void
tegra_surface_destroy(struct pipe_context *context,
                      struct pipe_surface *surface)
{
   pipe_resource_reference(&surface->texture, NULL);
   FREE(surface);
}

void
tegra_context_surface_init(struct pipe_context *context)
{
   context->create_surface = tegra_create_surface;
   context->surface_destroy = tegra_surface_destroy;
}
