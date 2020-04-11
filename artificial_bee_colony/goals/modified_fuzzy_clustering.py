import numpy as np
from scipy.spatial import distance
import random
from artificial_bee_colony.util import random_normal_01
from copy import copy

class ModFuzzyClustering:
    def __init__(self, weights, vectors, f, mr):
        self.weights = weights
        self.vectors = vectors
        self.f = f
        self.mr = mr

    @classmethod
    def random(self, vectors, f, mr):
        weights = np.zeros([4, 80])
        for i in range(80):
            row = np.array([random_normal_01() for _ in range(4)])
            weights[0][i], weights[1][i], weights[2][i], weights[3][i] = row / sum(row)
            
        return ModFuzzyClustering(weights, vectors, f, mr)
    
    def compute_fitness(self):
        result = 0
        for cluster in self.weights:
            center = sum((self.vectors.T * cluster).T) / sum(cluster)
            for vector, weight in zip(self.vectors, cluster):
                result += weight * distance.euclidean(vector, center)
        return 1 / result

    def explore(self, gmax, buddy, rando1, rando2):
        new_weights = copy(self.weights)
        mixing_probs = np.array([random.uniform(0, 1) for _ in new_weights.T])
        for i, mixing_prob in enumerate(mixing_probs):
            if mixing_prob <= self.mr:
                new_weight = np.clip(gmax.weights.T[i] + self.f * (self.weights.T[i] - buddy.weights.T[i]) + self.f * (rando1.weights.T[i] - rando2.weights.T[i]), 0, 1)
                new_weights.T[i] = new_weight / sum(new_weight)
            
        return ModFuzzyClustering(new_weights, self.vectors, self.f, self.mr)

    def params(self):
        return (self.vectors, self.f, self.mr)

class DEFuzzyClustering:
    def __init__(self, weights, vectors, f):
        self.weights = weights
        self.vectors = vectors
        self.f = f

    @classmethod
    def random(self, vectors, f):
        weights = np.zeros([4, 80])
        for i in range(80):
            row = np.array([random_normal_01() for _ in range(4)])
            weights[0][i], weights[1][i], weights[2][i], weights[3][i] = row / sum(row)
            
        return DEFuzzyClustering(weights, vectors, f)
    
    def compute_fitness(self):
        result = 0
        for cluster in self.weights:
            center = sum((self.vectors.T * cluster).T) / sum(cluster)
            for vector, weight in zip(self.vectors, cluster):
                result += weight * distance.euclidean(vector, center)
        return 1 / result
    
    def explore(self, gmax, buddy, rando1, rando2):
        new_weights = copy(self.weights)
        mixed = 3#random.randrange(len(new_weights.T))
        new_weight = np.clip(gmax.weights.T[mixed] + self.f * (self.weights.T[mixed] - buddy.weights.T[mixed]) + self.f * (rando1.weights.T[mixed] - rando2.weights.T[mixed]), 0, 1)
        new_weights.T[mixed] = new_weight / sum(new_weight)
            
        return DEFuzzyClustering(new_weights, self.vectors, self.f)

    def params(self):
        return (self.vectors, self.f)

class FastExploreFuzzyClustering:
    def __init__(self, weights, vectors, mr):
        self.weights = weights
        self.vectors = vectors
        self.mr = mr

    @classmethod
    def random(self, vectors, mr):
        weights = np.zeros([4, 80])
        for i in range(80):
            row = np.array([random_normal_01() for _ in range(4)])
            weights[0][i], weights[1][i], weights[2][i], weights[3][i] = row / sum(row)
            
        return FastExploreFuzzyClustering(weights, vectors, mr)
    
    def compute_fitness(self):
        result = 0
        for cluster in self.weights:
            center = sum((self.vectors.T * cluster).T) / sum(cluster)
            for vector, weight in zip(self.vectors, cluster):
                result += weight * distance.euclidean(vector, center)
        return 1 / result

    def explore(self, gmax, buddy, rando1, rando2):
        new_weights = copy(self.weights)
        mixing_probs = np.array([random.uniform(0, 1) for _ in new_weights.T])
        mixings = mixing_probs <= self.mr
        if not any(mixings):
            mixings[random.randrange(len(mixings))] = True
        for i, mixing in enumerate(mixings):
            if mixing:
                new_weight = np.clip(self.weights.T[i] + np.random.uniform(-1, 1) * (self.weights.T[i] - buddy.weights.T[i]), 0, 1)
                new_weights.T[i] = new_weight / sum(new_weight)
            
        return FastExploreFuzzyClustering(new_weights, self.vectors, self.mr)

    def params(self):
        return (self.vectors, self.mr)
