import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime

# File we get data from
filename = 'temps.csv'

# Variables to store data
times = []
light = []
temps = {
    "temp1": [], # Bottom Back Right
    "temp2": [], # Top Back Right
    "temp3": [], # Top Back Left
    "temp4": [], # Bottom Back Left
    "temp5": [], # Outside
    "temp6": [], # Top Front Right
    "temp7": [], # Bottom Front Right
    "temp8": [], # Top Front Left
    "temp9": [] # Bottom Front Left
}

# Load the data
with open(filename, 'r') as f:
    for line in f:
        line = line.strip()
        # Skip empty lines
        if line.startswith('#'):
            continue
        if line == '':
            continue
        # Split csv line
        parts = line.split(',')
        # Skip any lines where temperature reads less than -10, probably a bad read
        if (float(parts[2]) < -10 or
            float(parts[3]) < -10 or
            float(parts[4]) < -10 or
            float(parts[5]) < -10 or
            float(parts[6]) < -10 or
            float(parts[7]) < -10 or
            float(parts[8]) < -10 or
            float(parts[9]) < -10 or
            float(parts[10]) < -10):
            continue
        # Add time to times list as datetime time object
        times.append(mdates.date2num(datetime.strptime(parts[0].replace("-", "/"), '%Y/%m/%d/%H:%M:%S.%f')))
        # Add light to light list
        light.append(parts[1])
        # Add temperatures to temps list
        temps["temp1"].append(float(parts[2]))
        temps["temp2"].append(float(parts[3]))
        temps["temp3"].append(float(parts[4]))
        temps["temp4"].append(float(parts[5]))
        temps["temp5"].append(float(parts[6]))
        temps["temp6"].append(float(parts[7]))
        temps["temp7"].append(float(parts[8]))
        temps["temp8"].append(float(parts[9]))
        temps["temp9"].append(float(parts[10]))

# Convert the data to numpy arrays
times = np.array(times)
light = np.array(light)
temps["temp1"] = np.array(temps["temp1"])
temps["temp2"] = np.array(temps["temp2"])
temps["temp3"] = np.array(temps["temp3"])
temps["temp4"] = np.array(temps["temp4"])
temps["temp5"] = np.array(temps["temp5"])
temps["temp6"] = np.array(temps["temp6"])
temps["temp7"] = np.array(temps["temp7"])
temps["temp8"] = np.array(temps["temp8"])
temps["temp9"] = np.array(temps["temp9"])

# Create the plot
fig, ax1 = plt.subplots()
# Set labels
ax1.set_xlabel('Time')
ax1.set_ylabel('Temperature')
ax1.set_title('Temperature over time')

# Plot the data
ax1.plot(times, temps["temp1"], label="BBR")
ax1.plot(times, temps["temp2"], label="TBR")
ax1.plot(times, temps["temp3"], label="TBL")
ax1.plot(times, temps["temp4"], label="BBL")
ax1.plot(times, temps["temp5"], label="Outside")
ax1.plot(times, temps["temp6"], label="TFR")
ax1.plot(times, temps["temp7"], label="BFR")
ax1.plot(times, temps["temp8"], label="TFL")
ax1.plot(times, temps["temp9"], label="BFL")
plt.legend()

# Make a new axis for the light and plot it
ax2 = ax1.twinx()
ax2.set_ylabel('Light')
ax2.plot(times, light, label="light", linestyle='--', color='black', linewidth=0.2)

# Set the x axis to be dates
formatter = mdates.DateFormatter('%d - %H:%M')
ax1.xaxis.set_major_formatter(formatter)
fig.autofmt_xdate()

plt.show()