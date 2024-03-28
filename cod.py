import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv('matrix_data.csv')

plt.figure(figsize=(10, 6))
plt.plot(data['topn'], data['metoda_0'], label='Metoda 0 (Construire Heap)', marker='o')
plt.plot(data['topn'], data['metoda_1'], label='Metoda 1 (Sortare)', marker='x')
plt.title('Compararea')
plt.xlabel('Top N Valori')
plt.ylabel('Timp(ms)')
plt.legend()
plt.grid(True)
plt.show()

