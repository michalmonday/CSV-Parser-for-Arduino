'''
This script opens the port associated with Arduino, 
writes to it, reads response, and terminates once it receives
the "end" message.

Usage examples:
python arduino_serial.py --help
python arduino_serial.py --csv hurricanes.csv
python arduino_serial.py --csv hurricanes.csv --baudrate 115200
python arduino_serial.py --csv ../path_to_some_other_csv/data.csv --baudrate 9600

The sript requires "pyserial" library to be installed, 
it can be installed through pip:
python -m pip install pyserial
'''

import serial # pip install pyserial
import serial.tools.list_ports as list_ports
import time
import argparse

parser = argparse.ArgumentParser()
parser.add_argument(
        '--csv',
        metavar = '',
        required = True,
        type = argparse.FileType(mode='r'),
        help = 'CSV file to send through serial'
        )

parser.add_argument(
        '--baudrate',
        type = int,
        default = 115200,
        required = False,
        metavar = '',
        help = 'What BAUD rate (communication speed) to use. This must match "Serial.begin(<BAUD>);" in Arduino code (e.g. 9600, 115200).'
        )

args = parser.parse_args()

with open(args.csv.name, 'rb') as f:
    csv_bytes = f.read()

arduino_ports = []
print('All ports:')
for i, p in enumerate(list_ports.comports()):
    print(f'{i+1}. device={p.device} name={p.name} description={p.description} manufacturer={p.manufacturer}')
    # for k,v in vars(p).items():
    #     print(k,v)
    if "arduino" in str(p).lower():
        arduino_ports.append(p)

print()

if not arduino_ports:
    raise Exception('Arduino port was not found')

if len(arduino_ports) > 1:
    print('Multiple arduino ports were found:')
    for i, p in enumerate(arduino_ports):
        print(f'  {i+1}. device={p.device} name={p.name} description={p.description} manufacturer={p.manufacturer}')

    i = int(input('\nSelect the port to use\n> '))
    port = arduino_ports[i-1].device
else:
    print('A single arduino port was found and will be used')
    print(arduino_ports[0])
    port = arduino_ports[0].device

    
s = serial.Serial()
s.baudrate = 115200
s.port = port
s.open()

s.write(csv_bytes)
s.flush()

try:
    while True:
        time.sleep(0.01)
        if s.in_waiting:  # Or: while ser.inWaiting():
            in_str = ""
            while s.in_waiting:
                in_str += s.read().decode('utf-8')
            print('Received:')
            print(in_str)
            if in_str.endswith('end'):
                exit()
finally:
    print('Closing port')
    s.close()
    
