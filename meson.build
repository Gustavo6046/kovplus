project('kovplus', 'c++')

cmake = import('cmake')

fmt = cmake.subproject('fmt')

kovplus = library('kovplus', 'kovplus.cpp', fmt)

executable('kovplus-demo', 'demo.cpp', kovplus, fmt)
