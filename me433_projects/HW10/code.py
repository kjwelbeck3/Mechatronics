import time
from ulab import numpy as np

## The three frequecies
freqs = [np.pi, np.pi / 2, np.pi * 2]

amplitude = 1
dt = 0.1
m = len(freqs)
els = np.zeros((m, 1024))
n = els.shape[1]

for i in range(n):
    for j in range(m):
        els[j, i] = amplitude * np.sin(dt * i * freqs[j])

summed = np.sum(els, axis=0)
real, im = np.fft.fft(summed)

## outputting to serial
for i in range(n):
    print("(" + str(real[i]) + ",)")
    # if (i % 5) == 0:
    time.sleep(0.01)
