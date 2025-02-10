if not meson.is_cross_build()

hello_world = executable('hello_world',
    include_directories: '.',
    sources: [
        'dont_panic.cpp',
        'main.cpp',
    ],
    dependencies: [
        cib_dep,
        fmt_dep,
    ],
)

endif