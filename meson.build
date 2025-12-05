project('cib', 'cpp',
    default_options: [
        'cpp_std=c++20'
    ]
)

stdx_dep = subproject('stdx').get_variable('stdx_dep')
conc_dep = subproject('conc').get_variable('conc_dep')
mp11_dep = subproject('boost-mp11').get_variable('mp11_dep')
if meson.is_cross_build()
    avr_stdcpp_dep = subproject('avr-libstdcpp').get_variable('avr_stdcpp_dep')
else
    avr_stdcpp_dep = []
    fmt_dep = declare_dependency(
    compile_args: 
            # Eliminates include <locale>
            '-DFMT_STATIC_THOUSANDS_SEPARATOR',
    dependencies: subproject('fmt').get_variable('fmt_header_only_dep'),
    )

endif

cib_dep = declare_dependency(
    include_directories: 'include',
    compile_args: [
        '-Wno-missing-braces',
    ],
    dependencies: [
        stdx_dep,
        conc_dep,
        mp11_dep,
        avr_stdcpp_dep,
    ],
)

subdir('tools')
subdir('examples/hello_world')
subdir('examples/blinky')
