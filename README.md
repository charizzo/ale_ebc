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
    - Download and unzip https://github.com/nlohmann/json/releases/download/v3.10.5/include.zip in `.../ale_ebc/include`
        - `mv include/include/nlohmann .`
3. `make clean ; make`

## params/ale.json ##

- `game_rom`: `"atari_bins/freeway.bin"` --  The path to the romfile to run
- `display_screen`: `false` -- Displays the game screen
- `sound`: `false` -- Enable game sounds
- `max_num_frames`: `0` -- The program will quit after this number of frames. 0 means never.
- `max_num_frames_per_episode`: `300` --  Ends each episode after this number of frames. 0 means never.
- `color_averaging`: `false` --  Phosphor blends screens to reduce flicker
- `record_screen_dir`: `""` -- Saves game screen images to save_directory. "" means don't save screenshots.
- `record_sound_filename`: `""` -- Saves game sound file as a .wav. "" means don't save the game's sound file. There is a known bug with saving and syncing up the sound file with the game's screenshots-turned-video: https://github.com/mgbellemare/Arcade-Learning-Environment/issues/121
- `repeat_action_probability`: `0` -- Stochasticity in the environment. It is the probability the previous action will repeated without executing the new one.
- `run_length_encoding`: `true` --  Encodes data using run-length encoding. Used for FIFO only (therefore, unused here)
- `bias`: `false` -- Implements an input bias neuron to prevent network stagnation (though is is probably not needed..)
- `input_type`: `"ebc_simple"` -- One of "RAM", "screen", "ebc_simple", "ebc_log", "ebc_log_ti"
- `luminance_threshold`: `0.05` -- For input_type "ebc_simple", "ebc_log", "ebc_log_ti": Threshold percentage between pixel at frame_i and frame_i-1 to determine whethere there's an event
- `screen_rows`: `210` -- For input_type "screen", "ebc_simple", "ebc_log", "ebc_log_ti": number of rows in each frame returned by ALE 
- `screen_cols`: `160` -- For input_type "screen", "ebc_simple", "ebc_log", "ebc_log_ti": number of columns in each frame returned by ALE 
- `pixel_refractory`: `0.001` -- For input_type "ebc_log_ti": refractory period of the camera pixels in seconds 
- `events_file`: `""` -- For input_type "ebc_simple", "ebc_log", "ebc_log_ti": Name of file to write simulated events to 
- `verbose`: `false` -- For all input types: Determines if json containing step information should be printed every timestep 

## Event Based Camera Observations ##
 
## Usage/Examples ##

* `./bin/example`
    * Runs 300 timesteps of instance of Freeway instane. Action 2 (Move up) is hardcoded into "actions" vector (that should probably just be an integer instead of a vector..)

## Future Work ##

1. Python bindings
2. Allow user to specify json object through constructor if they'd prefer to specify it on CL
3. Add more error checking logic to json initialization (logic errors or lack of necessary dependent parameters)