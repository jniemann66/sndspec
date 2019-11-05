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
--help                                            Help
~~~

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

