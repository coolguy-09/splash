# splash
A simple boot splash application.

## Credits
This project's main image loading core is provided by **stb_image.h** from Sean Barrett's **[nothings/stb](https://github.com/nothings/stb)** repository.

## What is splash?
splash is a simple boot splash application that is lightweight, fast and full of features.

## Features

### :page_facing_up: splash supports .png, .jpg/.jpeg, .bmp, .ppm, .tga, .gif (non-animated), and more. <br>
### :gear: splash includes tons of options like -c/--clear, -d/--duration, -v/--verbose and more. <br>
### :desktop_computer: splash directly uses the Linux framebuffer, offering broad compatibility with systems running different color depths and resolutions. <br>
### :feather: splash uses as little libraries as possible, making it super-lightweight.

## How to compile splash?
### First, lets install some tools:
For Debian/Ubuntu-based systems:
```bash
sudo apt install gcc binutils make
```
For Arch-based systems:
```bash
sudo pacman -S gcc binutils make
```
### After you downloaded the source code, just run:
```bash
make
```
### And...done, you have compiled splash successfully!

## How to install splash?
### There should be 4 files:
```
splash/
├── libsplash.so.1.0.1                    <- The library that splash uses
├── libsplash.so.1 -> libsplash.so.1.0.1  <- Symlink to the specific major version
├── libsplash.so -> libsplash.so.1        <- Symlink used by the linker at build time
└── splash                                <- The final executable program
```
### Copy the splash library, like this:
```bash
sudo cp -a libsplash.so* /usr/lib # or somewhere else
```
### Then update the linker config:
```bash
sudo ldconfig
```
### Copy the main splash binary, like this:
```bash
sudo cp -a splash /usr/bin # or somewhere else
```
### Congrats! You have fully installed splash to your system.

## How to use splash?
### You can see the usage by using --help:
```bash
splash --help
```
