from scipy.spatial import distance
import numpy as np
import random

class MeaningOfLife:
    def __init__(self, val, target):
        self.val = val
        self.target = target

    @classmethod
    def random(self, target):
        return MeaningOfLife(random.random() * 500, target)
    
    def compute_fitness(self):
        return 1 / abs(self.target - self.val)

    def explore(self, buddy):
        return MeaningOfLife(self.val + np.random.uniform(-1, 1) * (self.val - buddy.val), self.target)

    def params(self):
        return (self.target,)

class MeaningsOfLife:
    def __init__(self, val, target):
        self.val = val
        self.target = target

    @classmethod
    def random(self, target):
        return MeaningsOfLife(np.array([random.random() * 500 for _ in range(len(target))]), target)
    
    def compute_fitness(self):
        return 1 / distance.euclidean(self.target, self.val)

    def explore(self, buddy):
        return MeaningsOfLife(self.val + np.random.uniform(-1, 1) * (self.val - buddy.val), self.target)

    def params(self):
        return (self.target,)
