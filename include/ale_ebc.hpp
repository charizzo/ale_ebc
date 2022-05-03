#include <ale_interface.hpp>
#include <set>
#include "nlohmann/json.hpp"

using nlohmann::json;
using std::string;
using std::vector;
using std::set;

namespace ale_ebc
{


   /** These are the valid player states. If you change this, please make sure that
   you update the member variable Static_Info::player_state_strings and PlayerStateSize.*/

    enum PlayerState
    {
        UNINIT,
        ALIVE,
        TIME
    };


    /** `Static_Info` contains the base information about the application that is read from the JSON.
    Once it is initialized, it does not change. */

    class Static_Info
    {
    public:
        ActionVect legal_actions;            /**< The set of legal actions for the particular game in question. */
        string game_rom;                     /**< This is the path to the Atari game ROM file that is to be emulated. */
        bool bias;                           /**< Determines whether or not to include a bias neuron in the observation space. */
        vector<string> player_state_strings; /**< The player states as strings */
        string input_type;                   /**< Used to determine whether "raw" byte values are fed to the network, or "relative" values are fed. */
        double luminance_threshold;          /**< Used for si.input_type = "ebc" only. Decimal percentage value that determines whether or not an event occurs at a pixel location. Higher value means less events are noticed (lower sensititvity) and vice versa.*/
        int screen_rows;                     /**< Used for one of three ebc input options -- num rows in a frame received by ale.getScreenGreyscale(). */
        int screen_cols;                     /**< Used for one of three ebc input options -- num cols in a frame received by ale.getScreenGreyscale(). */
        string events_file;                  /**< If specified in json file, this is the file to write out the simulated ebc events to. */
        bool verbose;
        double pixel_refractory;
    };

    /** `Dynamic_Info` contains the information that changes as the application runs.  In theory,
    we could have one `Static_Info` and multiple `Dynamic_Info` instances, each in its own thread.
    However, we're not doing that just yet. */

    class Dynamic_Info
    {
    public:
        PlayerState player_state;         /**< See the PlayerState definition for values and meanings. */
        vector<int> observation;       /**< In make_ram_observation(), we turn the game state into the observation vector that is returned from step() and reset(). */ 
        vector< vector <int> > observation_ebc_log_ti; 
        vector<unsigned char> *grayscale_output_buffer;
        vector<unsigned char> *previous_frame_buffer;
        vector<double> *crossings;
        vector<string> cached_events;
        double reward;                    /**< This is the total reward accumulated by the agent. */
        double agent_action;              /**< This is one of the 18 actions that the agent can choose. */
        long long rng_seed;               /**< Seed for the rng */
    };

    class Ale_Ebc
    {
    public:
        Ale_Ebc();
        Ale_Ebc(const json &j);

        bool reset(vector<int> &observations, long long _seed);
        bool reset(vector< vector<int> > &observations, long long _seed); //For ebc_log_ti
        bool step(const double action,
                  vector<int> &observations,
                  double &reward);
        bool step(const double action,                         //For ebc_log_ti
            vector< vector <int> > &observations,
            double &reward);

    protected:
        Static_Info si;              /**< Information about the app that doesn't change. */
        Dynamic_Info di;             /**< Information that changes. */
        mutable ALEInterface ale;    /**< The mutable is because the const to_json functions calls non-const get___() functions from the Ale_Interface. */

        void from_json(const json &j);
        void to_json(json &j) const;
        void clear();
        bool seed(long long _seed);

        void make_ram_observation();       /**< Creates a vector of the observation from the current game RAM state */
        void make_screen_observation();    /**< Creates a vector of the observation from the current game's screen */
        void make_ebc_observation();       /**< Creates a vector of the observation from the current game's screen passed through ebc simulation */
        void make_ebc_log_ti_observation();
        void output_reset_information();   /**< Send reset info to output */
        void output_step_information();    /**< Send step info to output */
        void output_done_information();    /**< Send end-of-run info to output */
        void print_for_debugging();
    };

}; // end namespace ale_ebc
