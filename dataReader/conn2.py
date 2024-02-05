import numpy as np
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

            time.sleep(.5)
            self.zeroPoint = [self.global_ptX[0], self.global_ptY[0]]

            self.fig, self.ax = plt.subplots(subplot_kw={'projection': 'polar'})
            self.quiver = None
            self.bar = None
            self.ani = FuncAnimation(self.fig, self.draw_chart, interval=100, save_count=10)
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

    def draw_chart(self, frame):
        if self.quiver:
            self.quiver.remove()
        if self.bar:
            self.bar.remove()

        direction_rad = np.radians(self.points_to_direction(self.avgWind[0], self.avgWind[1]))
        rotated_direction_rad = np.mod(np.radians(90) - direction_rad, 2 * np.pi)

        self.bar = self.ax.bar(rotated_direction_rad, self.points_to_strength(self.avgWind[0], self.avgWind[1]), color='b', alpha=0.5, width=0.5, bottom=0)

        _, max_radius = self.ax.get_ylim()
        self.quiver = self.ax.quiver(rotated_direction_rad, 0, 0, max_radius, angles='xy', scale_units='xy', scale=1, color='r', width=0.03)

        self.ax.set_theta_zero_location('N')
        self.ax.set_theta_direction(-1)

        return self.quiver, self.bar

    def points_to_direction(self, x, y):
        angle_rad = np.arctan2(y, x)
        angle_deg = np.degrees(angle_rad)
        strength = (angle_deg + 360) % 360
        return strength
    
    def points_to_strength(self, x, y):
        return np.sqrt(x**2 + y**2)

            
            
conn = CONNECT('COM9', 115200)
