import numpy as np
from scipy.spatial import distance
from artificial_bee_colony.util import random_normal_01
import random
from copy import copy

class FuzzyClustering:
    def __init__(self, weights, vectors):
        self.weights = weights
        self.vectors = vectors

    @classmethod
    def random(self, vectors):
        weights = np.zeros([4, 80])
        for i in range(80):
            row = np.array([random_normal_01() for _ in range(4)])
            weights[0][i], weights[1][i], weights[2][i], weights[3][i] = row / sum(row)
            
        centers = np.array([(np.random.rand(2) * 180 - 90) * random_normal_01() for _ in range(4)])
        return FuzzyClustering(weights, vectors)
    
    def compute_fitness(self):
        result = 0
        for cluster in self.weights:
            center = sum((self.vectors.T * cluster).T) / sum(cluster)
            for vector, weight in zip(self.vectors, cluster):
                result += weight * distance.euclidean(vector, center)
        return 1 / result
    
    def explore(self, buddy):
        new_weights = copy(self.weights)
        mixed = random.randrange(len(new_weights.T))
        new_weight = np.clip(self.weights.T[mixed] + np.random.uniform(-1, 1) * (self.weights.T[mixed] - buddy.weights.T[mixed]), 0, 1)
        new_weights.T[mixed] = new_weight / sum(new_weight)
            
        return FuzzyClustering(new_weights, self.vectors)

    def params(self):
        return (self.vectors,)

class FuzzyClusteringNeighbors:
    @staticmethod
    def generate_neighbors(vectors, n):
        neighbors = np.empty([len(vectors), n], dtype=int)
        for i, vector in enumerate(vectors):
            v_neighbors = list(range(n)[:i]) + list(range(1, n + 1)[i:])
            distances = [distance.euclidean(vector, vectors[j]) for j in v_neighbors]
            order = np.argsort(distances)
            distances = [distances[j] for j in order]
            v_neighbors = [v_neighbors[j] for j in order]
            for j in (set(range(len(vectors))) - set(v_neighbors) - {i}):
                dist = distance.euclidean(vector, vectors[j])
                for k in range(n):
                    if dist < distances[k]:
                        distances.insert(k, dist)
                        distances.pop()
                        v_neighbors.insert(k, j)
                        v_neighbors.pop()
                        break
            for j in range(n):
                neighbors[i][j] = v_neighbors[j]
        return neighbors
    
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
            
        return FuzzyClusteringNeighbors(weights, vectors, neighbors)
    
    def compute_fitness(self):
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
        lower, upper = FuzzyClusteringNeighbors.compute_explore_bounds(self.weights.T[mixed], buddy.weights.T[mixed])
        new_weight = np.random.uniform(lower, upper)
        new_weights.T[mixed] = new_weight / sum(new_weight)
            
        return FuzzyClusteringNeighbors(new_weights, self.vectors, self.neighbors)

    def params(self):
        return (self.vectors, self.neighbors)

class FuzzyClusteringWithCenters:
    def __init__(self, weights, vectors, centers):
        self.weights = weights
        self.vectors = vectors
        self.centers = centers

    @classmethod
    def random(self, vectors, centers):
        weights = np.zeros([4, 80])
        for i in range(80):
            row = np.array([random_normal_01() for _ in range(4)])
            weights[0][i], weights[1][i], weights[2][i], weights[3][i] = row / sum(row)
            
        return FuzzyClusteringWithCenters(weights, vectors, centers)
    
    def compute_fitness(self):
        result = 0
        for cluster, center in zip(self.weights, self.centers):
            for vector, weight in zip(self.vectors, cluster):
                result += weight * distance.euclidean(vector, center)
        return 1 / result

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
        lower, upper = FuzzyClustering.compute_explore_bounds(self.weights.T[mixed], buddy.weights.T[mixed])
        new_weight = np.random.uniform(lower, upper)
        new_weights.T[mixed] = new_weight / sum(new_weight)
            
        return FuzzyClusteringWithCenters(new_weights, self.vectors, self.centers)

    def params(self):
        return (self.vectors, self.centers)
