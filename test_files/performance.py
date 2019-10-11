# from math import pow, atan, tan

# def generate_slow_number(num1, num2):
#   base = num1 + num2
#   result = 0
#   for i in range(int(pow(base, 7))):
#     result += atan(i) * tan(i)
#   return result

from math import pow, atan, tan
import multiprocessing

queue = multiprocessing.Queue()

def job(queue, num1, num2):
  base = num1 + num2
  result = 0
  for i in range(int(pow(base, 7))):
    result += atan(i) * tan(i)
  print("result : ", result, flush=True)
  queue.put_nowait(result)

def generate_slow_number(num1, num2):
  global queue
  p = multiprocessing.Process(target=job, args=(queue, num1, num2, ))
  p.start()
  x = queue.get(block=True)
  p.join()
  print("return x : ", x, flush=True)
  return x
