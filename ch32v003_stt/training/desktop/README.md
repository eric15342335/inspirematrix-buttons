# desktop-linux app compantion to the ch32v003 code that writes 8-bit audio out over the UART

## Training

To train,  speak a digit, if it's recognized wrongly then press the correct
digit on the keyboard within 1 second of the result being reported to add
this new audio sample to the 'codebook'

## Put serial port to RAW mode

```bash
stty -F /dev/ttyACM0 raw
stty -F /dev/ttyACM0 -echo -echoe -echok
stty -F /dev/ttyACM0 230400
```

Note that once the serial port is put into RAW mode,
audio can be tested/listened back to with something like:
```bash
cat /dev/ttyACM0 > /tmp/audio.raw
(press CTRL-C once finished)
sox  -r 6400 -c 1 -b8 -eunsigned /tmp/audio.raw /tmp/audio.wav
play /tmp/audio.wav
```

(or just do `cat /dev/ttyACO` to get a rough visual representation of the
output from the microphone)
