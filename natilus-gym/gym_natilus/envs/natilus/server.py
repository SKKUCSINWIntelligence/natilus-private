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
        self.m_action_size = 36 # default for 6X6 Map 
        
    def _recv(self):
        """
        Recieve form connectd socket.
        """
        recv = self.m_socket.recv()
        return recv
    
    def _initialize(self):
        """
        Initialize Server Class, with # Sensors.
        """
        recv = self._recv()
        dict = json.loads(recv)
        self.m_socket.send(b"1")

        self.m_action_size = dict['sensor_num']
        return dict['sensor_num']
        
    def _observation(self):
        """
        Load Observation state.
        """
        recv = self._recv()
        dict = json.loads(recv)
        obs = dict['obs']
            
        self.m_socket.send(b"1")
        
        return obs
        
    def _reward(self):
        """
        Load Reward.
        """
        reward = 0
        
        recv = self._recv()
        dict = json.loads(recv)
        reward = dict['reward']
        self.m_socket.send(b"1")
        
        return reward
        
    def _action(self, action):
	"""
        Send Actions.
        """
        for i in range(self.m_action_size) :
            recv = self._recv()
            snd = str(action[i]).encode()
            self.m_socket.send(snd)       
        
    def _end(self):
        """
        Load episode End.
        """
        recv = self._recv()
        end = self.byteToint(recv)
        self.m_socket.send(b"1")
        
        if end == 1:
            return True
        return False
     
    def byteToint (self, byte):
        """
        (Util) Chang byte to int.
        """
       result = int.from_bytes (byte, byteorder = 'big', signed=True)
       return result
