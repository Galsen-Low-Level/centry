project('centry', 'c',
  version : '0.1',
  default_options : ['c_std=gnu99','warning_level=3', 'bindir=/usr/local/bin'])
 
pkg=import('pkgconfig')
cc=meson.get_compiler('c') 

add_project_arguments( 
  cc.get_supported_arguments( 
    '-Wformat',
    '-Wunused',
    '-Wcomment',
    '-Wformat',
    '-Wreturn-type',
    '-Wunused-parameter',
    '-Wparentheses',
    '-Wpedantic',
    ),
  language:'c' 
  )

curses=dependency('curses')  
executable('centry',
           'centry.c',
           'main.c',
           dependencies:curses,
           install : true)
