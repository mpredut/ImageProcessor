#include <sys/resource.h>
#include <iostream>
#include <fstream>
#include <chrono>

#include <iostream>
#include <fstream>
#include <string>
#include "utils.h"
#include "image.h"
#include "processor.h"


// GenerateMatrix.cpp
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>


#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>

// Function to load the model coefficients from a file
std::vector<double> loadCoefficients(const std::string &filePath) {
    std::vector<double> coefficients;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Could not open the file for reading: " << filePath << std::endl;
        return coefficients;
    }

    double value;
    while (file >> value) {
        coefficients.push_back(value);
    }

    file.close();
    return coefficients;
}

double sigmoid(double z) {
    return 1.0 / (1.0 + std::exp(-z));
}

// Function to calculate the prediction of the Logistic Regression model
double predict(const std::vector<double> &features, const std::vector<double> &coefficients, double intercept) {
    double z = intercept; // z is the weighted sum of features
    for (size_t i = 0; i < features.size(); ++i) {
        z += features[i] * coefficients[i];
    }
    return sigmoid(z); // Apply the sigmoid function to the weighted sum
}

int read_model() {
    // Path to the files containing the coefficients and the intercept
    std::string coefPath = "logistic_regression_coef.txt";
    std::string interceptPath = "logistic_regression_intercept.txt";

    // Loading the coefficients and the intercept
    std::vector<double> coefficients = loadCoefficients(coefPath);
    std::vector<double> interceptVals = loadCoefficients(interceptPath);
    if (coefficients.empty() || interceptVals.empty()) {
        std::cerr << "Error loading the coefficients or the intercept." << std::endl;
        return 1;
    }
    double intercept = interceptVals[0]; // Assuming the intercept is the first and only element

    // Example features for a prediction
    std::vector<double> exampleFeatures = {0.5, 1.5, -0.5}; // Should match the size of the coefficients

    // Calculating the prediction
    double prediction = predict(exampleFeatures, coefficients, intercept);
    std::cout << "Predicted probability by the model: " << prediction << std::endl;

    return 0;
}
