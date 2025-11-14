#include "Class.h"
#include "Define.h"
#include "Random.h"
#include <cmath>

// 小轿车类实现
Sedan::Sedan(int lane, int carlength, int carwidth, int x, int y, int speed)
    : Vehicle()
{
    this->lane = lane;
    this->carlength = carlength;
    this->carwidth = carwidth;
    this->x = x;
    this->y = y;
    // 小轿车速度范围：80-150
    this->speed = 80 + rand() % 71;
    this->haschanged = false;
    this->color = RGB(rand() % 256, rand() % 256, rand() % 256);
    this->isChangingLane = false;
    this->isGoing2change = false;
    this->targetLane = 0;
    this->changeProgress = 0.0f;
    this->startX = 0;
    this->startY = 0;
    this->endX = 0;
    this->endY = 0;
    this->isTooClose = false;
    this->originalColor = this->color;
    this->isBrokenDown = false;
}

// SUV类实现
SUV::SUV(int lane, int carlength, int carwidth, int x, int y, int speed)
    : Vehicle()
{
    this->lane = lane;
    this->carlength = carlength;
    this->carwidth = carwidth;
    this->x = x;
    this->y = y;
    // SUV速度范围：60-140
    this->speed = 60 + rand() % 81;
    this->haschanged = false;
    this->color = RGB(rand() % 256, rand() % 256, rand() % 256);
    this->isChangingLane = false;
    this->isGoing2change = false;
    this->targetLane = 0;
    this->changeProgress = 0.0f;
    this->startX = 0;
    this->startY = 0;
    this->endX = 0;
    this->endY = 0;
    this->isTooClose = false;
    this->originalColor = this->color;
    this->isBrokenDown = false;
}

// 大卡车类实现
Truck::Truck(int lane, int carlength, int carwidth, int x, int y, int speed)
    : Vehicle()
{
    this->lane = lane;
    this->carlength = carlength;
    this->carwidth = carwidth;
    this->x = x;
    this->y = y;
    // 大卡车速度范围：50-100
    this->speed = 50 + rand() % 51;
    this->haschanged = false;
    this->color = RGB(rand() % 256, rand() % 256, rand() % 256);
    this->isChangingLane = false;
    this->isGoing2change = false;
    this->targetLane = 0;
    this->changeProgress = 0.0f;
    this->startX = 0;
    this->startY = 0;
    this->endX = 0;
    this->endY = 0;
    this->isTooClose = false;
    this->originalColor = this->color;
    this->isBrokenDown = false;
    // 大卡车尺寸更大
    this->carlength = (int)(carlength * 1.5);
    this->carwidth = (int)(carwidth * 1.3);
}

// 获取小轿车的安全距离 - 最短
int Sedan::getSafeDistance() const
{
    return SAFE_DISTANCE * 0.8; // 比标准安全距离短20%
}

// 获取SUV的安全距离 - 中等
int SUV::getSafeDistance() const
{
    return SAFE_DISTANCE; // 使用标准安全距离
}

// 获取大卡车的安全距离 - 最远
int Truck::getSafeDistance() const
{
    return SAFE_DISTANCE * 1.5; // 比标准安全距离长50%
}

// 内联函数：计算变道垂直速度
inline float calculateVerticalSpeed(float progress, VehicleType type)
{
    float t = progress;
    switch (type)
    {
    case VehicleType::SEDAN: // 小轿车 - 变道最快，使用更陡峭的曲线
        return 6 * t - 6 * t * t;
    case VehicleType::SUV: // SUV - 变道速度中等，使用中等陡峭度的曲线
        return 4.5 * t * t - 3.5 * t * t * t;
    case VehicleType::TRUCK: // 大卡车 - 变道最慢，使用更平缓的曲线
        return 2 * t * t - t * t * t;
    default:
        return 3 * t * t - 2 * t * t * t; // 默认使用基类的曲线
    }
}

// 内联函数：获取变道进度增量
inline float getChangeProgressIncrement(VehicleType type)
{
    switch (type)
    {
    case VehicleType::SEDAN: // 小轿车 - 变道最快
        return 0.04f;        // 比基类快一倍
    case VehicleType::SUV:   // SUV - 变道速度中等
        return 0.03f;        // 比基类稍快
    case VehicleType::TRUCK: // 大卡车 - 变道最慢
        return 0.01f;        // 比基类慢一半
    default:
        return 0.02f; // 默认使用基类的进度增量
    }
}

// 内联函数：获取预测步数
inline int getPredictionSteps(VehicleType type)
{
    switch (type)
    {
    case VehicleType::SEDAN: // 小轿车 - 变道最快，需要更少的步数
        return 15;
    case VehicleType::SUV: // SUV - 变道速度中等，需要中等数量的步数
        return 25;
    case VehicleType::TRUCK: // 大卡车 - 变道最慢，需要更多的步数
        return 50;
    default:
        return 30; // 默认使用基类的步数
    }
}

