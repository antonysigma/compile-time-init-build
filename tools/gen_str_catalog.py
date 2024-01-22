import re
import sys
import json
import xml.etree.ElementTree as et

levels = ["MAX", "FATAL", "ERROR", "WARN", "INFO", "USER1", "USER2", "TRACE"]

input_file = sys.argv[1]
cpp_file = sys.argv[2]
json_file = sys.argv[3]
xml_file = sys.argv[4]

# fwver_dash = sys.argv[5]
# cust_name = sys.argv[6]
# bit_mask_files = sys.argv[7:]

catalog_re = re.compile(r"^.+?(unsigned [^ ]+ catalog<(.+?)>\(\))\s*$")

string_re = re.compile(
    r"message<\(logging::level\)(\d+), sc::undefined<sc::args<(.*)>, char, (.*)>\s*>"
)

string_id = 0
cataloged_strings = set()

out = open(cpp_file, "w")

messages = []

out.write(
    """
#include <log/catalog/catalog.hpp>

"""
)

# https://stackoverflow.com/questions/174890/how-to-output-cdata-using-elementtree
def _escape_cdata(text):
    try:
        if "&" in text:
            text = text.replace("&", "&amp;")
        # if "<" in text:
        # text = text.replace("<", "&lt;")
        # if ">" in text:
        # text = text.replace(">", "&gt;")
        return text
    except TypeError:
        raise TypeError("cannot serialize %r (type %s)" % (text, type(text).__name__))


# https://stackoverflow.com/questions/749796/pretty-printing-xml-in-python
# TODO switch to indent function on ElementTree when it is available in python
def prettify(element, indent="  "):
    queue = [(0, element)]  # (level, element)
    while queue:
        level, element = queue.pop(0)
        children = [(level + 1, child) for child in list(element)]
        if children:
            element.text = "\n" + indent * (level + 1)  # for child open
        if queue:
            element.tail = "\n" + indent * queue[0][0]  # for sibling open
        else:
            element.tail = "\n" + indent * (level - 1)  # for parent close
        queue[0:0] = children  # prepend so children come before siblings


# with open(fwver_dash, "r") as fwVer:
#     for line in fwVer:
#         if 'PATCH_MAJOR_VERSION' in line:
#             major = int(line.split('=')[1].split(';')[0],16)
#         if 'PATCH_MINOR_VERSION' in line:
#             minor = int(line.split('=')[1].split(';')[0],16)
#     fwRuntimeVer = major << 16 | minor

et._escape_cdata = _escape_cdata

# client_name = cust_name.upper() + " SPARC FW" # FIXME: get from command line
client_name = "CIB Framework FW"  # FIXME: get from command line

syst_collateral = et.Element("syst:Collateral")
syst_collateral.set("xmlns:syst", "http://www.mipi.org/1.0/sys-t")
syst_collateral.set("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")
syst_collateral.set(
    "xsi:schemaLocation",
    "http://www.mipi.org/1.0/sys-t https://www.mipi.org/schema/sys-t/sys-t_1-0.xsd",
)
syst_client = et.SubElement(syst_collateral, "syst:Client")
syst_client.set("Name", client_name)
syst_fwversion = et.SubElement(syst_collateral, "syst:FwVersion")
syst_fwversion.set("FW_Version", "VERSION")  # FIXME: get from command line
syst_guids = et.SubElement(syst_client, "syst:Guids")
syst_guid = et.SubElement(syst_guids, "syst:Guid")
syst_guid.set(
    "ID", "{00000000-0017-0001-0000-000000000000}"
)  # FIXME: get from command line
syst_guid.set(
    "Mask", "{00000000-FFFF-FFFF-8000-000000000000}"
)  # FIXME: get from command line
syst_short_message = et.SubElement(syst_client, "syst:Short32")
syst_catalog_message = et.SubElement(syst_client, "syst:Catalog32")


def find_arg_split_pos(s, start):
    angle_count = 0
    for i in range(start, len(s)):
        if s[i] == ">":
            angle_count -= 1
        elif s[i] == "<":
            angle_count += 1
        elif s[i] == ",":
            if angle_count == 0:
                return i
    return len(s)


def split_args(s):
    args = []
    start = 0
    while start < len(s):
        pos = find_arg_split_pos(s, start)
        args.append(s[start:pos].strip())
        start = pos + 1
    return args


with open(input_file, "r") as f:
    for line in f:
        catalog_m = catalog_re.match(line)

        if catalog_m:
            string_m = string_re.match(catalog_m.group(2))
            catalog_type = catalog_m.group(1)
            log_level = string_m.group(1)
            arg_tuple = string_m.group(2)
            string_tuple = string_m.group(3).replace("(char)", "")
            string_value = "".join(
                [chr(int(c)) for c in re.split(r"\s*,\s*", string_tuple)]
            )

            if catalog_type not in cataloged_strings:
                cataloged_strings.add(catalog_type)
                out.write("/*\n")
                out.write('    "' + string_value + '"\n')
                out.write("\n")
                out.write("   " + arg_tuple + "\n")
                out.write(" */\n")
                out.write(
                    "template<> {} {{\n    return {};\n}}\n".format(
                        catalog_type, string_id
                    )
                )
                out.write("\n")

                args = split_args(arg_tuple)

                msg_type = "msg"
                if string_value.startswith("flow."):
                    msg_type = "flow"

                messages.append(
                    dict(
                        level=levels[int(log_level)],
                        msg=string_value,
                        type=msg_type,
                        id=string_id,
                        arg_types=args,
                        arg_count=len(args),
                    )
                )

                if len(args) == 0:
                    syst_format = et.SubElement(syst_short_message, "syst:Format")
                    syst_format.set("ID", "0x%08X" % string_id)
                    syst_format.set("Mask", "0x0FFFFFFF")
                    printf_string = string_value
                else:
                    syst_format = et.SubElement(syst_catalog_message, "syst:Format")
                    syst_format.set("ID", "0x%08X" % string_id)
                    syst_format.set("Mask", "0xFFFFFFFF")
                    printf_string = re.sub(r"{}", r"%d", string_value)
                    printf_string = re.sub(r"{:(.*?)}", r"%\1", printf_string)
                syst_format.text = "<![CDATA[" + printf_string + "]]>"

                string_id += 1

str_catalog = dict(messages=messages)

# for bit_mask_file in bit_mask_files:
#     bit_mask_def = json.load(open(bit_mask_file))
#     str_catalog.update(bit_mask_def)

json.dump(str_catalog, open(json_file, "w"), indent=4)

prettify(syst_collateral, "    ")
xml_string = et.tostring(syst_collateral, encoding="utf8", method="xml")

with open(xml_file, "wb") as xf:
    xf.write(xml_string)
