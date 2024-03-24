from coToDis import calculate_distance
#Variables
temps = []
pressures = []
altitude = []
lattitude = []
longditude = []
locations = []
filepath = "data.txt.txt"


datafile = open(filepath, "r")

lines = datafile.readlines()
for line in lines:
    #Temp
    if "temperature" in line.lower():
        line = line.replace("Temperature:", "")
        line = line.replace("C", "")
        
        temps.append(line)
    #Pressure
    if "pressure" in line.lower():
        line = line.replace("Pressure:","")
        line = line.replace("hPa", "")
        pressures.append(line)
    #altitude
    if "altitude" in line.lower():
        line = line.replace("Altitude:","")
        line = line.replace("m","")
        altitude.append(line)
    #locations
    if "location" in line.lower():
        line = line.replace("Location:","")
        locSplit = line.split(",")
        lat = float(locSplit[0])
        lon = float(locSplit[1])
        locations.append((lat, lon))
        
print("Temps")
for i in temps:
    print(i)
print("Pressures")
for i in pressures:
    print(i)
print("altitudes")
for i in altitude:
    print(i)

totaldistance = 0
for i in range(len(locations)-1):
    
    lat1, lon1 = locations[i]
    lat2, lon2 = locations[i+1]
    print(lat1, lon1, lat2, lon2)
    distance = calculate_distance(lat1, lon1, lat2, lon2)
    totaldistance += distance
    

print(totaldistance)
        