// 通用变道函数，使用内联函数处理不同类型的车辆
bool performSmoothLaneChange(Vehicle &vehicle, int laneHeight, const vector<Vehicle> &allVehicles, VehicleType type)
{
    // 如果车辆已抛锚，不能变道
    if (vehicle.isBrokenDown)
    {
        return false;
    }

    if (vehicle.isChangingLane)
    {
        // 更新变道进度 - 使用车辆类型特定的进度增量
        vehicle.changeProgress += getChangeProgressIncrement(type);

        if (vehicle.changeProgress >= 1.0f)
        {
            // 变道完成
            vehicle.changeProgress = 1.0f;
            vehicle.isChangingLane = false;
            vehicle.isGoing2change = false;
            vehicle.lane = vehicle.targetLane;
            vehicle.speed = vehicle.speed * 2; // 恢复速度
            return true;
        }

        // 使用车辆类型特定的垂直速度函数计算垂直方向速度
        float verticalSpeed = calculateVerticalSpeed(vehicle.changeProgress, type);

        // 计算垂直方向上的位置变化
        float deltaY = (vehicle.endY - vehicle.startY) * verticalSpeed;
        vehicle.y = vehicle.startY + (int)deltaY;

        return false;
    }

    // 确定目标车道
    int tempTargetLane = 0;
    if (vehicle.lane == 0 || vehicle.lane == 3)
    {
        tempTargetLane = vehicle.lane + 1;
    }
    else if (vehicle.lane == 2 || vehicle.lane == 5)
    {
        tempTargetLane = vehicle.lane - 1;
    }
    else if (vehicle.lane == 1 || vehicle.lane == 4)
    {
        tempTargetLane = vehicle.lane + (rand() % 2 ? 1 : -1);
    }

    // 创建虚拟车辆用于轨迹预测
    VirtualVehicle virtualCar(vehicle.x, vehicle.y, vehicle.carlength, vehicle.carwidth);

    // 添加当前位置
    virtualCar.addTrajectoryPoint(vehicle.x, vehicle.y);

    // 预测变道轨迹
    int currentX = vehicle.x;
    int currentY = vehicle.y;
    int targetY = laneHeight * tempTargetLane + (int)(0.5 * laneHeight);
    int currentSpeed = (vehicle.y < laneHeight * 3) ? vehicle.speed : -vehicle.speed;

    // 获取车辆类型特定的预测步数
    int predictionSteps = getPredictionSteps(type);

    // 预测变道轨迹
    for (int i = 1; i <= predictionSteps; ++i)
    {
        // 计算进度
        float t = min(1.0f, i * getChangeProgressIncrement(type));

        // 计算垂直位置 - 使用车辆类型特定的垂直速度函数
        float verticalSpeed = calculateVerticalSpeed(t, type);
        float deltaY = (targetY - currentY) * verticalSpeed;
        int newY = currentY + (int)deltaY;

        // 计算水平位置（保持原有速度）
        int newX = currentX + i * currentSpeed;

        // 添加到轨迹
        virtualCar.addTrajectoryPoint(newX, newY);
    }

    // 检查与其他车辆的轨迹是否相交
    for (const auto &other : allVehicles)
    {
        if (&other == &vehicle)
            continue; // 跳过自己

        // 为其他车辆创建虚拟车辆
        VirtualVehicle otherVirtual(other.x, other.y, other.carlength, other.carwidth);

        // 判断其他车辆是否在变道中
        if (other.isChangingLane)
        {
            // 如果其他车辆也在变道，预测其变道轨迹
            float otherProgress = other.changeProgress;
            int otherStartX = other.startX;
            int otherStartY = other.startY;
            int otherEndX = other.endX;
            int otherEndY = other.endY;
            int otherSpeed = (other.y < laneHeight * 3) ? other.speed : -other.speed;

            // 预测其他车辆的变道轨迹
            for (int i = 1; i <= predictionSteps; ++i)
            {
                // 更新进度
                float t = min(1.0f, otherProgress + i * 0.02f);

                // 计算垂直位置
                float verticalSpeed = 3 * t * t - 2 * t * t * t;
                float deltaY = (otherEndY - otherStartY) * verticalSpeed;
                int newY = otherStartY + (int)deltaY;

                // 计算水平位置（保持原有速度）
                int newX = other.x + i * otherSpeed;

                // 添加到轨迹
                otherVirtual.addTrajectoryPoint(newX, newY);
            }
        }
        else
        {
            // 其他车辆直线行驶，预测其直线轨迹
            int otherSpeed = (other.y < laneHeight * 3) ? other.speed : -other.speed;
            for (int i = 1; i <= predictionSteps; ++i)
            {
                int newX = other.x + i * otherSpeed;
                otherVirtual.addTrajectoryPoint(newX, other.y);
            }
        }

        // 检查轨迹是否相交
        if (virtualCar.isTrajectoryIntersecting(otherVirtual, predictionSteps))
        {
            vehicle.isGoing2change = false; // 取消准备变道状态
            return false;                   // 轨迹相交，变道不安全，取消变道
        }
    }

    // 变道安全，设置目标车道和变道参数
    vehicle.targetLane = tempTargetLane;
    vehicle.startX = vehicle.x;
    vehicle.startY = vehicle.y;
    vehicle.endX = vehicle.x + 25; // 向前移动50像素
    vehicle.endY = laneHeight * vehicle.targetLane + (int)(0.5 * laneHeight);

    // 开始变道
    vehicle.isChangingLane = true;
    vehicle.changeProgress = 0.0f;
    return false;
}

// 小轿车变道函数
bool Sedan::smoothLaneChange(int laneHeight, const vector<Vehicle> &allVehicles)
{
    return performSmoothLaneChange(*this, laneHeight, allVehicles, VehicleType::SEDAN);
}

// SUV变道函数
bool SUV::smoothLaneChange(int laneHeight, const vector<Vehicle> &allVehicles)
{
    return performSmoothLaneChange(*this, laneHeight, allVehicles, VehicleType::SUV);
}

// 大卡车变道函数
bool Truck::smoothLaneChange(int laneHeight, const vector<Vehicle> &allVehicles)
{
    return performSmoothLaneChange(*this, laneHeight, allVehicles, VehicleType::TRUCK);
}
