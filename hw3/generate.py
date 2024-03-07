import random
n = int(input())
file = open('input.txt', 'w')
file.write(str(n)+'\n')
for i in range(n):
    val = random.randint(-1000000, 1000000)
    # print(val)
    file.write(str(val)+' ')
file.write('\n')

file.close()