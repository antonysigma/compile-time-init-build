binary_custom_lib = static_library('binary_custom',
    sources: [
        'lib.cpp',
    ],
    cpp_args: [
        '-fconcepts-diagnostics-depth=10',
    ],
    dependencies: [
        cib_dep,
    ],
)

binary_custom_main_sym = custom_target('main.undefined_symbols.txt',
    output: 'main.undefined_symbols.txt',
    input: [
        # There should be just one object file: main.o
        binary_custom_lib,
    ],
    command: [
        'nm',
        '-uC',
        '@INPUT@'
    ],
    capture: true,
)

binary_custom_logger_stub_bin = custom_target('logger_stub.[cpp|go]',
    output: [
        'logger_stub.cpp',
        'serial_monitor.go',
    ],
    input: [
        # There should be just one object file: lib.o
        binary_custom_main_sym,
    ],
    command: [
        python_exe,
        gen_str_catalog_py,
        '-i@INPUT0@',
        '-c@OUTPUT0@',
        '-m@OUTPUT1@',
    ],
    build_by_default: true,
)

binary_custom_exe = executable('binary_custom',
    sources: [
        binary_custom_logger_stub_bin[0],
        'main.cpp',
    ],
    link_with: binary_custom_lib,
    dependencies: [
        cib_dep,
    ],
)
