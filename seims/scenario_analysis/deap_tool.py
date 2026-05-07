import bisect
from collections import defaultdict, namedtuple
from itertools import chain
import math
from operator import attrgetter, itemgetter
import random
import copy
import numpy

######################################
# Non-Dominated Sorting   (NSGA-II)  #
######################################
import numpy as np


def selNSGA2(individuals, k, nd='standard'):
    """Apply NSGA-II selection operator on the *individuals*. Usually, the
    size of *individuals* will be larger than *k* because any individual
    present in *individuals* will appear in the returned list at most once.
    Having the size of *individuals* equals to *k* will have no effect other
    than sorting the population according to their front rank. The
    list returned contains references to the input *individuals*. For more
    details on the NSGA-II operator see [Deb2002]_.

    :param individuals: A list of individuals to select from.
    :param k: The number of individuals to select.
    :param nd: Specify the non-dominated algorithm to use: 'standard' or 'log'.
    :returns: A list of selected individuals.

    .. [Deb2002] Deb, Pratab, Agarwal, and Meyarivan, "A fast elitist
       non-dominated sorting genetic algorithm for multi-objective
       optimization: NSGA-II", 2002.
    """
    if nd == 'standard':
        pareto_fronts = sortNondominated(individuals, k)
    elif nd == 'log':
        pareto_fronts = sortLogNondominated(individuals, k)
    else:
        raise Exception('selNSGA2: The choice of non-dominated sorting '
                        'method "{0}" is invalid.'.format(nd))

    for front in pareto_fronts:
        assignCrowdingDist(front)

    chosen = list(chain(*pareto_fronts[:-1]))
    k = k - len(chosen)
    if k > 0:
        sorted_front = sorted(pareto_fronts[-1], key=attrgetter("fitness.crowding_dist"), reverse=True)
        chosen.extend(sorted_front[:k])

    return chosen


def selNSGA2_prefer(original_pop, k, nd='standard', preference_params=None):
    """Apply NSGA-II selection operator with preference-based scoring."""
    if preference_params is None:
        raise ValueError("preference_params must be provided for preference-based scoring")
    # 创建种群深拷贝
    individuals = copy.deepcopy(original_pop)
    # Non-dominated sorting
    if nd == 'standard':
        pareto_fronts = sortNondominated(individuals, k)
    elif nd == 'log':
        pareto_fronts = sortLogNondominated(individuals, k)
    else:
        raise Exception('selNSGA2: Invalid non-dominated sorting method')

    # 为每个前沿分配偏好得分
    for front_id, front in enumerate(pareto_fronts):
        assign_preference_scores(front, preference_params, front_id + 1)
        assignCrowdingDist(front)
    # Select individuals from the fronts
    chosen = list(chain(*pareto_fronts[:-1]))
    k_remaining = k - len(chosen)

    if k_remaining > 0:
        # Sort last front by preference score (descending) and select top k_remaining
        #sorted_front = sorted(pareto_fronts[-1], key=lambda x: x.fitness.preference_score, reverse=True)

        sorted_front = sorted(
            pareto_fronts[-1],
            key=lambda x: (-x.fitness.preference_score, -x.fitness.crowding_dist)
        )
        chosen.extend(sorted_front[:k_remaining])

    return chosen


def selNSGA2_multiuser_prefer(original_pop, k, nd='standard', user_preference_params=None):
    """Apply preference selection per user, then merge the selected populations."""
    if not user_preference_params:
        return selNSGA2(original_pop, k, nd=nd)

    user_pops = []
    for preference_params in user_preference_params:
        if not preference_params:
            continue
        user_pops.append(
            selNSGA2_prefer(original_pop, k, nd=nd, preference_params=preference_params)
        )

    if not user_pops:
        return selNSGA2(original_pop, k, nd=nd)
    if len(user_pops) == 1:
        return user_pops[0]

    return merge_multiuser_pops(user_pops, k)


def satisfaction_score(value, h, direction, r):
    """计算单个指标的满意度得分（高斯衰减）"""
    # 安全处理r=0的情况
    #safe_r = max(r, 1e-10)
    if direction == 'less':
        if value <= h:
            return 1.0
        if value >= h+r:
            return 0.0
        deviation = (value - h)
    elif direction == 'greater':
        if value >= h:
            return 1.0
        if value <= h-r:
            return 0.0
        deviation = (h - value)
    elif direction == 'equal':
        return 1.0
    elif direction == 'between':
        #deviation = abs(value - h) / r + 1
        deviation = 1
        if h >= value >= r:
            return 1.0
        else:
            return 0.0
    else:
        raise ValueError(f"Invalid direction: {direction}. Use 'less' or 'greater'.")

    # 使用余弦函数：当 delta/r=0.5 时，结果为 0.5
    ratio = deviation / r
    #exponent = math.log(5)  # ln(9) ≈ 2.1972
    #score =  math.exp(-exponent * deviation)

    score = 0.5 * (1 + math.cos(math.pi * ratio))
    return round(score, 6)  # 避免浮点误差


def calculate_bmp_preference(individual, bmp_rules):
    """
    根据BMP规则计算个体偏好分数
    :param individual: 个体对象，需包含gene属性
    :param bmp_rules: 通过mine_bmp_rules生成的规则字典
    :return: 偏好得分（数值越大表示越符合规则）
    """
    score = 0

    # 1. 位置强制规则得分（完全匹配）
    position_rules = bmp_rules.get('position_specific_rules', {})
    rule_count = 0
    total_count = len(position_rules.items())
    for pos, expected_type in position_rules.items():
        actual_type = extract_bmp_type(individual[pos])
        if actual_type == expected_type:
            rule_count += 1
    bmp_pos_score = rule_count / total_count if total_count != 0 else 1
    type_rules = bmp_rules.get('type_distribution_rules', {})

    # 统计个体基因类型
    type_counter = individual.bmp_type_count

    # 计算类型增益系数
    type_weights = {}
    for bmp_type in set(type_rules.keys()):
        if type_counter.get(bmp_type):
            ave_count = type_rules.get(bmp_type, 0)
            if ave_count > 0:
                type_score = type_counter[bmp_type] / ave_count if type_counter[bmp_type] < ave_count else 1
            elif ave_count < 0:
                type_score = 1 - type_counter[bmp_type] / -ave_count if -ave_count > type_counter[bmp_type] else 0
            else:
                type_score = 1
        else:
            type_score = 1
        type_weights[bmp_type] = type_score  # 防止除零

    score = min(bmp_pos_score, min(type_weights.values()) if type_weights else 1)

    return score


def calculate_preference_score(individual, preference_params):
    """计算个体的综合偏好得分（取最小值）

    动态从个体属性读取指标值，安全兼容：
    - 空间优化（只有 economy/environment）
    - 时空优化（有全套指标）
    - 代理模型（部分指标为估算值或 0）
    """
    scores = []
    for key in preference_params:
        if key == 'bmp_rules':
            score = calculate_bmp_preference(individual, preference_params[key])
            scores.append(score)
            continue
        h, direction, r = preference_params[key]
        # 动态获取指标值，支持直接属性和 fitness.values 回退
        if key == 'economy':
            value = getattr(individual, 'economy', None)
            if value is None:
                value = individual.fitness.values[0]
        elif key == 'environment':
            value = getattr(individual, 'environment', None)
            if value is None:
                value = individual.fitness.values[1]
        else:
            value = getattr(individual, key, None)
        # 指标不存在时跳过（不惩罚该维度，避免崩溃）
        if value is None:
            continue
        score = satisfaction_score(value, h, direction, r)
        scores.append(score)

    # 返回最小值作为偏好得分
    return min(scores) if scores else 0.0


