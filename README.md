# vidgrok

vidgrok is a tool for realtime visualization of video data captured by low-cost logic analyzers. Its main intent is to display raw video signals of historic home computers. vidgrok uses [libsigrok](https://sigrok.org/)
 for capturing and [SDL2](https://www.libsdl.org/) for rendering.

## Installation / Building

```
# Install required dependencies (on Debian)
apt-get update
apt-get install -y meson libsigrokcxx-dev libglibmm-2.4-dev libsdl2-dev libcxxopts-dev g++

# Build and install
meson setup build
cd build
meson compile
meson install
```

## Usage

For displaying a video signal, [the logic analyzer](https://sigrok.org/wiki/Supported_hardware) needs to be connected to these signals:

- Ground
- Vertical sync (Default: Channel 0)
- Horizontal sync (Default: Channel 1)
- Pixel data (Default: Channel 2)

When calling without arguments, the following defaults are assumed:

```
vidgrok --sample-rate 12000000 --vsync 0 --hsync 1 --data 2 --width 800 --height 330
```

The detailed list of possible command line arguments can be obtained by

```
vidgrok --help
```

### Example (Robotron Z 1013)

The video signal of Robotron Z 1013 can be captured with a sample rate of 12 MHz by directly connecting to the logic ICs like this:

- VSync A6 Pin 12 (Channel 0)
- HSync A9 Pin 11 (Channel 1)
- Data A32 Pin 12 (Channel 2)

Calling vidgrok with these arguments produces the output below.

```
vidgrok --sample-rate 12000000 --vsync 0 --hsync 1 --data 2 --invert-data --highlight-vsync --highlight-hsync --width 800 --height 330
```

![Robotron Z 1013 video signal](doc/z1013_12mhz.png)

The blue area corresponds to the horizontal and the red area to the vertical blanking intervals.

### Example (VGA)

A really bad image can be obtained from VGA resolution output (640x480) with a sample rate of 24 MHz when directly connecting to the 15-pin D-sub connector:

- Ground Pin 6
- VSync Pin 14
- HSync Pin 13
- Data Pin 1 (Red)
 
```
vidgrok --sample-rate 24000000 --vsync 0 --hsync 1 --data 2 --invert-vsync --highlight-vsync --highlight-hsync --width 850 --height 500
```

Since the color/intensity data is an analog signal, only the bright pixels trigger the threshold for the input channel. The screenshot shows the MS DOS Editor.

![VGA 640x480 signal (DOS Editor)](doc/vga_640x480_24mhz.png)

## Authors

Stefan Schramm (<mail@stefanschramm.net>)

## License

[GPLv3](https://www.gnu.org/licenses/gpl-3.0)
