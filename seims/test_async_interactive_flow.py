#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Async Interactive Optimization Complete Flow Test
"""
import sys
import os
import tempfile
import json
import pickle
import time

sys.path.insert(0, 'seims')

print('=' * 70)
print('ASYNC INTERACTIVE OPTIMIZATION - COMPLETE FLOW TEST')
print('=' * 70)

from scenario_analysis.async_interactive_algorithm import AsyncInteractiveAlgorithm

# Create temp directories
temp_dir = tempfile.mkdtemp()
signal_dir = os.path.join(temp_dir, 'signals')
checkpoint_dir = os.path.join(temp_dir, 'checkpoints')
os.makedirs(signal_dir, exist_ok=True)
os.makedirs(checkpoint_dir, exist_ok=True)

print()
print('STEP 1: Initialize AsyncInteractiveAlgorithm')
print('-' * 50)

# Create IA with async enabled
ia = AsyncInteractiveAlgorithm(
    enable_interactive=True,
    interactive_interval=30,  # Interact every 30 generations
    users={
        'user1': {'preference_param': {'economy': [80, 'less', 20], 'environment': [5, 'greater', 2]}},
        'user2': {'preference_param': {'economy': [70, 'less', 15], 'environment': [4, 'greater', 1]}}
    },
    enable_async=True,
    task_id='test_flow',
    signal_dir=signal_dir,
    checkpoint_dir=checkpoint_dir,
    timeout_seconds=10
)

print('  task_id:', ia.task_id)
print('  interactive_interval:', ia.interactive_interval)
print('  users:', list(ia.users.keys()))
print('  enable_async:', ia.enable_async)
print('  merged_prefs:', ia.merged_prefs)

# Mock population
class MockInd:
    def __init__(self, id, genes, economy, env):
        self.id = id
        self.genes = genes
        self.economy = economy
        self.environment = env
        self.fitness = type('Fitness', (), {'values': (economy, env)})()
    def __iter__(self):
        return iter(self.genes)

def create_mock_pop(size=10, start_id=0):
    return [MockInd(start_id + i, [1.0, 2.0, 3.0], 100.0 + i*10, 0.5 + i*0.05) for i in range(size)]

mock_pop = create_mock_pop(10)

print()
print('STEP 2: Simulate Optimization Running (Gens 1-29)')
print('-' * 50)
print('  Optimization running... no interaction needed')

print()
print('STEP 3: Generation 30 - Interaction Point Reached')
print('-' * 50)
print('  should_interact(30):', ia.should_interact(30))

print()
print('STEP 4: Write WAIT_INTERACTION Signal')
print('-' * 50)
ia._write_wait_signal(mock_pop, 30)

wait_file = os.path.join(signal_dir, 'WAIT_INTERACTION_test_flow_gen30.signal')
solutions_file = os.path.join(signal_dir, 'SOLUTIONS_test_flow_gen30.json')

print('  WAIT_INTERACTION file exists:', os.path.exists(wait_file))
print('  SOLUTIONS file exists:', os.path.exists(solutions_file))

# Read and display signal content
with open(wait_file, 'r') as f:
    signal_data = json.load(f)
print()
print('  Signal Content:')
print('    type:', signal_data['type'])
print('    generation:', signal_data['generation'])
print('    population size:', signal_data['population_summary']['size'])
print('    timestamp:', signal_data['timestamp'])

print()
print('STEP 5: Save Checkpoint')
print('-' * 50)
ia._save_checkpoint(mock_pop, 30)
checkpoint_file = os.path.join(checkpoint_dir, 'checkpoint_test_flow_gen30.pkl')
print('  Checkpoint file exists:', os.path.exists(checkpoint_file))

print()
print('STEP 6: User Submits Preferences (Write CONTINUE file)')
print('-' * 50)

continue_data = {
    'type': 'CONTINUE',
    'task_id': 'test_flow',
    'generation': 30,
    'users_preferences': [
        {'economy': [60, 'less', 15], 'environment': [3, 'greater', 1]},
        {'economy': [70, 'less', 10], 'environment': [8, 'greater', 3]}
    ],
    'merge_method': 'weighted_sum'
}

continue_file = os.path.join(signal_dir, 'CONTINUE_test_flow_gen30.json')
with open(continue_file, 'w') as f:
    json.dump(continue_data, f, indent=2)
print('  CONTINUE file written:', os.path.exists(continue_file))

print()
print('STEP 7: New Container - Load from Checkpoint')
print('-' * 50)

ia_new = AsyncInteractiveAlgorithm(
    enable_interactive=True,
    interactive_interval=30,
    users={
        'user1': {'preference_param': {'economy': [80, 'less', 20], 'environment': [5, 'greater', 2]}},
        'user2': {'preference_param': {'economy': [70, 'less', 15], 'environment': [4, 'greater', 1]}}
    },
    enable_async=True,
    task_id='test_flow',
    signal_dir=signal_dir,
    checkpoint_dir=checkpoint_dir,
    timeout_seconds=10
)

print('  Before loading - merged_prefs:', ia_new.merged_prefs)

loaded_pop = ia_new.load_from_checkpoint(30)

print('  After loading from checkpoint:')
print('    Population loaded:', loaded_pop is not None)
print('    merged_prefs restored:', ia_new.merged_prefs)

print()
print('STEP 8: Update Preferences from CONTINUE')
print('-' * 50)
ia_new._update_preferences_from_continue(continue_data)
print('  After _update_preferences_from_continue:')
print('    current_prefs:', ia_new.current_prefs)

print()
print('STEP 9: Continue Optimization (Gens 31-60)')
print('-' * 50)
print('  Optimization continues with updated preferences...')

print()
print('STEP 10: Generation 60 - Second Interaction Point')
print('-' * 50)
print('  should_interact(60):', ia_new.should_interact(60))

# Save checkpoint for gen 60
new_pop = create_mock_pop(10, start_id=300)
ia_new._save_checkpoint(new_pop, 60)
ia_new._write_wait_signal(new_pop, 60)

print('  Checkpoint saved for gen 60')
print('  WAIT_INTERACTION signal written for gen 60')

print()
print('STEP 11: Verify Multiple Checkpoints Exist')
print('-' * 50)
gen_30_checkpoint = os.path.join(checkpoint_dir, 'checkpoint_test_flow_gen30.pkl')
gen_60_checkpoint = os.path.join(checkpoint_dir, 'checkpoint_test_flow_gen60.pkl')
print('  Gen 30 checkpoint exists:', os.path.exists(gen_30_checkpoint))
print('  Gen 60 checkpoint exists:', os.path.exists(gen_60_checkpoint))

latest_gen = ia_new.get_latest_checkpoint_gen()
print('  Latest checkpoint gen:', latest_gen)

print()
print('STEP 12: Write Second CONTINUE for Gen 60')
print('-' * 50)

continue_data_60 = {
    'type': 'CONTINUE',
    'task_id': 'test_flow',
    'generation': 60,
    'users_preferences': [
        {'economy': [50, 'less', 10], 'environment': [6, 'greater', 2]},
        {'economy': [65, 'less', 8], 'environment': [10, 'greater', 4]}
    ],
    'merge_method': 'weighted_sum'
}

continue_file_60 = os.path.join(signal_dir, 'CONTINUE_test_flow_gen60.json')
with open(continue_file_60, 'w') as f:
    json.dump(continue_data_60, f, indent=2)
print('  Second CONTINUE file written')

# Load gen 60 checkpoint in new container
ia_final = AsyncInteractiveAlgorithm(
    enable_interactive=True,
    interactive_interval=30,
    users={
        'user1': {'preference_param': {'economy': [80, 'less', 20], 'environment': [5, 'greater', 2]}},
        'user2': {'preference_param': {'economy': [70, 'less', 15], 'environment': [4, 'greater', 1]}}
    },
    enable_async=True,
    task_id='test_flow',
    signal_dir=signal_dir,
    checkpoint_dir=checkpoint_dir,
    timeout_seconds=10
)

loaded_pop_60 = ia_final.load_from_checkpoint(60)
ia_final._update_preferences_from_continue(continue_data_60)

print('  Loaded checkpoint gen 60')
print('  merged_prefs after 2nd update:', ia_final.merged_prefs)

print()
print('=' * 70)
print('ASYNC INTERACTIVE OPTIMIZATION FLOW TEST COMPLETED SUCCESSFULLY')
print('=' * 70)

# Cleanup
import shutil
shutil.rmtree(temp_dir)
