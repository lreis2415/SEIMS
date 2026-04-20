"""Surrogate model predictor for integration with optimizer."""
import numpy as np
import joblib
import json
import os
from typing import List, Union


class SurrogatePredictor:
    """Surrogate model predictor for fast BMP scenario evaluation."""

    def __init__(self, model_path: str, scaler_X_path: str, scaler_y_path: str, metadata_path: str):
        """
        Load trained surrogate model.

        Args:
            model_path: Path to trained model (.h5)
            scaler_X_path: Path to input scaler (.pkl)
            scaler_y_path: Path to output scaler (.pkl)
            metadata_path: Path to model metadata (.json)
        """
        from tensorflow import keras

        self.model = keras.models.load_model(model_path)
        self.scaler_X = joblib.load(scaler_X_path)
        self.scaler_y = joblib.load(scaler_y_path)

        with open(metadata_path, 'r') as f:
            self.metadata = json.load(f)

        self.output_vars = self.metadata['output_vars']

    def predict(self, gene_values: Union[List[int], np.ndarray]) -> dict:
        """
        Predict watershed response for given BMP scenario.

        Args:
            gene_values: BMP scenario gene values

        Returns:
            Dictionary of predicted values for each output variable
        """
        # Convert to array
        if isinstance(gene_values, list):
            gene_values = np.array(gene_values)

        # Reshape if needed
        if gene_values.ndim == 1:
            gene_values = gene_values.reshape(1, -1)

        # Standardize input
        X_scaled = self.scaler_X.transform(gene_values)

        # Predict
        y_scaled = self.model.predict(X_scaled, verbose=0)

        # Inverse transform output
        y_pred = self.scaler_y.inverse_transform(y_scaled)

        # Return as dictionary
        result = {}
        for i, var in enumerate(self.output_vars):
            result[var] = float(y_pred[0, i]) if y_pred.ndim > 1 else float(y_pred[0])

        return result

    def predict_batch(self, gene_values_list: List[List[int]]) -> List[dict]:
        """Predict for multiple scenarios."""
        X = np.array(gene_values_list)
        X_scaled = self.scaler_X.transform(X)
        y_scaled = self.model.predict(X_scaled, verbose=0)
        y_pred = self.scaler_y.inverse_transform(y_scaled)

        results = []
        for i in range(len(gene_values_list)):
            result = {}
            for j, var in enumerate(self.output_vars):
                result[var] = float(y_pred[i, j]) if y_pred.ndim > 1 else float(y_pred[i])
            results.append(result)

        return results
