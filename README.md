# FFMPEG PHP extension

This project was originally released on sourceforge.net. This distribution
includes patches applied by Townnews.com throughout the years to keep the
extension operational.

This version is tested against PHP 5.6 - it presently does not support PHP
7 though this will happen sometime in the next year. It previously only
worked with FFMPEG 1.2.12 or older but has been modified so that it compiles
now against FFMPEG 3.4.0 (Cantor).

## Configuration

Some custom expansions on this version include support for setting some
resource limits via INI:

```ini
; Load the PHP extension
extension=ffmpeg.so

; Maximum threads used for processing videos
ffmpeg.threads=1

; Memory allocation size in bytes
ffmpeg.max_alloc=67108864
```
