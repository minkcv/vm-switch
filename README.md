# Vapor Spec for Nintendo Switch

An implementation of [Vapor Spec](https://github.com/minkcv/vm) for Nintendo Switch Homebrew.

Vapor Spec is a virtual game platform. You can develop games for Vapor Spec on a desktop and the games can be run on Windows, Linux, and the Nintendo Switch.

## Building the Vapor Spec platform for the Switch
You can skip this and go to the section below if you just want to run games.

See the [devkitPro Getting Started page](https://devkitpro.org/wiki/Getting_Started).

Install dependencies

    pacman -S devkitA64 libnx switch-tools switch-sdl2 switch-mesa switch-glad switch-glm

then, after cloning this repo,

    $ cd vm-switch
    $ make

Should produce `vm-switch.nro` for switch homebrew.

## Running Vapor Spec on the Switch
See the [Releases Section](https://github.com/minkcv/vm-switch/releases) to download all the necessary files.

Put the `vm-switch.nro` and the `vaporspec` folder in the `switch` folder on your SD card.

The SD card should now look like this:

    switch/
        vm-switch.nro
        vaporspec/
            gamelist.txt
            pong.bin
            (other game files)

You can add other games by adding a line in `gamelist.txt` with the following items separated by semicolons.

- The name of the game
- The filename of the game binary
- The filename of the game rom (optional)

Example `gamelist.txt` (note the last semicolon is still required even when no rom is used):

    pong;pong.bin;;
    mars;mars.bin;mars.rom;
    tetris;tetris.bin;;

I AM NOT RESPONSIBLE FOR DAMAGE OR DATA LOSS ON YOUR NINTENDO SWITCH
