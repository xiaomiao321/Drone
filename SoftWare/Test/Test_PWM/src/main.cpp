#include <Arduino.h>

#include <Servo.h>

// 定义电调连接的引脚（使用支持PWM的引脚，例如9号引脚）
const int ESC_PIN = 9;

// 创建Servo对象来控制电调
Servo esc;

void setup()
{
  // 初始化串口，用于调试输出（可选）
  Serial.begin(9600);

  // 将Servo对象附加到指定引脚
  esc.attach(ESC_PIN);

  // 初始化电调：先发送最小信号（1000us）以臂化ESC
  esc.writeMicroseconds(1000);  // 最小脉宽，通常为1000us（取决于ESC型号）
  delay(2000);  // 等待2秒，让ESC臂化
  Serial.println("ESC Initialized");
}

void loop()
{
  // 测试程序：从最小速度到最大速度扫频，然后反向
  for (int speed = 1000; speed <= 2000; speed += 10)
  {  // 从1000us到2000us递增
    esc.writeMicroseconds(speed);
    Serial.print("PWM: ");
    Serial.println(speed);
    delay(20);  // 缓慢变化以观察效果
  }

  delay(1000);  // 最大速度保持1秒

  for (int speed = 2000; speed >= 1000; speed -= 10)
  {  // 从2000us到1000us递减
    esc.writeMicroseconds(speed);
    Serial.print("PWM: ");
    Serial.println(speed);
    delay(20);
  }

  delay(1000);  // 最小速度保持1秒
}