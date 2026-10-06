# About
This is a program to enable global hotkeys in LiveSplit running with Wine/Proton on Linux, similar to how the feature functions on Windows. It reads inputs from a device of your choosing and sends corresponding commands to LiveSplit using its built-in TCP server. You have to build it yourself currently, and most things are hard-coded for now, but it's still extremely simple, all things considered.

This was built for keyboards (as was LiveSplit itself), but technically you can assign gamepad hotkeys and more, you just need to find the right input device. Currently you can only read 1 device per build of the program.

# Building
<!--TODO: rewrite this part for non-keyboards-->
Simply clone the repo to a location of your choosing, then navigate to the directory in a terminal and run `make`. If you want to edit the default key bindings, those are set in the `commandMaps` struct. The first key is the regular keybinding, and the second is a modifier key you need to hold down (e.g. Shift+KeypadMinus, the default for resetting splits). A modifier of `-1`  means the modifier is disabled.

For a list of valid keys, reference the macros beginning with "KEY_" [here](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/uapi/linux/input-event-codes.h).

# Usage

1. Enable the LiveSplit TCP server

You can either do this manually each time under `Control > Start TCP Server`, or you can enable `Settings > LiveSplit Server > Startup Behavior: > Start TCP Server` once and then forget about it.

2. Disable native hotkeys

Set any hotkeys that you plan to use with this program to "None" in the LiveSplit settings (press Esc while remapping the key). Otherwise you'll get double inputs while LiveSplit is in focus.

3. Find your device's file descriptor

Typically you can find these located somewhere under `/dev/input`. It'll probably be easiest to find it under `/dev/input/by-id`. Keyboards will likely be a file ending with "kbd".

If your device isn't there, it will still be under `/dev/input` as a file called `event#`, where `#` is a number. To find which, try running `cat /proc/bus/input/devices`, find the entry with `N: Name="[your_device_name]"`, then look for its `H: Handlers=[...] event#`.

4. Back in the directory where you cloned this repo, run the `ghk` binary with the full path to your keyboard file descriptor as the first argument. You can optionally add a different host IP and port for the LiveSplit server as the second and third arguments if you need something other than the default, but if you're not sure, you don't.

- `./ghk /dev/input/by-id/your-keyboard-kbd`
- `./ghk /dev/input/event22`

From here everything should just work. You can kill the program when you're done with Ctrl+C.

# Plans

In the future I'd like to make it so you don't need to build this yourself, plus I'd like to move the things that are hard coded, like the key bindings, to a separate config file. Oh, and it'd also be cool if you didn't have to go digging for your keyboard file descriptor manually.

# Misc

No LLMs were involved at any point in this project. Contributions are welcome so long as all code is 100% human written.
