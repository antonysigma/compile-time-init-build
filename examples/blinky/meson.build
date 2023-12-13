arduinocore_proj = subproject('arduinocore-avr')

blinky_lib = static_library('blinky',
    sources: [
        'main.cpp',
    ],
    dependencies: [
        cib_dep,
        # avr-ar does not support fat object files, why?
        arduinocore_proj.get_variable('arduinocore_dep'),
    ],
)

python_exe = find_program('python3')

main_sym = custom_target('main.undefined_symbols.txt',
    output: 'main.undefined_symbols.txt',
    input: [
        # There should be just one object file: main.o
        blinky_lib,
    ],
    command: [
        'nm',
        '-uC',
        '@INPUT@'
    ],
    capture: true,
)

logger_stub_bin = custom_target('logger_stub.[cpp|xml]',
    output: [
        'logger_stub.cpp',
        'logger_stub.json',
        'logger_stub.xml',
    ],
    input: [
        # There should be just one object file: main.o
        main_sym,
    ],
    command: [
        python_exe,
        gen_str_catalog_py,
        '@INPUT0@',
        '@OUTPUT@',
    ],
    build_by_default: true,
)

blinky_exe = executable('blinky',
    sources: [
        logger_stub_bin[0],
    ],
    link_with: blinky_lib,
    dependencies: [
        cib_dep,
        # avr-ar does not support fat object files, why?
        arduinocore_proj.get_variable('arduinocore_dep'),
    ],
)

blinky_hex = custom_target('blinky.hex',
    input: blinky_exe,
    output: 'blinky.hex',
    command: [
        '/opt/avr-gcc/bin/avr-objcopy',
        '-O', 'ihex',
        '-R', '.eeprom',
        '@INPUT@',
        '@OUTPUT@',
    ],
    build_by_default: false,
)

avrdude_exe = find_program('avrdude')
avrdude_conf = files('/usr/share/arduino/hardware/tools/avrdude.conf')
flash_log = custom_target('flash.log',
    input: blinky_hex,
    output: 'flash.log',
    command: [
        avrdude_exe, '-q', '-V',
        '-p', 'atmega328p',
        '-C', avrdude_conf,
        '-D', '-c', 'arduino',
        '-b', '115200',
        '-P', '/dev/ttyACM0',
        '-U', 'flash:w:@INPUT@:i',
    ],
    capture: true,
)

alias_target('stub', logger_stub_bin)
alias_target('hex', blinky_hex)
alias_target('flash', flash_log)
