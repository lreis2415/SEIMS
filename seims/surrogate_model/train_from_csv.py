"""Train surrogate model directly from collected_data.csv"""
import os
import sys
import json
import argparse
import pandas as pd
import numpy as np
from datetime import datetime

# Add seims to path
seims_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
if seims_root not in sys.path:
    sys.path.insert(0, seims_root)

from surrogate_model.training.data_preprocessor import DataPreprocessor
from surrogate_model.training.evaluator import ModelEvaluator
from surrogate_model.models.sklearn_ann import SklearnANN


def train_from_csv(csv_file, output_dir, config):
    """Train model from CSV file."""

    print("=" * 80)
    print("Training Surrogate Model from CSV")
    print("=" * 80)

    # Load data
    print(f"\n[1/4] Loading data from {csv_file}...")
    df = pd.read_csv(csv_file)
    print(f"  Loaded {len(df)} samples")

    # Prepare data
    print("\n[2/4] Preprocessing data...")
    output_vars = config.get('output_vars', ['SED'])
    use_onehot = config.get('use_onehot', True)

    # Extract features (gene columns) and targets (output variables)
    gene_cols = [col for col in df.columns if col.startswith('gene_')]
    X = df[gene_cols].values
    y = df[output_vars].values

    print(f"  Features: {X.shape[1]} genes")
    print(f"  Targets: {len(output_vars)} variables")
    print(f"  One-hot encoding: {'enabled' if use_onehot else 'disabled'}")

    # Split and preprocess
    preprocessor = DataPreprocessor(output_vars, use_onehot=use_onehot)

    # Combine X and y into DataFrame for preprocessor
    df_combined = pd.DataFrame(X, columns=gene_cols)
    for i, var in enumerate(output_vars):
        df_combined[var] = y[:, i]

    X_train, X_val, X_test, y_train, y_val, y_test = preprocessor.prepare_data(
        df_combined,
        test_size=config.get('test_size', 0.2),
        val_size=config.get('validation_size', 0.1),
        random_state=42
    )

    print(f"  Train: {len(X_train)} samples")
    print(f"  Val:   {len(X_val)} samples")
    print(f"  Test:  {len(X_test)} samples")

    # Save scalers
    models_dir = os.path.join(output_dir, 'models')
    os.makedirs(models_dir, exist_ok=True)
    preprocessor.save_scalers(models_dir)

    # Train model
    print("\n[3/4] Training neural network...")
    model = SklearnANN(
        input_dim=X_train.shape[1],
        output_dim=len(output_vars),
        hidden_layers=config.get('hidden_layers', [128, 64, 32])
    )

    model.fit(X_train, y_train, X_val, y_val)

    # Evaluate
    print("\n[4/4] Evaluating model...")
    evaluator = ModelEvaluator(output_vars)

    y_pred_test = model.predict(X_test)
    metrics_test = evaluator.evaluate(y_test, y_pred_test)
    evaluator.print_metrics(metrics_test, "Test Set Performance")

    # Plot results
    plots_dir = os.path.join(output_dir, 'plots')
    evaluator.plot_predictions(y_test, y_pred_test, plots_dir, preprocessor.scaler_y)

    # Save model
    model_file = os.path.join(models_dir, 'surrogate_model.pkl')
    model.save(model_file)
    print(f"\n  Model saved to {model_file}")

    # Save metadata
    metadata = {
        'model_type': 'sklearn_mlp',
        'input_dim': X_train.shape[1],
        'output_dim': len(output_vars),
        'output_vars': output_vars,
        'n_samples': len(df),
        'training_date': datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
        'performance': metrics_test,
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
    parser = argparse.ArgumentParser(description='Train surrogate model from CSV')
    parser.add_argument('--csv', required=True, help='Path to collected_data.csv')
    parser.add_argument('--output', required=True, help='Output directory')
    parser.add_argument('--config', help='Config JSON file')
    parser.add_argument('--hidden-layers', nargs='+', type=int, default=[128, 64, 32],
                        help='Hidden layer sizes')
    parser.add_argument('--test-size', type=float, default=0.2, help='Test set size')
    parser.add_argument('--val-size', type=float, default=0.1, help='Validation set size')
    parser.add_argument('--use-onehot', action='store_true', help='Use one-hot encoding')

    args = parser.parse_args()

    # Load config from file if provided
    if args.config:
        with open(args.config, 'r') as f:
            config = json.load(f)
    else:
        config = {
            'output_vars': ['SED'],
            'hidden_layers': args.hidden_layers,
            'test_size': args.test_size,
            'validation_size': args.val_size,
            'use_onehot': args.use_onehot
        }

    train_from_csv(args.csv, args.output, config)
