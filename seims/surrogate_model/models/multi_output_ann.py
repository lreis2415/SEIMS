"""Multi-output neural network model."""
import numpy as np
from tensorflow import keras
from tensorflow.keras import layers, callbacks
from typing import List, Tuple
import os


class MultiOutputANN:
    """Multi-output artificial neural network for surrogate modeling."""

    def __init__(self, input_dim: int, output_dim: int, hidden_layers: List[int] = None):
        """
        Initialize model.

        Args:
            input_dim: Number of input features
            output_dim: Number of output variables
            hidden_layers: List of hidden layer sizes
        """
        self.input_dim = input_dim
        self.output_dim = output_dim
        self.hidden_layers = hidden_layers or [128, 64, 32]
        self.model = None
        self.history = None

    def build(self):
        """Build neural network architecture."""
        model = keras.Sequential()

        # Input layer
        model.add(layers.Input(shape=(self.input_dim,)))

        # Hidden layers
        for units in self.hidden_layers:
            model.add(layers.Dense(units, activation='relu'))
            model.add(layers.BatchNormalization())
            model.add(layers.Dropout(0.2))

        # Output layer
        model.add(layers.Dense(self.output_dim))

        # Compile
        model.compile(
            optimizer=keras.optimizers.Adam(learning_rate=0.001),
            loss='mse',
            metrics=['mae']
        )

        self.model = model
        return model

    def train(self, X_train: np.ndarray, y_train: np.ndarray,
              X_val: np.ndarray, y_val: np.ndarray,
              epochs: int = 100, batch_size: int = 32,
              output_dir: str = None) -> dict:
        """
        Train the model.

        Args:
            X_train: Training features
            y_train: Training targets
            X_val: Validation features
            y_val: Validation targets
            epochs: Number of training epochs
            batch_size: Batch size
            output_dir: Directory to save model checkpoints

        Returns:
            Training history
        """
        if self.model is None:
            self.build()

        # Callbacks
        callback_list = [
            callbacks.EarlyStopping(
                monitor='val_loss',
                patience=20,
                restore_best_weights=True
            ),
            callbacks.ReduceLROnPlateau(
                monitor='val_loss',
                factor=0.5,
                patience=10,
                min_lr=1e-6
            )
        ]

        if output_dir:
            os.makedirs(output_dir, exist_ok=True)
            callback_list.append(
                callbacks.ModelCheckpoint(
                    os.path.join(output_dir, 'best_model.h5'),
                    monitor='val_loss',
                    save_best_only=True
                )
            )

        # Train
        self.history = self.model.fit(
            X_train, y_train,
            validation_data=(X_val, y_val),
            epochs=epochs,
            batch_size=batch_size,
            callbacks=callback_list,
            verbose=1
        )

        return self.history.history

    def predict(self, X: np.ndarray) -> np.ndarray:
        """Make predictions."""
        return self.model.predict(X, verbose=0)

    def save(self, filepath: str):
        """Save model to file."""
        self.model.save(filepath)
        print(f"  Model saved to {filepath}")

    @staticmethod
    def load(filepath: str) -> 'MultiOutputANN':
        """Load model from file."""
        model = keras.models.load_model(filepath)
        ann = MultiOutputANN(
            input_dim=model.input_shape[1],
            output_dim=model.output_shape[1]
        )
        ann.model = model
        return ann
