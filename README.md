# gdwebm

A **Godot 4.1+** GDExtension for playing **WebM** files.

## Purpose

The goal of `gdwebm` is to provide support for WebM files and all supported codecs (VP8, VP9, AV1, Opus, Vorbis) in Godot. This allows you to use modern video and audio formats in your projects unlike the built-in Ogg Theora support.

## Usage

> [!WARNING]
> This extension is currently in **major development** and is considered **unstable**.
> Use with caution.

To use this extension:
1. Clone this repository (`git clone https://github.com/cherrythecool/gdwebm.git`).
2. Build the extension using the provided script for your OS (`./build_mac.sh` for macOS for example) (see GDExtension documentation for more platform-specifics (if you want to build by hand, each thirdparty library may require different commands or tools).
4. Put a `.webm` in your project and use the extension.

## Build Requirements

* Anything the [Godot Documentation requires](https://docs.godotengine.org/en/latest/engine_details/development/compiling/index.html#building-for-target-platforms) for building the engine is required for building this GDExtension
* `cmake` (for some thirdparty libraries used, ex: libwebm)
* `meson` (for some thirdparty libraries used, ex: dav1d)

## Supported Features

Currently, the extension supports parsing the WebM container and decoding the following codecs:

### Video:
* AV1

### Audio:
* Opus

## Future Features

We plan to implement full support for all other standard WebM codecs:

### Video:
* VP8
* VP9

### Audio:
* Vorbis

## Credits

**libwebm**: WebM Project / The Chromium Authors
**libopus**: Xiph.Org Foundation
