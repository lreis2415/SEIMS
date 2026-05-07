"""Quick hyperparameter optimization test."""
import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.neural_network import MLPRegressor
from sklearn.metrics import r2_score, mean_squared_error

# Load data
csv_file = "D:/EGC/SEIMS-dev/data/youwuzhen/surrogate_training_slppos_500/simulation_results/collected_data.csv"
df = pd.read_csv(csv_file)

gene_cols = [col for col in df.columns if col.startswith('gene_')]
X = df[gene_cols].values
y = df['SED'].values.reshape(-1, 1)

# Split
X_temp, X_test, y_temp, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
X_train, X_val, y_train, y_val = train_test_split(X_temp, y_temp, test_size=0.125, random_state=42)

# Scale
scaler_X = StandardScaler()
scaler_y = StandardScaler()
X_train = scaler_X.fit_transform(X_train)
X_val = scaler_X.transform(X_val)
X_test = scaler_X.transform(X_test)
y_train = scaler_y.fit_transform(y_train)
y_val = scaler_y.transform(y_val)
y_test = scaler_y.transform(y_test)

# Test configurations
configs = [
    {"name": "Current", "hidden": (256,128,64,32), "alpha": 0.0001, "lr": 0.001},
    {"name": "More Reg", "hidden": (256,128,64,32), "alpha": 0.001, "lr": 0.001},
    {"name": "Simpler", "hidden": (128,64,32), "alpha": 0.001, "lr": 0.001},
    {"name": "Smaller", "hidden": (128,64), "alpha": 0.001, "lr": 0.0005},
]

print("=" * 80)
print("Hyperparameter Optimization Test")
print("=" * 80)

for cfg in configs:
    print(f"\n[{cfg['name']}] hidden={cfg['hidden']}, alpha={cfg['alpha']}, lr={cfg['lr']}")

    model = MLPRegressor(
        hidden_layer_sizes=cfg['hidden'],
        alpha=cfg['alpha'],
        learning_rate_init=cfg['lr'],
        max_iter=1000,
        random_state=42,
        verbose=False
    )

    model.fit(X_train, y_train.ravel())

    train_r2 = r2_score(y_train, model.predict(X_train))
    val_r2 = r2_score(y_val, model.predict(X_val))
    test_r2 = r2_score(y_test, model.predict(X_test))
    test_rmse = np.sqrt(mean_squared_error(y_test, model.predict(X_test)))

    print(f"  Train R2: {train_r2:.4f}")
    print(f"  Val R2:   {val_r2:.4f}")
    print(f"  Test R2:  {test_r2:.4f}")
    print(f"  Test RMSE: {test_rmse:.4f}")
    print(f"  Overfit:  {train_r2 - test_r2:.4f}")

print("\n" + "=" * 80)
