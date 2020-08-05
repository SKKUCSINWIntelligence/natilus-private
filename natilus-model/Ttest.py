import torch
from Transformer import Transformer

net = Transformer(64, 2, 25, 1, 3, True)

input = torch.rand(1, 144*3, 2)
#print(input)

out = net(input)

#print(out.shape)
#out = out.view(-1, 72)
#print(out)
