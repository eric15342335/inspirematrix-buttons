import serial
import pyaudio
# Configure serial port on windows
# macos: /dev/ttyACM0 ?
ser = serial.Serial('COM3', 115200)
# Configure PyAudio
p = pyaudio.PyAudio()
stream = p.open(format=pyaudio.paUInt8,
                channels=1,
                rate=6400,
                output=True)
# Main playback loop
try:
    while True:
        # Read data from serial port
        data = ser.read(16)  # Increased buffer size for smoother playback
        stream.write(data)
except KeyboardInterrupt:
    print("Playback stopped.")
finally:
    # Clean up
    stream.stop_stream()
    stream.close()
    p.terminate()
    ser.close()
