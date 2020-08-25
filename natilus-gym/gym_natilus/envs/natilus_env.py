import gym
import numpy as np
import queue
import copy
import math
import torch

from gym import spaces
from gym_natilus.envs.natilus import Server
from gym_natilus.envs import Generator

class NatilusEnv(gym.Env):
    """
    Gym Environment for Natilus.
    """
    def __init__(self):
        port = input("Port: ");
        self.server = Server(port)
        self.sensor_num = self.server._initialize()
        self.sensor_xnum = int(self.sensor_num ** 0.5)
        print("Sensor Num:", self.sensor_num)
        print("Sensor XNum:", self.sensor_xnum)

        self.obsMod = int(input("Obs Mode (1. track, 2. temp 3. multi): "))
        self.actMod = int(input("Action Mode (1. LA3 2. GAN): "))
        self.rlMod = int(input("RL Mode (1. General, 2. Transformer): "))
        self.infoNum = int(input("Info Num: "))
        self.history_num = int(input("History Num: "))
        self.reward_text = input("Reward Txt (.txt): ")
        
        # We have 9 Point here. Useed in Point Embedding.
        self.point_num = 9
        if self.sensor_xnum == 6:
            self.cell_num = 4
            self.point = [0, 1, 2, 0, 1, 2, 0, 1, 2]
            self.ypoint = [0, 0, 0, 1, 1, 1, 2, 2, 2]
        elif self.sensor_xnum == 8:
            self.cell_num = 4
            self.point = [0, 2, 4, 0, 2, 4, 0, 2, 4]
            self.ypoint = [0, 0, 0, 2, 2, 2, 4, 4, 4]
        elif self.sensor_xnum == 10:
            self.cell_num = 4
            self.point = [0, 3, 6, 0, 3, 6, 0, 3, 6]
            self.ypoint = [0, 0, 0, 3, 3, 3, 6, 6, 6]
        elif self.sensor_xnum == 12:
            self.cell_num = 6    
            self.point = [0, 3, 6, 0, 3, 6, 0, 3, 6]
            self.ypoint = [0, 0, 0, 3, 3, 3, 6, 6, 6]
        elif self.sensor_xnum == 14:
            self.cell_num = 6
            self.point = [0, 4, 8, 0, 4, 8, 0, 4, 8]
            self.ypoint = [0, 0, 0, 4, 4, 4, 8, 8, 8]
        elif self.sensor_xnum == 16:
            self.cell_num = 6
            self.point = [0, 5, 10, 0, 5, 10, 0, 5, 10]
            self.ypoint = [0, 0, 0, 5, 5, 5, 10, 10, 10]
        self.action_point = 16

        # Observiation & Action Space
        if self.rlMod == 1:
            self.observe_num = self.sensor_num
        else:
            self.observe_num = self.point_num*self.cell_num*self.cell_num
        print("Observe Num:", self.observe_num)
        if self.obsMod == 1:
            self.observation_space = spaces.Box (low=0, high=4, shape=(self.observe_num * self.history_num, self.infoNum), dtype=np.float32)
        elif self.obsMod == 2:
            self.observation_space = spaces.Box (low=-10, high=10, shape=(self.observe_num * self.history_num, self.infoNum), dtype=np.float32)
        elif self.obsMod == 3:
            self.observation_space = spaces.Box (low=0, high=1, shape=(self.observe_num * self.history_num, self.infoNum), dtype=np.float32)
        
        
        """if self.obsMod == 3:
            self.action_space = spaces.Box (low=-1, high=1, shape=((self.sensor_xnum-2)*(self.sensor_xnum-2),), dtype=np.float32)
            self.server.m_action_size = (self.sensor_xnum-2) * (self.sensor_xnum-2)
        else:"""
        #self.action_space = spaces.Box (low=-1, high=1, shape=(self.sensor_num,), dtype=np.float32)
        self.action_space = spaces.Box (low=-1, high=1, shape=(self.action_point,), dtype=np.float32) 
        
        self.history = []
        self.past = np.zeros((self.infoNum, self.sensor_xnum, self.sensor_xnum), dtype="f")  
        self.sum_reward = 0
        
        if self.actMod == 2:
            modelName = input("Gan Model Name (.tar):")
            device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
            self.generator = Generator.Generator().to(device)
            self.ganModel = torch.load("./ganModels/"+modelName)
            self.generator.load_state_dict(self.ganModel['model_state_dict'])
            self.generator.eval ()

    def step(self, action):
        if self.actMod == 2:
            """ Gan Model """
            ganInput = torch.cuda.FloatTensor(np.reshape(action, (1,16)))
            action = self.generator(ganInput)
            action = action.detach().cpu().numpy()
            action = action/np.sum(action)
            action = action.squeeze()
            action = np.reshape(action, (144))
  
        elif self.actMod == 1:
            """ LA3 Model """
            action = self.LA3(action)
            action = self.clipFunc(action)
            action = self.softmax(action)
        
        self.server._action (action)
        done = self.server._end()
        reward = self.server._reward() 
        obs = np.array(self.server._observation(), dtype=np.float32)

        self.sum_reward = self.sum_reward + reward;
        if self.obsMod == 2:
            obs = self.obs_figure_temp(obs)
        elif self.obsMod == 1:
            obs = self.obs_figure_track(obs)
        elif self.obsMod == 3:
            obs = self.obs_figure_multi(obs)

        if done:
            self.file = open(self.reward_text, 'a')
            data = "%f\n" % self.sum_reward
            self.file.write(data)
            print('Sum Reward: {}'.format(self.sum_reward))
            self.file.close()
         
        return obs, reward, done, {"None":1}
    
    def reset(self):
        self.history.clear()
        for i in range(self.history_num):
            self.history.append(np.zeros((self.observe_num, self.infoNum), dtype="f"))
        self.sum_reward = 0;
        
        obs = np.array(self.server._observation(), dtype=np.float32)
        if self.obsMod == 2:
            obs = self.obs_figure_temp(obs)
        elif self.obsMod == 1:
            obs = self.obs_figure_track(obs)
        elif self.obsMod == 3:
            obs = self.obs_figure_multi(obs)
        
        return obs
        
    def render(self):
        pass
    
    def obs_figure_temp(self, obs):
        temp = copy.deepcopy(obs)
        obs[0] = obs[0] - self.past[0]
        
        # Observation Pre-processing
        obs = np.round (obs)
        for i in range(self.sensor_xnum):
            for j in range(self.sensor_xnum):
                # change temp diff map
                if obs[0][i][j] >= 10:
                    obs[0][i][j] = 10
                if obs[0][i][j] <= -10:
                    obs[0][i][j] = -10

                # change last update map
                if obs[1][i][j] == 0:
                    obs[1][i][j] = 0
                elif obs[1][i][j] <= 33:
                    obs[1][i][j] = 1
                elif obs[1][i][j] <= 66:
                    obs[1][i][j] = 2
                elif obs[1][i][j] <= 99:
                    obs[1][i][j] = 3 
                else:
                    obs[1][i][j] = 4

        obs = np.reshape(obs, (self.infoNum, self.sensor_num))
        obs = np.transpose(obs, (1,0)) 
        
        # For Transformer Mode
        if self.rlMod == 2:
            tmp_obs = []
            q = -1
            for i in range(self.point_num):
                if i % 3 == 0:
                    q += 1 
                for j in range(self.cell_num):
                    for k in range(self.cell_num):
                        tmp_obs.append(copy.deepcopy(obs[self.point[i] + self.point[q]* self.sensor_xnum + self.sensor_xnum*j + k]))
            obs = np.array(tmp_obs)

        self.past = copy.deepcopy(temp)
        
        # For History Embedding
        if self.history_num != 1:
            del self.history[0]
            self.history.append(obs) 
            _obs = []
        
            for i in range(self.observe_num):
                for j in range(self.history_num):
                    _obs.append(self.history[j][i])
            obs = np.array(_obs, dtype=np.float32)

        return obs
    
    def obs_figure_track(self, obs):
        temp = copy.deepcopy(obs)
        #obs[1] = obs[1] - self.past[1]

        for i in range(self.sensor_xnum):
            for j in range(self.sensor_xnum):
                # change last udpate map
                if obs[1][i][j] == 0:
                    obs[1][i][j] = 0
                elif obs[1][i][j] <= 33:
                    obs[1][i][j] = 1
                elif obs[1][i][j] <= 66:
                    obs[1][i][j] = 2
                elif obs[1][i][j] <= 99:
                    obs[1][i][j] = 3 
                else:
                    obs[1][i][j] = 4
                
                if self.infoNum >= 3:
                    obs[2][i][j] = round(obs[2][i][j] / (60 * 4), 2)
                    if obs[2][i][j] >= 1:
                        obs[2][i][j] = 1
        
     
        obs[0] = self.smoothing(obs[0])       
        
        obs = np.reshape(obs, (self.infoNum, self.sensor_num))
        obs = np.transpose(obs, (1,0))
        
        if self.rlMod == 2:
            tmp_obs = []
            q = -1
            for i in range(self.point_num):
                if i % 3 == 0:
                    q += 1
                for j in range(self.cell_num):
                    for k in range(self.cell_num):
                        tmp_obs.append(copy.deepcopy(obs[self.point[i] + self.point[q]* self.sensor_xnum + self.sensor_xnum*j + k]))
            obs = np.array(tmp_obs)

        self.past = copy.deepcopy(temp)

        if self.history_num != 1:
            del self.history[0]
            self.history.append(obs)
        
            _obs = []
        
            for i in range(self.observe_num):
                for j in range(self.history_num):
                    _obs.append(self.history[j][i])
            obs = np.array(_obs, dtype=np.float32)

        return obs

    def obs_figure_multi(self, obs):
        temp = copy.deepcopy(obs)
        #obs[1] = obs[1] - self.past[1]
        
        for i in range(self.sensor_xnum):
            for j in range(self.sensor_xnum):
                # change last udpate map
                if obs[1][i][j] == 0:
                    obs[1][i][j] = 0
                elif obs[1][i][j] <= 10:
                    obs[1][i][j] = 0.1
                elif obs[1][i][j] <= 20:
                    obs[1][i][j] = 0.2
                elif obs[1][i][j] <= 30:
                    obs[1][i][j] = 0.3
                elif obs[1][i][j] <= 40:
                    obs[1][i][j] = 0.4
                elif obs[1][i][j] <= 50:
                    obs[1][i][j] = 0.5
                elif obs[1][i][j] <= 60:
                    obs[1][i][j] = 0.6
                elif obs[1][i][j] <= 70:
                    obs[1][i][j] = 0.7
                elif obs[1][i][j] <= 80:
                    obs[1][i][j] = 0.8
                elif obs[1][i][j] <= 90:
                    obs[1][i][j] = 0.9
                else:
                    obs[1][i][j] = 1

                # change change time map
                if obs[2][i][j] == 0:
                    obs[2][i][j] = 0
                elif obs[2][i][j] <= 33:
                    obs[2][i][j] = 1/4.0
                elif obs[2][i][j] <= 66:
                    obs[2][i][j] = 2/4.0
                elif obs[2][i][j] <= 99:
                    obs[2][i][j] = 3/4.0
                else:
                    obs[2][i][j] = 4/4.0

        # change multi object 
        """
        sum = 0
        for i in range(self.sensor_xnum):
            for j in range(self.sensor_xnum):
                if obs[0][i][j] > 0: 
                    sum += obs[0][i][j]
        if sum != 0:
            for i in range(self.sensor_xnum):
                for j in range(self.sensor_xnum):
                    if obs[0][i][j] > 0:
                        obs[0][i][j] = obs[0][i][j] / sum
            obs[0] = np.round(obs[0], 2)
        """
        sum = np.sum(obs[0])
        if sum != 0:
            obs[0] = obs[0] / sum
            obs[0] = np.round(obs[0], 2) 

        obs = np.reshape(obs, (self.infoNum, self.sensor_num))
        obs = np.transpose(obs, (1,0))

        if self.rlMod == 2:
            tmp_obs = []
            q = -1
            for i in range(self.point_num):
                if i % 3 == 0:
                    q += 1
                for j in range(self.cell_num):
                    for k in range(self.cell_num):
                        tmp_obs.append(copy.deepcopy(obs[self.point[i] + self.point[q]* self.sensor_xnum + self.sensor_xnum*j + k]))
            obs = np.array(tmp_obs)
        
        self.past = copy.deepcopy(temp)

        if self.history_num != 1:
            del self.history[0]
            self.history.append(obs)
        
            _obs = []
        
            for i in range(self.observe_num):
                for j in range(self.history_num):
                    _obs.append(self.history[j][i])
            obs = np.array(_obs, dtype=np.float32)

        return obs

    def smoothing(self, obs):
        x = -1
        y = -1
        for i in range(self.sensor_xnum):
            for j in range(self.sensor_xnum):
                if obs[i][j] == 1:
                    x = i
                    y = j
        
        if x != -1:
            for i in range(self.sensor_xnum):
                for j in range(self.sensor_xnum):
                    if i != x or  j != y:
                        d = (((x-i)*(x-i) + (y-j)*(y-j)) ** 0.5) * 2
                        obs[i][j] = round(1/d, 2)
        return obs

    def softmax(self, arr):
        exp_a = np.exp(arr)
        sum_exp_a = np.sum(exp_a)
        y = exp_a / sum_exp_a

        return y
    
    def clipFunc(self, action):
        _actions = []
        
        thresh = 0.5  
        for a in action: 
            if a <= thresh:
                _actions.append(-2)
            else:
                _actions.append(a*2)
        return _actions

    def LA3(self, action):
        _actions = []
        length = 1
        if self.sensor_xnum == 8:
            xcell = [1, 3, 5, 7, 1, 3, 5, 7, 1, 3, 5, 7, 1, 3, 5, 7]
            ycell = [1, 1, 1, 1, 3, 3, 3, 3, 5, 5, 5, 5, 7, 7, 7, 7]
        elif self.sensor_xnum == 12:
            xcell = [1.5, 4.5, 7.5, 10.5, 1.5, 4.5, 7.5, 10.5, 1.5, 4.5, 7.5, 10.5, 1.5, 4.5, 7.5, 10.5]
            ycell = [1.5, 1.5, 1.5, 1.5, 4.5, 4.5, 4.5, 4.5, 7.5, 7.5, 7.5, 7.5, 10.5, 10.5, 10.5, 10.5]
        elif self.sensor_xnum == 16:
            xcell = [2, 6, 10, 14, 2, 6, 10, 14, 2, 6, 10, 14, 2, 6, 10, 14]
            ycell = [2, 2, 2, 2, 6, 6, 6, 6, 10, 10, 10, 10, 14, 14, 14, 14]
        elif self.sensor_xnum == 20:
            xcell = [2.5, 7.5, 12.5, 17.5, 2.5, 7.5, 12.5, 17.5, 2.5, 7.5, 12.5, 17.5, 2.5, 7.5, 12.5, 17.5]
            ycell = [2.5, 2.5, 2.5, 2.5, 7.5, 7.5, 7.5, 7.5, 12.5, 12.5, 12.5, 12.5, 17.5, 17.5, 17.5, 17.5]
        elif self.sensor_xnum == 24:
            xcell = [3, 9, 15, 21, 3, 9, 15, 21, 3, 9, 15, 21, 3, 9, 15, 21]
            ycell = [3, 3, 3, 3, 9, 9, 9, 9, 15, 15, 15, 15, 21, 21, 21, 21]

        for i in range(self.sensor_xnum):
            for j in range(self.sensor_xnum):
                y = i*length + 1
                x = j*length + 1
                
                score = 0
                for k in range(self.action_point): 
                    cell_x = xcell[k] #self.point[k]*2 + (self.cell_num/2 * 2)
                    cell_y = ycell[k] #self.ypoint[k]*2 + (self.cell_num/2 *2)
                    dist = ((x-cell_x)*(x-cell_x)+(y-cell_y)*(y-cell_y)) ** 0.5
                    if dist < 1:
                        dist = 1
                    score += action[k] / dist
                _actions.append(score)
        return _actions

        """
        if self.rlMod == 2:
            tmp_obs = []
            p = 0
            q = 0
            for i in range(point_num*point_num):
                if i % point_num == 0 and i != 0:
                    p = p + self.sensor_xnum * cell_num
                    q = 0
                for j in range(cell_num):
                    for k in range(cell_num):
                        #print(p+q*cell_num+self.sensor_xnum*j+k)
                        tmp_obs.append(copy.deepcopy(obs[p+q*cell_num+self.sensor_xnum*j+k]))
                q += 1
            obs = np.array(tmp_obs)
        """
