import matplotlib.pyplot as plt
import numpy as np

fig = plt.figure()
ax = fig.add_subplot(projection='3d')

ax.set_title('Temperatures')
ax.set_xlabel('Length')
ax.set_ylabel('Height')
ax.set_zlabel('Temperature [*C]')

xs = []
ys = []
zs = []

for y in range(0, 10):
    ys.append(y)
    for x in range(0, 10):
        xs.append(x)
        z = 2*x + 3*y
        zs.append(z)

xs=np.array(xs)
ys=np.array(ys)
zs=np.array(zs)

ax.plot_surface(xs, ys, zs)
plt.pause(0.05)
        
plt.show()

