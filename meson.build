project('kovplus', 'cpp')

cmake = import('cmake')

# enable PIC when building fmt with CMake
opt_var = cmake.subproject_options()
opt_var.add_cmake_defines({'CMAKE_POSITION_INDEPENDENT_CODE': true})

# fmt dependency
fmt = cmake.subproject('fmt', options: opt_var)
fmt_lib = fmt.dependency('fmt')

# kovplus
kovplus_sources = [
	'kovplus.cpp'
]

kovplus_static = static_library('kovplus-static', sources: kovplus_sources, dependencies: fmt_lib)
kovplus_shared = shared_library('kovplus', sources: kovplus_sources, dependencies: fmt_lib)

executable('kovplus-demo', 'demo.cpp', link_with: kovplus_static)
