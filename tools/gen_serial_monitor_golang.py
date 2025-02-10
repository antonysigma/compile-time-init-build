import argparse
import re
from dataclasses import dataclass
from pathlib import Path
from typing import TextIO

from parsimonious.grammar import Grammar
from parsimonious.nodes import Node, NodeVisitor

grammar = Grammar(r"""
undefined_symbol_file = newline* path symbol+
path = ~r"[^:]+:\n"i
symbol = ws undefined_keyword (log_symbol / other_symbol) newline
other_symbol = ~r"[^\n]*"i
log_symbol = uint32_type_alias ws "catalog<message<" logging_level string_format closing_bracket ws ">()"
logging_level = "(logging::level)" digits delimiter
string_format = "sc::lazy_string_format<" string_constant delimiter string_format_arguments closing_bracket
string_format_arguments = "cib::tuple<" c_integral_types? (delimiter c_integral_types)* closing_bracket
string_constant = "sc::string_constant<char" char_array closing_bracket

char_array = (delimiter char_keyword ascii_code)+

ascii_code = digits

digits = ~"[0-9]+"i
ws = ~r"\s*"i

const_keyword = "const"
char_keyword = "(char)"
uint32_type_alias = "unsigned long" / "unsigned int"
c_integral_types = ~r"(unsigned )?(char|short|int|long( long)?)"i
c_floating_types = ~r"(long )?(float|double)"i
delimiter = "," ws
closing_bracket = ws ">"
undefined_keyword = "U "
newline = "\n"
""")

ctype_array = list[str]


@dataclass
class LogMessage:
    id: int
    level: int
    msg: bytes
    arg_types: ctype_array
    symbol_name: str


class CIBLogging(NodeVisitor):
    """Export the log messages to a hardcoded map[uint8]FormattedString type."""

    message_count = 0

    def visit_undefined_symbol_file(self, _, visited_children) -> list[LogMessage]:
        message_table: list[LogMessage] = []

        for _, _, symbol, _ in visited_children[2]:
            if isinstance(symbol, list) and isinstance(symbol[0], LogMessage):
                message_table.append(symbol[0])

        return message_table

    def visit_log_symbol(self, node, visited_children) -> LogMessage:
        logging_level, (string_format, args) = visited_children[3:5]
        self.message_count += 1

        return LogMessage(
            id=self.message_count - 1,
            level=logging_level,
            msg=string_format,
            arg_types=args,
            symbol_name=node.text,
        )

    def visit_string_format(self, _, visited_children) -> tuple[bytes, ctype_array]:
        _, string_format, _, string_format_args, _ = visited_children
        return string_format, string_format_args

    def visit_string_constant(self, _, visited_children) -> bytes:
        return visited_children[1]

    def visit_string_format_arguments(self, _, visited_children) -> ctype_array:
        _, first_item, remainder, _ = visited_children
        if not isinstance(first_item, list):
            # Empty list of arguments -> short message
            return []

        args = [first_item[0].text]
        for _, item in remainder:
            if isinstance(item, Node):
                args.append(item.text)

        return args

    def visit_logging_level(self, node, _) -> int:
        return int(node.children[1].text)

    def visit_char_array(self, _, visited_children) -> bytes:
        ascii_array: list[int] = [ascii_code for _, _, ascii_code in visited_children]
        return bytearray(ascii_array)

    def visit_digits(self, node, _) -> int:
        return int(node.text)

    def generic_visit(self, node, visited_children):
        return visited_children or node


def serialMonitorCodeGen(file: TextIO, message_table: list[LogMessage]) -> None:
    file.write("""package main

import (
	"fmt"
	"log"
	"time"

	"go.bug.st/serial"
)

type FormatMapping struct {
	FormatString  string
	ArgumentCount uint
}

var message = map[byte]FormatMapping {
""")

    for m in message_table:
        printf_string = re.sub(r"{}", r"%d", m.msg.decode("utf-8"))
        printf_string = re.sub(r"{:(.*?)}", r"%\1", printf_string)
        file.write(f'\t{m.id:#02x}: {{"{printf_string:s}", {len(m.arg_types):d}}},\n')

    max_args = max((len(m.arg_types) for m in message_table))

    file.write(
        r"""}

func main() {
	// Open the serial port
	mode := &serial.Mode{
		BaudRate: 115200,
	}
	port, err := serial.Open("/dev/ttyACM0", mode)
	if err != nil {
		log.Fatalf("failed to open serial port: %v", err)
	}
	defer port.Close()

"""
        f"\tvar buffer [{max_args + 1:d}]byte // Increased buffer size to read multiple bytes at once"
        r"""
	for {
		// Read from serial port into buffer
		n, err := port.Read(buffer[0:1])
		if err != nil {
			log.Printf("error reading from serial port: %v", err)
			continue
		}
		if n > 0 {
			timestamp := time.Now().Format("15:04:05.000")
			// Check mapping
			if matched, found := message[buffer[0]]; found {
				if matched.ArgumentCount > 0 {
					port.Read(buffer[1:1+matched.ArgumentCount])
					fmt.Printf("%s\t"+matched.FormatString+"\n", timestamp, buffer[1], buffer[2])
				} else {
					fmt.Printf("%s\t%s\n", timestamp, matched.FormatString)
				}
			} else {
				fmt.Printf("%s\t0x%02X\n", timestamp, buffer[0])
			}
		}
	}
}"""
    )


def defineMissingSymbols(file: TextIO, messages: list[LogMessage]) -> None:
    file.write("""#include <log/catalog/catalog.hpp>
#include <log/log.hpp>
#include <sc/lazy_string_format.hpp>
#include <sc/string_constant.hpp>
""")

    for m in messages:
        file.write(f"""
/** {m.msg.decode("utf-8")} */
template<> {m.symbol_name} {{
    return {m.id:d};
}}
""")


@dataclass
class InputOutputFiles:
    symbol: Path
    cpp: Path
    serial_monitor: Path


def parseArgs() -> InputOutputFiles:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-i",
        "--input",
        type=Path,
        help="Input undefined symbol list",
        required=True,
    )
    parser.add_argument(
        "-c",
        "--cpp",
        type=Path,
        help="Generate the logging implementation C++ code",
        required=True,
    )
    parser.add_argument(
        "-m",
        "--serial_monitor",
        type=Path,
        help="Generate the serial monitor app in Go language",
        required=True,
    )

    args = parser.parse_args()

    result = InputOutputFiles(
        symbol=args.input,
        cpp=args.cpp,
        serial_monitor=args.serial_monitor,
    )

    assert result.symbol.exists(), (
        f"Symbol file: {result.symbol.absolute().as_posix()} for found"
    )
    return result


if __name__ == "__main__":
    files = parseArgs()

    with open(files.symbol, "r") as f:
        ast = grammar.parse(f.read())

    cib_logging = CIBLogging()
    message_catalog: list[LogMessage] = cib_logging.visit(ast)

    with open(files.serial_monitor, "w") as f:
        serialMonitorCodeGen(f, message_catalog)

    with open(files.cpp, "w") as f:
        defineMissingSymbols(f, message_catalog)
