import json
import re

text = open("src/config_desc.cpp", encoding="utf-8").read()
items = re.findall(r'\{"([^"]+)", "([^"]*)"\}', text)
desc = {k: v for k, v in items}
s = json.dumps(desc, separators=(",", ":"))
out = open("src/config_desc_json.h", "w", encoding="utf-8")
out.write("#ifndef CONFIG_DESC_JSON_H\n#define CONFIG_DESC_JSON_H\n\n")
out.write("static const char CONFIG_DESC_JSON[] PROGMEM = R\"json(")
out.write(s)
out.write(")json\";\n\n#endif\n")
out.close()
print("wrote src/config_desc_json.h", len(s), "bytes")
