# Surrogate Model Training Framework for SEIMS

## Quick Start

### 1. Install Dependencies

```bash
cd /d/EGC/SEIMS-dev/seims/surrogate_model
pip install -r requirements.txt
```

### 2. Prepare Configuration

Edit `config/example_config.json` or create your own configuration file.

### 3. Run Training

```bash
python main.py --config config/example_config.json
```

## Configuration Parameters

See `USER_GUIDE.md` for detailed parameter descriptions.

## Output Files

After training, the following files will be generated in `work_dir`:

```
work_dir/
├── samples/
│   └── samples.csv                  # Generated BMP scenarios
├── simulation_results/
│   └── collected_data.csv           # SEIMS simulation results
├── models/
│   ├── surrogate_model.h5           # Trained model
│   ├── scaler_X.pkl                 # Input scaler
│   ├── scaler_y.pkl                 # Output scaler
│   ├── model_metadata.json          # Model metadata
│   └── evaluation_report.txt        # Performance report
├── plots/
│   ├── predictions_*.png            # Prediction plots
│   └── training_history.png         # Training curves
└── logs/
    └── surrogate_*.log              # Training logs
```

## Using the Surrogate Model

```python
from surrogate_model.predict import SurrogatePredictor

predictor = SurrogatePredictor(
    'path/to/surrogate_model.h5',
    'path/to/scaler_X.pkl',
    'path/to/scaler_y.pkl',
    'path/to/model_metadata.json'
)

# Predict for a single scenario
result = predictor.predict(gene_values)
print(result)  # {'SED': 12345.6, 'Q': 234.5, ...}

# Predict for multiple scenarios
results = predictor.predict_batch([gene_values_1, gene_values_2, ...])
```

## Integration with Optimizer

See `USER_GUIDE.md` for detailed integration instructions.
