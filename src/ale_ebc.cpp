#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include "ale_ebc.hpp"

using namespace std;

typedef std::runtime_error SRE;
typedef std::logic_error SLE;

/* This is used in conjunction with Parameter_Check_Json() to verify that the
    json has the correct keys and vals. Nearly all of the keys are required. */

static json ale_specs = {
    {"game_rom","S"},
    {"display_screen","B"},
    {"sound","B"},
    {"max_num_frames","I"},
    {"max_num_frames_per_episode","I"},
    {"color_averaging","B"},
    {"record_screen_dir","S"},
    {"record_sound_filename","S"},
    {"repeat_action_probability","D"},
    {"run_length_encoding","B"},
    {"bias", "B"},
    {"random_seed","I"},
    {"luminance_threshold","D"},
    {"screen_rows","I"},
    {"screen_cols","I"},
    {"pixel_refractory","D"},
    {"events_file","S"},
    {"input_type","S"},
    {"verbose","B"},
    {"Necessary", {"game_rom","max_num_frames_per_episode","input_type"}}};

namespace ale_ebc
{

    /** The constructor simply calls from_json() to create an initialized class instance. */

    Ale_Ebc::Ale_Ebc()
    {
        ifstream fin;
        json params;

        fin.open("./params/ale_ebc.json");
        if(fin.fail()){
            fprintf(stderr,"Ale_Ebc(): Couldn't open params file. Make sure .../ale_ebc/params/ale_ebc.json exists.\n");
            exit(1);
        }
        fin >> params;
        from_json(params);

    }

   /** The constructor simply calls from_json() to create an initialized class instance. */ 
    Ale_Ebc::Ale_Ebc(const json &j)
    {
        from_json(j);
    }

    /** clear() sets the instance's state to uninitialized. This is done by setting
    `player_state` to `UNINIT`, and clearing the `observation` vector.  Please see
    the Doxygen comments on the `Ale` class for a definition of how the application's
    state is determined from `player_state`.*/

    void Ale_Ebc::clear()
    {
        di.player_state = UNINIT;
        di.observation.clear();
        di.observation_ebc_log_ti.clear();
    }

    /** from_json() goes through the standard steps of error checking, 
    and then setting parameters. */

