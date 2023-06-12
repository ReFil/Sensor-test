import serial
import csv
import datetime
import time

ser = serial.Serial('/dev/ttyUSB0')  # Replace with the correct serial port for your device
ser.flushInput()
# Open the CSV file for writing
filename = f"pm25_{datetime.datetime.now():%Y-%m-%d_%H-%M-%S}.csv"
with open(filename, 'w', newline='') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(["Date", "PM2.5 (µg/m³)"])

    while True:
        ser.write(bytes(b'\xAA\xB4\x06\x01\x00\x00\x00\x00\x00\xFF\xFF\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\x06\xAB'))
        while ser.inWaiting() < 10:
            pass
        data = ser.read(10)
        # Check that the response is valid
        if data[0] == 0xAA and data[1] == 0xC0 and sum(data[2:8]) % 256 == data[8] and data[9] == 0xAB:
            pm25 = (data[2] + data[3] * 256) / 10.0
            print(f"PM2.5: {pm25}")
            
            # Log the result to the CSV file
            writer.writerow([datetime.datetime.now(), pm25])
        else:
            print("Invalid response")
        
        # Wait for 1 second
        time.sleep(1)
