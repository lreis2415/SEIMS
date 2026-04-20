"""Main entry point for surrogate model training."""
import sys
import os
import argparse
import json
from datetime import datetime

# Add SEIMS root to path
seims_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
if seims_root not in sys.path:
    sys.path.insert(0, seims_root)

from surrogate_model.utils import SurrogateConfig, setup_logger
from surrogate_model.utils.config_converter import json_to_ini
from surrogate_model.sampling import ScenarioSampler
from surrogate_model.simulation import BatchRunner
from surrogate_model.training import DataPreprocessor, ModelEvaluator
# Delay import of MultiOutputANN to avoid TensorFlow loading at startup


def main(config_file: str):
    """Main training pipeline."""
    # Load configuration
    config = SurrogateConfig(config_file)

    # Setup directories
    work_dir = config.work_dir
    os.makedirs(work_dir, exist_ok=True)
    os.makedirs(os.path.join(work_dir, 'samples'), exist_ok=True)
    os.makedirs(os.path.join(work_dir, 'simulation_results'), exist_ok=True)
    os.makedirs(os.path.join(work_dir, 'training'), exist_ok=True)
    os.makedirs(os.path.join(work_dir, 'models'), exist_ok=True)
    os.makedirs(os.path.join(work_dir, 'plots'), exist_ok=True)

    # Setup logger
    logger = setup_logger(work_dir)
    logger.info("=" * 80)
    logger.info("Surrogate Model Training Pipeline")
    logger.info("=" * 80)

    # Generate INI from JSON config
    base_ini = config.get('seims', 'config_ini', '')
    if not base_ini or not os.path.exists(base_ini):
        base_ini = 'D:/EGC/SEIMS-dev/data/youwuzhen/model_configs/scenario_analysis.ini'

    config_ini = os.path.join(work_dir, 'generated_config.ini')
    with open(config_file, 'r') as f:
        json_config = json.load(f)
    json_to_ini(json_config, base_ini, config_ini)
    logger.info(f"Generated config.ini from JSON: {config_ini}")
    logger.info(f"Using base INI for BMP settings: {base_ini}")

    # Stage 1: Generate BMP scenario samples
    print("\n[1/6] Generating BMP scenario samples...")
    sampler = ScenarioSampler(config_ini)
    samples = sampler.sample(
        n_samples=config.n_samples,
        seed=config.get('sampling', 'seed', 12345)
    )
    samples_file = os.path.join(work_dir, 'samples', 'samples.csv')
    sampler.save_samples(samples, samples_file)
    logger.info(f"Generated {len(samples)} samples with {sampler.gene_num} genes each")

    # Stage 2: Run SEIMS simulations
    runner = BatchRunner(config_ini, config.output_vars)
    n_jobs = config.get('simulation', 'parallel_jobs', 4)
    results_df = runner.run(samples, n_jobs=n_jobs)

    results_file = os.path.join(work_dir, 'simulation_results', 'collected_data.csv')
    runner.save_results(results_df, results_file)
    logger.info(f"Simulation completed: {len(results_df)} successful runs")

    # Stage 3: Prepare training data
    print("\n[3/6] Preparing training data...")
    preprocessor = DataPreprocessor(config.output_vars)
    X_train, X_val, X_test, y_train, y_val, y_test = preprocessor.prepare_data(
        results_df,
        test_size=config.test_size,
        val_size=config.get('training', 'validation_size', 0.1),
        random_state=42
    )

    models_dir = os.path.join(work_dir, 'models')
    preprocessor.save_scalers(models_dir)
    logger.info("Data preprocessing completed")

    # Stage 4: Build and train model
    print("\n[4/6] Training neural network...")
    try:
        from surrogate_model.models import MultiOutputANN
        use_tensorflow = True
    except ImportError as e:
        logger.warning(f"TensorFlow not available: {e}")
        logger.info("Using scikit-learn MLPRegressor instead")
        print("Using scikit-learn for training (TensorFlow not available)")
        use_tensorflow = False

    if use_tensorflow:
        model = MultiOutputANN(
            input_dim=X_train.shape[1],
            output_dim=len(config.output_vars),
            hidden_layers=config.get('training', 'hidden_layers', [128, 64, 32])
        )

        history = model.train(
            X_train, y_train,
            X_val, y_val,
            epochs=config.epochs,
        batch_size=config.get('training', 'batch_size', 32),
        output_dir=models_dir
    )
    else:
        # Use scikit-learn
        from surrogate_model.models.sklearn_ann import SklearnANN

        model = SklearnANN(
            input_dim=X_train.shape[1],
            output_dim=len(config.output_vars),
            hidden_layers=config.get('training', 'hidden_layers', [128, 64, 32])
        )

        model.fit(X_train, y_train, X_val, y_val)
        history = None  # sklearn doesn't return history

    logger.info("Model training completed")

    # Stage 5: Evaluate model
    print("\n[5/6] Evaluating model...")
    evaluator = ModelEvaluator(config.output_vars)

    # Evaluate on test set
    y_pred_test = model.predict(X_test)
    metrics_test = evaluator.evaluate(y_test, y_pred_test)
    evaluator.print_metrics(metrics_test, "Test Set Performance")

    # Plot results
    plots_dir = os.path.join(work_dir, 'plots')
    evaluator.plot_predictions(y_test, y_pred_test, plots_dir, preprocessor.scaler_y)
    if history is not None:
        evaluator.plot_training_history(history, plots_dir)

    # Stage 6: Save model and metadata
    print("\n[6/6] Saving model and metadata...")
    if use_tensorflow:
        model_file = os.path.join(models_dir, 'surrogate_model.h5')
    else:
        model_file = os.path.join(models_dir, 'surrogate_model.pkl')
    model.save(model_file)

    # Save metadata
    metadata = {
        'model_type': 'sklearn_mlp' if not use_tensorflow else 'multi_output_ann',
        'input_dim': X_train.shape[1],
        'output_dim': len(config.output_vars),
        'output_vars': config.output_vars,
        'n_samples': len(samples),
        'n_successful': len(results_df),
        'training_date': datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
        'performance': metrics_test,
        'config': {
            'model_dir': config.model_dir,
            'spatial_unit': config.get('spatial', 'unit'),
            'config_method': config.get('spatial', 'config_method')
        }
    }

    metadata_file = os.path.join(models_dir, 'model_metadata.json')
    with open(metadata_file, 'w') as f:
        json.dump(metadata, f, indent=2)

    # Save evaluation report
    report_file = os.path.join(models_dir, 'evaluation_report.txt')
    with open(report_file, 'w', encoding='utf-8') as f:
        f.write("=" * 80 + "\n")
        f.write("Surrogate Model Evaluation Report\n")
        f.write("=" * 80 + "\n\n")
        f.write(f"Training Date: {metadata['training_date']}\n")
        f.write(f"Samples: {metadata['n_samples']} (successful: {metadata['n_successful']})\n")
        f.write(f"Input Dimension: {metadata['input_dim']}\n")
        f.write(f"Output Variables: {', '.join(metadata['output_vars'])}\n\n")
        f.write("-" * 80 + "\n")
        f.write("Test Set Performance\n")
        f.write("-" * 80 + "\n")
        for var, m in metrics_test.items():
            f.write(f"\n{var}:\n")
            f.write(f"  R2 = {m['r2']:.4f}\n")
            f.write(f"  RMSE = {m['rmse']:.4f}\n")
            f.write(f"  MAE = {m['mae']:.4f}\n")

    print("\n" + "=" * 80)
    print("Training completed successfully!")
    print("=" * 80)
    print(f"Model saved to: {model_file}")
    print(f"Scalers saved to: {models_dir}")
    print(f"Metadata saved to: {metadata_file}")
    print(f"Plots saved to: {plots_dir}")
    print("\nTo use the surrogate model in optimization:")
    print(f"  from surrogate_model.predict import SurrogatePredictor")
    print(f"  predictor = SurrogatePredictor(")
    print(f"      '{model_file}',")
    print(f"      '{os.path.join(models_dir, 'scaler_X.pkl')}',")
    print(f"      '{os.path.join(models_dir, 'scaler_y.pkl')}',")
    print(f"      '{metadata_file}'")
    print(f"  )")
    print(f"  result = predictor.predict(gene_values)")
    print("=" * 80)

    logger.info("All stages completed successfully")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Train surrogate model for SEIMS')
    parser.add_argument('--config', type=str, required=True,
                        help='Path to configuration JSON file')
    args = parser.parse_args()

    main(args.config)
