from math import pow, atan, tan

def generate_slow_number(base):
  result = 0
  for i in range(int(pow(12, 7))):
    result += atan(i) * tan(i)
  return result
