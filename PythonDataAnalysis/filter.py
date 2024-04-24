from coToDis import calculate_distance
import matplotlib.pyplot as plt

temps = []
pressures = []
altitude = []
lattitude = []
longditude = []
locations = []
degrees = []
distances = []
MagY = []
packetNum = 6
filepath = "data2.txt"
fig, ax = plt.subplots()

datafile = open(filepath, "r")

lines = datafile.readlines()

for line in lines:
    if "bmp" in line.lower():
        lineAr = line.replace(",", " ").split()
        temps.append(lineAr[1])
        pressures.append(lineAr[2])
        altitude.append(lineAr[3])
    if "gps" in line.lower():
        lineAr = line.replace(",", " ").split()
        lattitude.append(lineAr[1])
        longditude.append(line[2])
        locations.append((lineAr[1],lineAr[2]))
        degrees.append(lineAr[3])
    if "mag" in line.lower():
        lineAr = line.replace(",", " ").split()
        MagY.append(lineAr[1])

time = packetNum/2;
timestamp = []
for i in range (0, len(temps), 1):
    timestamp.append(i);

def plot(xLabel, yLabel, title, xpoints, ypoints, path):
    
    ax.set_xlabel(xLabel)
    ax.set_ylabel(yLabel)
    ax.set_title(title)

    plt.plot(xpoints, ypoints)
    plt.savefig(path)
    plt.show()


plot("Time(s)", "Pressure(hPa)", "Pressure over Time", timestamp, pressures, "presstime.png")

plot("Time(s)", "Temperature(C)", "Temperature over Time", timestamp, temps, "temptime.png")

plot("Time(s)", "Altitude(m)", "Altitude over Time", timestamp, altitude, "alttime.png")












        
