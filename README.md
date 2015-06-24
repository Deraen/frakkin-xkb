# Frakkin-XKB

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

Also LightDM tries it's best to mess up the keyboard layout, but this
should be fixable with `~/.Xkbmap`.

#### Using udev rule to call xkbcomp

Xkbcomp needs to run as user running the X session, this requires some
hacks. Additionally `RUN+=` rule in udev will block for duration of the
script and this causes Xorg to wait until the script is finished and then
to set the default options, overwriting our options.

### Solution

Call xkbcomp from `.xsession` and using `frakkin-xkb`:

xkblayout.sh:
```bash
#!/bin/bash
xkbcomp -I"$HOME/.xkb" "$HOME/.xkb/keymap/default" "${DISPLAY%%.*}"
```

.xsession:
```bash
xkblayout.sh
frakkin-xkb xkblayout.sh
```

Frakkin-xkb listens for XInput events and when new devices are adeed, calls the given command.
