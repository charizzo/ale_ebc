#include "ale_ebc.hpp"
#include <nlohmann/json.hpp>

using namespace ale_ebc;
using nlohmann::json;

int main(){
    Ale_Ebc *test;
    vector <int> observations;
    bool done;
    double reward, total_reward,action;

    action = 2;

    /*Construct and initialize ALE object with json of required params*/
    test = new Ale_Ebc({{"game_rom","atari_bins/freeway.bin"},{"max_num_frames_per_episode",300},{"input_type","ebc_log"}});

    /*Let user seed the run by passing a seed to reset along with observations vector for initialization*/
    test->reset(observations,2098);

    done = false;
    total_reward = 0;
    while(!done){
        done = test->step(action,observations,reward);
        total_reward += reward;
    }

    printf("Total Reward: %f\n",total_reward);

    delete test;
    return 0;
}