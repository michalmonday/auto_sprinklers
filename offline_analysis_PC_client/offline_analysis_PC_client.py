
import pandas as pd
import matplotlib.pyplot as plt
import sys
import os
import numpy as np
import datetime
import matplotlib.dates as mdates
import argparse
import requests

parser = argparse.ArgumentParser(description="Offline analysis client for Auto Sprinklers")
parser.add_argument('--ip', type=str, default="192.168.0.83", help="IP address of the Auto Sprinklers ESP device")
parser.add_argument('--port', type=int, default=80, help="Port of the Auto Sprinklers ESP device")
parser.add_argument('--saved-logs-dir', type=str, default="saved_logs", help="Directory to save downloaded logs")
parser.add_argument('--days-to-show', type=int, default=3, help="Number of days to show in the plot (maximum)")
parser.add_argument('--force-redownload', action='store_true', help="Force re-download of log files even if they exist locally")
args = parser.parse_args()

if not os.path.exists(args.saved_logs_dir):
    os.makedirs(args.saved_logs_dir)

def get_file_list():
    try:
        response = requests.get(f"http://{args.ip}:{args.port}/api/list-files")
        response.raise_for_status()
        # it's csv list (each filename in new line)
        file_list = response.text.strip().split("\n")
        # return sorted by date, oldest first (yyyy-mm-dd.csv)
        return sorted(file_list)
    except requests.RequestException as e:
        print(f"Error fetching file list: {e}")
        return []

def download_file(filename):
    try:
        response = requests.get(f"http://{args.ip}:{args.port}/api/file", params={"name": f'/logs/{filename}'})
        response.raise_for_status()
        local_path = os.path.join(args.saved_logs_dir, os.path.basename(filename))
        with open(local_path, 'wb') as f:
            f.write(response.content)
        print(f"Downloaded {filename} to {local_path}")
        return local_path
    except requests.RequestException as e:
        print(f"Error downloading file {filename}: {e}")
        return None

file_list = get_file_list()[-args.days_to_show:]  # get only last N days
for filename in file_list:
    # only download a file if not already present
    local_path = os.path.join(args.saved_logs_dir, os.path.basename(filename))
    if not os.path.exists(local_path) or args.force_redownload:
        download_file(filename)

# Now read all downloaded files merge csv data and plot
all_data = []
for filename in file_list:
    local_path = os.path.join(args.saved_logs_dir, os.path.basename(filename))
    if os.path.exists(local_path):
        df = pd.read_csv(local_path)
        all_data.append(df)
    else:
        print(f"File not found: {local_path}")

# csvs have format:
# year,month,day,hour,minute,second,valve_open,temperature,soil_moisture,light_level,rain_detected,humidity
# 2025,8,25,15,42,29,0,26.10,0.40,30.58,0,44.00
# 2025,8,25,15,47,29,0,26.00,0.22,32.21,0,44.00
# 2025,8,25,15,52,29,0,26.00,0.33,32.06,0,45.00
# 2025,8,25,15,57,29,0,26.00,0.36,31.54,0,45.00
# 2025,8,25,16,2,29,0,26.10,0.43,31.54,0,45.00
# 2025,8,25,16,7,29,0,26.10,0.20,31.36,0,44.99
# 2025,8,25,16,12,29,0,26.10,0.47,30.96,0,45.00
# 2025,8,25,16,17,29,0,26.00,0.46,30.55,0,44.00

if all_data:
    df = pd.concat(all_data, ignore_index=True)
    # create a datetime column
    df['datetime'] = pd.to_datetime(df[['year', 'month', 'day', 'hour', 'minute', 'second']])
    df.set_index('datetime', inplace=True)

    fig, axs = plt.subplots(5, 1, figsize=(15, 20), sharex=True)

    # Plot valve state
    axs[0].step(df.index, df['valve_open'], where='post', label='Valve Open', color='blue')
    axs[0].set_ylabel('Valve State')
    axs[0].set_yticks([0, 1])
    axs[0].set_yticklabels(['Closed', 'Open'])
    axs[0].legend()

    # Plot temperature
    axs[1].plot(df.index, df['temperature'], label='Temperature (°C)', color='red')
    axs[1].set_ylabel('Temperature (°C)')
    axs[1].legend()

    # Plot soil moisture
    axs[2].plot(df.index, df['soil_moisture'], label='Soil Moisture', color='green')
    axs[2].set_ylabel('Soil Moisture')
    axs[2].legend()

    # Plot light level
    axs[3].plot(df.index, df['light_level'], label='Light Level', color='orange')
    axs[3].set_ylabel('Light Level')
    axs[3].legend()

    # Plot humidity
    axs[4].plot(df.index, df['humidity'], label='Humidity (%)', color='purple')
    axs[4].set_ylabel('Humidity (%)')
    axs[4].legend()

    # Format x-axis with date labels
    plt.xlabel('Date Time')
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.show()







