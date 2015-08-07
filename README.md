### Instructions.

- Install Yotta http://docs.yottabuild.org/#installing
- Install [srecord](http://srecord.sourceforge.net/); add it to your path

### Building with ARMCC

Install KEIL v4, do the license dance, figure things out, add to your path, etc.

```
yotta target bbc-microbit-classic-armcc
yotta login
yotta update
yotta build
```

### Building with GCC

Patch `module.json` so that `microbit-dal` points to `#experimental` instead of
`#master`.

```
yotta target bbc-microbit-classic-gcc
yotta login
yotta update
yotta build
```

### Notes

Yotta doesn't clean up properly when: switching targets, switching branches in
module.json. Always do `yotta clean` just to make sure. If you modified
`module.json`, do `yotta clean` *and* `yotta update`.