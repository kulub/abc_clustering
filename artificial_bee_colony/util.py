from scipy.stats import truncnorm
import random

truncnorm_gen = truncnorm(0, 1)
def random_normal_01():
    return truncnorm_gen.rvs()

def randrange_except(stop, forbidden):
    result = random.randrange(0, stop - 1)
    return result + int(result >= forbidden)

def multi_randrange_except(stop, samples, forbidden):
    result = random.sample(range(stop - 1), samples)
    return [r + int(r >= forbidden) for r in result]