def assign_preference_scores(front, preference_params, front_id):
    """为整个前沿的个体分配偏好得分"""
    for ind in front:
        score = calculate_preference_score(ind, preference_params)
        setattr(ind.fitness, 'preference_score', score)
        setattr(ind.fitness, 'front_id', front_id)


def sortNondominated(individuals, k, first_front_only=False):
    """Sort the first *k* *individuals* into different nondomination levels
    using the "Fast Nondominated Sorting Approach" proposed by Deb et al.,
    see [Deb2002]_. This algorithm has a time complexity of :math:`O(MN^2)`,
    where :math:`M` is the number of objectives and :math:`N` the number of
    individuals.

    :param individuals: A list of individuals to select from.
    :param k: The number of individuals to select.
    :param first_front_only: If :obj:`True` sort only the first front and
                             exit.
    :returns: A list of Pareto fronts (lists), the first list includes
              nondominated individuals.

    .. [Deb2002] Deb, Pratab, Agarwal, and Meyarivan, "A fast elitist
       non-dominated sorting genetic algorithm for multi-objective
       optimization: NSGA-II", 2002.
    """
    if k == 0:
        return []

    map_fit_ind = defaultdict(list)
    for ind in individuals:
        map_fit_ind[ind.fitness].append(ind)
    fits = list(map_fit_ind.keys())

    current_front = []
    next_front = []
    dominating_fits = defaultdict(int)
    dominated_fits = defaultdict(list)

    # Rank first Pareto front
    for i, fit_i in enumerate(fits):
        for fit_j in fits[i + 1:]:
            if fit_i.dominates(fit_j):
                dominating_fits[fit_j] += 1
                dominated_fits[fit_i].append(fit_j)
            elif fit_j.dominates(fit_i):
                dominating_fits[fit_i] += 1
                dominated_fits[fit_j].append(fit_i)
        if dominating_fits[fit_i] == 0:
            current_front.append(fit_i)

    fronts = [[]]
    for fit in current_front:
        fronts[-1].extend(map_fit_ind[fit])
    pareto_sorted = len(fronts[-1])

    # Rank the next front until all individuals are sorted or
    # the given number of individual are sorted.
    if not first_front_only:
        N = min(len(individuals), k)
        while pareto_sorted < N:
            fronts.append([])
            for fit_p in current_front:
                for fit_d in dominated_fits[fit_p]:
                    dominating_fits[fit_d] -= 1
                    if dominating_fits[fit_d] == 0:
                        next_front.append(fit_d)
                        pareto_sorted += len(map_fit_ind[fit_d])
                        fronts[-1].extend(map_fit_ind[fit_d])
            current_front = next_front
            next_front = []

    return fronts


MAX_HISTORY_SIZE = 16


# 新增函数：交互式选择
def interactive_selection(pop, good_pop, bad_pop, his_reason, user_id, preference_param=None):
    """支持多用户的交互选择函数"""
    print(f"\n--- 用户 {user_id} 正在选择 ---")
    """用户交互选择过程"""
    # 根据 preference_param 动态确定有效指标列表（排除 bmp_rules）
    if preference_param is not None:
        VALID_METRICS = {k for k in preference_param.keys() if k != 'bmp_rules'}
    else:
        VALID_METRICS = {
            'economy', 'environment', 'abandon_possibility',
            'cost_variation', 'return_on_invest', 'env_on_invest'
        }
    # sorted_pop = sorted(pop, key=lambda x: x.fitness.preference_score, reverse=True)
    id_to_ind = {ind.id: ind for ind in pop}
    # 显示前2/3个方案
    pop_ids = [ind.id for ind in pop]
    dis_num = int(len(pop)*2/3)
    print(f"\n当前最优的{dis_num}个方案（关注指标：{', '.join(sorted(VALID_METRICS))}）：")
    for i, ind in enumerate(pop[:dis_num]):
        # 基本目标值
        parts = [
            f"ID: {ind.id:4d}",
            f"经济: {ind.fitness.values[0]:.2f}",
            f"环境: {ind.fitness.values[1]:.2f}",
        ]
        # 动态追加 preference_param 中配置的其他指标（安全访问）
        for k in sorted(VALID_METRICS - {'economy', 'environment'}):
            val = getattr(ind, k, None)
            if val is not None:
                try:
                    parts.append(f"{k}: {float(val):.3f}")
                except (TypeError, ValueError):
                    parts.append(f"{k}: {val}")
            else:
                parts.append(f"{k}: N/A")
        print(" | ".join(parts))

    while True:
        try:
            good_input = input("请输入好方案ID（用逗号分隔）: ")
            bad_input = input("请输入差方案ID（用逗号分隔）: ")
            next_gene = input("下一代交互迭代次数: ")
            new_good_id = list(set(int(id_str.strip()) for id_str in good_input.split(',') if id_str))
            new_bad_id = list(set(int(id_str.strip()) for id_str in bad_input.split(',') if id_str))

            # 验证输入合法性
            invalid_good_id = [id for id in new_good_id if id not in pop_ids]
            invalid_bad_id = [id for id in new_bad_id if id not in pop_ids]
            if invalid_good_id or invalid_bad_id:
                print(f"无效ID - 好方案: {invalid_good_id}，差方案: {invalid_bad_id}")
                continue
            new_good_inds = [id_to_ind[id] for id in new_good_id]
            new_bad_inds = [id_to_ind[id] for id in new_bad_id]
            # 收集差评原因
            bad_reasons = []
            for bad_id in new_bad_id:
                while True:
                    reason_input = input(
                        f"请为差方案ID {bad_id} 输入原因指标（逗号分隔，可选：{', '.join(VALID_METRICS)}）: "
                    ).strip()
                    reasons = [r.strip() for r in reason_input.split(',') if r.strip()]

                    # 验证指标有效性
                    invalid_metrics = [m for m in reasons if m not in VALID_METRICS]
                    if invalid_metrics:
                        print(f"无效指标: {invalid_metrics}，请重新输入")
                        continue

                    bad_reasons.append(reasons)
                    break
            break
        except ValueError:
            print("输入格式错误，请使用逗号分隔的数字")

    combined_good = good_pop + new_good_inds
    combined_bad = bad_pop + new_bad_inds
    combined_bad_reasons = his_reason + bad_reasons
    # 去重：保留每个ID最新的个体
    good_pop_updated = list({ind.id: ind for ind in combined_good}.values())
    bad_pop_updated = list({ind.id: ind for ind in combined_bad}.values())
    # 维护历史记录大小
    MAX_HISTORY_SIZE = 16
    if len(good_pop_updated) > MAX_HISTORY_SIZE:
        good_pop_updated = good_pop_updated[-MAX_HISTORY_SIZE:]
    if len(bad_pop_updated) > MAX_HISTORY_SIZE:
        bad_pop_updated = bad_pop_updated[-MAX_HISTORY_SIZE:]
        # 对齐原因列表（按最终bad_pop顺序）
    final_bad_reasons = []
    for ind in bad_pop_updated:
        if ind.id in new_bad_id:
            idx = new_bad_id.index(ind.id)
            final_bad_reasons.append(bad_reasons[idx])
        else:
            final_bad_reasons.append([])  # 历史记录无原因
    return good_pop_updated, bad_pop_updated, final_bad_reasons, int(next_gene)


