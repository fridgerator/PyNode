# from math import pow, atan, tan

# def generate_slow_number(num1, num2):
#   base = num1 + num2
#   result = 0
#   for i in range(int(pow(base, 7))):
#     result += atan(i) * tan(i)
#   return result

from math import pow, atan, tan
import multiprocessing

def job(queue, num1, num2):
  base = num1 + num2
  result = 0
  for i in range(int(pow(base, 7))):
    result += atan(i) * tan(i)
  print("result : ", result, flush=True)
  queue.put(result)

def generate_slow_number(num1, num2):
  queue = multiprocessing.Queue()
  p = multiprocessing.Process(target=job, args=(queue, num1, num2, ))
  p.start()
  x = queue.get()
  p.join()
  print("return x : ", x, flush=True)
  return x
