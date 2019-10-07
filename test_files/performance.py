from math import pow, atan, tan

def generate_slow_number(num1, num2):
  base = num1 + num2
  result = 0
  for i in range(int(pow(base, 7))):
    result += atan(i) * tan(i)
  return result
