import argparse
import os
import numpy as np
import math

import torchvision.transforms as transforms
from torchvision.utils import save_image

from torch.utils.data import DataLoader
from torchvision import datasets
from torch.autograd import Variable

import torch.nn as nn
import torch.nn.functional as F
import torch
Tensor = torch.cuda.FloatTensor 


parser = argparse.ArgumentParser()
parser.add_argument("--n_epochs", type=int, default=200, help="number of epochs of training")
parser.add_argument("--batch_size", type=int, default=64, help="size of the batches")
parser.add_argument("--lr", type=float, default=0.0002, help="adam: learning rate")
parser.add_argument("--b1", type=float, default=0.5, help="adam: decay of first order momentum of gradient")
parser.add_argument("--b2", type=float, default=0.999, help="adam: decay of first order momentum of gradient")
parser.add_argument("--n_cpu", type=int, default=8, help="number of cpu threads to use during batch generation")
parser.add_argument("--latent_dim", type=int, default=16, help="dimensionality of the latent space")
parser.add_argument("--img_size", type=int, default=12, help="size of each image dimension")
parser.add_argument("--channels", type=int, default=1, help="number of image channels")
parser.add_argument("--sample_interval", type=int, default=100, help="number of image channels")
opt = parser.parse_args()

class Generator(nn.Module):
    def __init__(self):
        super(Generator, self).__init__()

        self.init_size = 12
        #self.init_size = opt.img_size // 4
        self.l1 = nn.Sequential(nn.Linear(opt.latent_dim, 4* self.init_size ** 2))

        self.conv_blocks = nn.Sequential(
            nn.BatchNorm1d(576),
            nn.Linear(4*self.init_size**2,8*self.init_size**2),
            nn.BatchNorm1d(1152),
            nn.LeakyReLU(0.2, inplace=True),
            nn.Linear(8*self.init_size**2,4*self.init_size**2),
            nn.BatchNorm1d(576, 0.8),
            nn.LeakyReLU(0.2, inplace=True),
            nn.Linear(4*self.init_size**2,self.init_size**2),
            nn.Tanh(),
        )

    def forward(self, noise):
        out = self.l1(noise)
        #out = out.resize(out.shape[0],4*self.init_size**2)
        out = self.conv_blocks(out)
        img = out.view(out.shape[0],1, self.init_size, self.init_size)
        img= (img+1)/2
        return img
