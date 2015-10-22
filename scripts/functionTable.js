"use strict";

if (process.argv.length < 3) {
  console.log("Genarete JSON metadata and .cpp file with function table.")
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
            nsStack.push({ ns: m[3], endMark: m[1] + "}" + (m[2] == "class" ? ";" : ""), lineNo: lineNo })
            justPushed = true;
        } else {
            if (top.endMark == ln) {
                nsStack.pop();
                return;
            }
        }

        m = /^(\s*)(\w+)([\*\&]*\s+[\*\&]*)(\w+)\s*\(([^\(\)]*)\)\s*(;\s*$|\{|$)/.exec(ln)
        if (top && m && !/^(else|return)$/.test(m[2])) {
            if (top.fnIndent != m[1]) {
                console.log("skip by indent:", ln)
                return;
            }
            var ns = nsStack.slice(1).map(s => s.ns).filter(s => !!s).join("::")
            if (ns == "")
                return;
            var name = ns + "::" + m[4]
            var tp = "F"
            if (m[2] == "void" && m[3].trim() == "")
                tp = "P"
            var args = 0
            if (m[5].trim() == "")
                args = 0
            else
                args = m[5].replace(/[^,]/g, "").length + 1

            fullfuns[name] = {
                name: name,
                type: tp,
                args: args,
                full: name
            }

            prefixes.forEach(p => {
                if (name.slice(0, p.length) == p &&
                    /::/.test(name.slice(p.length)))
                    name = name.slice(p.length)
            })
            basenames[name] = 1;
        }
    })

    if (nsStack.length != 1) {
        console.log("non-empty namespace stack", nsStack)
        process.exit(1)
    }
})

var ptrs = ""
var functions = []

Object.keys(basenames).forEach(bn => {
    for (let p of prefixes) {
        let fn = p + bn
        let inf = fullfuns[fn]
        if (inf) {
            // basenames[bn] = inf
            inf.name = bn
            if (inf.full == "touch_develop::" + bn)
                delete inf.full;
            ptrs += `(uint32_t)(void*)::${fn},  // ${inf.type + inf.args} {shim:${bn}}\n`;
            functions.push(inf)
            break;
        }
    }
})

var metainfo = {
  functions: functions,
  enums: enums
}


fs.writeFileSync("build/pointers.inc", ptrs)
fs.writeFileSync("build/metainfo.json", JSON.stringify(metainfo, null, 2))


// vim: ts=4 sw=4
