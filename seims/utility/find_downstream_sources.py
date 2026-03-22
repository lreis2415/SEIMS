import os
import sys

def load_flow_data(file_path):
    """
    读取流向文件并构建双向索引。
    
    Args:
        file_path: 流向文件路径
        
    Returns:
        tuple: (upstream_map, downstream_map)
               upstream_map: {下游ID: [上游ID列表]}
               downstream_map: {当前ID: 下游ID}  <-- 新增这个返回
    """
    upstream_map = {}
    downstream_map = {}  # === 修改: 新增下游映射 ===
    
    if not os.path.exists(file_path):
        print(f"错误: 找不到文件 {file_path}")
        return None, None

    print("正在加载流向数据...")
    with open(file_path, 'r') as f:
        lines = f.readlines()
        
        if len(lines) < 2:
            return upstream_map, downstream_map

        # 跳过前两行
        for i, line in enumerate(lines):
            if i < 2: 
                continue
            
            parts = line.strip().split()
            if len(parts) >= 3:
                try:
                    cell_id = int(parts[0])      # 当前栅格
                    downstream_id = int(parts[2]) # 下游栅格
                    
                    # === 1. 构建上游映射 (原功能) ===
                    if downstream_id not in upstream_map:
                        upstream_map[downstream_id] = []
                    upstream_map[downstream_id].append(cell_id)
                    
                    # === 2. 构建下游映射 (新功能) ===
                    # 注意：D8算法通常是一个栅格流向一个下游
                    downstream_map[cell_id] = downstream_id
                    
                except ValueError:
                    continue
                    
    return upstream_map, downstream_map

def trace_downstream(target_id, downstream_map):
    """
    === 新增函数 ===
    寻找指定ID的下游路径，直到出口。
    
    Args:
        target_id: 起始ID
        downstream_map: 正向流向映射 {ID: Next_ID}
        
    Returns:
        list: [target_id, next_id, ..., outlet_id]
    """
    path = []
    current_id = target_id
    
    # 防止死循环的集合
    visited = set()
    
    while True:
        path.append(current_id)
        visited.add(current_id)
        
        # 查找当前栅格流向哪里
        next_id = downstream_map.get(current_id)
        
        # 如果没有下游 (到达出口或无效值)，停止
        # 有些模型用 -1 或 0 表示出口，这里假设不在字典里就是出口
        if next_id is None:
            break
            
        # 如果下游已经在路径里，说明有环 (死循环)，强制停止
        if next_id in visited:
            print(f"警告: 检测到循环流向 {current_id} -> {next_id}")
            break
            
        current_id = next_id
        
    return path

# --- 以下保留原有的上游搜索函数 (为了代码完整性) ---

def find_longest_path(target_id, upstream_map):
    # (原代码保持不变，用于查找上游最长路径)
    longest_path = []
    if target_id not in upstream_map:
        return [target_id]
    stack = [(target_id, [target_id])]
    while stack:
        current_id, current_path = stack.pop()
        parents = upstream_map.get(current_id, [])
        if not parents:
            if len(current_path) > len(longest_path):
                longest_path = current_path
        else:
            for parent in parents:
                if parent not in current_path:
                    new_path = current_path + [parent]
                    stack.append((parent, new_path))
    return longest_path[::-1]

def find_sources(target_id, upstream_map):
    # (原代码保持不变，用于查找源头)
    sources = set()
    visited = set()
    stack = [target_id]
    visited.add(target_id)
    if target_id not in upstream_map:
        return {target_id}
    while stack:
        current_id = stack.pop()
        parents = upstream_map.get(current_id, [])
        if not parents:
            sources.add(current_id)
        else:
            for parent in parents:
                if parent not in visited:
                    visited.add(parent)
                    stack.append(parent)
    return sources

def main():
    # 默认文件路径
    default_path = r"e:\code\SEIMS\data\youwuzhen\demo_youwuzhen30m_model\workspace\layering_info\0_FLOWOUT_INDEX_D8.txt"
    
    target_ids = []
    if len(sys.argv) > 1:
        for arg in sys.argv[1:]:
            try:
                target_ids.append(int(arg))
            except ValueError:
                pass
    
    if not target_ids:
        print("未指定ID，将使用测试ID: 1944")
        target_ids = [1944]
    
    # === 修改: 接收两个返回值 ===
    upstream_map, downstream_map = load_flow_data(default_path)
    
    if upstream_map is not None:
        for tid in target_ids:
            print("="*50)
            print(f"分析栅格 ID: {tid}")
            
            # === 新增: 查找下游 ===
            print(f"\n[下游分析]")
            downstream_path = trace_downstream(tid, downstream_map)
            print(f"流向路径长度: {len(downstream_path)}")
            if len(downstream_path) > 1:
                print(f"直接下游: {downstream_path[1]}")
                print(f"最终出口: {downstream_path[-1]}")
                # 如果路径太长，只打印前10个和后5个
                if len(downstream_path) > 20:
                     print(f"完整路径: {downstream_path[:10]} ... {downstream_path[-5:]}")
                else:
                     print(f"完整路径: {downstream_path}")
            else:
                print("该栅格通过文件未找到下游（可能是流域出口）。")

            # === 原有: 查找上游 ===
            print(f"\n[上游分析]")
            sources = find_sources(tid, upstream_map)
            print(f"上游源头数量: {len(sources)}")
            # longest = find_longest_path(tid, upstream_map)
            # print(f"最长汇流路径长度: {len(longest)}")
            output_filename = f"path_trace_{tid}.txt"
            with open(output_filename, "w") as f:
                # 写入一行，逗号分隔
                f.write(",".join(map(str, downstream_path)))
            print(f"完整路径已保存至文件: {os.path.abspath(output_filename)}")

if __name__ == "__main__":
    main()