import sys
import os
sys.path.append(os.path.join(os.getcwd(),"build"))
import ale_ebc
import json

# Load json file of only necessary params
data = json.load(open("params/ale_ebc_necessary.json"))
test = ale_ebc.Ale_Ebc(data) 

# Let user seed the run by passing a seed to reset along with observations vector for initialization
# Use [] with input types EXCEPT for ebc_log_ti
rv, observations = test.reset([],2098)

done = False
total_reward = 0
action = 2
while done != True:
    done,observations,reward = test.step(action,[])
    total_reward += reward