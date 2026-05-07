"""Train alternative models (RF, XGBoost) with One-Hot encoding."""
import os
import sys
import json
import argparse
import pandas as pd
import numpy as np
from datetime import datetime

seims_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
if seims_root not in sys.path:
    sys.path.insert(0, seims_root)

from surrogate_model.training.data_preprocessor import DataPreprocessor
from surrogate_model.training.evaluator import ModelEvaluator
from surrogate_model.models.random_forest import RandomForestModel

try:
    from surrogate_model.models.xgboost_model import XGBoostModel
    XGBOOST_AVAILABLE = True
except ImportError:
    XGBOOST_AVAILABLE = False


def train_model(csv_file, output_dir, model_type='rf', config=None):
    print("=" * 80)
    print(f"Training {model_type.upper()} Surrogate Model with One-Hot Encoding")
    print("=" * 80)

    # Load data
    print(f"\n[1/4] Loading data from {csv_file}...")
    df = pd.read_csv(csv_file)
    print(f"  Loaded {len(df)} samples")

    # Prepare data
    print("\n[2/4] Preprocessing data...")
    output_vars = config.get('output_vars', ['SED'])

    gene_cols = [col for col in df.columns if col.startswith('gene_')]
    X = df[gene_cols].values
    y = df[output_vars].values

    print(f"  Features: {X.shape[1]} genes")
    print(f"  Targets: {len(output_vars)} variables")
    print(f"  One-hot encoding: enabled")

    # Split and preprocess with One-Hot
    preprocessor = DataPreprocessor(output_vars, use_onehot=True)

    df_combined = pd.DataFrame(X, columns=gene_cols)
    for i, var in enumerate(output_vars):
        df_combined[var] = y[:, i]

    X_train, X_val, X_test, y_train, y_val, y_test = preprocessor.prepare_data(
        df_combined,
        test_size=config.get('test_size', 0.15),
        val_size=config.get('validation_size', 0.1),
        random_state=42
    )

    print(f"  Train: {len(X_train)} samples, {X_train.shape[1]} features (after One-Hot)")
    print(f"  Val:   {len(X_val)} samples")
    print(f"  Test:  {len(X_test)} samples")

    # Save scalers
    models_dir = os.path.join(output_dir, 'models')
    os.makedirs(models_dir, exist_ok=True)
    preprocessor.save_scalers(models_dir)

    # Train model
    print(f"\n[3/4] Training {model_type.upper()} model...")

    if model_type == 'rf':
        model = RandomForestModel(
            n_estimators=config.get('n_estimators', 200),
            max_depth=config.get('max_depth', None),
            min_samples_split=config.get('min_samples_split', 5),
            min_samples_leaf=config.get('min_samples_leaf', 2),
            random_state=42
        )
    elif model_type == 'xgboost':
        if not XGBOOST_AVAILABLE:
            raise ImportError("XGBoost is not installed. Install with: pip install xgboost")
        model = XGBoostModel(
            n_estimators=config.get('n_estimators', 200),
            max_depth=config.get('max_depth', 6),
            learning_rate=config.get('learning_rate', 0.1),
            subsample=config.get('subsample', 0.8),
            colsample_bytree=config.get('colsample_bytree', 0.8),
            random_state=42
        )
    else:
        raise ValueError(f"Unknown model type: {model_type}")

    model.fit(X_train, y_train, X_val, y_val)

    # Evaluate
    print("\n[4/4] Evaluating model...")
    evaluator = ModelEvaluator(output_vars)

    y_pred_train = model.predict(X_train)
    y_pred_val = model.predict(X_val)
    y_pred_test = model.predict(X_test)

    metrics_train = evaluator.evaluate(y_train, y_pred_train)
    metrics_val = evaluator.evaluate(y_val, y_pred_val)
    metrics_test = evaluator.evaluate(y_test, y_pred_test)

    evaluator.print_metrics(metrics_train, "Training Set")
    evaluator.print_metrics(metrics_val, "Validation Set")
    evaluator.print_metrics(metrics_test, "Test Set")

    # Plot results
    plots_dir = os.path.join(output_dir, 'plots')
    evaluator.plot_predictions(y_test, y_pred_test, plots_dir, preprocessor.scaler_y)

    # Save model
    model_file = os.path.join(models_dir, 'surrogate_model.pkl')
    model.save(model_file)
    print(f"\n  Model saved to {model_file}")

    # Save metadata
    metadata = {
        'model_type': model_type,
        'input_dim': X_train.shape[1],
        'output_dim': len(output_vars),
        'output_vars': output_vars,
        'n_samples': len(df),
        'training_date': datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
        'performance': {
            'train': metrics_train,
            'val': metrics_val,
            'test': metrics_test
        },
        'config': config
    }

    metadata_file = os.path.join(models_dir, 'model_metadata.json')
    with open(metadata_file, 'w') as f:
        json.dump(metadata, f, indent=2)

    print(f"  Metadata saved to {metadata_file}")
    print("\n" + "=" * 80)
    print("Training completed successfully!")
    print("=" * 80)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Train alternative surrogate models')
    parser.add_argument('--csv', required=True, help='Path to collected_data.csv')
    parser.add_argument('--output', required=True, help='Output directory')
    parser.add_argument('--model', choices=['rf', 'xgboost'], default='rf',
                        help='Model type: rf (Random Forest) or xgboost')
    parser.add_argument('--config', help='Config JSON file')

    args = parser.parse_args()

    if args.config:
        with open(args.config, 'r') as f:
            config = json.load(f)
    else:
        config = {
            'output_vars': ['SED'],
            'test_size': 0.15,
            'validation_size': 0.1
        }

    train_model(args.csv, args.output, args.model, config)
