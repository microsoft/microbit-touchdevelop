"use strict";

if (process.argv.length < 3) {
  console.log("Generate JSON metadata and .cpp file with function table.")
  console.log("USAGE: node generateEmbedInfo.js file.cpp/h...")
  process.exit(1)
}

var fs = require('fs');

var funs = {}

var prefixes = ["bitvm::bitvm_", "bitvm::", "touch_develop::", ""]
var fullfuns = {}
var basenames = {}
var enums = {}

process.argv.slice(2).forEach(function (fn) {
    var type = null;
    var numArgs = 0;
    var idx = 0;

    var nsStack = [];
    var lineNo = 0;
    var justPushed = true;
    nsStack.push({ ns: "", endMark: "end of file", lineNo: 0, fnIndent: "" })

    var isInlude = /yotta_modules/.test(fn)


    fs.readFileSync(fn, "utf8").split(/\n/).forEach(function(ln) {
        lineNo++;

        ln = ln.replace(/\s*$/, "")

        if (ln == "") return;

        if (isInlude) {
            ln = ln.replace(/\/\/.*/, "")
            m = /^\s*#define\s+(\w+)\s+(\d+)\s*$/.exec(ln)
            if (m) {
                enums[m[1]] = parseInt(m[2])
            }
            return;
        }

        var top = nsStack[nsStack.length - 1]

        if (justPushed) {
            m = /^(\s*)/.exec(ln)
            top.fnIndent = m[1]
            justPushed = false;
        }

        var m = /^(\s*)(class|namespace) (\w+)/.exec(ln)
        if (!/;$/.test(ln) && m) {
            var isClass = m[2] == "class"
            nsStack.push({ ns: m[3], endMark: m[1] + "}" + (isClass ? ";" : ""), lineNo: lineNo, isClass: isClass })
            justPushed = true;
        } else {
            if (top.endMark == ln) {
                nsStack.pop();
                return;
            }
        }

        m = /^(\s*)(\w+)([\*\&]*\s+[\*\&]*)(\w+)\s*\(([^\(\)]*)\)\s*(;\s*$|\{|$)/.exec(ln)
        if (top && m && !/^(else|return)$/.test(m[2])) {
            if (top.isClass) {
                return; // no class methods yet
            }
            if (top.fnIndent != m[1]) {
                console.log("skip by indent:", ln)
                return;
            }
            var ns = nsStack.slice(1).map(s => s.ns).filter(s => !!s).join("::")
            var name = ns + "::" + m[4]
            if (ns == "") name = m[4]
            var tp = "F"
            if (m[2] == "void" && m[3].trim() == "")
                tp = "P"
            var args = 0
            if (m[5].trim() == "")
                args = 0
            else
                args = m[5].replace(/[^,]/g, "").length + 1

            console.log(fn + ": found " + name);
            var inf = fullfuns[name] = {
                proto: "",
                name: name,
                type: tp,
                args: args,
                full: name,
            }

            prefixes.forEach(p => {
                if (name.slice(0, p.length) == p &&
                    /::/.test(name.slice(p.length)))
                    name = name.slice(p.length)
            })
            basenames[name] = 1;
            var fmt = (s,n) => s.length >= n ? s + " " : (s + "                                                    ").slice(0, n)
            var rettp = (m[2] + m[3]).replace(/\s+/, "")
            inf.proto = fmt(rettp, 15) + fmt(name, 30) + fmt("(" + m[5] + ");", 40)
        }
    })

    if (nsStack.length != 1) {
        console.log("non-empty namespace stack", nsStack)
        process.exit(1)
    }
})

var ptrs = ""
var protos = ""
var functions = []

var funnames = Object.keys(basenames)
funnames.sort()
funnames.forEach(bn => {
    for (let p of prefixes) {
        let fn = p + bn
        let inf = fullfuns[fn]
        if (inf) {
            //basenames[bn] = inf
            inf.name = bn
            if (inf.full == "touch_develop::" + bn)
                delete inf.full;
            let tp = inf.type + inf.args
            if (inf.full == "bitvm::" + bn)
              tp += " bvm"
            if (inf.full == "bitvm::bitvm_" + bn)
              tp += " over"
            ptrs += `(uint32_t)(void*)::${fn},  // ${tp} {shim:${bn}}\n`;
            functions.push(inf)
            protos += inf.proto + "// " + tp + "\n";
            break;
        }
    }
})

var metainfo = {
  functions: functions,
  enums: enums
}

function write(fn, cont)
{
    if (fs.existsSync(fn)) {
        var curr = fs.readFileSync(fn, "utf8")
        if (curr == cont)
            return;
    }
    fs.writeFileSync(fn, cont)
}

write("generated/pointers.inc", ptrs)
write("build/protos.h", protos)
write("generated/metainfo.json", JSON.stringify(metainfo, null, 2))


// vim: ts=4 sw=4
