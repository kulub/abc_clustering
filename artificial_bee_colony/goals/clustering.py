import numpy as np
from scipy.spatial import distance
from artificial_bee_colony.util import random_normal_01
import random
from copy import copy

class Clustering:
    def __init__(self, clusters, centers, vectors):
        self.clusters = clusters
        self.centers = centers
        self.vectors = vectors

    @classmethod
    def random(self, vectors):
        clusters = np.array([random.choice([0, 1, 2, 3]) for _ in range(80)])
        centers = np.array([(np.random.rand(2) * 180 - 90) * random_normal_01() for _ in range(4)])
        return Clustering(clusters, centers, vectors)
    
    def compute_fitness(self):
        result = 0
        for cluster, center in enumerate(self.centers):
            for assigned_cluster, vector in enumerate(self.vectors):
                if self.clusters[assigned_cluster] == cluster:
                    result += distance.euclidean(vector, center)
        return 1 / result

    def explore(self, buddy):
        if bool(random.getrandbits(1)):
            new_clusters = copy(self.clusters)
            exchanged = random.randrange(len(new_clusters))
            new_clusters[exchanged] = buddy.clusters[exchanged]
            new_centers = self.centers
        else:
            new_centers = copy(self.centers)
            mixed = random.randrange(len(new_centers))
            new_centers[mixed] = self.centers[mixed] + random.uniform(-1, 1) * (self.centers[mixed] - buddy.centers[mixed])
            new_clusters = self.clusters
        return Clustering(new_clusters, new_centers, self.vectors)

    def params(self):
        return (self.vectors,)
