#include <graphics.h>
#include <vector>
#include <ctime>
#include <conio.h>
#include <Windows.h>
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>

#include "Random.h"
#include "Class.h"
#include "Define.h"
#include "VehicleTypes.h"
#include "BridgeLightingControl.h"
using namespace std;


int main()
{
    Bridge bridge;
    bridge.bridgeLength = 100;
    bridge.bridgeWidth = 50;
    bridge.widthScale = 1;
    
    int windowWidth, windowHeight;
    double scale;
    bridge.calculateWindowSize(windowWidth, windowHeight, scale);

    vector<Vehicle*> vehicles;
    srand(unsigned int(time(0)));
    double time = 0;
    
    normal_distribution<> normalwidth(3, 0.1);
    normal_distribution<> normallength(6, 0.1);
    uniform_int_distribution<int> int_dist(20, 120);
    RandomGenerator rng(int_dist);
    
    // 创建天气效果管理器
    WeatherEffectManager weatherManager(windowWidth, windowHeight);
    
    // 定义天气按钮位置和尺寸
    const int btnWidth = 80;
    const int btnHeight = 35;
    const int btnSpacing = 10;
    const int btnStartX = windowWidth - 300;
    const int btnStartY = 70;
    
    // 三个按钮的矩形区域
    struct ButtonRect {
        int x1, y1, x2, y2;
        WeatherMode mode;
        const wchar_t* text;
    };
    
    ButtonRect weatherButtons[3] = {
        {btnStartX, btnStartY, btnStartX + btnWidth, btnStartY + btnHeight, NOTHING, L"正常"},
        {btnStartX + btnWidth + btnSpacing, btnStartY, btnStartX + 2 * btnWidth + btnSpacing, btnStartY + btnHeight, RAIN, L"下雨"},
        {btnStartX + 2 * (btnWidth + btnSpacing), btnStartY, btnStartX + 3 * btnWidth + 2 * btnSpacing, btnStartY + btnHeight, SNOW, L"下雪"}
    };
    
    bool running = true;
    
    while (running)
    {
        cleardevice();
        
        // 绘制天气效果（在最底层）
        weatherManager.update();
        
        // 显示桥的参数信息
        wchar_t info[256];
        swprintf_s(info, L"桥长： %.0fm  桥宽：%.0fm  桥宽放大率： %.1f", 
                  bridge.bridgeLength, bridge.bridgeWidth, bridge.widthScale);
        settextstyle(20, 0, L"Arial");
        outtextxy(10, 10, info);
        
        // 显示时间
        wchar_t info2[256];
        swprintf_s(info2, L"时间： %.0fs", time);
        settextstyle(20, 0, L"Arial");
        outtextxy(windowWidth - 150, 10, info2);
        
        // 显示当前天气状态
        wchar_t weatherInfo[128];
        WeatherMode currentWeather = weatherManager.getCurrentWeather();
        const wchar_t* weatherText = L"正常";
        if (currentWeather == RAIN) weatherText = L"下雨";
        else if (currentWeather == SNOW) weatherText = L"下雪";
        swprintf_s(weatherInfo, L"当前天气： %s", weatherText);
        settextstyle(20, 0, L"Arial");
        outtextxy(btnStartX, 40, weatherInfo);
        
        // 绘制天气控制按钮
        for (int i = 0; i < 3; i++)
        {
            // 判断是否是当前选中的天气
            bool isActive = (weatherButtons[i].mode == currentWeather);
            
            // 设置按钮颜色：选中的按钮高亮
            if (isActive)
            {
                setfillcolor(RGB(0, 120, 215));  // 蓝色高亮
                setlinecolor(RGB(0, 84, 153));
            }
            else
            {
                setfillcolor(RGB(70, 70, 70));   // 灰色
                setlinecolor(RGB(200, 200, 200));
            }
            
            // 绘制按钮矩形
            fillroundrect(weatherButtons[i].x1, weatherButtons[i].y1, 
                         weatherButtons[i].x2, weatherButtons[i].y2, 8, 8);
            
            // 绘制按钮文字
            settextstyle(22, 0, L"Arial");
            settextcolor(WHITE);
            setbkmode(TRANSPARENT);
            
            // 计算文字居中位置
            int textX = weatherButtons[i].x1 + (btnWidth - textwidth(weatherButtons[i].text)) / 2;
            int textY = weatherButtons[i].y1 + (btnHeight - textheight(weatherButtons[i].text)) / 2;
            outtextxy(textX, textY, weatherButtons[i].text);
        }

        // 绘制车道
        setlinecolor(WHITE);
        settextcolor(WHITE);
        int laneCount = 6;
        int laneHeight = (int)(windowHeight / laneCount);
        for (int i = 0; i < laneCount - 1; ++i)
        {
            drawDashedLine(0, (i + 1) * laneHeight, windowWidth, (i + 1) * laneHeight);
        }
        
        // 绘制车道清除按钮（改为圆角）
        for (int i = 0; i < laneCount; ++i)
        {
            int buttonX = 5;
            int buttonY = laneHeight * i + (int)(0.5 * laneHeight) - (int)(laneHeight / 4);
            int buttonWidth = 40;
            int buttonHeight = (int)(laneHeight / 2);

            // 使用与天气按钮相同的灰色主题
            setfillcolor(RGB(70, 70, 70));
            setlinecolor(RGB(200, 200, 200));
            
            // 改为圆角矩形！
            fillroundrect(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight, 8, 8);

            // 绘制箭头文字
            settextstyle((int)(laneHeight / 2), 0, L"Arial");
            settextcolor(WHITE);
            setbkmode(TRANSPARENT);
            outtextxy(buttonX + 10, buttonY, i < laneCount / 2 ? L"→" : L"←");
        }

        // 检查鼠标点击
        if (MouseHit())
        {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN)
            {
                // 检查是否点击车道清除按钮
                if (msg.x < 45)
                {
                    int clickedLane = msg.y / laneHeight;
                    if (clickedLane >= 0 && clickedLane < laneCount)
                    {
                        clearLane(vehicles, clickedLane);
                    }
                }
                
                // 检查是否点击天气按钮
                for (int i = 0; i < 3; i++)
                {
                    if (msg.x >= weatherButtons[i].x1 && msg.x <= weatherButtons[i].x2 &&
                        msg.y >= weatherButtons[i].y1 && msg.y <= weatherButtons[i].y2)
                    {
                        weatherManager.setWeather(weatherButtons[i].mode);
                        break;
                    }
                }
            }
        }
        
        // 保留键盘快捷键（可选）
        if (_kbhit())
        {
            char key = _getch();
            if (key == 'r' || key == 'R') {
                weatherManager.setWeather(RAIN);
            }
            else if (key == 's' || key == 'S') {
                weatherManager.setWeather(SNOW);
            }
            else if (key == 'n' || key == 'N') {
                weatherManager.setWeather(NOTHING);
            }
            else if (key == 27 || key == 'q' || key == 'Q') {
                running = false;
            }
        }

        // 生成新车
        if (rand() % 10 == 0)
        {
            int lane = rand() % 6;
            int newX = lane < 3 ? 0 : windowWidth;
            int newY = laneHeight * lane + (int)(0.5 * laneHeight);
            bool isPositionSafe = true;
            int carwidth = RandomGenerator{normalwidth}() * scale * bridge.widthScale;
            int carlength = RandomGenerator{normallength}() * scale;

            for (const auto& existingVehicle : vehicles)
            {
                if (existingVehicle->lane != lane)
                    continue;

                int distance = abs(existingVehicle->x - newX) - 
                             (existingVehicle->carlength / 2 + carlength / 2);

                if (distance < SAFE_DISTANCE)
                {
                    isPositionSafe = false;
                    break;
                }
            }

            if (isPositionSafe)
            {
                int vehicleType = rand() % 3;
                Vehicle* newVehicle = nullptr;
                
                if (vehicleType == 0)
                {
                    newVehicle = new Sedan(
                        lane,
                        carlength,
                        carwidth,
                        newX,
                        newY,
                        (int)rng.generate());
                }
                else if (vehicleType == 1)
                {
                    newVehicle = new SUV(
                        lane,
                        carlength,
                        carwidth,
                        newX,
                        newY,
                        (int)rng.generate());
                }
                else
                {
                    newVehicle = new Truck(
                        lane,
                        carlength,
                        carwidth,
                        newX,
                        newY,
                        (int)rng.generate());
                }
                
                if (newVehicle) {
                    vehicles.push_back(newVehicle);
                }
            }
        }

        // 更新车辆的位置
        int middleY = windowHeight / 2;

        for (auto& v : vehicles)
        {
            COLORREF originalColor = v->color;
            if (v->speed == 0)
            {
                v->handleDangerousSituation();
            }
            
            v->moveForward(middleY);
            v->checkFrontVehicleDistance(vehicles, v->getSafeDistance());

            if (v->isGoing2change)
            {
                if (v->smoothLaneChange(laneHeight, vehicles))
                {
                    v->haschanged = true;
                }
            }

            if (v->isTooClose)
            {
                bool stillTooClose = false;
                for (const auto other : vehicles)
                {
                    if (other == v)
                        continue;
                    if (other->lane != v->lane)
                        continue;

                    bool isMovingRight = (v->lane < 3);
                    bool isFrontVehicle = isMovingRight ? (other->x > v->x) : (other->x < v->x);

                    if (isFrontVehicle)
                    {
                        int distance = abs(other->x - v->x) - 
                                     (other->carlength / 2 + v->carlength / 2);
                        if (distance <= SAFE_DISTANCE)
                        {
                            stillTooClose = true;
                            break;
                        }
                    }
                }

                if (!stillTooClose)
                {
                    v->color = v->originalColor;
                    v->isTooClose = false;
                }
            }
        }

        // 移除离开车辆并释放内存
        vehicles.erase(
            remove_if(vehicles.begin(), vehicles.end(),
                [windowWidth](Vehicle* v) {
                    if (v->x < 0 || v->x > windowWidth) {
                        delete v;
                        return true;
                    }
                    return false;
                }),
            vehicles.end());
        // 绘制车辆
        for (const auto& v : vehicles)
        {
            v->predictAndDrawTrajectory(laneHeight, windowHeight / 2, 30, vehicles);
            v->draw();
        }

        Sleep(60);
        time += 0.2;
    }

    // 程序结束前释放所有车辆内存
    for (auto v : vehicles)
    {
        delete v;
    }
    vehicles.clear();

    closegraph();
    return 0;
}
