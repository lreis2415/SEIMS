"""Random Forest surrogate model."""
import joblib
from sklearn.ensemble import RandomForestRegressor


class RandomForestModel:
    def __init__(self, n_estimators=200, max_depth=None, min_samples_split=5,
                 min_samples_leaf=2, random_state=42, n_jobs=-1):
        self.model = RandomForestRegressor(
            n_estimators=n_estimators,
            max_depth=max_depth,
            min_samples_split=min_samples_split,
            min_samples_leaf=min_samples_leaf,
            random_state=random_state,
            n_jobs=n_jobs
        )

    def fit(self, X_train, y_train, X_val=None, y_val=None):
        self.model.fit(X_train, y_train.ravel())
        return self

    def predict(self, X):
        return self.model.predict(X).reshape(-1, 1)

    def save(self, filepath):
        joblib.dump(self.model, filepath)

    def load(self, filepath):
        self.model = joblib.load(filepath)
        return self
