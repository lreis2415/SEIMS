#!/bin/bash
# SEIMS优化器容器入口脚本
#
# 功能:
# 1. 等待MongoDB服务启动
# 2. 验证环境配置
# 3. 执行优化任务

set -e

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}SEIMS Scenario Optimization Framework${NC}"
echo -e "${GREEN}========================================${NC}"

# ==========================================
# 1. 等待MongoDB启动
# ==========================================
if [ -n "$MONGODB_HOST" ]; then
    echo -e "${YELLOW}Waiting for MongoDB at $MONGODB_HOST:$MONGODB_PORT...${NC}"

    TIMEOUT=60
    ELAPSED=0

    while ! nc -z "$MONGODB_HOST" "$MONGODB_PORT" 2>/dev/null; do
        if [ $ELAPSED -ge $TIMEOUT ]; then
            echo -e "${RED}Error: MongoDB connection timeout after ${TIMEOUT}s${NC}"
            exit 1
        fi
        sleep 1
        ELAPSED=$((ELAPSED + 1))
    done

    echo -e "${GREEN}MongoDB is ready!${NC}"
fi

# ==========================================
# 2. 验证环境
# ==========================================
echo -e "${YELLOW}Validating environment...${NC}"

# 检查Python
if ! command -v python3 &> /dev/null; then
    echo -e "${RED}Error: Python3 not found${NC}"
    exit 1
fi

PYTHON_VERSION=$(python3 --version)
echo -e "${GREEN}✓ Python: $PYTHON_VERSION${NC}"

# 检查SEIMS模块
if ! python3 -c "import sys; sys.path.insert(0, '/app'); from seims.scenario_analysis.unified_optimizer_v2 import UnifiedOptimizerV2" 2>/dev/null; then
    echo -e "${YELLOW}Warning: SEIMS modules may not be properly installed${NC}"
fi

# 检查数据目录
if [ -d "/data/youwuzhen/demo_youwuzhen30m_longterm_model" ]; then
    echo -e "${GREEN}✓ Built-in case data found${NC}"
else
    echo -e "${YELLOW}Warning: Built-in case data not found, ensure data is mounted${NC}"
fi

# ==========================================
# 3. 处理命令行参数
# ==========================================
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    echo ""
    echo "Usage: docker run [OPTIONS] seims-optimizer [ARGS]"
    echo ""
    echo "Arguments:"
    echo "  --config FILE         Configuration file path"
    echo "  --preset NAME         Use preset configuration"
    echo "  --mode MODE           Optimization mode (spatial/temporal/spatio_temporal)"
    echo "  --help                Show this help message"
    echo ""
    echo "Examples:"
    echo "  # Using configuration file"
    echo "  docker run -v \$(pwd)/workspace:/workspace seims-optimizer --config /workspace/config.json"
    echo ""
    echo "  # Using preset"
    echo "  docker run -v \$(pwd)/data:/data seims-optimizer --preset quick_scan -m /data/youwuzhen/..."
    echo ""
    echo "Environment Variables:"
    echo "  MONGODB_HOST          MongoDB hostname (default: mongodb)"
    echo "  MONGODB_PORT          MongoDB port (default: 27017)"
    echo "  MONGODB_USER          MongoDB username"
    echo "  MONGODB_PASSWORD      MongoDB password"
    echo ""
    exit 0
fi

# ==========================================
# 4. 执行优化
# ==========================================
echo -e "${GREEN}Starting optimization...${NC}"
echo ""

cd /app/seims/scenario_analysis

# 执行Python脚本
exec python3 run_unified_v2.py "$@"
