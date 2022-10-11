# ALE_EBC Simulation Software Wrapper #

This repo is a wrapper around the Arcade Learning Environment (ALE) that faciliates training agents on classical Atari games/problems. The wrapper interacts with ALE on a frame-by-frame basis to simulate events as if they were being captured by an event-based camera. The different kind of observations that are supported by ALE are the current game state encoded as 128 bytes of RAM or a (typically 160\*210) image of the current frame. This wrapper allows for the user to select whether to use the 128 bytes of RAM as the observation, a grayscale or RGB image of the current frame, or events simulated between frame *i* and frame *i-1*.

## Compilation ##

1. Install ALE (follwing steps in ALE readme)
    1. Download and untar/unzip https://github.com/mgbellemare/Arcade-Learning-Environment/releases/tag/v0.6.1 in `.../ale_ebc/`
    2. Change the untarred/unzipped directory's name to `ALE`
    3. Install necessary packages:
        - Linux: `sudo apt-get libsdl1.2-dev libsdl-gfx1.2-dev libsdl-image1.2-dev cmake`
        - Mac: `brew install cmake sdl`
    4. `cd ALE` (`pwd` should be `.../ale_ebc/ALE`)
    5. Build the ALE framework:
        - 1: `mkdir build && cd build`
        - 2: `cmake -DUSE_SDL=ON -DUSE_RLGLUE=OFF -DBUILD_EXAMPLES=ON ..`
        - 3: `make -j 4`
    6. `cd ../..` (`pwd` should be `.../ale_ebc/`)
    7. Copy the ALE library into the microapp's lib/ directory: `cp ALE/libale.so lib/`
2. Install Nlohmann Json
    - Download https://github.com/nlohmann/json/releases/download/v3.10.5/json.hpp 
        - `mkdir .../ale_ebc/include/nlohmann`
        - `mv ~/Downloads/json.hpp .../ale_ebc/include/nlohmann`
3. `make clean ; make [all,examples,python]`

## Python Workflow ##

If you choose to use the python bindings, you will also need to download the following header file that enables translation between python json and nlohmann::json
- Download `https://github.com/pybind/pybind11_json/blob/master/include/pybind11_json/pybind11_json.hpp` and store it in `include/pybind11_json/`
    - In the header file, you might need to modify `#include "pybind11/pybind11.h"` to `#include <pybind11/pybind11.h>`

## Atari Rom Files ##

The atari bins needed to run the library are not distributed with the ale_ebc library. However, they can be downloaded from here: http://www.atarimania.com/top-atari-atari-2600-vcs-_G_2_7.html 

## params/ale_ebc.json ##

The ale_ebc library assumes `params/ale_ebc.json` exists and will try to read it as a config file if the empty constructor (Ale_Ebc()) is used. If it doesn't exist, user must pass a json object to the ale_ebc constructor with the two minimum necessary parameters (the rest will have default values). Refer to `.../ale_ebc/ALE/doc/manual/manual.pdf` for more detailed information about the ALE specific parameters. In my experimentation, there are a few that don't work as expected (e.g. `sound` and `display_screen`).

- `game_rom`: `"atari_bins/freeway.bin"` --  ALE param: The path to the romfile to run. 
- `display_screen`: `false` -- ALE param: Displays the game screen
- `sound`: `false` -- ALE param: Enable game sounds
- `max_num_frames`: `0` -- ALE param: The program will quit after this number of frames. 0 means never.
- `max_num_frames_per_episode`: `300` --  ALE param: Ends each episode after this number of frames. 0 means never. 
- `color_averaging`: `false` --  ALE param: Phosphor blends screens to reduce flicker
- `record_screen_dir`: `""` -- ALE param: Saves game screen images to save_directory. "" means don't save screenshots.
- `record_sound_filename`: `""` -- ALE param: Saves game sound file as a .wav. "" means don't save the game's sound file. There is a known bug with saving and syncing up the sound file with the game's screenshots-turned-video: https://github.com/mgbellemare/Arcade-Learning-Environment/issues/121; Furthermore, this funcitonality doesn't actually work both on Mac (doens't compile) and Linux (doesn't save .wav file)
- `repeat_action_probability`: `0` -- ALE param: Stochasticity in the environment. It is the probability the previous action will repeated without executing the new one.
- `run_length_encoding`: `true` --  ALE param: Encodes data using run-length encoding. Used for FIFO only (therefore, unused here)
- `bias`: `false` -- Implements an input bias neuron to prevent network stagnation (though is is probably not needed..)
- `input_type`: `"ebc_simple"` -- One of "RAM", "screen_gray", "screen_rgb", "ebc_simple", "ebc_log", "ebc_log_ti" 
- `luminance_threshold`: `0.05` -- For input_type "ebc_simple", "ebc_log", "ebc_log_ti": Threshold percentage between pixel at frame_i and frame_i-1 to determine whethere there's an event
- `screen_rows`: `210` -- For input_type "screen", "ebc_simple", "ebc_log", "ebc_log_ti": number of rows in each frame returned by ALE 
- `screen_cols`: `160` -- For input_type "screen", "ebc_simple", "ebc_log", "ebc_log_ti": number of columns in each frame returned by ALE 
- `pixel_refractory`: `0.001` -- For input_type "ebc_log_ti": refractory period of the camera pixels in seconds 
- `events_file`: `""` -- For input_type "ebc_simple", "ebc_log", "ebc_log_ti": Name of file to write simulated events to 
- `verbose`: `false` -- For all input types: Determines if json containing step information should be printed every timestep 

