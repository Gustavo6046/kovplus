project('kovplus', 'cpp')

# kovplus
kovplus_sources = [
	'kovplus.cpp'
]

kovplus_static = static_library('kovplus-static', sources: kovplus_sources)
kovplus_shared = shared_library('kovplus', sources: kovplus_sources)

executable('kovplus-demo', 'demo.cpp', link_with: kovplus_static)
