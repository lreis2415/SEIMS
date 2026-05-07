"""Benchmark: SEIMS vs Surrogate Model optimization speed - Simplified."""
import time
import numpy as np
import pickle

print("=" * 80)
print("Optimization Speed Benchmark: SEIMS vs Surrogate Model")
print("=" * 80)

# Configuration
n_individuals = 60
n_generations = 10
n_genes = 105

print(f"\nScenario: {n_individuals} individuals x {n_generations} generations")
print(f"Total evaluations: {n_individuals * n_generations}")

# ============================================================================
# 1. SEIMS-based optimization
# ============================================================================
print("\n" + "=" * 80)
print("[1] SEIMS-based Optimization")
print("=" * 80)

seims_time_per_eval = 6.4  # seconds (from 500-sample training)
total_seims_time = n_individuals * n_generations * seims_time_per_eval

print(f"  Time per evaluation: {seims_time_per_eval:.1f} seconds")
print(f"  Total time (sequential): {total_seims_time:.0f} seconds = {total_seims_time/60:.1f} minutes")
print(f"  Total time (8 parallel): {total_seims_time/8:.0f} seconds = {total_seims_time/8/60:.1f} minutes")

# ============================================================================
# 2. Surrogate model-based optimization
# ============================================================================
print("\n" + "=" * 80)
print("[2] Surrogate Model-based Optimization")
print("=" * 80)

model_file = "D:/EGC/SEIMS-dev/data/youwuzhen/surrogate_training_slppos_500/models/surrogate_model.pkl"

try:
    with open(model_file, 'rb') as f:
        model = pickle.load(f)

    # Generate random scenarios
    np.random.seed(42)
    scenarios = np.random.randint(0, 5, size=(n_individuals * n_generations, n_genes)).astype(float)

    # Benchmark
    start_time = time.time()
    predictions = model.predict(scenarios)
    end_time = time.time()

    surrogate_time = end_time - start_time
    time_per_eval = surrogate_time / len(scenarios)

    print(f"  Time per evaluation: {time_per_eval*1000:.2f} milliseconds")
    print(f"  Total time: {surrogate_time:.3f} seconds")

    # ============================================================================
    # 3. Comparison
    # ============================================================================
    print("\n" + "=" * 80)
    print("Speed Comparison")
    print("=" * 80)

    speedup_seq = total_seims_time / surrogate_time
    speedup_par = (total_seims_time / 8) / surrogate_time

    print(f"\n{'Method':<30} {'Time':<20} {'Speedup':<15}")
    print("-" * 65)
    print(f"{'SEIMS (sequential)':<30} {total_seims_time/60:>8.1f} min       {'1x':<15}")
    print(f"{'SEIMS (8 parallel)':<30} {total_seims_time/8/60:>8.1f} min       {8:>8.0f}x")
    print(f"{'Surrogate Model':<30} {surrogate_time:>8.3f} sec       {speedup_seq:>8.0f}x")

    print("\n" + "=" * 80)
    print("Key Findings")
    print("=" * 80)
    print(f"  - Surrogate is {speedup_seq:,.0f}x faster than sequential SEIMS")
    print(f"  - Surrogate is {speedup_par:,.0f}x faster than parallel SEIMS (8 jobs)")
    print(f"  - 60 individuals x 10 generations:")
    print(f"      SEIMS: {total_seims_time/8/60:.1f} minutes")
    print(f"      Surrogate: {surrogate_time:.2f} seconds")

    # Practical examples
    print("\n" + "=" * 80)
    print("Practical Optimization Scenarios")
    print("=" * 80)

    configs = [
        ("Quick test", 20, 50),
        ("Standard", 60, 100),
        ("Thorough", 100, 200),
        ("Extensive", 200, 500),
    ]

    print(f"\n{'Scenario':<15} {'Pop x Gen':<15} {'Evaluations':<15} {'SEIMS (8j)':<15} {'Surrogate':<15}")
    print("-" * 85)
    for name, pop, gen in configs:
        n_evals = pop * gen
        seims_t = n_evals * seims_time_per_eval / 8
        surr_t = n_evals * time_per_eval

        if seims_t < 60:
            seims_str = f"{seims_t:.1f} sec"
        elif seims_t < 3600:
            seims_str = f"{seims_t/60:.1f} min"
        else:
            seims_str = f"{seims_t/3600:.1f} hr"

        if surr_t < 1:
            surr_str = f"{surr_t*1000:.0f} ms"
        else:
            surr_str = f"{surr_t:.2f} sec"

        print(f"{name:<15} {pop} x {gen:<10} {n_evals:<15} {seims_str:<15} {surr_str:<15}")

    print("\n" + "=" * 80)
    print("Conclusion: Surrogate model enables real-time optimization")
    print("=" * 80)

except FileNotFoundError:
    print("  [ERROR] Model not found - wait for training to complete")
    print("=" * 80)