    void Ale_Ebc::from_json(const json &j)
    {

        clear();

        // Set some default ALE parameters that might not be specified in file or string json
        if(j.contains("display_screen"))
            ale.setBool("display_screen", j["display_screen"]);
        else   
            ale.setBool("display_screen", false); 

        if(j.contains("sound"))
            ale.setBool("sound",j["sound"]);
        else
            ale.setBool("sound",false);

        if(j.contains("max_num_frames"))
            ale.setInt("max_num_frames",j["max_num_frames"]);
        else    
            ale.setInt("max_num_frames",0); 
        
        if(j.contains("color_averaging"))
            ale.setBool("color_averaging", j["color_averaging"]);
        else
            ale.setBool("color_averaging", false); 
        
        if(j.contains("record_sound_filename"))
            ale.setString("record_sound_filename",j["record_sound_filename"]);
        else
            ale.setString("record_sound_filename",""); 

        if(j.contains("repeat_action_probability"))    
            ale.setFloat("repeat_action_probability",j["repeat_action_probability"]);
        else
            ale.setFloat("repeat_action_probability",false); 

        if(j.contains("run_length_encoding"))
            ale.setBool("run_length_encoding",j["run_length_encoding"]);
        else    
            ale.setBool("run_length_encoding",false);

        if(j.contains("record_screen_dir"))
            ale.setString("record_screen_dir",j["record_screen_dir"]);
        else
            ale.setString("record_screen_dir",""); 


        if(j.contains("max_num_frames_per_episode")){
            ale.setInt("max_num_frames_per_episode",j["max_num_frames_per_episode"]);
        }else{
            ale.setInt("max_num_frames_per_episode",300); 
            cout << "'max_num_frames_per_episode' unspecified; using default value of 300 timesteps per episode." << endl;
        }
  
        if(j.contains("bias"))
            si.bias = j["bias"];
        else
            si.bias = false;

        if(j.contains("verbose"))
            si.verbose = j["verbose"];
        else
            si.verbose = false;
        

        si.events_file = "";
        si.game_rom = j["game_rom"];
        si.input_type = j["input_type"];

        //If doing "ebc", set "luminance_threshold", "screen_rows", "screen_cols", and "pixel_refractory" (for "ebc_log_ti" only)
        if(si.input_type.find("ebc") != string::npos){
            if(j.contains("luminance_threshold")){
                si.luminance_threshold = j["luminance_threshold"];
            }else{
                si.luminance_threshold = 0.05; 
                cout << "'luminance_threshold' unspecified; using default value of 0.05." << endl; 
            }
            if(j.contains("screen_rows") && j.contains("screen_cols")){
                si.screen_rows = j["screen_rows"];
                si.screen_cols = j["screen_cols"];
            }else{
                si.screen_rows = 210;
                si.screen_cols = 160; 
                cout << "WARNING: Assuming game_rom screen dimensions of 210 rows x 160 cols. This should work for most game_roms, but there might be exceptions." << endl;
            }
            if(j.contains("pixel_refractory"))
                si.pixel_refractory = j["pixel_refractory"];
            else
                si.pixel_refractory = 0.001;

            if(j.contains("events_file")){
                si.events_file = j["events_file"];
                cout << "Writing simulated events to " << si.events_file << endl;
            }
        }else if(si.input_type.find("screen") != string::npos){
            si.luminance_threshold = 0;
            if(j.contains("screen_rows") && j.contains("screen_cols")){
                si.screen_rows = j["screen_rows"];
                si.screen_cols = j["screen_cols"];
            }else{
                si.screen_rows = 210;
                si.screen_cols = 160; 
                cout << "WARNING: Assuming game_rom screen dimensions of 210 rows x 160 cols. This should work for most game_roms, but there might be exceptions." << endl;
            }
        }else if(si.input_type == "RAM"){       
            si.luminance_threshold = 0;
            si.screen_rows = 0;
            si.screen_cols = 0;
        }else{
            //Make this more detailed
            throw SRE("from_json(): 'input_type' must be one of: RAM, screen_gray, screen_rgb, ebc_simple, ebc_log, ebc_log_ti"); 
        }
        
        di.reward = 0;
        di.player_state = ALIVE; /* This sets the data structure as initialized. */
    }


    /** to_json() adds the key/value pairs to the json. */

    void Ale_Ebc::to_json(json &j) const
    {
        if (di.player_state == UNINIT)
            throw SRE("to_json() called on uninitialized Ale");

        j["display_screen"] = ale.getBool("display_screen");
        j["sound"] = ale.getBool("sound");
        j["max_num_frames"] = ale.getInt("max_num_frames");
        j["max_num_frames_per_episode"] = ale.getInt("max_num_frames_per_episode");
        j["color_averaging"] = ale.getBool("color_averaging");
        j["record_screen_dir"] = ale.getString("record_screen_dir");
        j["record_sound_filename"] = ale.getString("record_sound_filename");
        j["repeat_action_probability"] = ale.getFloat("repeat_action_probability");
        j["run_length_encoding"] = ale.getBool("run_length_encoding");
        j["random_seed"] = ale.getInt("random_seed");
        j["bias"] = si.bias;
        j["game_rom"] = si.game_rom;
        j["input_type"] = si.input_type;
        j["luminance_threshold"] = si.luminance_threshold;
        j["screen_rows"] = si.screen_rows;
        j["screen_cols"] = si.screen_cols;
        j["events_file"] = si.events_file;
        j["verbose"] = si.verbose;
        j["pixel_refractory"] = si.pixel_refractory;
        
    }

    /** Seed the rng. */

    bool Ale_Ebc::seed(long long _seed)
    {
        if (di.player_state == UNINIT)
            throw SRE("seed() called on uninitialized Ale");

        di.rng_seed = _seed;
        ale.setInt("random_seed",_seed);
        return true;
    }

