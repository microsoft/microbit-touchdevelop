var fs = require('fs');

var funs = {}

var hex = fs.readFileSync(process.argv[2], "utf8").split(/\r?\n/)

process.argv.slice(3).forEach(function (fn) {
    var type = null;
    var numArgs = 0;
    var idx = 0;
    fs.readFileSync(fn, "utf8").split(/\n/).forEach(function(ln) {
        ln = ln.replace(/\r/g, "")
        var m = /^\s*void \*\s*const\s*functions\[/.exec(ln)
        if (m) {
            type = "X"
            numArgs = 0;
            idx = 0;
            return
        }

        m = /^\s*const int enums\[\]/.exec(ln)
        if (m) {
            type = "Enums";
            numArgs = 0;
            idx = 0;
            return
        }

        if (type) {
            m = /^\s*\/\/ (FUNC|PROC)(\d+)/.exec(ln)
            if (/^\s*\};/.test(ln)) {
                type = null
            } else if (m) {
                type = m[1];
                numArgs = parseInt(m[2]);
            } else if (/^\s*$/.test(ln) || /^\s*#/.test(ln)) {
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
    })

})

var s = "TDev.bytecodeInfo = " + JSON.stringify({
    functions: funs,
    hex: hex
}, null, 2)
fs.writeFileSync("build/bytecode.js", s)

// vim: ts=4 sw=4
