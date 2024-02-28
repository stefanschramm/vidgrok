# vidgrok

vidgrok is a tool for realtime visualization of data captured by low-cost logic analyzers. Its main intent is to display raw video signals of historic home computers. vidgrok uses libsigrok for capturing and SDL2 for rendering.

## Installation / Building

    # Install required dependencies (on Debian)
    apt-get update
    apt-get install -y meson libsigrokcxx-dev libglibmm-2.4-dev libsdl2-dev libcxxopts-dev g++
    
    # Build and install
    meson setup build
    cd build
    meson compile
    meson install

## Usage

    vidgrok

For displaying a video signal, the logic analyzer needs to be connected to these signals:

- Ground
- Vertical sync (Default: Channel 0)
- Horizontal sync (Default: Channel 1)
- Pixel data (Default: Channel 2)

### Example (Robotron Z 1013)

- Vertical sync: A6 Pin 12
- Horizontal sync: A9 Pin 11
- Pixel data: A22 Pin 10
- Sample rate: 12000000 Hz

## Authors

Stefan Schramm (<mail@stefanschramm.net>)

## License

[GPLv3](https://www.gnu.org/licenses/gpl-3.0)