def interactive_selection_nonblocking(pop, good_pop, bad_pop, his_reason,
                                       user_id, preference_param=None,
                                       signal_dir="/data/config",
                                       task_id="task", generation=0):
    """
    非阻塞式交互选择 - 通过信号文件轮询机制实现

    Args:
        pop: 当前种群
        good_pop: 历史好方案列表
        bad_pop: 历史差方案列表
        his_reason: 历史差评原因
        user_id: 用户标识
        preference_param: 偏好参数
        signal_dir: 信号文件目录
        task_id: 任务标识
        generation: 当前代数

    Returns:
        (good_pop_updated, bad_pop_updated, final_bad_reasons, next_interval)
    """
    import os
    import time
    import json

    # 根据 preference_param 动态确定有效指标列表
    if preference_param is not None:
        VALID_METRICS = {k for k in preference_param.keys() if k != 'bmp_rules'}
    else:
        VALID_METRICS = {
            'economy', 'environment', 'abandon_possibility',
            'cost_variation', 'return_on_invest', 'env_on_invest'
        }

    id_to_ind = {ind.id: ind for ind in pop}
    pop_ids = [ind.id for ind in pop]
    dis_num = int(len(pop) * 2 / 3)

    # 构建方案展示数据
    display_solutions = []
    for ind in pop[:dis_num]:
        sol = {
            'id': ind.id,
            'economy': ind.fitness.values[0],
            'environment': ind.fitness.values[1],
        }
        for k in VALID_METRICS - {'economy', 'environment'}:
            val = getattr(ind, k, None)
            sol[k] = float(val) if val is not None else None
        display_solutions.append(sol)

    # 1. 写入等待信号文件
    wait_file = os.path.join(signal_dir, f"WAIT_INTERACTION_{task_id}_gen{generation}.signal")
    os.makedirs(signal_dir, exist_ok=True)
    with open(wait_file, 'w') as f:
        f.write(f"generation={generation}\n")
        f.write(f"user_id={user_id}\n")
        f.write(f"timestamp={time.time()}\n")
        f.write(f"num_solutions={dis_num}\n")

    # 同时写入当前代方案数据供前端展示
    solutions_file = os.path.join(signal_dir, f"SOLUTIONS_{task_id}_gen{generation}.json")
    solutions_data = {
        'generation': generation,
        'user_id': user_id,
        'valid_metrics': list(VALID_METRICS),
        'solutions': display_solutions,
        'history_good': [{'id': ind.id} for ind in good_pop],
        'history_bad': [{'id': ind.id} for ind in bad_pop],
    }
    with open(solutions_file, 'w') as f:
        json.dump(solutions_data, f, indent=2)

    print(f"[Interactive] User {user_id} waiting at generation {generation}")
    print(f"[Interactive] Signal file: {wait_file}")

    # 2. 轮询等待用户提交 (超时1小时)
    continue_file = os.path.join(signal_dir, f"CONTINUE_{task_id}_gen{generation}.json")
    start_time = time.time()
    timeout_seconds = 3600  # 1小时超时

    while time.time() - start_time < timeout_seconds:
        if os.path.exists(continue_file):
            try:
                with open(continue_file, 'r') as f:
                    user_input = json.load(f)

                # 删除信号文件
                if os.path.exists(wait_file):
                    os.remove(wait_file)
                if os.path.exists(continue_file):
                    os.remove(continue_file)
                if os.path.exists(solutions_file):
                    os.remove(solutions_file)

                print(f"[Interactive] User {user_id} submitted at generation {generation}")

                # 解析用户输入
                new_good_id = user_input.get('good_ids', [])
                new_bad_id = user_input.get('bad_ids', [])
                next_interval = user_input.get('next_interval', 30)
                bad_reasons_input = user_input.get('bad_reasons', {})

                # 验证输入合法性
                new_good_id = [int(id_str) for id_str in new_good_id if id_str in pop_ids]
                new_bad_id = [int(id_str) for id_str in new_bad_id if id_str in pop_ids]

                new_good_inds = [id_to_ind[id] for id in new_good_id if id in id_to_ind]
                new_bad_inds = [id_to_ind[id] for id in new_bad_id if id in id_to_ind]

                # 构建bad_reasons列表（按new_bad_id顺序）
                bad_reasons = []
                for bad_id in new_bad_id:
                    reasons = bad_reasons_input.get(str(bad_id), [])
                    bad_reasons.append(reasons if isinstance(reasons, list) else [])

                # 合并历史记录
                combined_good = good_pop + new_good_inds
                combined_bad = bad_pop + new_bad_inds
                combined_bad_reasons = his_reason + bad_reasons

                # 去重
                good_pop_updated = list({ind.id: ind for ind in combined_good}.values())
                bad_pop_updated = list({ind.id: ind for ind in combined_bad}.values())

                # 维护历史记录大小
                MAX_HISTORY_SIZE = 16
                if len(good_pop_updated) > MAX_HISTORY_SIZE:
                    good_pop_updated = good_pop_updated[-MAX_HISTORY_SIZE:]
                if len(bad_pop_updated) > MAX_HISTORY_SIZE:
                    bad_pop_updated = bad_pop_updated[-MAX_HISTORY_SIZE:]

                # 对齐原因列表
                final_bad_reasons = []
                for ind in bad_pop_updated:
                    if ind.id in new_bad_id:
                        idx = new_bad_id.index(ind.id)
                        final_bad_reasons.append(bad_reasons[idx])
                    else:
                        final_bad_reasons.append([])

                return good_pop_updated, bad_pop_updated, final_bad_reasons, int(next_interval)

            except Exception as e:
                print(f"[Interactive] Error reading continue file: {e}")
                time.sleep(2)
                continue

        time.sleep(2)  # 每2秒轮询一次

    # 超时处理
    print(f"[Interactive] User {user_id} timeout at generation {generation}")
    if os.path.exists(wait_file):
        os.remove(wait_file)
    if os.path.exists(solutions_file):
        os.remove(solutions_file)

    return good_pop, bad_pop, his_reason, 30


