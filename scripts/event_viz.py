import numpy as np
import pandas as pd 
import plotly.express as px
import argparse

def read_txt_ec_data(datafile, rounding=2): # 2 when using temporal interpolation. 1 when using discrete frame numbers
    timestamps = []
    rows = []
    cols = []
    polarities = []

    with open(datafile, 'r') as f:
        for line in f.readlines():
            timestamps.append(float(line.split(",")[0]))
            cols.append(int(line.split(",")[1]))
            rows.append(int(line.split(",")[2]))
            polarities.append(int(line.split(",")[3]))

    #Hack to prevent plotly axis autozoom
    timestamps.append(1)
    cols.append(1)
    rows.append(1)
    polarities.append(-1)

    # Data formatted as an array of events.
    # Each entry in the array: timestamp, x, y, polarity.
    print("X",np.min(cols), np.max(cols))
    print("Y",np.min(rows), np.max(rows))
    print("Unique Frames: %d" % len(np.unique(timestamps)))

    df = pd.DataFrame()
    df['timestamp'] = timestamps
    df['x'] = cols
    df['y'] = rows
    df['polarity'] = polarities
    df['polarity'] = df['polarity'].astype(str)


    minClockTime = df['timestamp'][0]
    df['relTime'] = df['timestamp'] - minClockTime
    df['roundedTime'] = np.around(df['relTime'], rounding)

    return df

def event_scatter_plot(df, title):
    fig = px.scatter(df,
                    x='x',
                    y='y',
                    color='polarity',
                    animation_frame='roundedTime',
                    title=title,
                    range_x=[0,160],
                    range_y=[210,0])
    fig.update_yaxes(autorange="reversed")
    fig.show()
    
if __name__ == '__main__': 
    parser = argparse.ArgumentParser(description="Event-Based Camera Simulation Scatter Plot")
    parser.add_argument("--events_file","-e",type=str,required=True)
    args = parser.parse_args()

    df = read_txt_ec_data(args.events_file)
    event_scatter_plot(df, 'ALE_EBC Plot')