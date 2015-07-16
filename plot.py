import matplotlib.pyplot as plt
with open("log.txt", "r") as myfile:
	data = myfile.read().split()
plt.plot(data)
plt.show()