    /** make_ram_observations() creates the "observation" vector from 
    the current application state.  It's its own method, because it is
    called from both reset() and step(). */

    void Ale_Ebc::make_ram_observation()
    {
        int i;
        ALERAM ram;

        di.observation.clear();
        ram = ale.getRAM();

        /** The observation given to the agent is either the current state of the game's RAM -- 128 bytes -- or some subset of those 128 bytes. */

        for( i = 0; i < RAM_SIZE; i++){
            di.observation.push_back((int)ram.get(i));
        }

        /* Optional Bias Neuron */
        if (si.bias)
        {
            di.observation.push_back(1);
        }
    }

    /** make_screen_observations() creates the "observation" vector from 
    the current application state.  It's its own method, because it is
    called from both reset() and step(). */

    void Ale_Ebc::make_screen_observation()
    {
        int i;

        di.observation.clear();

        if(si.input_type == "screen_gray")
            ale.getScreenGrayscale(*(di.grayscale_output_buffer));
        else
            ale.getScreenRGB(*(di.grayscale_output_buffer));

        /** Take the screen's grayscale or RGB values (that we're assuming is 210 * 160) and cast them to ints so that we can return observation of ints */

        for( i = 0; i < (int)di.grayscale_output_buffer->size(); i++){
            di.observation.push_back((int)di.grayscale_output_buffer->at(i));
        }

        /* Optional Bias Neuron */
        if (si.bias)
        {
            di.observation.push_back(1);
        }
    }

    /** make_ebc_observations() creates the "observation" vector from 
    the current application   It's its own method, because it is
    called from both reset() and step(). */ 
    void Ale_Ebc::make_ebc_observation()
    {
        int i, tmpFrameNum, row, col, polarity = 0;
        double current_pxl = 0, previous_pxl = 0, delta_luminance = 0; 
        double epsilon = 0.0000005; //This is because taking the natural log of 0 results in explosion.
        char buffer[200];
        
        if(si.luminance_threshold == 0)
            throw SRE("'ebc' input type specified; threshold must be > 0"); 

        if(si.input_type != "ebc_simple" && si.input_type != "ebc_log")
            throw SRE("input_type must be one of 'ebc_simple' or 'ebc_log'");
        
        tmpFrameNum = ale.getFrameNumber();

        di.observation.clear();
        ale.getScreenGrayscale(*(di.grayscale_output_buffer));
        
        /** If first frame, we cannot simulate ebc events. Observation will simply be 0s. **/
        // Based on looking at image, 160 cols(x-val) with 210 rows(y-val)
        
        if (tmpFrameNum == 0){
            di.observation = vector <int>(si.screen_rows * si.screen_cols,0); 
        }else{
            for(i = 0; i < (si.screen_rows * si.screen_cols); i++){
                row = i / si.screen_cols; 
                col = i % si.screen_cols; 

                current_pxl = (si.input_type == "ebc_simple") ? di.grayscale_output_buffer->at(i) : log(di.grayscale_output_buffer->at(i) + epsilon);
                previous_pxl = (si.input_type == "ebc_simple") ? di.previous_frame_buffer->at(i) : log(di.previous_frame_buffer->at(i) + epsilon);
                delta_luminance = current_pxl - previous_pxl;

                if(si.input_type == "ebc_simple" && abs(delta_luminance) < si.luminance_threshold * 255){ //Have not crossed threshold * max_pixel value, move on.
                    di.observation.push_back(0);
                    continue;
                }else if(si.input_type == "ebc_log" && abs(delta_luminance) < si.luminance_threshold){
                    di.observation.push_back(0);
                    continue;
                }

                polarity = (delta_luminance > 0) ? 1 : -1;
                di.observation.push_back(polarity);
                
                if(si.events_file.length() > 0){
                    sprintf(buffer,"%d,%d,%d,%d",tmpFrameNum,col,row,polarity);
                    di.cached_events.push_back(buffer);
                }
                
            }
        }

        ale.getScreenGrayscale(*(di.previous_frame_buffer));
        
        /* Optional Bias Neuron - This will probably just look like a hot/noisy pixel*/
        if (si.bias)
        {
            di.observation.push_back(1);
        }

        return;
    }

