#include "ale_ebc.hpp"

using namespace ale_ebc;

int main(){
    Ale_Ebc *test;
    //vector < vector<int> > observations;
    vector <int> observations;
    bool done;
    double reward, total_reward,action;

    action = 2;

    /*Construct and initialize ALE object with params file*/
    test = new Ale_Ebc();

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