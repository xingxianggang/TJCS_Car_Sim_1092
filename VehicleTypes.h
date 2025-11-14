
#ifndef VEHICLE_TYPES_H
#define VEHICLE_TYPES_H

#include "Class.h"


// 小轿车类
struct Sedan : public Vehicle {
    Sedan(int lane, int carlength, int carwidth, int x, int y, int speed);
    // 重写变道函数，实现更快的变道曲线
    bool smoothLaneChange(int laneHeight, const vector<Vehicle> &allVehicles);
    // 获取小轿车的安全距离
    int getSafeDistance() const;
};

// SUV类
struct SUV : public Vehicle {
    SUV(int lane, int carlength, int carwidth, int x, int y, int speed);
    // 重写变道函数，实现中等的变道曲线
    bool smoothLaneChange(int laneHeight, const vector<Vehicle> &allVehicles);
    // 获取SUV的安全距离
    int getSafeDistance() const;
};

// 大卡车类
struct Truck : public Vehicle {
    Truck(int lane, int carlength, int carwidth, int x, int y, int speed);
    // 重写变道函数，实现更慢的变道曲线
    bool smoothLaneChange(int laneHeight, const vector<Vehicle> &allVehicles);
    // 获取大卡车的安全距离
    int getSafeDistance() const;
};

#endif // VEHICLE_TYPES_H
