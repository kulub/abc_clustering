import numpy as np
from scipy.spatial import distance
from artificial_bee_colony.util import random_normal_01
import random
from copy import copy

class GLFuzzyClustering:
    def __init__(self, weights, vectors, neighbors):
        self.weights = weights
        self.vectors = vectors
        self.neighbors = neighbors

    @classmethod
    def random(self, vectors, neighbors):
        weights = np.zeros([4, 80])
        for i in range(80):
            row = np.array([random_normal_01() for _ in range(4)])
            weights[0][i], weights[1][i], weights[2][i], weights[3][i] = row / sum(row)

        centers = np.array([(np.random.rand(2) * 180 - 90) * random_normal_01() for _ in range(4)])
        
        return GLFuzzyClustering(weights, vectors, neighbors)

    def compute_global_fitness(self):
        result = 0
        for cluster in self.weights:
            center = sum((self.vectors.T * cluster).T) / sum(cluster)
            for vector, weight in zip(self.vectors, cluster):
                result += weight * distance.euclidean(vector, center)
        result = 1 / result

        return result

    def compute_local_fitness(self):
        result = 0
        for i, weights in enumerate(self.weights.T):
            for neighbor in self.neighbors[i]:
                result += sum(abs(weights - self.weights.T[neighbor]))
        result = 1 / result
        
        return result

    @staticmethod
    def compute_explore_bounds(a, b):
        #(upper - a) is to (1 - a) what (a - lower) is to a;
        #preserve the spirit of ABC - change self to buddy, or move it by the diff away from buddy
        upper = np.maximum(b, a + (1 - a) * (a - b) / (a + 1e-8))
        lower = np.minimum(b, a - a * (b - a) / (1 - a + 1e-8))
        return lower, upper
    
    def explore(self, buddy):
        new_weights = copy(self.weights)
        mixed = random.randrange(len(new_weights.T))
        lower, upper = GLFuzzyClustering.compute_explore_bounds(self.weights.T[mixed], buddy.weights.T[mixed])
        new_weight = np.random.uniform(lower, upper)
        new_weights.T[mixed] = new_weight / sum(new_weight)

        return GLFuzzyClustering(new_weights, self.vectors, self.neighbors)

    def params(self):
        return (self.vectors, self.neighbors)