def update_preference_params(his_good_pop, his_bad_pop, old_params, bad_pop_reasons):
    """直接根据好差种群提取规则"""
    # 校验输入一致性
    assert len(his_bad_pop) == len(bad_pop_reasons), "坏种群与原因列表长度不一致"
    good_pop = his_good_pop
    bad_pop = his_bad_pop
    new_bmp_rules = {}
    old_bmp_rules = {}
    global inverse_sig_diff, new_dir
    new_params = {}
    if len(good_pop) == MAX_HISTORY_SIZE:
        new_bmp_rules = mine_bmp_rules(good_pop, bad_pop)

    # 构建原因反向索引 {metric: [相关bad个体值]}
    metric_reason_map = defaultdict(list)
    for bad_ind, reasons in zip(his_bad_pop, bad_pop_reasons):
        for metric in reasons:
            val = getattr(bad_ind, metric, None)
            if val is not None:
                metric_reason_map[metric].append(val)
    for metric in old_params:
        if metric == 'bmp_rules':
            old_bmp_rules = old_params[metric]
            continue
        old_h, old_dir, old_r = old_params[metric]
        # 获取标注原因的坏样本值（优先使用用户标注）
        bad_vals = metric_reason_map.get(metric, [])

        # 安全获取好方案指标值（属性不存在时跳过）
        good_vals = [v for v in (getattr(ind, metric, None) for ind in good_pop) if v is not None]

        # 提取指标值
        #bad_vals = [getattr(ind, metric) for ind in bad_pop]

        # 空值处理
        if not good_vals or not bad_vals:
            new_params[metric] = (old_h, old_dir, old_r)
            continue

        # 统计检验
        from scipy.stats import mannwhitneyu
        try:
            _, p_value = mannwhitneyu(good_vals, bad_vals, alternative='less' if old_dir == 'less' else 'greater')
            sig_diff = p_value < 0.05

        except:
            sig_diff = False
        if not sig_diff:
            try:
                _, p_value = mannwhitneyu(good_vals, bad_vals, alternative='greater' if old_dir == 'less' else 'less')
                inverse_sig_diff = p_value < 0.05
            except:
                inverse_sig_diff = False
            if inverse_sig_diff:
                new_dir = 'greater' if old_dir == 'less' else 'less'
            else:
                new_dir = old_dir
        else:
            new_dir = 'less' if old_dir == 'less' else 'greater'
        bad_mean = np.mean(bad_vals)
        # 排序处理
        reverse = (new_dir == 'greater')
        sorted_good = sorted(good_vals, reverse=reverse)
        if sig_diff or inverse_sig_diff:
            # 存在显著差异
            # bad_mean = np.mean(bad_vals)

            # 寻找第一个优于差均值的个体值
            h_candidates = sorted_good[::-1]
            for val in h_candidates:
                if (new_dir == 'less' and val < min(bad_vals)) or (new_dir == 'greater' and val > max(bad_vals)):
                    h = val
                    break
                else:
                    h = np.mean(good_vals)  # 无满足则取最优

            # 计算r值
            if new_dir == 'less':
                r = min(bad_vals) - h
            else:
                r = h - max(bad_vals)
            print(f"指标{metric}上存在显著差异| h:{h:.4f}| r:{r:.4f} | badmean:{bad_mean:.4f}")
        else:
            '''
            new_dir = 'equal'
            h = old_h
            r = old_r'''
            new_dir = old_dir
            if new_dir == 'less':
                h = max(max(bad_vals), max(good_vals))
                r = 0
            else:
                h = min(min(bad_vals), min(good_vals))
                r = 0
            # 无显著差异
            '''
            q3_index = int(0.8 * len(sorted_good))
            h = sorted_good[q3_index] if q3_index < len(sorted_good) else sorted_good[-1]

            if new_dir == 'less':
                r = max(good_vals) - h
            else:
                r = h - min(good_vals)
            print(f"指标{metric}上不存在显著差异| h:{h:.4f}| r:{r:.4f}")'''
        new_params[metric] = (h, new_dir, r)
    if new_bmp_rules:
        new_params['bmp_rules'] = new_bmp_rules
    elif old_bmp_rules:
        new_params['bmp_rules'] = old_bmp_rules

    return new_params


def extract_bmp_type(value):
    if value == 0:
        return 0
    return int(value) // 1000  # 提取千位数作为BMP类型


def analyze_position_patterns(good_pop, bad_pop):
    patterns = {}
    if not good_pop or not bad_pop:
        return patterns

    gene_length = len(good_pop[0])

    for pos in range(gene_length):
        # 提取好方案在该位置的BMP类型
        good_types = [extract_bmp_type(ind[pos]) for ind in good_pop]
        unique_good = set(good_types)

        # 检查是否所有好方案在该位置有相同非零类型
        if len(unique_good) == 1 and 0 not in unique_good:
            target_type = unique_good.pop()

            # 检查差方案是否全部为0
            bad_types = [extract_bmp_type(ind[pos]) for ind in bad_pop]
            if all(t == 0 for t in bad_types):
                patterns[pos] = target_type

    return patterns


def analyze_bmp_distribution(good_pop, bad_pop):
    # 统计类型频率
    def count_types(population):
        counter = defaultdict(int)
        for ind in population:
            for bmp, value in ind.bmp_type_count.items():
                if bmp != 0:
                    counter[bmp] += value
        return counter

    good_counts = count_types(good_pop)
    bad_counts = count_types(bad_pop)

    # 分析分布规律
    distribution_rules = []

    # 规则1：好方案高频但差方案低频的类型
    all_types = set(good_counts.keys()).union(bad_counts.keys())
    dis_rule = {}
    for bmp in all_types:
        good = good_counts.get(bmp, 0) / len(good_pop) if len(good_pop) != 0 else 0
        bad = bad_counts.get(bmp, 0) / len(bad_pop) if len(bad_pop) != 0 else 0
        if good - bad > 0.3 * good:
            dis_rule[bmp] = good
        elif bad - good > 0.3 * bad:
            dis_rule[bmp] = -bad

    return dis_rule


# 主函数
def mine_bmp_rules(good_pop, bad_pop):
    # 规律1：位置强制规则
    position_rules = analyze_position_patterns(good_pop, bad_pop)

    # 规律2：类型分布规则
    type_rules = analyze_bmp_distribution(good_pop, bad_pop)

    return {
        "position_specific_rules": position_rules,
        "type_distribution_rules": type_rules,

    }


def assignCrowdingDist(individuals):
    """Assign a crowding distance to each individual's fitness. The
    crowding distance can be retrieve via the :attr:`crowding_dist`
    attribute of each individual's fitness.
    """
    if len(individuals) == 0:
        return

    distances = [0.0] * len(individuals)
    crowd = [(ind.fitness.values, i) for i, ind in enumerate(individuals)]

    nobj = len(individuals[0].fitness.values)

    for i in range(nobj):
        crowd.sort(key=lambda element: element[0][i])
        distances[crowd[0][1]] = float("inf")
        distances[crowd[-1][1]] = float("inf")
        if crowd[-1][0][i] == crowd[0][0][i]:
            continue
        norm = nobj * float(crowd[-1][0][i] - crowd[0][0][i])
        for prev, cur, next in zip(crowd[:-2], crowd[1:-1], crowd[2:]):
            distances[cur[1]] += (next[0][i] - prev[0][i]) / norm

    for i, dist in enumerate(distances):
        individuals[i].fitness.crowding_dist = dist

def merge_multiuser_prefs(user_prefs_list):
    merged_prefs = {}
    # 获取所有指标（假设所有用户的指标相同）
    metrics = user_prefs_list[0].keys()
    for metric in metrics:
        if metric == 'bmp_rules':
            continue
        # 收集所有用户的h、dir、r
        all_h = []
        all_dir = []
        all_r = []
        for user_pref in user_prefs_list:
            h, dir, r = user_pref[metric]
            all_h.append(h)
            all_dir.append(dir)
            all_r.append(r)
        merged_h, merged_dir, merged_r = merge_single_metric(all_h, all_dir, all_r)
        merged_prefs[metric] = (merged_h, merged_dir, merged_r)
    return merged_prefs


