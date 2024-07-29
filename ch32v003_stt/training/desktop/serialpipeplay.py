import serial
import pyaudio
import statistics
# Configure serial port on windows
# macos: /dev/ttyACM0 ?
ser = serial.Serial('COM3',230400)
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
        data = ser.read(1024)  # Increased buffer size for smoother playback
        data_array = list(map(int, list(data)))
        data_avg = int(sum(data_array) / len(data_array))
        data_min = min(data_array)
        data_max = max(data_array)
        data_sd = statistics.stdev(data_array)
        print(f"{data_array}, avg={data_avg}, min={data_min}, max={data_max}, sd={data_sd}")
        stream.write(data)
except KeyboardInterrupt:
    print("Playback stopped.")
finally:
    # Clean up
    stream.stop_stream()
    stream.close()
    p.terminate()
    ser.close()
