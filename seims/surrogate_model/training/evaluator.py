"""Model evaluator."""
import numpy as np
from sklearn.metrics import r2_score, mean_squared_error, mean_absolute_error
from typing import Dict, List
import matplotlib.pyplot as plt
import seaborn as sns
import os


class ModelEvaluator:
    """Evaluate surrogate model performance."""

    def __init__(self, output_vars: List[str]):
        """
        Initialize evaluator.

        Args:
            output_vars: List of output variable names
        """
        self.output_vars = output_vars

    def evaluate(self, y_true: np.ndarray, y_pred: np.ndarray) -> Dict:
        """
        Evaluate model performance.

        Args:
            y_true: True values (scaled)
            y_pred: Predicted values (scaled)

        Returns:
            Dictionary of metrics for each output variable
        """
        metrics = {}

        for i, var in enumerate(self.output_vars):
            y_t = y_true[:, i] if y_true.ndim > 1 else y_true
            y_p = y_pred[:, i] if y_pred.ndim > 1 else y_pred

            metrics[var] = {
                'r2': r2_score(y_t, y_p),
                'rmse': np.sqrt(mean_squared_error(y_t, y_p)),
                'mae': mean_absolute_error(y_t, y_p)
            }

        return metrics

    def print_metrics(self, metrics: Dict, title: str = "Model Performance"):
        """Print evaluation metrics."""
        print(f"\n{title}")
        print("=" * 60)
        for var, m in metrics.items():
            print(f"{var}:")
            print(f"  R2 = {m['r2']:.4f}")
            print(f"  RMSE = {m['rmse']:.4f}")
            print(f"  MAE = {m['mae']:.4f}")

    def plot_predictions(self, y_true: np.ndarray, y_pred: np.ndarray,
                         output_dir: str, scaler_y=None):
        """
        Plot predictions vs true values.

        Args:
            y_true: True values (scaled)
            y_pred: Predicted values (scaled)
            output_dir: Directory to save plots
            scaler_y: Scaler to inverse transform values
        """
        os.makedirs(output_dir, exist_ok=True)

        # Inverse transform if scaler provided
        if scaler_y is not None:
            # Ensure 2D shape for scaler
            if y_true.ndim == 1:
                y_true = y_true.reshape(-1, 1)
            if y_pred.ndim == 1:
                y_pred = y_pred.reshape(-1, 1)
            y_true = scaler_y.inverse_transform(y_true)
            y_pred = scaler_y.inverse_transform(y_pred)

        for i, var in enumerate(self.output_vars):
            y_t = y_true[:, i] if y_true.ndim > 1 else y_true
            y_p = y_pred[:, i] if y_pred.ndim > 1 else y_pred

            plt.figure(figsize=(8, 8))
            plt.scatter(y_t, y_p, alpha=0.5, s=20)

            # Perfect prediction line
            min_val = min(y_t.min(), y_p.min())
            max_val = max(y_t.max(), y_p.max())
            plt.plot([min_val, max_val], [min_val, max_val], 'r--', lw=2, label='Perfect prediction')

            plt.xlabel(f'True {var}')
            plt.ylabel(f'Predicted {var}')
            plt.title(f'{var} Predictions vs True Values')
            plt.legend()
            plt.grid(True, alpha=0.3)
            plt.tight_layout()

            plot_file = os.path.join(output_dir, f'predictions_{var}.png')
            plt.savefig(plot_file, dpi=150)
            plt.close()

        print(f"  Saved prediction plots to {output_dir}")

    def plot_training_history(self, history: dict, output_dir: str):
        """Plot training history."""
        os.makedirs(output_dir, exist_ok=True)

        plt.figure(figsize=(12, 4))

        # Loss
        plt.subplot(1, 2, 1)
        plt.plot(history['loss'], label='Train Loss')
        plt.plot(history['val_loss'], label='Val Loss')
        plt.xlabel('Epoch')
        plt.ylabel('Loss (MSE)')
        plt.title('Training History - Loss')
        plt.legend()
        plt.grid(True, alpha=0.3)

        # MAE
        plt.subplot(1, 2, 2)
        plt.plot(history['mae'], label='Train MAE')
        plt.plot(history['val_mae'], label='Val MAE')
        plt.xlabel('Epoch')
        plt.ylabel('MAE')
        plt.title('Training History - MAE')
        plt.legend()
        plt.grid(True, alpha=0.3)

        plt.tight_layout()
        plot_file = os.path.join(output_dir, 'training_history.png')
        plt.savefig(plot_file, dpi=150)
        plt.close()

        print(f"  Saved training history plot to {plot_file}")
