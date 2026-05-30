# NRF24L01 无线计时系统 - 调试指南

## 📌 修复内容总结

### 1. 添加了NRF硬件检测
- Finish节点开机会显示 "NRF: OK" 或 "NRF: ERROR!"
- 如果显示ERROR,说明NRF模块没有正确连接

### 2. 添加了详细的调试信息
- Finish节点会显示接收到的所有数据
- Start节点通过串口输出发送状态

### 3. 修复了数据验证
- 现在只有收到 "GO" 才会启动计时
- 收到其他数据会显示 "Wrong Signal"

---

## 🔧 测试步骤

### 步骤1: 检查Finish节点(终点)

**期望显示序列:**
```
1. 上电 → "System Init..."
2. 1秒后 → "NRF: OK" (重要!)
3. 1秒后 → "Ready! / Wait for GO... / Listening..."
```

**如果显示 "NRF: ERROR!":**
- ❌ SPI接线有问题
- ❌ NRF模块损坏
- ❌ CSN/CE引脚配置错误

**检查接线:**
```
NRF模块底座 → STM32 (Finish节点)
VCC  → 5V (你的底座有稳压,可以用5V)
GND  → GND
CE   → 检查main.h中定义的引脚
CSN  → 检查main.h中定义的引脚
SCK  → PA5 (SPI1_SCK)
MOSI → PA7 (SPI1_MOSI)
MISO → PA6 (SPI1_MISO)
```

---

### 步骤2: 检查Start节点(起点)

**需要USB转TTL连接UART1查看调试信息:**
```
Start节点 → USB转TTL → 电脑
PA9 (TX)  → RX
PA10 (RX) → TX (不用接)
GND       → GND
```

**打开串口助手(115200波特率),上电后应该看到:**
```
NRF:OK
```

**如果看到 "NRF:ERROR!":**
- 检查Start节点的NRF接线(同上)

---

### 步骤3: 测试发送和接收

**操作:**
1. 确保两个节点都显示 "NRF:OK"
2. 按下Start节点的按键(PB1)

**Start节点串口应该输出:**
```
Sending GO...
Sent 10 times
```

**Finish节点OLED应该显示:**
```
Got Signal!
D0=G D1=O
H:47 4F
```
然后2秒后显示:
```
RUNNING!!!
```

---

## 🐛 常见问题排查

### 问题1: Finish显示 "NRF: ERROR!"

**原因:** SPI通信失败

**解决方案:**
1. 检查SPI引脚接线(尤其是MISO,最容易接错)
2. 检查CSN和CE引脚定义
3. 用万用表测量NRF模块供电是否正常(3.3V或5V)
4. 尝试更换NRF模块

**查看CSN/CE引脚定义:**
打开 `Finish_Node/Core/Inc/main.h` 搜索 "NRF"

---

### 问题2: 两个节点都显示OK,但收不到数据

**可能原因:**
- 两个模块距离太远
- 天线方向不对
- 供电不稳定

**测试方法:**
1. 两个模块靠近到20cm以内
2. 确保两个天线平行
3. 检查Start节点串口是否输出 "Sending GO..."

---

### 问题3: Finish显示 "Got Signal!" 但数据不是 "GO"

**可能原因:**
- Start节点的TxData没有正确初始化
- 数据传输过程中损坏

**检查Start节点main.c:**
```c
uint8_t TxData[] = "GO"; 
```
确保这行代码存在且正确

---

### 问题4: 收到GO但不显示RUNNING

**可能原因:**
- 字符串比较失败
- RxData数组没有正确接收

**查看Finish节点显示的16进制值:**
- 应该是: `H:47 4F`
- 47是'G'的ASCII码
- 4F是'O'的ASCII码

如果不是这个值,说明发送端有问题

---

## 🔍 高级调试

### 查看NRF寄存器值

在 `nrf24l01p.c` 的 `nrf24l01p_check()` 函数中添加:

```c
uint8_t config = read_reg(RF24L01P_REG_CONFIG);
uint8_t status = read_reg(RF24L01P_REG_STATUS);
uint8_t rf_ch = read_reg(RF24L01P_REG_RF_CH);
// 通过OLED或串口输出这些值
```

**正常值:**
- CONFIG: 0x0B (RX模式) 或 0x0A (TX模式)
- RF_CH: 0x28 (频道40)
- STATUS: 0x0E

---

## 📋 确认清单

在报告问题前,请确认:

- [ ] Finish节点显示 "NRF: OK"
- [ ] Start节点串口输出 "NRF:OK"
- [ ] 两个模块距离在1米以内
- [ ] 供电稳定(LED不闪烁)
- [ ] SPI引脚接线正确(尤其MISO)
- [ ] 两个NRF模块都有天线
- [ ] 按键能正常触发(串口有输出)

---

## 💡 关于供电

你提到用了NRF底座,如果底座有3.3V稳压芯片(AMS1117-3.3):
- ✅ 可以接STM32的5V引脚
- ✅ 底座会自动转换成3.3V供电NRF

如果不确定底座是否有稳压:
- 用万用表测量NRF模块的VCC引脚
- 应该是3.3V左右
- 如果是5V,说明没有稳压,NRF可能已损坏

---

## 📞 下一步

编译并烧录程序后:

1. **先测试Finish节点** - 必须显示 "NRF: OK"
2. **再测试Start节点** - 串口必须输出 "NRF:OK"  
3. **测试通信** - 按键后观察Finish节点显示

如果还有问题,请告诉我:
- Finish节点显示什么?
- Start节点串口输出什么?
- 接收到的数据16进制值是多少?

这样我能快速定位问题!

