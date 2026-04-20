"""XGBoost surrogate model."""
import joblib
import xgboost as xgb


class XGBoostModel:
    def __init__(self, n_estimators=200, max_depth=6, learning_rate=0.1,
                 subsample=0.8, colsample_bytree=0.8, random_state=42, n_jobs=-1):
        self.model = xgb.XGBRegressor(
            n_estimators=n_estimators,
            max_depth=max_depth,
            learning_rate=learning_rate,
            subsample=subsample,
            colsample_bytree=colsample_bytree,
            random_state=random_state,
            n_jobs=n_jobs,
            tree_method='hist'
        )

    def fit(self, X_train, y_train, X_val=None, y_val=None):
        eval_set = [(X_val, y_val.ravel())] if X_val is not None else None
        self.model.fit(
            X_train, y_train.ravel(),
            eval_set=eval_set,
            verbose=False
        )
        return self

    def predict(self, X):
        return self.model.predict(X).reshape(-1, 1)

    def save(self, filepath):
        joblib.dump(self.model, filepath)

    def load(self, filepath):
        self.model = joblib.load(filepath)
        return self