    /** reset() starts the app over.  It creates the first observation. */

    bool Ale_Ebc::reset(vector<int> &observations, long long _seed)
    {

        if (di.player_state == UNINIT)
            throw SRE("reset() called on uninitialized Ale");

        seed(_seed);
        ale.loadROM(si.game_rom);
        si.legal_actions = ale.getLegalActionSet();

        /* Create the first/new observation to return as a vector */

        if(si.input_type == "RAM")
        {
            make_ram_observation();
        }
        else if(si.input_type == "screen_gray"){
            di.grayscale_output_buffer = new vector<unsigned char>(si.screen_rows * si.screen_cols,0);
            make_screen_observation();
        }
        else if(si.input_type == "screen_rgb"){
            di.grayscale_output_buffer = new vector<unsigned char>(si.screen_rows * si.screen_cols * 3,0);
            make_screen_observation();
        }
        else if(si.input_type.find("ebc") != string::npos){
            di.grayscale_output_buffer = new vector<unsigned char>(si.screen_rows * si.screen_cols,0);
            di.previous_frame_buffer = new vector<unsigned char>(si.screen_rows * si.screen_cols,0);
            di.cached_events.clear();
            make_ebc_observation();
        }else{
            throw SRE("reset(): Invalid observation type specified");
        }

        observations = di.observation;

        /* Outputs initial json information upon game creation/running. */

        output_reset_information();

        return true;
    }

    /** step() checks the action for correctness and sets the reward.
    It then determines whether the app is done, and if not, it 
    generates the next observation.  */

    bool Ale_Ebc::step(const double action,
                      vector<int> &observations,
                      double &reward)
    {
        int i;
        ofstream fout;

        /* Error check */

        if (di.player_state == UNINIT)
            throw SRE("step() called on uninitialized Ale");
        if (di.observation.size() == 0)
            throw SRE("reset() needs to be called before step().");
        if (di.player_state != ALIVE)
            throw SRE("step() called on completed Ale");
        if (si.input_type == "ebc_log_ti")
            throw SRE("step() called with 1D vector and with 'ebc_log_ti' observation type is not legal; use 2D vector");

        /** Capture the reward returned by ale being given the agent's action. Add that reward to the reward of the total game.*/

        reward = ale.act(si.legal_actions[(int)action]); 
        di.agent_action = action;
        di.reward += reward;

        
        if (di.player_state != ALIVE || ale.game_over()) 
        {
            output_done_information();
            if(si.input_type.find("screen") != string::npos){
                delete di.grayscale_output_buffer;
            }else if(si.input_type.find("ebc") != string::npos){
                delete di.grayscale_output_buffer;
                delete di.previous_frame_buffer;

                if(si.events_file.length() > 0){
                    fout.open(si.events_file,ios::out);
                    if(fout.fail()){
                        throw SRE("Could not open events file to write cached events too\n");
                    }
                    for(i = 0; i < (int) di.cached_events.size(); i++){
                        fout << di.cached_events[i] << endl;
                    }
                    fout.close();
                }
            }

            return true;
        }

        /* Send information to the output stream */
        if(si.verbose)
            output_step_information();

        /* Generate new observation from internal state information */
        
        if(si.input_type == "RAM")
        {
            make_ram_observation();
        }
        else if(si.input_type.find("screen") != string::npos)
        {
            make_screen_observation();
        }
        else if(si.input_type.find("ebc") != string::npos){
            make_ebc_observation();
        }

        //print_for_debugging(); //Temporary for debugging

        observations = di.observation;
        return false; //Return false because we are not done
    }

    /** reset() starts the app over.  It creates the first observation for ebc_log_ti */

