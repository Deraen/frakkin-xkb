# Frakking-XKB

For some reason in Ubuntu or Xorg in general it is somehow quite hard to
reliably configure custom XKB keymaps.

## Description

`xkbcomp` is a way to load custom keymaps from e.g. your home folder.
This allows one to customize keyboard layout beyond default options.

### Problems

#### Calling xkbcomp in .xsession

The options won't effect devices plugged in later.

#### Setting the XKB keymap and options in `/etc/default/keyboard`

This trigger a udev rule which sets some variables which are then used by
Xorg evdev driver. Problem is that using custom XKB rules with this will
require editing files in `/usr/share`.

#### Using udev rule to call xkbcomp

Xkbcomp needs to run as user running the X session, this requires some
hacks. Additionally `RUN+=` rule in udev will block for duration of the
script and this causes Xorg to wait until the script is finished and then
to set the default options, overwriting our options.

### Solution

Call xkbcomp from `.xsession` and using `frakking-xkb`:

xkblayout.sh:
```bash
#!/bin/bash
xkbcomp -I"$HOME/.xkb" "$HOME/.xkb/keymap/default" "${DISPLAY%%.*}"
```

```bash
$ frakking-xkb -t 1000 xkblayout.sh
```

You will probably need to use timeout of around 1000 milliseconds to allow
Xorg to register the input.
