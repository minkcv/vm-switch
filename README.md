# Vapor Spec VM for Nintendo Switch

An implementation of [Vapor Spec](https://github.com/minkcv/vm) for Nintendo Switch Homebrew.

Do you want to develop games for Windows, Linux, and Nintendo Switch Homebrew without having to make any changes to the game code at all? This is the place for you!

## Building the nro for the Switch
See the [devkitPro Getting Started page](https://devkitpro.org/wiki/Getting_Started).

Install dependencies

    pacman -S devkitA64 libnx switch-tools switch-sdl2

then, after cloning this repo,

    $ cd vm-switch
    $ make

Should produce `vm-switch.nro` for switch homebrew.

## Running on the Switch
Put the vm-switch.nro in the switch folder on your SD card.

Then put your Vapor Spec games in a folder called `vaporspec` in the `switch` folder on your SD card.

Make `gamelist.txt` in the vaporspec folder. The SD card should now look like this

    switch/
        vm-switch.nro
        vaporspec/
            gamelist.txt
            pong.bin

In the `gamelist.txt` file make a line with the following items separated by semicolons.

- The name of the game
- The filename of the game binary
- The filename of the game rom (optional)

Example `gamelist.txt` (note the last semicolon is still required even when no rom is used):

    pong by Will Smith;pong.bin;;
    mars by Will Smith;mars.bin;mars.rom;
    tetris by Will Smith;tetris.bin;;

I AM NOT RESPONSIBLE FOR DAMAGE OR DATA LOSS ON YOUR NINTENDO SWITCH
