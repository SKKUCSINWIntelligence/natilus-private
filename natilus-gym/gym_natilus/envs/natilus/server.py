import zmq, json
import numpy as np
import math

from struct import *

class Server():
    def __init__(self, port):
        self.m_port = port
        self.m_context = zmq.Context ()
        self.m_socket = self.m_context.socket(zmq.REP)
        self.m_socket.bind ("tcp://*:" + port)
        self.m_action_size = 75 
        
    def _recv(self):
        recv = self.m_socket.recv()
        #print(recv)
        return recv
    
    def _initialize(self): 
        recv = self._recv()
        dict = json.loads(recv)
        self.m_socket.send(b"1")

        self.m_action_size = dict['sensor_num']
        return dict['sensor_num']
        
    def _observation(self):
        n = 1 # should be set properly, number of history
        
        recv = self._recv()
        dict = json.loads(recv)
        obs = dict['obs']
            
        self.m_socket.send(b"1")
        
        return obs
        
    def _reward(self):
        reward = 0
        #Get Reward
        recv = self._recv()
        dict = json.loads(recv)
        reward = dict['reward']
        self.m_socket.send(b"1")
        
        return reward
        
    def _action(self, action):
        #Send action
	
        for i in range(self.m_action_size) :
            recv = self._recv()
            snd = str(action[i]).encode()
            #print(action[i])
            #print(snd)
            self.m_socket.send(snd)       
        
    def _end(self):
        recv = self._recv()
        end = self.byteToint(recv)
        self.m_socket.send(b"1")
        
        if end == 1:
            return True
        return False
		
    def _exp_convert(self, action): # 0.1 ~ 30.0
        if action < -1: action = -1
        if action > 1: action = 1
        a = math.log(action + 2) / math.log(1.0373) + 0.1 #1.0567
        if a >= 30.0: 
            a = 30
        if a < 1: 
            a = 0.1
        else:
            a = math.floor(a * 1) / 1
        
        return a
    def _convert(self, a):     
        # len = 2 / 30
        # if a < -1: action = 1
        # elif a >= -1 and a < -1+len: action = 1
        # elif a>= -1+len and a < -1+2*len: action = 2
        # elif a>= -1+2*len and a < -1+3*len: action = 3
        # elif a>= -1+3*len and a < -1+4*len: action = 4
        # elif a>= -1+4*len and a < -1+5*len: action = 5
        # elif a>= -1+5*len and a < -1+6*len: action = 6
        # elif a>= -1+6*len and a < -1+7*len: action = 7
        # elif a>= -1+7*len and a < -1+8*len: action = 8
        # elif a>= -1+8*len and a < -1+9*len: action = 9
        # elif a>= -1+9*len and a < -1+10*len: action = 10
        # elif a>= -1+10*len and a < -1+11*len: action = 11
        # elif a>= -1+11*len and a < -1+12*len: action = 12
        # elif a>= -1+12*len and a < -1+13*len: action = 13
        # elif a>= -1+13*len and a < -1+14*len: action = 14
        # elif a>= -1+14*len and a < -1+15*len: action = 15
        # elif a>= -1+15*len and a < -1+16*len: action = 16
        # elif a>= -1+16*len and a < -1+17*len: action = 17
        # elif a>= -1+17*len and a < -1+18*len: action = 18
        # elif a>= -1+18*len and a < -1+19*len: action = 19
        # elif a>= -1+19*len and a < -1+20*len: action = 20
        # elif a>= -1+20*len and a < -1+21*len: action = 21
        # elif a>= -1+21*len and a < -1+22*len: action = 22
        # elif a>= -1+22*len and a < -1+23*len: action = 23
        # elif a>= -1+23*len and a < -1+24*len: action = 24
        # elif a>= -1+24*len and a < -1+25*len: action = 25
        # elif a>= -1+25*len and a < -1+26*len: action = 26
        # elif a>= -1+26*len and a < -1+27*len: action = 27
        # elif a>= -1+27*len and a < -1+28*len: action = 28
        # elif a>= -1+28*len and a < -1+29*len: action = 29
        # elif a>= -1+29*len and a < -1+30*len: action = 30
        # else: action = 30
        
        # 03.27 woo
        
        action = 30
        return action
     
    def byteToint (self, byte):
       result = int.from_bytes (byte, byteorder = 'big', signed=True)
       return result
