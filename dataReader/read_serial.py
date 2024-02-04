import serial, json
import ast
from colorama import init, Fore, Back
import threading

class CONNECT:

    def __init__(self, port, baudrate) -> None:
        self.port = port
        self.baudrate = baudrate
        init(autoreset=True)
        if self.connect(port, baudrate):
            chartThread = threading.Thread(target = self.read())
            chartThread.start()
        else:
            print(f"{Fore.RED}Can't connect with {self.port}")

    def connect(self, port, baudrate):
        try:
            self.ser = serial.Serial(port, baudrate)
            return True
        except:
            return False

    def read(self):
        try:
            while True:
                data = self.ser.readline()
                if data:
                    decodedData = data.decode('utf-8')
                    print(f"{Fore.GREEN}--> {len(decodedData)} : {decodedData}")
                    rxData = json.loads(decodedData)
                    print(rxData['val'])
        except Exception as e:
            print(f"Wystąpił błąd: {e}")
        finally:
            self.ser.close()


conn = CONNECT('COM9', 115200)