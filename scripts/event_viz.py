import numpy as np
import pandas as pd 
import plotly.express as px
import argparse

def read_txt_ec_data(datafile, input_type, rounding=2):
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
    timestamps.append(np.min(timestamps))
    cols.append(1)
    rows.append(1)
    polarities.append(-1)
    timestamps.append(np.min(timestamps))
    cols.append(160)
    rows.append(210)
    polarities.append(-1)

    # Since events_file might be missing certain timesteps because there is no event activity, add a sentinel event at each timestep
    # between np.min(timestamps) and np.max(timestamps) in order to avoid massive temporal jumps in scatter plot viz
    if input_type != "ebc_log_ti":
        for i in range(int(np.min(timestamps)),int(np.max(timestamps))):
            if i not in np.unique(timestamps):
                timestamps.append(i)
                cols.append(160)
                rows.append(210)
                polarities.append(-1)
                timestamps.append(i)
                cols.append(1)
                rows.append(1)
                polarities.append(1) 
    '''
    else:
        for i in np.arange(np.min(timestamps),np.max(timestamps),timestamps[1]-timestamps[0]):
            if i not in np.unique(timestamps):
                timestamps.append(i)
                cols.append(160)
                rows.append(210)
                polarities.append(-1)
                timestamps.append(i)
                cols.append(1)
                rows.append(1)
                polarities.append(1)
                print(i)
    '''


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

    if input_type != "ebc_log_ti":
        df = df.sort_values(by=['timestamp'])

    minClockTime = df['timestamp'][0]
    df['relTime'] = df['timestamp'] - minClockTime
    df['roundedTime'] = np.around(df['relTime'], rounding)

    return df

def event_scatter_plot(df, title):
    fig = px.scatter(df,
                    x='x',
                    y='y',
                    color='polarity',
                    animation_frame='relTime',
                    title=title,
                    range_x=[0,160],
                    range_y=[210,0])
    fig.update_yaxes(autorange="reversed")
    fig.layout.updatemenus[0].buttons[0].args[1]["transition"]["duration"] = 0
    fig.show()
    
if __name__ == '__main__': 
    parser = argparse.ArgumentParser(description="Event-Based Camera Simulation Scatter Plot")
    parser.add_argument("--events_file","-e",type=str,required=True)
    parser.add_argument("--input_type","-i",type=str,default="ebc_simple",choices=["ebc_simple","ebc_log","ebc_log_ti"],required=False)
    args = parser.parse_args()

    df = read_txt_ec_data(args.events_file,args.input_type)
    event_scatter_plot(df, 'ALE_EBC Plot')