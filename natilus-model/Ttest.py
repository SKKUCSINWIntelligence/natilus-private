import torch
from SingleTransformer import Transformer

# sensor, info, point, head, history, test
net = Transformer(16, 3, 1, 3, True)

input = torch.rand(2, 16*3, 3)
#print(input)

out = net(input)

#print(out.shape)
print(out)
