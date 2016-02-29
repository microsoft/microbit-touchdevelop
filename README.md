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
`module.json`, do `yotta clean` *and* `yotta update`. Also, blast away
`yotta_modules` and `yotta_target`. Just to make sure.

The cloud compile service seems unhappy if the project doesn't compile (even
though main.cpp is meant to be always replaced). So make sure that the projects
compiles from a fresh checkout before pushing.

### Using local version of DAL in Touch Develop

* make sure Touch Develop is in `TouchDevelop` directory parallel to
  `microbit-touchdevelop` (for example, checkouts in `/c/microbit-touchdevelop`
  and `/c/TouchDevelop`
* in `TouchDevelop` build as usual and run `jake local`; then head to 
  `http://localhost:4242/editor/local/mbit.html?lite=www.microbit.co.uk`
* in `microbit-touchdevelop` do the yotta dance with GCC as described above
* in `microbit-touchdevelop` run either `make run` or `./run.sh`
* head to
  `http://localhost:4242/editor/local/mbit.html?lite=www.microbit.co.uk&microbit=local`