    bool Ale_Ebc::reset(vector< vector<int>> &observations, long long _seed)
    {

        if (di.player_state == UNINIT)
            throw SRE("reset() called on uninitialized Ale");
        if (si.input_type != "ebc_log_ti")
            throw SRE("reset() called with 2D vector and NOT 'ebc_log_ti' observation type is not legal; Change either observation type or the vector to 1D");

        seed(_seed);
        ale.loadROM(si.game_rom);
        si.legal_actions = ale.getLegalActionSet();

        /* Create the first/new observation to return as a vector */
        /* This function should only ever be invoked with ebc_log_ti observation type. */

        di.grayscale_output_buffer = new vector<unsigned char>(si.screen_rows * si.screen_cols,0);
        di.previous_frame_buffer = new vector<unsigned char>(si.screen_rows * si.screen_cols,0);
        di.crossings = new vector<double>(si.screen_rows * si.screen_cols,0);
        di.cached_events.clear();
        make_ebc_log_ti_observation();

        observations = di.observation_ebc_log_ti;

        /* Outputs initial json information upon game creation/running. */

        output_reset_information();

        return true;
    }

    /** step() checks the action for correctness and sets the reward.
    It then determines whether the app is done, and if not, it 
    generates the next observation. This is specifically for the ebc_log_ti observation type  */

    bool Ale_Ebc::step(const double action,
                      vector< vector<int> > &observations,
                      double &reward)
    {
        int i;
        ofstream fout;

        /* Error check */

        if (di.player_state == UNINIT)
            throw SRE("step() called on uninitialized Ale");
        if (di.observation_ebc_log_ti.size() == 0)
            throw SRE("reset() needs to be called before step().");
        if (di.player_state != ALIVE)
            throw SRE("step() called on completed Ale");
        if (si.input_type != "ebc_log_ti")
            throw SRE("step() called with 2D vector and NOT with ebc_log_ti observation type");

        /** Capture the reward returned by ale being given the agent's action. Add that reward to the reward of the total game.*/

        reward = ale.act(si.legal_actions[(int)action]); 
        di.agent_action = action;
        di.reward += reward;

        
        if (di.player_state != ALIVE || ale.game_over()) 
        {
            output_done_information();

            delete di.grayscale_output_buffer;
            delete di.previous_frame_buffer;
            delete di.crossings;

            if(si.events_file.length() > 0){
                fout.open(si.events_file,ios::out);
                if(fout.fail()){
                    throw SRE("Could not open events file to write cached events too\n");
                }

                for(i = 0; i < (int) di.cached_events.size(); i++){
                    fout << di.cached_events[i] << endl;
                }
                fout.close();
            }
            
            return true;
        }

        /* Send information to the output stream */
        if(si.verbose)
            output_step_information();

        /* Generate new observation from internal state information */
        /* This function should only ever be invoked with ebc_log_ti observation type. */
        make_ebc_log_ti_observation();

        //print_for_debugging(); //Temporary for debugging

        observations = di.observation_ebc_log_ti;
        return false; //Return false because we are not done
    }

