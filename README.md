# FFMPEG PHP extension

This project was originally released on sourceforge.net. This distribution
includes patches applied by Townnews.com throughout the years to keep the
extension operational.

It previously only worked with FFMPEG 1.2.12 or older but has been modified
so that it compiles now against FFMPEG 3.4.0 (Cantor). This extension has
been tested against:

* PHP 5.6
* PHP 7.2.6

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
