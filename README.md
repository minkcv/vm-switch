# Vapor Spec VM for Nintendo Switch

An implementation of [Vapor Spec](https://github.com/minkcv/vm) for Nintendo Switch Homebrew.

Do you want to develop games for Windows, Linux, and Nintendo Switch Homebrew without having to make any changes to the game code at all? This is the place for you!

## Building
See the [devkitPro Getting Started page](https://devkitpro.org/wiki/Getting_Started).

Install dependencies

    pacman -S devkitA64 libnx switch-tools switch-sdl2

then, after cloning this repo,

    $ cd vm-switch
    $ make

Should produce an nro for switch homebrew.

I AM NOT RESPONSIBLE FOR DAMAGE OR DATA LOSS ON YOUR NINTENDO SWITCH
