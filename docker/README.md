# SEIMS情景优化框架 - Docker部署快速指南

## 概述

本指南介绍如何使用Docker部署SEIMS情景优化框架，实现一键启动优化任务。

## 前置要求

- Docker Engine 20.10+
- Docker Compose 1.29+
- 至少8GB可用内存
- 至少20GB可用磁盘空间

## 快速开始

### 1. 构建Docker镜像

```bash
cd D:\EGC\SEIMS-dev

# 标准版本（包含所有工具）
docker build -t seims-optimizer:latest -f docker/Dockerfile .

# 或使用多阶段构建（镜像更小）
docker build -t seims-optimizer:latest -f docker/Dockerfile.multistage .
```

### 2. 启动服务

```bash
cd docker

# 启动MongoDB和SEIMS优化器
docker-compose up -d

# 查看服务状态
docker-compose ps

# 查看日志
docker-compose logs -f
```

### 3. 运行优化任务

#### 方式1: 使用配置文件

```bash
# 将配置文件放到workspace目录
cp your_config.json docker/workspace/

# 运行优化
docker-compose run --rm seims-optimizer --config /workspace/your_config.json
```

#### 方式2: 使用预设配置

```bash
# 快速扫描（50代，40个体）
docker-compose run --rm seims-optimizer \
  --preset quick_scan \
  -m /data/youwuzhen/demo_youwuzhen30m_longterm_model

# 空间优化+约束（100代，60个体）
docker-compose run --rm seims-optimizer \
  --preset spatial_constrained \
  -m /data/youwuzhen/demo_youwuzhen30m_longterm_model

# 时空优化+约束（300代，80个体）
docker-compose run --rm seims-optimizer \
  --preset spatio_temporal_constrained \
  -m /data/youwuzhen/demo_youwuzhen30m_longterm_model
```

#### 方式3: 使用命令行参数

```bash
docker-compose run --rm seims-optimizer \
  --mode spatial \
  --model-dir /data/youwuzhen/demo_youwuzhen30m_longterm_model \
  --generations 100 \
  --population-size 60 \
  --spatial-unit SLPPOS \
  --config-method HILLSLP \
  --output-dir /output/my_optimization
```

### 4. 查看结果

优化结果保存在 `docker/output/` 目录下：

```bash
ls docker/output/
```

### 5. 停止服务

```bash
cd docker
docker-compose down

# 如果需要删除数据卷
docker-compose down -v
```

## 配置文件示例

### 空间优化（无约束）

```json
{
  "mode": "spatial",
  "model": {
    "model_dir": "/data/youwuzhen/demo_youwuzhen30m_longterm_model"
  },
  "spatial": {
    "unit": "SLPPOS",
    "config_method": "HILLSLP"
  },
  "algorithm": {
    "GenerationsNum": 100,
    "PopulationSize": 60
  },
  "budget": {
    "enable_investment_quota": false
  }
}
```

### 时空优化（带约束）

```json
{
  "mode": "spatio_temporal",
  "model": {
    "model_dir": "/data/youwuzhen/demo_youwuzhen30m_longterm_model"
  },
  "spatial": {
    "unit": "SLPPOS",
    "config_method": "HILLSLP"
  },
  "temporal": {
    "enable_implementation_order": true,
    "implementation_period": 5,
    "change_frequency": 1
  },
  "algorithm": {
    "GenerationsNum": 300,
    "PopulationSize": 80
  },
  "budget": {
    "enable_investment_quota": true,
    "investment_each_period": [60, 50, 110],
    "investment_float_range": 0.2
  }
}
```

## 环境变量配置

可以通过环境变量覆盖默认配置：

```bash
# 在docker-compose.yml中修改或通过命令行传递
docker-compose run --rm \
  -e MONGODB_HOST=custom-mongodb \
  -e MONGODB_PORT=27017 \
  -e LOG_LEVEL=DEBUG \
  seims-optimizer --config /workspace/config.json
```

## 挂载自定义数据

如果需要使用自己的流域数据：

```bash
# 修改docker-compose.yml中的volumes配置
volumes:
  - /path/to/your/watershed/data:/data/custom_watershed

# 然后在配置文件中指定
{
  "model": {
    "model_dir": "/data/custom_watershed/model"
  }
}
```

## 故障排查

### MongoDB连接失败

```bash
# 检查MongoDB是否启动
docker-compose ps mongodb

# 查看MongoDB日志
docker-compose logs mongodb

# 手动测试连接
docker-compose exec mongodb mongo -u admin -p seims_admin_2024
```

### 优化器启动失败

```bash
# 查看详细日志
docker-compose logs seims-optimizer

# 进入容器调试
docker-compose run --rm --entrypoint /bin/bash seims-optimizer

# 验证配置文件
docker-compose run --rm seims-optimizer --dry-run --config /workspace/config.json
```

### 内存不足

```bash
# 增加Docker内存限制（在docker-compose.yml中）
services:
  seims-optimizer:
    mem_limit: 8g
    memswap_limit: 8g
```

## 高级用法

### 并行运行多个优化任务

```bash
# 使用不同的配置文件和输出目录
docker-compose run -d --name opt1 seims-optimizer --config /workspace/config1.json
docker-compose run -d --name opt2 seims-optimizer --config /workspace/config2.json

# 监控进度
docker logs -f opt1
docker logs -f opt2
```

### 导出优化结果

```bash
# 结果自动保存在output目录
tar -czf optimization_results.tar.gz docker/output/

# 或直接从容器复制
docker cp seims-optimizer:/output ./results
```

## 性能优化建议

1. **使用多阶段构建**：减小镜像体积，加快部署速度
2. **调整种群大小**：根据可用内存调整PopulationSize
3. **使用SCOOP并行**：在配置中启用并行计算
4. **MongoDB索引**：确保MongoDB已创建必要索引
5. **数据卷优化**：使用本地卷而非网络卷

## 云端部署

### AWS ECS

```bash
# 推送镜像到ECR
aws ecr get-login-password --region us-east-1 | docker login --username AWS --password-stdin <account>.dkr.ecr.us-east-1.amazonaws.com
docker tag seims-optimizer:latest <account>.dkr.ecr.us-east-1.amazonaws.com/seims-optimizer:latest
docker push <account>.dkr.ecr.us-east-1.amazonaws.com/seims-optimizer:latest
```

### Kubernetes

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: seims-optimizer
spec:
  replicas: 1
  template:
    spec:
      containers:
      - name: seims-optimizer
        image: seims-optimizer:latest
        env:
        - name: MONGODB_HOST
          value: mongodb-service
```

## 支持与反馈

如遇问题，请查看日志文件：
- 容器日志: `docker-compose logs`
- 优化器日志: `docker/output/seims_optimizer.log`
- MongoDB日志: `docker-compose logs mongodb`
