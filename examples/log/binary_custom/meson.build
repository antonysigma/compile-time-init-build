binary_custom_lib = static_library('binary_custom',
    sources: [
        'lib.cpp',
    ],
    dependencies: [
        cib_dep,
    ],
)

python_exe = find_program('python3')

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

binary_custom_logger_stub_bin = custom_target('logger_stub.[cpp|xml]',
    output: [
        'logger_stub.cpp',
        'logger_stub.xml',
    ],
    input: [
        # There should be just one object file: main.o
        binary_custom_main_sym,
    ],
    command: [
        python_exe,
        gen_str_catalog_py,
        '--input', '@INPUT0@',
        '--cpp_output', '@OUTPUT0@',
        '--xml_output', '@OUTPUT1@',
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

