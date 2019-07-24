from datetime import datetime
from datetime import timedelta
import random

def time_series_data():
  now = datetime.now()
  data = []
  for _ in range(50):
    now = now + timedelta(minutes = 10)
    data.append([
      str(now),
      random.random()
    ])
  return data


def multiply(a,b):
    return a * b

def return_immediate(x):
  return x

def merge_two_dicts(x, y):
  z = x.copy()
  z.update(y)
  return z
