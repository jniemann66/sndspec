# sndspec
*soundfile-to-spectrogram utility*

### usage

**sndspec filename [filename2 ...] [options]**

~~~
Usage: sndspec filename [filename2 ...] [options]

--dyn-range <n>                                   Set Dynamic Range in dB
-o, --output-dir <path>                           Set Output directory
-h, --height <n>                                  Set Image Height in Pixels
-w, --width <n>                                   Set Image Width in Pixels
-t, --time-range <start-time> <finish-time>       Set Time Range in seconds
--white-background                                White Background (instead of black) with inverted heatmap palette
-W, --window <name>                               Set the window function
--show-windows                                    Show a list of available window functions
-r, --recursive                                   Recursive directory traversal
--help                                            Help
~~~

### notes

- input filenames can be either directories or files. If they are directories, all suitable files within them are processed. Combinations of files and directories are ok. 
- if recursive directory traversal is enabled, directories within directories will also be processed
- case and punctuation of window names is ignored. Kaiser window is the default, and it is tuned to the requested dynamic-range


### todo

~~grid, tickmarks - frequency axis~~

~~tickmarks - time axis~~

~~heat map legend~~

~~axis labels~~

~~heading~~

~~Output directory selection~~

~~input range selection~~

~~white background option~~

~~directory traversal~~

~~Window function selection~~

- info, metadata
- channel selection / sum
- move renderer to another library (?) (to free from dependence on Cairo)
- Quadmath
- long double
- spectrums
- waveforms
- other color palettes
- log frequency scale

