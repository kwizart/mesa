# Copyright © 2017 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

c_args_for_egl = []
link_for_egl = []
deps_for_egl = []
incs_for_egl = [
  inc_include, inc_src, inc_loader, inc_gbm, include_directories('main'),
]
files_egl = files(
  'main/eglapi.c',
  'main/eglapi.h',
  'main/eglarray.c',
  'main/eglarray.h',
  'main/eglconfig.c',
  'main/eglconfig.h',
  'main/eglcontext.c',
  'main/eglcontext.h',
  'main/eglcurrent.c',
  'main/eglcurrent.h',
  'main/egldefines.h',
  'main/egldisplay.c',
  'main/egldisplay.h',
  'main/egldriver.c',
  'main/egldriver.h',
  'main/eglfallbacks.c',
  'main/eglglobals.c',
  'main/eglglobals.h',
  'main/eglimage.c',
  'main/eglimage.h',
  'main/egllog.c',
  'main/egllog.h',
  'main/eglsurface.c',
  'main/eglsurface.h',
  'main/eglsync.c',
  'main/eglsync.h',
  'main/eglentrypoint.h',
  'main/egltypedefs.h',
  'drivers/dri2/egl_dri2.c',
  'drivers/dri2/egl_dri2.h',
  'drivers/dri2/egl_dri2_fallbacks.h',
)

linux_dmabuf_unstable_v1_protocol_c = custom_target(
  'linux-dmabuf-unstable-v1-protocol.c',
  input : wayland_dmabuf_xml,
  output : 'linux-dmabuf-unstable-v1-protocol.c',
  command : [prog_wl_scanner, 'code', '@INPUT@', '@OUTPUT@'],
)

linux_dmabuf_unstable_v1_client_protocol_h = custom_target(
  'linux-dmabuf-unstable-v1-client-protocol.h',
  input : wayland_dmabuf_xml,
  output : 'linux-dmabuf-unstable-v1-client-protocol.h',
  command : [prog_wl_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
)

g_egldispatchstubs_c = custom_target(
  'g_egldispatchstubs.c',
  input : [
    'generate/gen_egl_dispatch.py', 'generate/eglFunctionList.py',
    'generate/egl.xml', 'generate/egl_other.xml'
  ],
  output : 'g_egldispatchstubs.c',
  command : [
    prog_python2, '@INPUT0@', 'source', '@INPUT1@', '@INPUT2@', '@INPUT3@'
  ],
  depend_files : files('generate/genCommon.py'),
  capture : true,
)

g_egldispatchstubs_h = custom_target(
  'g_egldispatchstubs.h',
  input : [
    'generate/gen_egl_dispatch.py', 'generate/eglFunctionList.py',
    'generate/egl.xml', 'generate/egl_other.xml'
  ],
  output : 'g_egldispatchstubs.h',
  command : [
    prog_python2, '@INPUT0@', 'header', '@INPUT1@', '@INPUT2@', '@INPUT3@'
  ],
  depend_files : files('generate/genCommon.py'),
  capture : true,
)

if with_platform_x11
  files_egl += files('drivers/dri2/platform_x11.c')
  if with_dri3
    files_egl += files('drivers/dri2/platform_x11_dri3.c')
    link_for_egl += libloader_dri3_helper
  endif
  deps_for_egl += [dep_x11_xcb, dep_xcb_dri2, dep_xcb_xfixes]
endif
if with_platform_drm
  files_egl += files('drivers/dri2/platform_drm.c')
  link_for_egl += libgbm
  incs_for_egl += include_directories('../gbm/main')
endif
if with_platform_surfaceless
  files_egl += files('drivers/dri2/platform_surfaceless.c')
endif
if with_platform_wayland
  deps_for_egl += [dep_wayland_client, dep_wayland_server]
  link_for_egl += libwayland_drm
  files_egl += files('drivers/dri2/platform_wayland.c')
  files_egl += [
    linux_dmabuf_unstable_v1_protocol_c,
    linux_dmabuf_unstable_v1_client_protocol_h,
    wayland_drm_client_protocol_h,
  ]
  incs_for_egl += include_directories(
    'wayland/wayland-egl', 'wayland/wayland-drm',
  )
endif
if with_platform_android
  deps_for_egl += dep_android
  files_egl += files('drivers/dri2/platform_android.c')
endif

# TODO: glvnd

if cc.has_function('mincore')
  c_args_for_egl += '-DHAVE_MINCORE'
endif

if not with_glvnd
  egl_lib_name = 'EGL'
  egl_lib_version = '1.0.0'
else
  egl_lib_name = 'EGL_mesa'
  egl_lib_version = '0.0.0'
  files_egl += [g_egldispatchstubs_h, g_egldispatchstubs_c]
  files_egl += files('main/eglglvnd.c', 'main/egldispatchstubs.c')
  install_data(
    'main/50_mesa.json',
    install_dir : join_paths(get_option('datadir'), 'glvnd', 'egl_vendor.d')
  )
endif

libegl = shared_library(
  egl_lib_name,
  files_egl,
  c_args : [
    c_vis_args,
    c_args_for_egl,
    '-DDEFAULT_DRIVER_DIR="@0@"'.format(dri_driver_dir),
    '-D_EGL_BUILT_IN_DRIVER_DRI2',
    '-D_EGL_NATIVE_PLATFORM=_EGL_PLATFORM_@0@'.format(egl_native_platform.to_upper()),
  ],
  include_directories : incs_for_egl,
  link_with : [link_for_egl, libloader, libxmlconfig, libglapi, libmesa_util],
  link_args : [ld_args_bsymbolic, ld_args_gc_sections],
  dependencies : [deps_for_egl, dep_dl, dep_libdrm, dep_clock, dep_thread],
  install : true,
  version : egl_lib_version,
)

pkg.generate(
  name : 'egl',
  description : 'Mesa EGL Library',
  version : meson.project_version(),
  libraries : libegl,
  libraries_private: gl_priv_libs,
  requires_private : gl_priv_reqs,
  extra_cflags : gl_pkgconfig_c_flags,
)

if with_platform_wayland
  subdir('wayland/wayland-egl')
endif

if with_tests
  if with_glvnd
    # TODO: add glvnd symbol check
  else
    test('egl-symbols-check',
      find_program('egl-symbols-check'),
      args : libegl
    )
  endif
  test('egl-entrypoint-check',
    find_program('egl-entrypoint-check'),
    env : [ 'srcdir=' + meson.current_source_dir() ]
  )
endif
