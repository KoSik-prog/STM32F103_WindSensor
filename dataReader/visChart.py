import threading
import serial
import ast
import time
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from colorama import Fore, init
import json

class CONNECT:
    global_ptX = []
    global_ptY = []
    avgWind = []

    def __init__(self, port, baudrate) -> None:
        self.port = port
        self.baudrate = baudrate
        init(autoreset=True)
        if self.connect(port, baudrate):
            readThread = threading.Thread(target=self.read)
            readThread.start()

            time.sleep(1)
            self.zeroPoint = [self.global_ptX[0], self.global_ptY[0]]

            ani = FuncAnimation(plt.gcf(), self.draw_chart, interval=100, save_count=10)
            plt.show()
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
                    rxData = json.loads(decodedData)
                    try:
                        self.avgWind = [rxData['avg']['X'], rxData['avg']['Y']]
                        for sensorReadData in rxData['val']:
                            self.global_ptX.append(sensorReadData['X'])
                            self.global_ptY.append(sensorReadData['Y'])
                    except json.JSONDecodeError as e:
                        print(f"{Fore.RED}JSON decode error: {e}")
        except Exception as e:
            print(f"Error: {e}")
        finally:
            self.ser.close()

    def draw_chart(self, maxPoints = 70, lim = 5000):
        if len(self.global_ptX) > maxPoints:
            self.global_ptX = self.global_ptX[-maxPoints:]
            self.global_ptY = self.global_ptY[-maxPoints:]

        if self.global_ptX and self.global_ptY:
            plt.clf()

            plt.scatter(self.global_ptX, self.global_ptY, s=20, c='gray')
            plt.scatter(self.avgWind[0], self.avgWind[1], s=100, c='red')
            plt.scatter(0, 0, s=10, c='green')

            print(f"Fluctuation X: {max(self.global_ptX) - min(self.global_ptX)} / Y: {max(self.global_ptY) - min(self.global_ptY)} AVG: {self.avgWind[0]} / {self.avgWind[1]}")

            plt.axis('on')
            plt.grid(True)
            plt.xlabel('X Axis')
            plt.ylabel('Y Axis')
            plt.title('Wind')
            plt.xlim(-lim, lim)
            plt.ylim(-lim, lim)

conn = CONNECT('COM9', 115200)
