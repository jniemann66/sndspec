# sndspec
*soundfile-to-spectrogram utility*

### usage

**sndspec filename [filename2 ...] [options]**

~~~
--dyn-range         Set Dynamic Range in dB
-o, --output-dir    Set Output directory
-h, --height        Set Image Height in Pixels
-w, --width         Set Image Width in Pixels
-t, --time-range    Set Time Range
--help              Help
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

- info, metadata
- directory traversal
- channel selection / sum
- move renderer to another library (?) (to free from dependence on Cairo)
- Quadmath
- long double
- Window function selection
- spectrums
- waveforms
- other color palettes
- log frequency scale