Recommended configurations of the parameters are in the `ale_ebc/params/ale_ebc.json` file. Of the ALE specific parameters, the most critical are `game_rom`, `max_num_frame_per_episode`, `input_type`, and `record_screen_dir` (if you'd like to view an agents performance on a testing seed). 

Defaults are set for all parameters except for `game_rom` and `input_type`, which the user must set. See examples below to see how to set parameters. 
 
## Usage/Examples ##

* `./bin/example`
    * Runs 300 timesteps of Freeway instance with ebc_log input_type. Action 2 (Move up) is hardcoded as the action. Demonstrates using json of necessary parameters instead of reading parameters from `params/ale_ebc.json`
* `./bin/example2`
    * Runs 300 timesteps of instance of Freeway instance with ebc_log_ti input_type. Action 2 (Move up) is hardcoded as the action. Uses `params/ale_ebc.json` to set all parameters. 
*  `python examples/example.py` 
    * Demonstrates how to use the library in python
    * Terminal output below:

            UNIX > python examples/example.py 
            A.L.E: Arcade Learning Environment (version 0.6.0)
            [Powered by Stella]
            Use -help for help screen.
            Warning: couldn't load settings file: ./ale.cfg
            'max_num_frames_per_episode' unspecified; using default value of 300 timesteps per episode.
            'luminance_threshold' unspecified; using default value of 0.05.
            WARNING: Assuming game_rom screen dimensions of 210 rows x 160 cols. This should work for most game_roms, but there might be exceptions.
            Game console created:
            ROM file:  atari_bins/freeway.bin
            Cart Name: Freeway (1981) (Activision) [!]
            Cart MD5:  8e0ab801b1705a740b476b7f588c6d16
            Display Format:  AUTO-DETECT ==> NTSC
            ROM Size:        2048
            Bankswitch Type: AUTO-DETECT ==> 2K


            WARNING: Possibly unsupported ROM: mismatched MD5.
            Cartridge_MD5: 8e0ab801b1705a740b476b7f588c6d16
            Cartridge_name: Freeway (1981) (Activision) [!]

            Running ROM file...
            Random seed is 2098
            {"bias":false,"color_averaging":false,"display_screen":false,"events_file":"","game_rom":"atari_bins/freeway.bin","input_type":"ebc_log","luminance_threshold":0.05,"max_num_frames":0,"max_num_frames_per_episode":300,"pixel_refractory":0.001,"random_seed":2098,"record_screen_dir":"","record_sound_filename":"","repeat_action_probability":0.0,"run_length_encoding":false,"screen_cols":160,"screen_rows":210,"sound":false,"verbose":false}

            Game Over.
            {"reward":1.0}

*  `python examples/example2.py`
    * Shows that in python, to use the ebc_log_ti input_type instead of the others, you pass an empty nested list ([[]]) as the observations argument


## Event Based Camera Observations ##

1. ebc_simple
    - Determines if an event occurs at a pixel at time *x* by taking the difference in the pixel's values between frames *x* and *x-1*. 
    - Same temporal resolution as ALE simulation ~60 FPS.
2. ebc_log
    - Determines if an event occurs at a pixel at time *x* by taking the difference in the pixel's *log*(values) between frames *x* and *x-1*.
    - Same temporal resolution as ALE simulation ~60 FPS. 
3. ebc_log_ti
    - Determines if events occurs at a pixel by taking the difference in the pixel's *log*(values) between frames *x* and *x-1*. The greater the difference between a pixel's log luminance intensity, the greater the amount of events that occur at that pixel. 
    - The maximum number of events that can occur at a specific pixel is governed by the `pixel_refractory` parameter.
        - **int max_events = (1 / FPS) / pixel_refractory --> (1 / 60) / 0.001 = 16**
    - Temporal resolution is increased by a factor of max_events 

