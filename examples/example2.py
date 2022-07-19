import sys
import os
sys.path.append(os.path.join(os.getcwd(),"build"))
import ale_ebc

# Construct and initialize ALE object using params file
test = ale_ebc.Ale_Ebc()

# Let user seed the run by passing a seed to reset along with observations vector for initialization
# Use [[]] with ebc_log_ti input_type
rv, observations = test.reset([[]],2098)

done = False
total_reward = 0
action = 2
while done != True:
    done,observations,reward = test.step(action,[[]])
    total_reward += reward
    