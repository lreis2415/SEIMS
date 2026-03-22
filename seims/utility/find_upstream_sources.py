import os
import sys

def load_flow_data(file_path):
    """
    读取流向文件并构建反向索引 (downstream -> [upstream_ids])。
    
    Args:
        file_path: 流向文件路径
        
    Returns:
        dict: 键为下游ID，值为上游ID列表
    """
    upstream_map = {}
    
    if not os.path.exists(file_path):
        print(f"错误: 找不到文件 {file_path}")
        return None

    print("正在加载流向数据...")
    with open(file_path, 'r') as f:
        # 读取所有行
        lines = f.readlines()
        
        # 简单检查文件行数
        if len(lines) < 2:
            return upstream_map

        # 跳过前两行（第一行是总数，第二行是表头）
        # 假设格式: ID DownstreamCount DownstreamID FlowOutFraction
        for i, line in enumerate(lines):
            if i < 2: 
                continue
            
            # 使用制表符或空格分割
            parts = line.strip().split()
            if len(parts) >= 3:
                try:
                    cell_id = int(parts[0])
                    downstream_id = int(parts[2])
                    
                    if downstream_id not in upstream_map:
                        upstream_map[downstream_id] = []
                    upstream_map[downstream_id].append(cell_id)
                except ValueError:
                    continue
                    
    return upstream_map

def find_longest_path(target_id, upstream_map):
    """
    寻找指定ID到所有源头中最长的一条路径。
    
    Args:
        target_id: 目标ID
        upstream_map: 反向流向映射
        
    Returns:
        list: 从源头到目标ID的路径列表 [source_id, ..., target_id]
              注意：这里的路径是从源头流向目标的顺序
    """
    # 存储最长路径
    longest_path = []
    
    # 检查目标ID本身是否存在于反向映射中
    if target_id not in upstream_map:
        return [target_id]

    # 使用栈进行DFS，不仅仅存储ID，还存储当前的路径
    # 栈元素: (current_id, current_path_from_target_upwards)
    # 注意：我们在遍历时是从下游往上游走，所以 path 是 [target, ..., current]
    stack = [(target_id, [target_id])]
    
    while stack:
        current_id, current_path = stack.pop()
        
        # 获取当前ID的所有直接上游
        parents = upstream_map.get(current_id, [])
        
        if not parents:
            # 如果没有上游，说明到达了源头，比较当前路径长度
            if len(current_path) > len(longest_path):
                longest_path = current_path
        else:
            # 如果有上游，继续向上遍历
            for parent in parents:
                # 简单防环检测：如果你已经在路径里了，就不走了
                if parent not in current_path:
                    # 创建新路径列表（复制当前路径并添加父节点）
                    new_path = current_path + [parent]
                    stack.append((parent, new_path))
    
    # 因为我们是从下游往上游找的，所以目前的 longest_path 是 [target, ..., source]
    # 通常流向是从 source 到 target，所以我们反转一下
    return longest_path[::-1]

def find_sources(target_id, upstream_map):
    """
    寻找指定ID的所有上游源头ID。
    源头ID是指没有上游ID的那些ID。
    
    Args:
        target_id: 目标ID
        upstream_map: 反向流向映射
        
    Returns:
        set: 所有上游源头ID的集合
    """
    sources = set()
    visited = set()
    
    # 使用栈进行深度优先搜索 (DFS)
    stack = [target_id]
    visited.add(target_id)
    
    # 检查目标ID本身是否存在于反向映射中
    # 如果目标ID没有任何上游（即它不在upstream_map的key中），那么它本身可能就是一个源头（或者孤立点）
    # 但根据逻辑，我们是从一个点往上找。如果这个点没有上游，它本身就是起始点。
    if target_id not in upstream_map:
        return {target_id}

    while stack:
        current_id = stack.pop()
        
        # 获取当前ID的所有直接上游
        parents = upstream_map.get(current_id, [])
        
        if not parents:
            # 如果没有上游，说明到达了源头
            sources.add(current_id)
        else:
            # 如果有上游，继续向上遍历
            for parent in parents:
                if parent not in visited:
                    visited.add(parent)
                    stack.append(parent)
                    
    return sources

def main():
    # 默认文件路径
    default_path = r"e:\code\SEIMS\data\youwuzhen\demo_youwuzhen30m_model\workspace\layering_info\0_FLOWOUT_INDEX_D8.txt"
    
    # 允许从命令行传递ID
    target_ids = []
    if len(sys.argv) > 1:
        for arg in sys.argv[1:]:
            try:
                target_ids.append(int(arg))
            except ValueError:
                pass
    
    if not target_ids:
        print("未指定ID，将使用测试ID: 45")
        target_ids = [1944]
    
    # 加载数据
    upstream_map = load_flow_data(default_path)
    
    if upstream_map is not None:
        for tid in target_ids:
            print(f"\n正在查找 ID {tid} 的上游源头及最长路径...")
            
            # 查找所有源头
            sources = find_sources(tid, upstream_map)
            sorted_sources = sorted(list(sources))
            print(f"结果: 找到 {len(sources)} 个源头 ID")
            print(f"源头列表: {sorted_sources}")
            
            # 查找最长路径
            longest_path = find_longest_path(tid, upstream_map)
            print(f"最长路径长度: {len(longest_path)}")
            print(f"最长路径源头: {longest_path[0]}")
            print(f"最长路径 (Source -> Target): {longest_path}")

if __name__ == "__main__":
    main()
