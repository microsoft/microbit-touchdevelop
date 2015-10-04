var fs = require('fs');

var funs = {}
var ops = {}

var hex = fs.readFileSync(process.argv[2], "utf8").split(/\r?\n/)

process.argv.slice(3).forEach(function (fn) {
    var type = null;
    var opcodes = false;
    var numArgs = 0;
    var idx = 0;
    fs.readFileSync(fn, "utf8").split(/\n/).forEach(function(ln) {
        ln = ln.replace(/\r/g, "")
        var m = /^\s*const void \*call(Func|Proc)(\d+)\[/.exec(ln)
        if (m) {
            type = m[1]
            numArgs = parseInt(m[2]);
            idx = 0;
            return
        }

        m = /^\s*const int enums\[\] =/.exec(ln)
        if (m) {
            type = "Enums";
            numArgs = 0;
            idx = 0;
            return
        }

        if (type) {
            if (/^\s*\};/.test(ln)) {
                type = null
            } else if (/^\s*\/\//.test(ln) || /^\s*$/.test(ln) || /^\s*#/.test(ln)) {
                // nothing
            } else {
                m = /^\s*(\(void\*\))?([\w:]+),?\s*$/.exec(ln)
                if (!m)
                    m = /^\s*(mbit)\(([\w:]+)\)\s*$/.exec(ln)
                if (m) {
                    var nm = m[2]
                    if (m[1] == "mbit") nm = "micro_bit::" + nm
                    funs[nm] = { type: type[0], args: numArgs, idx: idx }
                    idx++
                } else {
                    console.log("bad line: " + ln)
                }
            }
        }

        if (/^\s*\/\*OPCODES\*\/ typedef enum/.test(ln)) {
            opcodes = true;
            idx = 0;
            return
        }

        if (opcodes) {
            if (/^\s*\}/.test(ln)) {
                opcodes = false
            } else if (/^\s*\/\//.test(ln) || /^\s*$/.test(ln)) {
                // nothing
            } else {
                m = /^\s*([A-Z0-9]+),\s*\/\/ S([X0-9\-]+)?\s*$/.exec(ln)
                if (m) {
                    ops[m[1]] = { stack: parseInt(m[2]), idx: idx }
                    idx++;
                } else {
                    console.log("bad line: " + ln)
                }
            }
        }
    })

})

var s = "TDev.bytecodeInfo = " + JSON.stringify({
    opcodes: ops,
    functions: funs,
    hex: hex
}, null, 2)
fs.writeFileSync("build/bytecode.js", s)

// vim: ts=4 sw=4
