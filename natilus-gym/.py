import gym
import gym_natilus

env = gym.make('natilus-v0')

obs = env.reset()
print(obs) 
action = env.action_space.sample ()
obs, reward, done, info = env.step(action)
print(obs)
print(reward)
print(done)
