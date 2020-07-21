import math
import torch
import torch.nn as nn
import torch.nn.functional as F

class Transformer(nn.Module):
    def __init__(self, n_sensors=25, d_info=3, n_point=9, n_head=1, n_history=3):
        super(Transformer, self).__init__()

        self.n_sensors = n_sensors
        self.side = n_sensors ** 0.5
        self.d_info = d_info
        self.n_point = n_point
        self.n_head = n_head
        self.n_history = n_history
        
        # Hidden Node Size 
        #d_feedforward = math.ceil(n_sensors * n_history * d_info * 1.5)
        #print("Hidden Node Size:", d_feedforward)

        # Embedding
        self.embed = Embedding(n_sensors, d_info, n_history)
          
        # Self Attention
        self.sattn = TransformerEncoderLayer(d_info, n_head)
        
        self.device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

    def forward(self, x):
        """
        '''Embedding'''
        """
   
        out = self.embed(x) 
         
        """
        '''Self-Attention'''
        """
        out = torch.chunk(out, 9, dim=1)
        tmp = torch.tensor([]).to(self.device)
        
        for o in out:
            out = self.sattn(o)
            out = torch.sum(out, dim=1)
            tmp = torch.cat((tmp,out), 1)

        return tmp

class Embedding(nn.Module):
    def __init__(self, n_sensors=25, d_info=3, n_history=3, dropout=0.1, activation="relu"):
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
        
        if n_history > 1:
            device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
            self.factor = torch.zeros([1, 144*n_history, d_info]).to(device)
            f = 1 
            for i in range(n_sensors*n_history):
                self.factor[0][i] = f
                f += 1
                if f > self.n_history:
                    f = 1

    def forward (self, x):
        out = self.linear1(x)
        out = self.activation(out)
        out = self.linear2(out)  
        
        """
        Time Encoding
        """ 
        out = self.tanh(out)
   
        if self.n_history > 1:
            out = out * self.factor
        out = out.view(-1, 144, self.d_info*self.n_history)

        out = self.linear3(out)
        out = self.activation(out)
        out = self.linear4(out)
        
        return out

class TransformerEncoderLayer(nn.Module):
    def __init__(self, d_embed, n_head, dropout=0.1, activation="relu"):
        super(TransformerEncoderLayer, self).__init__()

        self.attn = MultiheadAttention(d_embed, n_head, dropout=dropout)
        #self.ffn = FeedForward(d_embed, dropout=dropout)
    
    def forward(self, x):
        out = self.attn(x, x, x)

        #out = self.ffn(out)
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
        
        #self.dropout1 = nn.Dropout(dropout)
        
        #self.layer_norm = nn.LayerNorm(d_embed)

    def forward(self, query, key, value):
        residual = query

        q = self.w_qs(query)
        k = self.w_ks(query)
        v = self.w_vs(query)


        k = k.permute(0,2,1)
        attn = torch.matmul(q, k)

        #attn = F.softmax(attn, dim=-1)
        #attn = self.dropout1(attn)
 
        out = torch.matmul(attn, v)
        #out += residual
        
        #out = self.layer_norm(out)
        return out

class FeedForward(nn.Module):
    def __init__(self, d_embed, dim_feedfowrard=256, dropout=0., activation="relu"):
        super(FeedForward, self).__init__() 

        self.linear1 = nn.Linear(d_embed, dim_feedfowrard)
        self.dropout = nn.Dropout(dropout)
        self.linear2 = nn.Linear(dim_feedfowrard, d_embed)
        #self.layer_norm = nn.LayerNorm(d_embed)
        self.activation = _get_activation_fn(activation)

    def forward(self, x):
        residual = x

        out = self.linear1(x)
        out = self.activation(out)
        out = self.linear2(out)

        out = self.dropout(out)
        #out += residual

        #out = self.layer_norm(out)
        return out

def _get_activation_fn(activation):
    if activation == "relu":
        return F.relu
    if activation == "tanh":
        return F.tanh