def merge_single_metric(all_h, all_dir, all_r):
    """
    合并单个指标的参数
    :param all_h: 所有用户的h值列表
    :param all_dir: 所有用户的方向列表
    :return: 合并后的(h, dir, r)
    """
    # 提取非equal方向
    non_equal_h = [h for h, d in zip(all_h, all_dir) if d != 'equal']
    non_equal_r = [r for r, d in zip(all_r, all_dir) if d != 'equal']
    non_equal_dirs = [d for d in all_dir if d != 'equal']

    if not non_equal_dirs:
        # 全为equal时保留第一个用户的参数
        return all_h[0], 'equal', 0.0
    if len(non_equal_dirs) == 1:
        return non_equal_h[0], non_equal_dirs[0], non_equal_r[0]
    # 判断方向类型
    unique_dirs = set(non_equal_dirs)

    if len(unique_dirs) == 1:
        greater_v = min([h - r for h, d, r in zip(all_h, all_dir, all_r) if d == 'greater'], default=-float('inf'))
        less_v = max([h + r for h, d, r in zip(all_h, all_dir, all_r) if d == 'less'], default=float('inf'))
        # 方向一致时的处理
        direction = unique_dirs.pop()
        if direction == 'less':
            min_h = min(non_equal_h)
            max_h = max(non_equal_h)
            #min_v = min(all_v)
            return min_h, 'less', less_v - min_h
        else:  # 'greater'
            max_h = max(non_equal_h)
            min_h = min(non_equal_h)
            return max_h, 'greater', max_h - greater_v
    else:
        # 处理方向冲突（less和greater共存）
        # 获取所有greater的最大h和less的最小h
        greater_h = max([h-r for h, d, r in zip(all_h, all_dir, all_r) if d == 'greater'], default=-float('inf'))
        less_h = min([h+r for h, d, r in zip(all_h, all_dir, all_r) if d == 'less'], default=float('inf'))

        # 计算交集体
        #if greater_h < less_h:
        return less_h, 'between', greater_h
        #else:
            # 无交集时取全体极值
            #return less_h, 'between', greater_h

def merge_multiuser_pops(user_pops, pop_size):
    """多用户种群合并策略"""
    # 1. 识别所有用户的共同个体
    id_counter = defaultdict(int)
    for user_pop in user_pops:
        for ind in user_pop:
            #if ind.fitness.preference_score > 0:
            id_counter[ind.id] += 1

    # 提取全用户共有的个体
    common_individuals = [ind for user_pop in user_pops
                          for ind in user_pop
                          if id_counter[ind.id] == len(user_pops)]
    # 去重处理
    common_ids = {ind.id for ind in common_individuals}
    common_individuals = [next(ind for user_pop in user_pops
                               for ind in user_pop
                               if ind.id == cid)
                          for cid in common_ids]

    # 2. 初始化新种群
    new_pop = common_individuals[:pop_size]
    selected_ids = {ind.id for ind in new_pop}

    # 3. 计算剩余配额
    remaining = pop_size - len(new_pop)
    if remaining <= 0:
        return new_pop[:pop_size]

    # 4. 配额分配与选择
    user_quotas = allocate_quotas(remaining, len(user_pops))

    # 5. 按配额选择个体
    for i, (user_pop, quota) in enumerate(zip(user_pops, user_quotas)):
        candidates = [ind for ind in user_pop
                      if ind.id not in selected_ids and hasattr(ind.fitness, 'preference_score')
                      ]


        # 按该用户的偏好得分排序
        candidates.sort(key=lambda x: (x.fitness.front_id, -x.fitness.preference_score, -x.fitness.crowding_dist))
        actual_quota = min(quota, len(candidates))
        selected = candidates[:actual_quota]

        # 添加并更新记录
        new_pop.extend(selected)
        selected_ids.update(ind.id for ind in selected)
        remaining -= len(selected)

        # 提前填充满时退出
        if remaining <= 0:
            break

    # 6. 补足机制
    if remaining > 0:
        fallback_candidates = [ind for user_pop in user_pops
                               for ind in user_pop
                               if ind.id not in selected_ids]
        # 按全局综合得分排序
        fallback_candidates.sort(
            key=lambda x: (
                x.fitness.front_id,  # 第一排序条件：前沿等级升序
                -x.fitness.crowding_dist  # 第二排序条件：拥挤度降序（通过取负数实现）
            )
        )
        new_pop.extend(fallback_candidates[:remaining])

    return new_pop[:pop_size]


def allocate_quotas(total, num_users):
    """配额分配算法"""
    base = total // num_users
    remainder = total % num_users
    return [base + 1 if i < remainder else base
            for i in range(num_users)]


def selTournamentDCD(individuals, k):
    """Tournament selection based on dominance (D) between two individuals, if
    the two individuals do not interdominate the selection is made
    based on crowding distance (CD). The *individuals* sequence length has to
    be a multiple of 4 only if k is equal to the length of individuals.
    Starting from the beginning of the selected individuals, two consecutive
    individuals will be different (assuming all individuals in the input list
    are unique). Each individual from the input list won't be selected more
    than twice.

    This selection requires the individuals to have a :attr:`crowding_dist`
    attribute, which can be set by the :func:`assignCrowdingDist` function.

    :param individuals: A list of individuals to select from.
    :param k: The number of individuals to select. Must be less than or equal
              to len(individuals).
    :returns: A list of selected individuals.
    """

    if k > len(individuals):
        raise ValueError("selTournamentDCD: k must be less than or equal to individuals length")

    if k == len(individuals) and k % 4 != 0:
        raise ValueError("selTournamentDCD: k must be divisible by four if k == len(individuals)")

    def tourn(ind1, ind2):
        if ind1.fitness.dominates(ind2.fitness):
            return ind1
        elif ind2.fitness.dominates(ind1.fitness):
            return ind2

        if ind1.fitness.crowding_dist < ind2.fitness.crowding_dist:
            return ind2
        elif ind1.fitness.crowding_dist > ind2.fitness.crowding_dist:
            return ind1

        if random.random() <= 0.5:
            return ind1
        return ind2

    individuals_1 = random.sample(individuals, len(individuals))
    individuals_2 = random.sample(individuals, len(individuals))

    chosen = []
    for i in range(0, k, 4):
        chosen.append(tourn(individuals_1[i], individuals_1[i + 1]))
        chosen.append(tourn(individuals_1[i + 2], individuals_1[i + 3]))
        chosen.append(tourn(individuals_2[i], individuals_2[i + 1]))
        chosen.append(tourn(individuals_2[i + 2], individuals_2[i + 3]))

    return chosen


#######################################
# Generalized Reduced runtime ND sort #
#######################################


def identity(obj):
    """Returns directly the argument *obj*.
    """
    return obj


def isDominated(wvalues1, wvalues2):
    """Returns whether or not *wvalues2* dominates *wvalues1*.

    :param wvalues1: The weighted fitness values that would be dominated.
    :param wvalues2: The weighted fitness values of the dominant.
    :returns: :obj:`True` if wvalues2 dominates wvalues1, :obj:`False`
              otherwise.
    """
    not_equal = False
    for self_wvalue, other_wvalue in zip(wvalues1, wvalues2):
        if self_wvalue > other_wvalue:
            return False
        elif self_wvalue < other_wvalue:
            not_equal = True
    return not_equal


