"""One-hot encoding for BMP categorical variables."""
import numpy as np
from sklearn.preprocessing import OneHotEncoder
import joblib


class BMPEncoder:
    """Encode BMP gene values as one-hot vectors."""

    def __init__(self, n_genes=105, n_categories=5):
        self.n_genes = n_genes
        self.n_categories = n_categories
        self.encoder = OneHotEncoder(
            categories=[list(range(n_categories))] * n_genes,
            sparse_output=False,
            handle_unknown='ignore'
        )

    def fit(self, X):
        """Fit encoder on gene values."""
        X = np.array(X).reshape(-1, self.n_genes)
        self.encoder.fit(X)
        return self

    def transform(self, X):
        """Transform gene values to one-hot encoding."""
        X = np.array(X).reshape(-1, self.n_genes)
        return self.encoder.transform(X)

    def fit_transform(self, X):
        """Fit and transform."""
        return self.fit(X).transform(X)

    def save(self, filepath):
        """Save encoder."""
        joblib.dump(self.encoder, filepath)

    def load(self, filepath):
        """Load encoder."""
        self.encoder = joblib.load(filepath)
        return self