        /** make_ebc_observations() creates the "observation" vector from 
    the current application   It's its own method, because it is
    called from both reset() and step(). */ 
    void Ale_Ebc::make_ebc_log_ti_observation()
    {
        int i, j, tmpFrameNum, row, col, max_spikes = 0, polarity = 0, spike_nums = 0;
        double current_pxl = 0, previous_pxl = 0, delta_luminance = 0, cross_update = 0, current_time = 0, time; 
        double delta_time = 1.0 / 60; //Ale produces frames with resolution of 60FPS. Therefore, delta time = 0.01666...
        double epsilon = 0.0000005; //This is because taking the natural log of 0 results in explosion.
        char buffer[200];
        
        if(si.luminance_threshold == 0)
            throw SRE("'ebc' input type specified; threshold must be > 0"); 

        if(si.input_type != "ebc_log_ti")
            throw SRE("Invoked incorrect overloaded reset()/step() functions.");

        tmpFrameNum = ale.getFrameNumber();
        time = delta_time * tmpFrameNum;
        max_spikes = (int)(delta_time / si.pixel_refractory); //Max time a pixel can spike over a given time interval

        di.observation_ebc_log_ti.clear();
        di.observation_ebc_log_ti = vector <vector <int> >(si.screen_rows * si.screen_cols,vector <int>(max_spikes,0));

        ale.getScreenGrayscale(*(di.grayscale_output_buffer));
        
        /** If first frame, we cannot simulate ebc events. Observation will simply be 0s. **/
        // Based on looking at image, 160 cols(x-val) with 210 rows(y-val)
        
        if (tmpFrameNum != 0){
            for(i = 0; i < (si.screen_rows * si.screen_cols); i++){
                row = i / si.screen_cols; 
                col = i % si.screen_cols;

                current_pxl = log(di.grayscale_output_buffer->at(i) + epsilon);
                previous_pxl = log(di.previous_frame_buffer->at(i) + epsilon); 
                delta_luminance = current_pxl - previous_pxl;

                if(abs(delta_luminance) < si.luminance_threshold) //Have not crossed threshold value for pixel, move on.
                    continue;

                polarity = (delta_luminance > 0) ? 1 : -1;

                //The rest below is used when we want to do temporal interpolation and inrcrease the resolution between frames
                //such that the higher the difference between current and previous pixel, the more events generated between the two
                //time frames (with some max_spikes upper limit).
                cross_update = polarity * si.luminance_threshold; 
                di.crossings->at(i) = log(di.crossings->at(i) + epsilon) + cross_update;

                spike_nums = (current_pxl - di.crossings->at(i)) / si.luminance_threshold;
                spike_nums = abs((int)spike_nums);

                spike_nums = (spike_nums > max_spikes) ? max_spikes : spike_nums; // How many spikes at this pixel
                current_time = (time - delta_time); 

                for(j=0; j < spike_nums; j++){
                    di.observation_ebc_log_ti[i][j] = polarity;
                     
                    if(si.events_file.length() > 0){
                        sprintf(buffer,"%lf,%d,%d,%d",current_time,col,row,polarity);
                        di.cached_events.push_back(buffer);
                    }
                    current_time += delta_time / spike_nums; 
                }
                
            }
        }

        ale.getScreenGrayscale(*(di.previous_frame_buffer));
        
        for(i = 0; i < (int)di.previous_frame_buffer->size(); i++){
            di.crossings->at(i) = double(di.previous_frame_buffer->at(i));
        }


        /* Optional Bias Neuron -- This will look like a hot/noisy pixel */
        if (si.bias)
        {
            di.observation_ebc_log_ti.push_back(vector <int>(max_spikes,1));
        }

        return;
    }

    
    /** This outputs viz information that we use at the end of a run. */

    void Ale_Ebc::output_done_information()
    {
        json done_params;

        if(si.verbose)
            output_step_information();
        cout << endl << "Game Over." << endl;
        done_params = json::object();
        done_params["reward"] = di.reward;
        cout << done_params << endl;
    }

    /** This takes the state information of the game at each timestep and 
        converts it to json for writing to an IO_Stream object */

    void Ale_Ebc::output_step_information()
    {
        json step_params;

        step_params = json::object();
        step_params["Lives"] = ale.lives();
        step_params["Current Reward"] = di.reward;
        step_params["Agent Action"] = di.agent_action;
        step_params["Timestep"] = ale.getFrameNumber();
        cout << step_params << endl;
    }

    /** This takes all of the initialized variables from the static_info, wraps
    them up in a json, and writes it to the IO_Stream. */

    void Ale_Ebc::output_reset_information()
    {
        json init_params;

        init_params = json::object();
        to_json(init_params);
        cout << init_params << endl;
    }

    /** This is simply some debugging code. */

    void Ale_Ebc::print_for_debugging()
    {

        printf("Current_Reward        %.3lf\n", di.reward);
        printf("Agent Action       %d\n", (int)di.agent_action);
        printf("\n");
    }

}; // namespace Ale
