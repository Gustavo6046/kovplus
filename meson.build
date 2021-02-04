project('kovplus', 'cpp')

# kovplus
kovplus_sources = [
	'src/sentence.cpp',
	'src/kovplus.cpp'
]

kovplus_include = [ 'inc' ]

kovplus_static = static_library('kovplus-static', sources: kovplus_sources, include_directories: kovplus_include)
kovplus_shared = shared_library('kovplus', sources: kovplus_sources, include_directories: kovplus_include)

executable('kovplus-demo', 'demo.cpp', link_with: kovplus_static, include_directories: kovplus_include)
