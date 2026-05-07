"""Data preprocessing for surrogate model training."""
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler, OneHotEncoder
from typing import Tuple
import joblib
import os


class DataPreprocessor:
    """Preprocess training data for surrogate model."""

    def __init__(self, output_vars: list, use_onehot: bool = True):
        """
        Initialize preprocessor.

        Args:
            output_vars: List of output variable names
            use_onehot: Use one-hot encoding for BMP categorical variables
        """
        self.output_vars = output_vars
        self.use_onehot = use_onehot
        self.scaler_X = StandardScaler()
        self.scaler_y = StandardScaler()
        if use_onehot:
            self.onehot_encoder = OneHotEncoder(
                categories=[list(range(5))] * 105,  # 105 genes, each 0-4
                sparse_output=False,
                handle_unknown='ignore'
            )

    def prepare_data(self, df: pd.DataFrame, test_size: float = 0.2,
                     val_size: float = 0.1, random_state: int = 42) -> Tuple:
        """
        Prepare training, validation, and test datasets.

        Args:
            df: DataFrame with gene values and simulation results
            test_size: Test set ratio
            val_size: Validation set ratio
            random_state: Random seed

        Returns:
            Tuple of (X_train, X_val, X_test, y_train, y_val, y_test)
        """
        # Extract features (gene values)
        gene_cols = [col for col in df.columns if col.startswith('gene_')]
        X = df[gene_cols].values

        # Extract targets (output variables)
        y = df[self.output_vars].values

        # One-hot encode if enabled
        if self.use_onehot:
            X = self.onehot_encoder.fit_transform(X)
            print(f"  One-hot encoding: {len(gene_cols)} genes -> {X.shape[1]} features")

        # Split train and test
        X_train_val, X_test, y_train_val, y_test = train_test_split(
            X, y, test_size=test_size, random_state=random_state
        )

        # Split train and validation
        val_ratio = val_size / (1 - test_size)
        X_train, X_val, y_train, y_val = train_test_split(
            X_train_val, y_train_val, test_size=val_ratio, random_state=random_state
        )

        # Standardize features
        X_train = self.scaler_X.fit_transform(X_train)
        X_val = self.scaler_X.transform(X_val)
        X_test = self.scaler_X.transform(X_test)

        # Standardize targets
        y_train = self.scaler_y.fit_transform(y_train)
        y_val = self.scaler_y.transform(y_val)
        y_test = self.scaler_y.transform(y_test)

        print(f"  Train: {X_train.shape[0]} samples")
        print(f"  Val:   {X_val.shape[0]} samples")
        print(f"  Test:  {X_test.shape[0]} samples")

        return X_train, X_val, X_test, y_train, y_val, y_test

    def save_scalers(self, output_dir: str):
        """Save scalers to disk."""
        os.makedirs(output_dir, exist_ok=True)
        joblib.dump(self.scaler_X, os.path.join(output_dir, 'scaler_X.pkl'))
        joblib.dump(self.scaler_y, os.path.join(output_dir, 'scaler_y.pkl'))
        if self.use_onehot:
            joblib.dump(self.onehot_encoder, os.path.join(output_dir, 'onehot_encoder.pkl'))
        print(f"  Saved scalers to {output_dir}")

    @staticmethod
    def load_scalers(output_dir: str) -> Tuple[StandardScaler, StandardScaler]:
        """Load scalers from disk."""
        scaler_X = joblib.load(os.path.join(output_dir, 'scaler_X.pkl'))
        scaler_y = joblib.load(os.path.join(output_dir, 'scaler_y.pkl'))
        return scaler_X, scaler_y
