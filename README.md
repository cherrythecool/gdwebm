# gdwebm

A **Godot 4.0+** GDExtension for playing **WebM** media files.

## Purpose

The goal of `gdwebm` is to provide native support for the WebM container and its associated codecs (VP8, VP9, AV1, Opus, Vorbis) directly within Godot Engine/Editor. This allows developers to use high-quality, open-format video and audio in their projects without relying on the built-in Ogg Theora support or external ffmpeg dependencies.

## Usage

> [!WARNING]
> This extension is currently in **ALPHA** and is considered **UNSTABLE**.
> Use with caution in production projects.

To use this extension:
1.  Clone this repository.
2.  Build the extension using `scons` (see generic GDExtension build instructions).
3.  Add the `VideoStreamWebM` resource to your Godot project.

## Supported Features

Currently, the extension supports parsing the WebM container and decoding the following streams:

*   **Container / Parsing**:
    *   [x] `.webm` / `.mkv` parsing (via `libwebm`)
*   **Audio**:
    *   [x] Opus (`A_OPUS`)
*   **Video**:
    *   [ ] *None (Placeholder visualization only)*

## Future Features

We plan to implement full support for all standard WebM codecs:

*   **Video**:
    *   [ ] VP8 (`V_VP8`)
    *   [ ] VP9 (`V_VP9`)
    *   [ ] AV1 (`V_AV1`)
*   **Audio**:
    *   [ ] Vorbis (`A_VORBIS`)

## Credits

*   **godot-cpp-template**: @godotengine
*   **libwebm**: WebM Project
*   **libopus**: Xiph.Org Foundation