def median(seq, key=identity):
    """Returns the median of *seq* - the numeric value separating the higher
    half of a sample from the lower half. If there is an even number of
    elements in *seq*, it returns the mean of the two middle values.
    """
    sseq = sorted(seq, key=key)
    length = len(seq)
    if length % 2 == 1:
        return key(sseq[(length - 1) // 2])
    else:
        return (key(sseq[(length - 1) // 2]) + key(sseq[length // 2])) / 2.0


def sortLogNondominated(individuals, k, first_front_only=False):
    """Sort *individuals* in pareto non-dominated fronts using the Generalized
    Reduced Run-Time Complexity Non-Dominated Sorting Algorithm presented by
    Fortin et al. (2013).

    :param individuals: A list of individuals to select from.
    :returns: A list of Pareto fronts (lists), with the first list being the
              true Pareto front.
    """
    if k == 0:
        return []

    # Separate individuals according to unique fitnesses
    unique_fits = defaultdict(list)
    for i, ind in enumerate(individuals):
        unique_fits[ind.fitness.wvalues].append(ind)

    # Launch the sorting algorithm
    obj = len(individuals[0].fitness.wvalues) - 1
    fitnesses = list(unique_fits.keys())
    front = dict.fromkeys(fitnesses, 0)

    # Sort the fitnesses lexicographically.
    fitnesses.sort(reverse=True)
    sortNDHelperA(fitnesses, obj, front)

    # Extract individuals from front list here
    nbfronts = max(front.values()) + 1
    pareto_fronts = [[] for i in range(nbfronts)]
    for fit in fitnesses:
        index = front[fit]
        pareto_fronts[index].extend(unique_fits[fit])

    # Keep only the fronts required to have k individuals.
    if not first_front_only:
        count = 0
        for i, front in enumerate(pareto_fronts):
            count += len(front)
            if count >= k:
                return pareto_fronts[:i + 1]
        return pareto_fronts
    else:
        return pareto_fronts[0]


def sortNDHelperA(fitnesses, obj, front):
    """Create a non-dominated sorting of S on the first M objectives"""
    if len(fitnesses) < 2:
        return
    elif len(fitnesses) == 2:
        # Only two individuals, compare them and adjust front number
        s1, s2 = fitnesses[0], fitnesses[1]
        if isDominated(s2[:obj + 1], s1[:obj + 1]):
            front[s2] = max(front[s2], front[s1] + 1)
    elif obj == 1:
        sweepA(fitnesses, front)
    elif len(frozenset(map(itemgetter(obj), fitnesses))) == 1:
        # All individuals for objective M are equal: go to objective M-1
        sortNDHelperA(fitnesses, obj - 1, front)
    else:
        # More than two individuals, split list and then apply recursion
        best, worst = splitA(fitnesses, obj)
        sortNDHelperA(best, obj, front)
        sortNDHelperB(best, worst, obj - 1, front)
        sortNDHelperA(worst, obj, front)


def splitA(fitnesses, obj):
    """Partition the set of fitnesses in two according to the median of
    the objective index *obj*. The values equal to the median are put in
    the set containing the least elements.
    """
    median_ = median(fitnesses, itemgetter(obj))
    best_a, worst_a = [], []
    best_b, worst_b = [], []

    for fit in fitnesses:
        if fit[obj] > median_:
            best_a.append(fit)
            best_b.append(fit)
        elif fit[obj] < median_:
            worst_a.append(fit)
            worst_b.append(fit)
        else:
            best_a.append(fit)
            worst_b.append(fit)

    balance_a = abs(len(best_a) - len(worst_a))
    balance_b = abs(len(best_b) - len(worst_b))

    if balance_a <= balance_b:
        return best_a, worst_a
    else:
        return best_b, worst_b


def sweepA(fitnesses, front):
    """Update rank number associated to the fitnesses according
    to the first two objectives using a geometric sweep procedure.
    """
    stairs = [-fitnesses[0][1]]
    fstairs = [fitnesses[0]]
    for fit in fitnesses[1:]:
        idx = bisect.bisect_right(stairs, -fit[1])
        if 0 < idx <= len(stairs):
            fstair = max(fstairs[:idx], key=front.__getitem__)
            front[fit] = max(front[fit], front[fstair] + 1)
        for i, fstair in enumerate(fstairs[idx:], idx):
            if front[fstair] == front[fit]:
                del stairs[i]
                del fstairs[i]
                break
        stairs.insert(idx, -fit[1])
        fstairs.insert(idx, fit)


def sortNDHelperB(best, worst, obj, front):
    """Assign front numbers to the solutions in H according to the solutions
    in L. The solutions in L are assumed to have correct front numbers and the
    solutions in H are not compared with each other, as this is supposed to
    happen after sortNDHelperB is called."""
    key = itemgetter(obj)
    if len(worst) == 0 or len(best) == 0:
        # One of the lists is empty: nothing to do
        return
    elif len(best) == 1 or len(worst) == 1:
        # One of the lists has one individual: compare directly
        for hi in worst:
            for li in best:
                if isDominated(hi[:obj + 1], li[:obj + 1]) or hi[:obj + 1] == li[:obj + 1]:
                    front[hi] = max(front[hi], front[li] + 1)
    elif obj == 1:
        sweepB(best, worst, front)
    elif key(min(best, key=key)) >= key(max(worst, key=key)):
        # All individuals from L dominate H for objective M:
        # Also supports the case where every individuals in L and H
        # has the same value for the current objective
        # Skip to objective M-1
        sortNDHelperB(best, worst, obj - 1, front)
    elif key(max(best, key=key)) >= key(min(worst, key=key)):
        best1, best2, worst1, worst2 = splitB(best, worst, obj)
        sortNDHelperB(best1, worst1, obj, front)
        sortNDHelperB(best1, worst2, obj - 1, front)
        sortNDHelperB(best2, worst2, obj, front)


def splitB(best, worst, obj):
    """Split both best individual and worst sets of fitnesses according
    to the median of objective *obj* computed on the set containing the
    most elements. The values equal to the median are attributed so as
    to balance the four resulting sets as much as possible.
    """
    median_ = median(best if len(best) > len(worst) else worst, itemgetter(obj))
    best1_a, best2_a, best1_b, best2_b = [], [], [], []
    for fit in best:
        if fit[obj] > median_:
            best1_a.append(fit)
            best1_b.append(fit)
        elif fit[obj] < median_:
            best2_a.append(fit)
            best2_b.append(fit)
        else:
            best1_a.append(fit)
            best2_b.append(fit)

    worst1_a, worst2_a, worst1_b, worst2_b = [], [], [], []
    for fit in worst:
        if fit[obj] > median_:
            worst1_a.append(fit)
            worst1_b.append(fit)
        elif fit[obj] < median_:
            worst2_a.append(fit)
            worst2_b.append(fit)
        else:
            worst1_a.append(fit)
            worst2_b.append(fit)

    balance_a = abs(len(best1_a) - len(best2_a) + len(worst1_a) - len(worst2_a))
    balance_b = abs(len(best1_b) - len(best2_b) + len(worst1_b) - len(worst2_b))

    if balance_a <= balance_b:
        return best1_a, best2_a, worst1_a, worst2_a
    else:
        return best1_b, best2_b, worst1_b, worst2_b


def sweepB(best, worst, front):
    """Adjust the rank number of the worst fitnesses according to
    the best fitnesses on the first two objectives using a sweep
    procedure.
    """
    stairs, fstairs = [], []
    iter_best = iter(best)
    next_best = next(iter_best, False)
    for h in worst:
        while next_best and h[:2] <= next_best[:2]:
            insert = True
            for i, fstair in enumerate(fstairs):
                if front[fstair] == front[next_best]:
                    if fstair[1] > next_best[1]:
                        insert = False
                    else:
                        del stairs[i], fstairs[i]
                    break
            if insert:
                idx = bisect.bisect_right(stairs, -next_best[1])
                stairs.insert(idx, -next_best[1])
                fstairs.insert(idx, next_best)
            next_best = next(iter_best, False)

        idx = bisect.bisect_right(stairs, -h[1])
        if 0 < idx <= len(stairs):
            fstair = max(fstairs[:idx], key=front.__getitem__)
            front[h] = max(front[h], front[fstair] + 1)


######################################
# Non-Dominated Sorting  (NSGA-III)  #
######################################


NSGA3Memory = namedtuple("NSGA3Memory", ["best_point", "worst_point", "extreme_points"])


class selNSGA3WithMemory(object):
    """Class version of NSGA-III selection including memory for best, worst and
    extreme points. Registering this operator in a toolbox is a bit different
    than classical operators, it requires to instantiate the class instead
    of just registering the function::

        >>> from deap import base
        >>> ref_points = uniform_reference_points(nobj=3, p=12)
        >>> toolbox = base.Toolbox()
        >>> toolbox.register("select", selNSGA3WithMemory(ref_points))

    """

    def __init__(self, ref_points, nd="log"):
        self.ref_points = ref_points
        self.nd = nd
        self.best_point = numpy.full((1, ref_points.shape[1]), numpy.inf)
        self.worst_point = numpy.full((1, ref_points.shape[1]), -numpy.inf)
        self.extreme_points = None

    def __call__(self, individuals, k):
        chosen, memory = selNSGA3(individuals, k, self.ref_points, self.nd,
                                  self.best_point, self.worst_point,
                                  self.extreme_points, True)
        self.best_point = memory.best_point.reshape((1, -1))
        self.worst_point = memory.worst_point.reshape((1, -1))
        self.extreme_points = memory.extreme_points
        return chosen


def selNSGA3(individuals, k, ref_points, nd="log", best_point=None,
             worst_point=None, extreme_points=None, return_memory=False):
    """Implementation of NSGA-III selection as presented in [Deb2014]_.

    This implementation is partly based on `lmarti/nsgaiii
    <https://github.com/lmarti/nsgaiii>`_. It departs slightly from the
    original implementation in that it does not use memory to keep track
    of ideal and extreme points. This choice has been made to fit the
    functional api of DEAP. For a version of NSGA-III see
    :class:`~deap.tools.selNSGA3WithMemory`.

    :param individuals: A list of individuals to select from.
    :param k: The number of individuals to select.
    :param ref_points: Reference points to use for niching.
    :param nd: Specify the non-dominated algorithm to use: 'standard' or 'log'.
    :param best_point: Best point found at previous generation. If not provided
        find the best point only from current individuals.
    :param worst_point: Worst point found at previous generation. If not provided
        find the worst point only from current individuals.
    :param extreme_points: Extreme points found at previous generation. If not provided
        find the extreme points only from current individuals.
    :param return_memory: If :data:`True`, return the best, worst and extreme points
        in addition to the chosen individuals.
    :returns: A list of selected individuals.
    :returns: If `return_memory` is :data:`True`, a namedtuple with the
        `best_point`, `worst_point`, and `extreme_points`.


    You can generate the reference points using the :func:`uniform_reference_points`
    function::

        >>> ref_points = tools.uniform_reference_points(nobj=3, p=12)   # doctest: +SKIP
        >>> selected = selNSGA3(population, k, ref_points)              # doctest: +SKIP

    .. [Deb2014] Deb, K., & Jain, H. (2014). An Evolutionary Many-Objective Optimization
        Algorithm Using Reference-Point-Based Nondominated Sorting Approach,
        Part I: Solving Problems With Box Constraints. IEEE Transactions on
        Evolutionary Computation, 18(4), 577-601. doi:10.1109/TEVC.2013.2281535.
    """
    if nd == "standard":
        pareto_fronts = sortNondominated(individuals, k)
    elif nd == "log":
        pareto_fronts = sortLogNondominated(individuals, k)
    else:
        raise Exception("selNSGA3: The choice of non-dominated sorting "
                        "method '{0}' is invalid.".format(nd))

    # Extract fitnesses as a numpy array in the nd-sort order
    # Use wvalues * -1 to tackle always as a minimization problem
    fitnesses = numpy.array([ind.fitness.wvalues for f in pareto_fronts for ind in f])
    fitnesses *= -1

    # Get best and worst point of population, contrary to pymoo
    # we don't use memory
    if best_point is not None and worst_point is not None:
        best_point = numpy.min(numpy.concatenate((fitnesses, best_point), axis=0), axis=0)
        worst_point = numpy.max(numpy.concatenate((fitnesses, worst_point), axis=0), axis=0)
    else:
        best_point = numpy.min(fitnesses, axis=0)
        worst_point = numpy.max(fitnesses, axis=0)

    extreme_points = find_extreme_points(fitnesses, best_point, extreme_points)
    front_worst = numpy.max(fitnesses[:sum(len(f) for f in pareto_fronts), :], axis=0)
    intercepts = find_intercepts(extreme_points, best_point, worst_point, front_worst)
    niches, dist = associate_to_niche(fitnesses, ref_points, best_point, intercepts)

    # Get counts per niche for individuals in all front but the last
    niche_counts = numpy.zeros(len(ref_points), dtype=numpy.int64)
    index, counts = numpy.unique(niches[:-len(pareto_fronts[-1])], return_counts=True)
    niche_counts[index] = counts

    # Choose individuals from all fronts but the last
    chosen = list(chain(*pareto_fronts[:-1]))

    # Use niching to select the remaining individuals
    sel_count = len(chosen)
    n = k - sel_count
    selected = niching(pareto_fronts[-1], n, niches[sel_count:], dist[sel_count:], niche_counts)
    chosen.extend(selected)

    if return_memory:
        return chosen, NSGA3Memory(best_point, worst_point, extreme_points)
    return chosen


def find_extreme_points(fitnesses, best_point, extreme_points=None):
    'Finds the individuals with extreme values for each objective function.'
    # Keep track of last generation extreme points
    if extreme_points is not None:
        fitnesses = numpy.concatenate((fitnesses, extreme_points), axis=0)

    # Translate objectives
    ft = fitnesses - best_point

    # Find achievement scalarizing function (asf)
    asf = numpy.eye(best_point.shape[0])
    asf[asf == 0] = 1e6
    asf = numpy.max(ft * asf[:, numpy.newaxis, :], axis=2)

    # Extreme point are the fitnesses with minimal asf
    min_asf_idx = numpy.argmin(asf, axis=1)
    return fitnesses[min_asf_idx, :]


def find_intercepts(extreme_points, best_point, current_worst, front_worst):
    """Find intercepts between the hyperplane and each axis with
    the ideal point as origin."""
    # Construct hyperplane sum(f_i^n) = 1
    b = numpy.ones(extreme_points.shape[1])
    A = extreme_points - best_point
    try:
        x = numpy.linalg.solve(A, b)
    except numpy.linalg.LinAlgError:
        intercepts = current_worst
    else:
        if numpy.count_nonzero(x) != len(x):
            intercepts = front_worst
        else:
            intercepts = 1 / x

            if (not numpy.allclose(numpy.dot(A, x), b) or
                numpy.any(intercepts <= 1e-6) or
                numpy.any((intercepts + best_point) > current_worst)):
                intercepts = front_worst

    return intercepts


def associate_to_niche(fitnesses, reference_points, best_point, intercepts):
    """Associates individuals to reference points and calculates niche number.
    Corresponds to Algorithm 3 of Deb & Jain (2014)."""
    # Normalize by ideal point and intercepts
    fn = (fitnesses - best_point) / (intercepts - best_point + numpy.finfo(float).eps)

    # Create distance matrix
    fn = numpy.repeat(numpy.expand_dims(fn, axis=1), len(reference_points), axis=1)
    norm = numpy.linalg.norm(reference_points, axis=1)

    distances = numpy.sum(fn * reference_points, axis=2) / norm.reshape(1, -1)
    distances = distances[:, :, numpy.newaxis] * reference_points[numpy.newaxis, :, :] / norm[numpy.newaxis, :,
                                                                                         numpy.newaxis]
    distances = numpy.linalg.norm(distances - fn, axis=2)

    # Retrieve min distance niche index
    niches = numpy.argmin(distances, axis=1)
    distances = distances[list(range(niches.shape[0])), niches]
    return niches, distances


def niching(individuals, k, niches, distances, niche_counts):
    selected = []
    available = numpy.ones(len(individuals), dtype=bool)
    while len(selected) < k:
        # Maximum number of individuals (niches) to select in that round
        n = k - len(selected)

        # Find the available niches and the minimum niche count in them
        available_niches = numpy.zeros(len(niche_counts), dtype=bool)
        available_niches[numpy.unique(niches[available])] = True
        min_count = numpy.min(niche_counts[available_niches])

        # Select at most n niches with the minimum count
        selected_niches = numpy.flatnonzero(numpy.logical_and(available_niches, niche_counts == min_count))
        numpy.random.shuffle(selected_niches)
        selected_niches = selected_niches[:n]

        for niche in selected_niches:
            # Select from available individuals in niche
            niche_individuals = numpy.flatnonzero(numpy.logical_and(niches == niche, available))
            numpy.random.shuffle(niche_individuals)

            # If no individual in that niche, select the closest to reference
            # Else select randomly
            if niche_counts[niche] == 0:
                sel_index = niche_individuals[numpy.argmin(distances[niche_individuals])]
            else:
                sel_index = niche_individuals[0]

            # Update availability, counts and selection
            available[sel_index] = False
            niche_counts[niche] += 1
            selected.append(individuals[sel_index])

    return selected


def uniform_reference_points(nobj, p=4, scaling=None):
    """Generate reference points uniformly on the hyperplane intersecting
    each axis at 1. The scaling factor is used to combine multiple layers of
    reference points.
    """

    def gen_refs_recursive(ref, nobj, left, total, depth):
        points = []
        if depth == nobj - 1:
            ref[depth] = left / total
            points.append(ref)
        else:
            for i in range(left + 1):
                ref[depth] = i / total
                points.extend(gen_refs_recursive(ref.copy(), nobj, left - i, total, depth + 1))
        return points

    ref_points = numpy.array(gen_refs_recursive(numpy.zeros(nobj), nobj, p, p, 0))
    if scaling is not None:
        ref_points *= scaling
        ref_points += (1 - scaling) / nobj

    return ref_points


######################################
# Strength Pareto         (SPEA-II)  #
######################################

def selSPEA2(individuals, k):
    """Apply SPEA-II selection operator on the *individuals*. Usually, the
    size of *individuals* will be larger than *n* because any individual
    present in *individuals* will appear in the returned list at most once.
    Having the size of *individuals* equals to *n* will have no effect other
    than sorting the population according to a strength Pareto scheme. The
    list returned contains references to the input *individuals*. For more
    details on the SPEA-II operator see [Zitzler2001]_.

    :param individuals: A list of individuals to select from.
    :param k: The number of individuals to select.
    :returns: A list of selected individuals.

    .. [Zitzler2001] Zitzler, Laumanns and Thiele, "SPEA 2: Improving the
       strength Pareto evolutionary algorithm", 2001.
    """
    N = len(individuals)
    L = len(individuals[0].fitness.values)
    K = math.sqrt(N)
    strength_fits = [0] * N
    fits = [0] * N
    dominating_inds = [list() for i in range(N)]

    for i, ind_i in enumerate(individuals):
        for j, ind_j in enumerate(individuals[i + 1:], i + 1):
            if ind_i.fitness.dominates(ind_j.fitness):
                strength_fits[i] += 1
                dominating_inds[j].append(i)
            elif ind_j.fitness.dominates(ind_i.fitness):
                strength_fits[j] += 1
                dominating_inds[i].append(j)

    for i in range(N):
        for j in dominating_inds[i]:
            fits[i] += strength_fits[j]

    # Choose all non-dominated individuals
    chosen_indices = [i for i in range(N) if fits[i] < 1]

    if len(chosen_indices) < k:  # The archive is too small
        for i in range(N):
            distances = [0.0] * N
            for j in range(i + 1, N):
                dist = 0.0
                for k in range(L):
                    val = individuals[i].fitness.values[k] - \
                          individuals[j].fitness.values[k]
                    dist += val * val
                distances[j] = dist
            kth_dist = _randomizedSelect(distances, 0, N - 1, K)
            density = 1.0 / (kth_dist + 2.0)
            fits[i] += density

        next_indices = [(fits[i], i) for i in range(N)
                        if i not in chosen_indices]
        next_indices.sort()
        # print next_indices
        chosen_indices += [i for _, i in next_indices[:k - len(chosen_indices)]]

    elif len(chosen_indices) > k:  # The archive is too large
        N = len(chosen_indices)
        distances = [[0.0] * N for i in range(N)]
        sorted_indices = [[0] * N for i in range(N)]
        for i in range(N):
            for j in range(i + 1, N):
                dist = 0.0
                for k in range(L):
                    val = individuals[chosen_indices[i]].fitness.values[k] - \
                          individuals[chosen_indices[j]].fitness.values[k]
                    dist += val * val
                distances[i][j] = dist
                distances[j][i] = dist
            distances[i][i] = -1

        # Insert sort is faster than quick sort for short arrays
        for i in range(N):
            for j in range(1, N):
                k = j
                while k > 0 and distances[i][j] < distances[i][sorted_indices[i][k - 1]]:
                    sorted_indices[i][k] = sorted_indices[i][k - 1]
                    k -= 1
                sorted_indices[i][k] = j

        size = N
        to_remove = []
        while size > k:
            # Search for minimal distance
            min_pos = 0
            for i in range(1, N):
                for j in range(1, size):
                    dist_i_sorted_j = distances[i][sorted_indices[i][j]]
                    dist_min_sorted_j = distances[min_pos][sorted_indices[min_pos][j]]

                    if dist_i_sorted_j < dist_min_sorted_j:
                        min_pos = i
                        break
                    elif dist_i_sorted_j > dist_min_sorted_j:
                        break

            # Remove minimal distance from sorted_indices
            for i in range(N):
                distances[i][min_pos] = float("inf")
                distances[min_pos][i] = float("inf")

                for j in range(1, size - 1):
                    if sorted_indices[i][j] == min_pos:
                        sorted_indices[i][j] = sorted_indices[i][j + 1]
                        sorted_indices[i][j + 1] = min_pos

            # Remove corresponding individual from chosen_indices
            to_remove.append(min_pos)
            size -= 1

        for index in reversed(sorted(to_remove)):
            del chosen_indices[index]

    return [individuals[i] for i in chosen_indices]


def _randomizedSelect(array, begin, end, i):
    """Allows to select the ith smallest element from array without sorting it.
    Runtime is expected to be O(n).
    """
    if begin == end:
        return array[begin]
    q = _randomizedPartition(array, begin, end)
    k = q - begin + 1
    if i < k:
        return _randomizedSelect(array, begin, q, i)
    else:
        return _randomizedSelect(array, q + 1, end, i - k)


def _randomizedPartition(array, begin, end):
    i = random.randint(begin, end)
    array[begin], array[i] = array[i], array[begin]
    return _partition(array, begin, end)


def _partition(array, begin, end):
    x = array[begin]
    i = begin - 1
    j = end + 1
    while True:
        j -= 1
        while array[j] > x:
            j -= 1
        i += 1
        while array[i] < x:
            i += 1
        if i < j:
            array[i], array[j] = array[j], array[i]
        else:
            return j


__all__ = ['selNSGA2', 'selNSGA3', 'selNSGA2_prefer', 'selNSGA2_multiuser_prefer', 'selNSGA3WithMemory', 'selSPEA2', 'sortNondominated',
           'sortLogNondominated',
           'selTournamentDCD', 'uniform_reference_points']
