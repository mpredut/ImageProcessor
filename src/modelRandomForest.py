
import pandas as pd
from google.colab import drive
drive.mount('/content/drive')

data_path = '/content/drive/My Drive/matrix_data.csv'

import io
#data = pd.read_csv(data_path)
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np


from scipy.interpolate import interp1d
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd




import pandas as pd
import numpy as np
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score, classification_report
import matplotlib.pyplot as plt

# Loading training and testing data
train_data_path = '/content/drive/My Drive/' + 'filtered_matrix_data_test.csv' 
test_data_path =  '/content/drive/My Drive/' + 'filtered_matrix_data.csv'
train_data = pd.read_csv(train_data_path)
test_data = pd.read_csv(test_data_path)


group_size = 2
# Calculating the best algorithm for each set of 3 rows from the training data
grouped = train_data.groupby(np.arange(len(train_data)) // group_size)
train_data['BestAlgorithm'] = grouped['ExecTime'].transform(lambda x: x.idxmin() % group_size)

# Preparing data for training
X_train = train_data[['Type', 'Dimension', 'topN']].iloc[grouped['ExecTime'].idxmin().values]
y_train = train_data['IDMethod'].iloc[grouped['ExecTime'].idxmin().values]

# Training the RandomForest model
model = RandomForestClassifier(n_estimators=100, random_state=42)
model.fit(X_train, y_train)

# Predictions on the test set
grouped_test = test_data.groupby(np.arange(len(test_data)) // group_size)
test_data['BestAlgorithm'] = grouped_test['ExecTime'].transform(lambda x: x.idxmin() % group_size)
X_test = test_data[['Type', 'Dimension', 'topN']].iloc[grouped_test['ExecTime'].idxmin().values]
y_test = test_data['IDMethod'].iloc[grouped_test['ExecTime'].idxmin().values]

# Preparing the test set
predictions = model.predict(X_test)

# Repeat each prediction three times to match the structure of the test dataset
repeated_predictions = np.repeat(predictions, group_size)

# Ensure the length of repeated predictions matches the number of rows in the test set
if len(repeated_predictions) == len(test_data):
    test_data['Predicted'] = repeated_predictions
else:
    print("Numarul de predicii repetate nu corespunde cu lungimea setului de testare.")


# Evaluarea modelului
accuracy = accuracy_score(y_test, predictions)
print(f'Accuracy: {accuracy*100:.2f}%')
print(classification_report(y_test, predictions))


# Adding predictions to test_data for visualization
test_data['Predicted'] = repeated_predictions


colors = ['blue', 'green', 'yellow']
pred_color = 'red'  # Color for prediction

# Setting the line width for actual and predicted plots
line_width_real = 2  # Line width for actual algorithm plots
line_width_pred = 1   # Line width for prediction plot

type_mapping = {
    0: "random Matrix",
    1: "Sorted Ascending",
    2: "Sorted Descending",
    3: "With Specific Distribution",
    4: "Uniform Distribution"
}

# Determining the total number of unique matrix types to configure the subplots
unique_types = sorted(test_data['Type'].unique())
num_types = len(unique_types)

plt.figure(figsize=(30, 10))

for index, type_value in enumerate(unique_types, start=1):
    plt.subplot(1, num_types, index)  # Configuring subplots on a single line
    
    # Extracting the subset of data for the current matrix type
    filtered_data = test_data[test_data['Type'] == type_value]
    type_name = type_mapping.get(type_value, "Unknown Type")
    
    # Plotting the actual execution times for each algorithm
    for method_id in sorted(filtered_data['IDMethod'].unique()):
        subset = filtered_data[filtered_data['IDMethod'] == method_id]
        plt.plot(subset['Dimension'] + subset['topN'], subset['ExecTime'], label=f'Algoritm {method_id}', linestyle='-', marker='o', linewidth=line_width_real)
    
     # Extracting the execution time specific to model predictions
    predicted_times = filtered_data.apply(lambda row: filtered_data[(filtered_data['Type'] == row['Type']) & (filtered_data['Dimension'] == row['Dimension']) & (filtered_data['topN'] == row['topN']) & (filtered_data['IDMethod'] == row['Predicted'])]['ExecTime'].iloc[0], axis=1)
    plt.plot(filtered_data['Dimension'] + filtered_data['topN'], predicted_times, label='Predicted Best Algorithm', color='red', linestyle='--', linewidth=line_width_pred)
    
    plt.title(f'Type {type_name}: Real vs Predicted Execution Time')
    plt.xlabel('Unique Index (Dimension + topN)')
    plt.ylabel('Execution Time')
    plt.legend()
    plt.grid(True)

plt.tight_layout()  # # Automatically adjust subplots to fit in the figure
plt.show()