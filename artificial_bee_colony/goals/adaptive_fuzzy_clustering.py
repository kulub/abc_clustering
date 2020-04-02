import numpy as np
import random
from copy import copy
from scipy.spatial import distance
from artificial_bee_colony.util import random_normal_01
import random

class AdaptiveFuzzyClustering:
    def __init__(self, weights, centers, vectors):
        self.weights = weights
        self.vectors = vectors
        self.centers = centers

    @classmethod
    def random(self, vectors):
        weights = np.zeros([4, 80])
        for i in range(80):
            row = np.array([random_normal_01() for _ in range(4)])
            weights[0][i], weights[1][i], weights[2][i], weights[3][i] = row / sum(row)

        centers = np.array([(np.random.rand(2) * 180 - 90) * random_normal_01() for _ in range(4)])
        
        return AdaptiveFuzzyClustering(weights, centers, vectors)
    
    def get_cluster(self, vector_idx):
        return np.argmax(self.weights.T[vector_idx])
                
    def compute_fitness(self):
        clusters = [self.get_cluster(i) for i in range(len(self.vectors))]
        center_fitness = 0
        for cluster, center in enumerate(self.centers):
            for assigned_cluster, vector in enumerate(self.vectors):
                if clusters[assigned_cluster] == cluster:
                    center_fitness += distance.euclidean(vector, center)
        center_fitness = 1 / center_fitness

        weight_fitness = 0
        for cluster, center in zip(self.weights, self.centers):
            for vector, weight in zip(self.vectors, cluster):
                weight_fitness += weight * distance.euclidean(vector, center)
        weight_fitness = 1 / weight_fitness

        return weight_fitness, center_fitness

    @staticmethod
    def compute_explore_bounds(a, b):
        #(upper - a) is to (1 - a) what (a - lower) is to a;
        #preserve the spirit of ABC - change self to buddy, or move it by the diff away from buddy
        upper = np.maximum(b, a + (1 - a) * (a - b) / (a + 1e-8))
        lower = np.minimum(b, a - a * (b - a) / (1 - a + 1e-8))
        return lower, upper
    
    def explore(self, buddy, disposition):
        if random.random() < disposition:
            new_weights = copy(self.weights)
            mixed = random.randrange(len(new_weights.T))
            lower, upper = AdaptiveFuzzyClustering.compute_explore_bounds(self.weights.T[mixed], buddy.weights.T[mixed])
            new_weight = np.random.uniform(lower, upper)
            new_weights.T[mixed] = new_weight / sum(new_weight)
            new_centers = self.centers
        else:
            new_centers = copy(self.centers)
            mixed = random.randrange(len(new_centers))
            new_centers[mixed] = self.centers[mixed] + random.uniform(-1, 1) * (self.centers[mixed] - buddy.centers[mixed])
            new_weights = self.weights
            
        return AdaptiveFuzzyClustering(new_weights, new_centers, self.vectors)

    def params(self):
        return (self.vectors,)
