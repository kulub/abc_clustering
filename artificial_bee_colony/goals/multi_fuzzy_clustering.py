import numpy as np
from scipy.spatial import distance
from artificial_bee_colony.util import random_normal_01
import random
from copy import copy

class MultiFuzzyClustering:
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
        
        return MultiFuzzyClustering(weights, centers, vectors)
    
    def compute_weight_fitness(self):
        result = 0
        for cluster, center in zip(self.weights, self.centers):
            for vector, weight in zip(self.vectors, cluster):
                result += weight * distance.euclidean(vector, center)
        return 1 / result

    def get_cluster(self, vector_idx, threshold = 0.6):
        weights = self.weights.T[vector_idx]
        for i, weight in enumerate(weights):
            if weight >= threshold:
                return i
        return len(weights)
    
    def get_cluster(self, vector_idx):
        return np.argmax(self.weights.T[vector_idx])
                
    def compute_center_fitness(self):
        clusters = [self.get_cluster(i) for i in range(len(self.vectors))]
        result = 0
        for cluster, center in enumerate(self.centers):
            for assigned_cluster, vector in enumerate(self.vectors):
                if clusters[assigned_cluster] == cluster:
                    result += distance.euclidean(vector, center)
        return 1 / result

    @staticmethod
    def compute_explore_bounds(a, b):
        #(upper - a) is to (1 - a) what (a - lower) is to a;
        #preserve the spirit of ABC - change self to buddy, or move it by the diff away from buddy
        upper = np.maximum(b, a + (1 - a) * (a - b) / (a + 1e-8))
        lower = np.minimum(b, a - a * (b - a) / (1 - a + 1e-8))
        return lower, upper
    
    def explore_weights(self, buddy):
        new_weights = copy(self.weights)
        mixed = random.randrange(len(new_weights.T))
        lower, upper = MultiFuzzyClustering.compute_explore_bounds(self.weights.T[mixed], buddy.weights.T[mixed])
        new_weight = np.random.uniform(lower, upper)
        new_weights.T[mixed] = new_weight / sum(new_weight)
            
        return MultiFuzzyClustering(new_weights, self.centers, self.vectors)

    def explore_centers(self, buddy):
        new_centers = copy(self.centers)
        mixed = random.randrange(len(new_centers))
        new_centers[mixed] = self.centers[mixed] + random.uniform(-1, 1) * (self.centers[mixed] - buddy.centers[mixed])

        return MultiFuzzyClustering(self.weights, new_centers, self.vectors)

    def replace_weights(self, other):
        self.weights = other.weights

    def replace_centers(self, other):
        self.centers = other.centers

    def params(self):
        return (self.vectors,)

    def facets(self):
        return ((MultiFuzzyClustering.compute_weight_fitness, MultiFuzzyClustering.explore_weights, MultiFuzzyClustering.replace_weights),
                (MultiFuzzyClustering.compute_center_fitness, MultiFuzzyClustering.explore_centers, MultiFuzzyClustering.replace_centers))
