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
2. Build the extension using the provided script for your OS (`./build_mac.sh` for macOS for example) (see GDExtension documentation for more platform-specifics, and use `cmake` for building thirdparty libraries if you're doing it manually).
4. Put a `.webm` in your project and use the extension.

## Supported Features

Currently, the extension supports parsing the WebM container and decoding the following codecs:

### Audio:
* Opus (`A_OPUS`)

## Future Features

We plan to implement full support for all standard WebM codecs:

### Video:
* [ ] VP8 (`V_VP8`)
* [ ] VP9 (`V_VP9`)
* [ ] AV1 (`V_AV1`)

### Audio:
* [ ] Vorbis (`A_VORBIS`)

## Credits

**libwebm**: WebM Project / The Chromium Authors
**libopus**: Xiph.Org Foundation
