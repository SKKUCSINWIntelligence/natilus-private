import math
import torch
import torch.nn as nn
import torch.nn.functional as F

class Transformer(nn.Module):
    def __init__(self, n_sensors=25, d_info=3, n_head=1, n_history=3, test=False):
        super(Transformer, self).__init__()

        self.n_sensors = n_sensors
        self.side = n_sensors ** 0.5
        self.d_info = d_info
        self.n_head = n_head
        self.n_history = n_history 

        # Embedding
        self.embed = Embedding(n_sensors, d_info, n_history, test=test)
          
        # Self Attention
        self.sattn = TransformerEncoderLayer(d_info, n_head)
         
        if test:
            self.device = "cpu"
        else:
            self.device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

    def forward(self, x):
        """
        '''Embedding'''
        """
   
        out = self.embed(x) 
         
        """
        '''Self-Attention'''
        """
        out = self.sattn(out)
        out = out.view(-1, self.n_sensors*self.d_info)
        
        # [batch, sensor X info] 
        return out

class Embedding(nn.Module):
    def __init__(self, n_sensors=25, d_info=3, n_history=3, dropout=0.1, activation="relu", test=False):
        super(Embedding, self).__init__()
        
        self.n_sensors = n_sensors
        self.d_info = d_info
        self.n_history = n_history 

        self.linear1 = nn.Linear(d_info, d_info*2)
        self.linear2 = nn.Linear(d_info*2, d_info)
        
        self.linear3 = nn.Linear(d_info*n_history, d_info*n_history*2)
        self.linear4 = nn.Linear(d_info*n_history*2, d_info)

        self.activation = _get_activation_fn(activation)
        self.tanh = _get_activation_fn("tanh")
        self.sigmoid = _get_activation_fn("sigmoid")

    def forward (self, x):
        """
        Flow Embedding
        """
        out = self.linear1(x)
        out = self.activation(out)
        out = self.linear2(out)  
        
        out = out.view(-1, self.n_sensors, self.d_info*self.n_history)
        #out = self.activation(out)

        """
        History Embedding
        """
        out = self.linear3(out)
        out = self.activation(out)
        out = self.linear4(out)
        #out = self.activation(out)

        return out

class TransformerEncoderLayer(nn.Module):
    def __init__(self, d_embed, n_head, dropout=0.1, activation="relu"):
        super(TransformerEncoderLayer, self).__init__()
        # d_embed = d_info
        self.attn = MultiheadAttention(d_embed, n_head, dropout=dropout)
    
    def forward(self, x):
        out = self.attn(x, x, x)

        return out

class MultiheadAttention(nn.Module):
    def __init__(self, d_embed, n_head, dropout=0.):
        super(MultiheadAttention, self).__init__()
        self.d_embed = d_embed
        self.n_head = n_head

        # Assume QKV have same embed dim
        self.w_qs = nn.Linear(d_embed, d_embed*n_head, bias=False)
        self.w_ks = nn.Linear(d_embed, d_embed*n_head, bias=False)
        self.w_vs = nn.Linear(d_embed, d_embed*n_head, bias=False)
    

    def forward(self, query, key, value):
        """
        Self Attention
        """
        residual = query
 
        q = self.w_qs(query)
        k = self.w_ks(query)
        v = self.w_vs(query)
         
        k = k.permute(0,2,1)
        attn = torch.matmul(q/((self.d_embed)**0.5), k)
        attn = F.softmax(attn, dim=-1)

        out = torch.matmul(attn, v)
        
        #print("1.", residual)
        #print("2.", out)
        
        #out = out + residual
        return out

class FeedForward(nn.Module):
    def __init__(self, d_embed, dim_feedfowrard=256, dropout=0., activation="relu"):
        super(FeedForward, self).__init__() 

        self.linear1 = nn.Linear(d_embed, dim_feedfowrard)
        self.dropout = nn.Dropout(dropout)
        self.linear2 = nn.Linear(dim_feedfowrard, d_embed)
        self.activation = _get_activation_fn(activation)

    def forward(self, x):
        residual = x

        out = self.linear1(x)
        out = self.activation(out)
        out = self.linear2(out)

        out = self.dropout(out)

        return out

def _get_activation_fn(activation):
    if activation == "relu":
        return F.relu
    if activation == "tanh":
        return F.tanh
    if activation == "sigmoid":
        return F.sigmoid
