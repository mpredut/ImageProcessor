
import pandas as pd

from google.colab import drive
drive.mount('/content/drive')

data_path = '/content/drive/My Drive/matrix_data.csv'

import io
data = pd.read_csv(data_path)
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np


from scipy.interpolate import interp1d
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from scipy.interpolate import interp1d


# Definirea maparii tipurilor
type_mappings = {
    0: "Random Matrix",
    1: "Sorted Ascending",
    2: "Sorted Descending",
    3: "With Specific Distribution",
    4: "Uniform Distribution"
}

# Metode de procesare definite
method_names = [
    "processImageHeapBest",
    "processImageParallelV512",
    "processImageCS",
    "processImageSet"
    # adauga aici noi nume de metode de testare daca este necesar
]

# Generarea dinamica a culorilor
colors = plt.cm.viridis(np.linspace(0, 1, len(method_names)))
colors = ['blue', 'green', 'red', 'yellow', 'black', 'gray']
##################

# Adaugarea unei coloane noi pentru suma normalizata
data['DimTopNSumNormalized'] = (data['Dimension'] + data['topN']) /10000

# Filtram datele pentru IDMethod == 0
method_0_data =  data[(data['Type'] == 0) & (data['IDMethod'] == 0)]

# Sortam valorile dupa 'Dimension' si 'topN' pentru a observa cum evolueaza 'ExecTime'
sorted_data = method_0_data.sort_values(by=['Dimension', 'topN'])
sorted_data = sorted_data.reset_index(drop=True)
#with pd.option_context('display.max_rows', None, 'display.max_columns', None):
  #print(sorted_data)
  #print(sorted_data[['Dimension', 'topN', 'ExecTime', 'DimTopNSumNormalized']])

sorted_data = sorted_data.reset_index()
fig, ax = plt.subplots(figsize=(15, 10))
ax.plot(sorted_data['index'], sorted_data['ExecTime'], label="test", color='red', linestyle='-')
# Ajustam etichetele pe axa Ox pentru a îmbunatati lizibilitatea
ax.set_xticks(sorted_data['index'])
#ax.set_xticklabels(sorted_data['DimTopNSumNormalized'], rotation=45, ha='right')

# Afisam graficul
plt.tight_layout()  # Ajusteaza automat subplots pentru a se încadra in zona de desenare
plt.show()


fig, ax = plt.subplots(figsize=(15, 10))
ax.plot(sorted_data['index'], sorted_data['ExecTime'] , label="test", color='red',  linestyle='-')
# Setam etichetele si titlul
ax.set_xlabel('DimTopNSumNormalized')
ax.set_ylabel('Execution Time')
ax.set_title('Execution Time for IDMethod 0')
ax.legend()
ax.grid(True)

# Afisam graficul
plt.show()

# Crearea unei figuri si a unui set de axe pentru plotare
fig, axs = plt.subplots(2, 3, figsize=(15, 10), sharex=True, sharey=True)
axs = axs.flatten()

# Plotarea datelor pentru fiecare tip si metoda
for type_value, matrix_type in type_mappings.items():
    ax = axs[type_value]
    for method_id, (method_name, color) in enumerate(zip(method_names, colors)):
        filtered_data = data[(data['Type'] == type_value) & (data['IDMethod'] == method_id)]
        if not filtered_data.empty:
            # Sortarea datelor pe baza lui 'DimTopNSumNormalized' pentru o mai buna vizualizare
            sorted_filtered_data = filtered_data.sort_values(by='DimTopNSumNormalized')
            ax.plot(sorted_filtered_data['DimTopNSumNormalized'], sorted_filtered_data['ExecTime'], label=method_name, color=color,  linestyle='-', markersize=4)
    ax.set_title(matrix_type)
    ax.legend()
    ax.grid(True)

# Ascunderea axelor nefolosite si ajustarea titlurilor
for i in range(len(type_mappings), len(axs)):
    axs[i].set_visible(False)

fig.text(0.5, 0.04, 'Normalized Sum of Image Dimension and topN (/10,000)', ha='center', va='center')
fig.text(0.04, 0.5, 'Execution Time (s)', ha='center', va='center', rotation='vertical')

plt.tight_layout()
plt.show()



#######
# Crearea unei figuri si a unui set de axe pentru plotare
fig, axs = plt.subplots(2, 3, figsize=(15, 10), sharex=True, sharey=True)
axs = axs.flatten()

# Plotarea datelor pentru fiecare tip si metoda
for type_value, matrix_type in type_mappings.items():
    ax = axs[type_value]
    for method_id, (method_name, color) in enumerate(zip(method_names, colors)):
        filtered_data = data[(data['Type'] == type_value) & (data['IDMethod'] == method_id)]
        if not filtered_data.empty:
            sorted_filtered_data = filtered_data.sort_values(by='DimTopNSumNormalized')
            f_interp = interp1d(sorted_filtered_data['DimTopNSumNormalized'], sorted_filtered_data['ExecTime'], kind='linear', fill_value='extrapolate')
            dimtopn_normalized_interp = np.linspace(data['DimTopNSumNormalized'].min(), data['DimTopNSumNormalized'].max(), num=500)
            ax.plot(dimtopn_normalized_interp, f_interp(dimtopn_normalized_interp), label=method_name, color=color)
    ax.set_title(matrix_type)
    ax.legend()
    ax.grid(True)

# Ascunderea axelor nefolosite
for i in range(len(type_mappings), len(axs)):
    axs[i].set_visible(False)

# Adaugarea titlurilor pentru axele X si Y
fig.text(0.5, 0.04, 'Normalized Sum of Image Dimension and topN (/10,000)', ha='center', va='center')
fig.text(0.04, 0.5, 'Execution Time (s)', ha='center', va='center', rotation='vertical')

plt.tight_layout()
plt.show()





# Crearea unei figuri si a unui set de axe pentru plotare
#len(type_mappings)//2 + len(type_mappings)%2
fig, axs = plt.subplots(2, 3, figsize=(15, 10), sharex=True, sharey=True)
axs = axs.flatten()
#fig, axs = plt.subplots(2, 3, figsize=(15, 10), sharex=True, sharey=True)

# Plotarea datelor
for type_value, matrix_type in type_mappings.items():
    ax = axs[type_value]
    for method_id, method_name in zip([0, 1], ["processImageSetCopy", "processImageSet"]):
        filtered_data = data[(data['Type'] == type_value) & (data['IDMethod'] == method_id)]
        if not filtered_data.empty:
            sorted_filtered_data = filtered_data.sort_values(by='DimTopNSumNormalized')
            f_interp = interp1d(sorted_filtered_data['DimTopNSumNormalized'], sorted_filtered_data['ExecTime'], kind='linear', fill_value='extrapolate')
            dimtopn_normalized_interp = np.linspace(data['DimTopNSumNormalized'].min(), data['DimTopNSumNormalized'].max(), num=500)
            color = 'blue' if method_id == 0 else 'green'
            ax.plot(dimtopn_normalized_interp, f_interp(dimtopn_normalized_interp), label=method_name, color=color)
    ax.set_title(matrix_type)
    ax.legend()
    ax.grid(True)

# Ascunderea axelor nefolosite
for i in range(len(type_mappings), len(axs)):
    axs[i].set_visible(False)

# Adaugarea titlurilor pentru axele X si Y
fig.text(0.5, 0.04, 'Normalized Sum of Image Dimension and topN (/10,000)', ha='center', va='center')
fig.text(0.04, 0.5, 'Execution Time (s)', ha='center', va='center', rotation='vertical')

plt.tight_layout()
plt.show()
