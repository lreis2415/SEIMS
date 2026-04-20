"""Scikit-learn based neural network for surrogate modeling."""
import numpy as np
import pickle
from sklearn.neural_network import MLPRegressor
from sklearn.metrics import r2_score, mean_squared_error, mean_absolute_error


class SklearnANN:
    """Multi-output neural network using scikit-learn."""

    def __init__(self, input_dim, output_dim, hidden_layers=[128, 64, 32]):
        self.input_dim = input_dim
        self.output_dim = output_dim
        self.hidden_layers = tuple(hidden_layers)

        self.model = MLPRegressor(
            hidden_layer_sizes=self.hidden_layers,
            activation='relu',
            solver='adam',
            alpha=0.0001,
            batch_size='auto',
            learning_rate='adaptive',
            learning_rate_init=0.001,
            max_iter=1000,
            random_state=42,
            early_stopping=False,  # Disable early stopping to avoid validation issues
            verbose=True
        )

    def fit(self, X_train, y_train, X_val=None, y_val=None):
        """Train the model."""
        print(f"Training model with {len(X_train)} samples...")
        self.model.fit(X_train, y_train)

        # Evaluate
        train_pred = self.model.predict(X_train)
        train_r2 = r2_score(y_train, train_pred)
        train_rmse = np.sqrt(mean_squared_error(y_train, train_pred))

        print(f"Training - R2: {train_r2:.4f}, RMSE: {train_rmse:.2f}")

        if X_val is not None and y_val is not None:
            val_pred = self.model.predict(X_val)
            val_r2 = r2_score(y_val, val_pred)
            val_rmse = np.sqrt(mean_squared_error(y_val, val_pred))
            print(f"Validation - R2: {val_r2:.4f}, RMSE: {val_rmse:.2f}")

        return self.model

    def predict(self, X):
        """Make predictions."""
        return self.model.predict(X)

    def evaluate(self, X, y):
        """Evaluate model performance."""
        y_pred = self.predict(X)

        r2 = r2_score(y, y_pred)
        rmse = np.sqrt(mean_squared_error(y, y_pred))
        mae = mean_absolute_error(y, y_pred)

        return {
            'r2': r2,
            'rmse': rmse,
            'mae': mae,
            'predictions': y_pred
        }

    def save(self, filepath):
        """Save model to file."""
        with open(filepath, 'wb') as f:
            pickle.dump(self.model, f)

    def load(self, filepath):
        """Load model from file."""
        with open(filepath, 'rb') as f:
            self.model = pickle.load(f)
