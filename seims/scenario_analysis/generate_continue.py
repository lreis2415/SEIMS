# -*- coding: utf-8 -*-
"""
generate_continue.py — 异步交互偏好更新独立工具

功能：
  根据用户对当前 Pareto 前沿的好/差方案分类，调用 update_preference_params()
  计算新的偏好参数，生成 CONTINUE JSON 供优化器续跑使用。

使用方式:
  python generate_continue.py \\
      --checkpoint /data/results/ci_async_ckpt/checkpoint_ci_test_async_gen10.pkl \\
      --classification classification.json \\
      --output /data/results/ci_async_signals/CONTINUE_ci_test_async_gen10.json

classification.json 格式:
  {
    "users": [
      {
        "user_id": "user1",
        "good_ids": [884125816, 142716674],
        "bad_ids": [637525276],
        "bad_reasons": {
          "637525276": ["economy", "cost_variation"]
        }
      }
    ]
  }

  bad_reasons 可选指标: economy, environment, cost_variation,
                         return_on_invest, env_on_invest, abandon_possibility
"""

import os
import sys
import json
import pickle
import argparse
import time
import logging

# 确保可以导入 deap_tool
_this_dir = os.path.dirname(os.path.abspath(__file__))
if _this_dir not in sys.path:
    sys.path.insert(0, _this_dir)

from deap_tool import update_preference_params

logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')
logger = logging.getLogger(__name__)

MAX_HISTORY_SIZE = 16


def load_checkpoint(checkpoint_file):
    """加载 checkpoint，返回 (population, users, generation)"""
    with open(checkpoint_file, 'rb') as f:
        data = pickle.load(f)
    population = data['population']
    users = data.get('users', {})
    generation = data.get('generation', -1)
    logger.info(f"Checkpoint loaded: gen={generation}, pop_size={len(population)}")
    return population, users, generation


def compute_user_preference(user_cls, population, users_state):
    """
    为单个用户计算新偏好参数。

    Args:
        user_cls: dict，包含 user_id, good_ids, bad_ids, bad_reasons
        population: checkpoint 中的种群列表
        users_state: checkpoint 中 users 状态 {user_id: {history_good, ...}}

    Returns:
        (user_id, new_preference_params)
    """
    user_id = user_cls.get('user_id', 'user1')
    good_ids = [int(i) for i in user_cls.get('good_ids', [])]
    bad_ids = [int(i) for i in user_cls.get('bad_ids', [])]
    bad_reasons_map = {str(i): v for i, v in user_cls.get('bad_reasons', {}).items()}

    id_to_ind = {ind.id: ind for ind in population}
    pop_ids = set(id_to_ind.keys())

    # 验证 ID 有效性
    pop_ids = set(id_to_ind.keys())
    invalid_good = [i for i in good_ids if i not in pop_ids]
    invalid_bad = [i for i in bad_ids if i not in pop_ids]
    if invalid_good or invalid_bad:
        logger.warning(f"User {user_id}: invalid good IDs {invalid_good}, invalid bad IDs {invalid_bad}")

    new_good_inds = [id_to_ind[i] for i in good_ids if i in id_to_ind]
    new_bad_inds = [id_to_ind[i] for i in bad_ids if i in id_to_ind]
    new_bad_reasons = [bad_reasons_map.get(str(i), []) for i in bad_ids if i in id_to_ind]

    # 合并历史
    user_data = users_state.get(user_id, {})
    hist_good = user_data.get('history_good', [])
    hist_bad = user_data.get('history_bad', [])
    hist_bad_reasons = user_data.get('history_bad_reasons', [])

    combined_good = hist_good + new_good_inds
    combined_bad = hist_bad + new_bad_inds
    combined_reasons = hist_bad_reasons + new_bad_reasons

    # 去重（保留最新）
    good_pop = list({ind.id: ind for ind in combined_good}.values())
    bad_pop = list({ind.id: ind for ind in combined_bad}.values())

    # 对齐原因列表（按最终 bad_pop 顺序）
    bad_id_to_reason = {}
    bad_id_to_reason = {}
    for ind, reason in zip(combined_bad, combined_reasons):
        bad_id_to_reason[ind.id] = reason  # 后面的覆盖前面的（保留最新）
    final_reasons = [bad_id_to_reason.get(ind.id, []) for ind in bad_pop]

    # 截断历史大小
    if len(good_pop) > MAX_HISTORY_SIZE:
        good_pop = good_pop[-MAX_HISTORY_SIZE:]
    if len(bad_pop) > MAX_HISTORY_SIZE:
        bad_pop = bad_pop[-MAX_HISTORY_SIZE:]
        final_reasons = final_reasons[-MAX_HISTORY_SIZE:]

    old_params = user_data.get('preference_param', {})

    logger.info(f"User {user_id}: {len(good_pop)} good (history+new), {len(bad_pop)} bad (history+new)")

    if not good_pop or not bad_pop:
        logger.warning(f"User {user_id}: insufficient good/bad samples, keeping old preferences")
        return user_id, old_params

    new_params = update_preference_params(good_pop, bad_pop, old_params, final_reasons)
    logger.info(f"User {user_id}: preferences updated → {list(new_params.keys())}")
    return user_id, new_params


