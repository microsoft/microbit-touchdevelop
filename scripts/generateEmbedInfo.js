"use strict";

if (process.argv.length < 4) {
  console.log("Generate JSON/JavaScript file embeddeding a given hex file and meta-information about it.")
  console.log("USAGE: node generateEmbedInfo.js file.hex")
  process.exit(1)
}

var fs = require('fs');

var hex = fs.readFileSync(process.argv[2], "utf8").split(/\r?\n/)
var metainfo = JSON.parse(fs.readFileSync("generated/metainfo.json", "utf8"))

var s = "TDev.bytecodeInfo = {\n";
s += "functions: [\n";
metainfo.functions.forEach(f => {
    s += "  " + JSON.stringify(f) + ",\n"
})
s += "],\n";
function addfld(n, v) {
  s += n + ": " + JSON.stringify(v, null, 2) + ",\n";
};
addfld("enums", metainfo.enums)
addfld("hex", hex)
s += "}\n"
fs.writeFileSync("build/bytecode.js", s)

// vim: ts=4 sw=4
