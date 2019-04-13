# ffalbum-split

### Ultra easy command line utility to split whole albums into particular songs using ffmpeg

### To quote the program's help message:
```
FFALBUM-SPLIT

This is a tool for splitting albums using ffmpeg.
A whole album audio file can be split into single tracks from the album.
(Ffmpeg required to be accessible by typing 'ffmpeg' in the system console.)
Usage: ffalbum-split <album audio file> <chart file>

album audio file - any audio file whose extension is supported by your version of ffmpeg
chart file - track chart for your album

The track chart needs to be written as in the following example:
h:m:s - <Track title>
h:m:s - <Another title>
        where 'h' stands for hours, 'm' stands for minutes and 's' stands for seconds.
The title of the track must not contain any quotation marks - "   Colons will also cause errors - :
You can also alternatively omit hours and use the following format:
m:s - <Track title>

It is required for the tracks in the chart to be listed in separate lines.

The album audio file is expected to be named in the following way:

<Band name> - <Album title>

The individual titles will then be named accordingly to that in the following manner:

<Band name> - <Track title>
```