def generate_continue(checkpoint_file, classification_file, output_file):
    """
    主流程：读取 checkpoint + 分类文件，计算偏好参数，写入 CONTINUE JSON。
    """
    # 1. 加载 checkpoint
    population, users_state, generation = load_checkpoint(checkpoint_file)

    # 2. 加载分类
    with open(classification_file, 'r', encoding='utf-8') as f:
        classification = json.load(f)

    # 支持顶层 {users: [...]} 或直接 [{...}] 两种格式
    users_cls_list = classification.get('users', [classification])
    if isinstance(users_cls_list, dict):
        users_cls_list = [users_cls_list]

    # 3. 为每个用户计算新偏好参数
    users_preferences = []
    for user_cls in users_cls_list:
        user_id, new_params = compute_user_preference(user_cls, population, users_state)

        # 将 tuple 转为 list 以便 JSON 序列化
        serializable_params = {}
        serializable_params = {}
        for metric, val in new_params.items():
            if metric == 'bmp_rules':
                serializable_params[metric] = val
            else:
                serializable_params[metric] = list(val)

        users_preferences.append({
            'user_id': user_id,
            'preference_params': serializable_params
        })

    # 4. 写入 CONTINUE JSON
    continue_data = {
        'users_preferences': users_preferences,
        'merge_method': 'weighted_sum',
        'generated_from_generation': generation,
        'timestamp': time.strftime('%Y-%m-%dT%H:%M:%S')
    }

    os.makedirs(os.path.dirname(os.path.abspath(output_file)), exist_ok=True)
    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(continue_data, f, indent=2, ensure_ascii=False)

    logger.info(f"CONTINUE JSON written: {output_file}")
    return continue_data


def main():
    parser = argparse.ArgumentParser(
        description='生成 CONTINUE JSON 供异步交互优化器续跑使用',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    parser.add_argument('--checkpoint', '-c', required=True,
                        help='checkpoint .pkl 文件路径')
    parser.add_argument('--classification', '-i', required=True,
                        help='用户分类 JSON 文件路径')
    parser.add_argument('--output', '-o', required=True,
                        help='输出 CONTINUE JSON 文件路径')
    parser.add_argument('--dry-run', action='store_true',
                        help='仅计算，不写文件，打印结果到 stdout')

    args = parser.parse_args()

    result = generate_continue(args.checkpoint, args.classification, args.output)

    if args.dry_run:
        print(json.dumps(result, indent=2, ensure_ascii=False))
    else:
        print(f"Done. CONTINUE JSON: {args.output}")


if __name__ == '__main__':
    main()
