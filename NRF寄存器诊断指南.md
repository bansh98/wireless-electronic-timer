# NRF24L01 寄存器诊断指南

## 📋 现在测试步骤

### 1️⃣ Start节点 (起点) - 串口输出

重新编译并烧录 `Start_Node`

**打开串口助手 (9600波特率)**

上电后应该看到:
```
Testing NRF...
CONFIG=0x??
STATUS=0x??
SETUP_AW=0x??
RF_CH=0x??
NRF: OK!
```
或
```
NRF: ERROR!
->NRF not connected!
```

---

### 2️⃣ Finish节点 (终点) - OLED显示

重新编译并烧录 `Finish_Node`

**上电后OLED显示:**
```
System Init...
  ↓
Reading NRF...
  ↓
CFG:?? ST:??
AW:??
  ↓
NRF: OK
```
或显示错误信息

---

## 🔍 寄存器值判断表

### ✅ 正常情况 (NRF工作正常)

| 寄存器 | 地址 | 正常值 | 说明 |
|--------|------|--------|------|
| CONFIG | 0x00 | 0x08-0x0F | 配置寄存器 |
| STATUS | 0x07 | 0x0E | 状态寄存器 |
| SETUP_AW | 0x03 | 0x03 | 地址宽度 (5字节) |
| RF_CH | 0x05 | 0x02-0x7F | 射频频道 |

**如果看到这些值,说明SPI通信正常!**

---

### ❌ 异常情况诊断

#### 情况1: 所有寄存器都是 `0xFF`
```
CONFIG=0xFF
STATUS=0xFF
SETUP_AW=0xFF
RF_CH=0xFF
```

**原因:** NRF模块没有连接或损坏

**解决方案:**
1. 检查NRF模块是否插好
2. 检查7根线连接 (VCC/GND/CE/CSN/SCK/MOSI/MISO)
3. 用万用表测量NRF的VCC引脚,应该是3.3V
4. 尝试更换NRF模块

---

#### 情况2: 所有寄存器都是 `0x00`
```
CONFIG=0x00
STATUS=0x00
SETUP_AW=0x00
RF_CH=0x00
```

**原因:** SPI通信有问题

**检查:**
1. ❌ MOSI/MISO 接反了 (最常见!)
2. ❌ SCK 没连接
3. ❌ CSN 引脚配置错误
4. ❌ SPI时钟配置有问题

**测试方法:**
- 交换MOSI和MISO的线,重新测试

---

#### 情况3: 能读到值,但检测失败

比如:
```
CONFIG=0x08
STATUS=0x0E
SETUP_AW=0x01  ← 不是0x03
```

**原因:** NRF模块工作,但配置异常

**解决方案:**
- 可能是NRF模块型号不同
- 尝试修改 `nrf24l01p_check()` 函数的判断条件

---

## 🔧 常见接线错误对照表

### Start节点和Finish节点接线相同:

| NRF引脚 | STM32引脚 | 功能 | 常见错误 |
|---------|-----------|------|----------|
| VCC | 5V | 供电 | ❌ 接3.3V可能供电不足 |
| GND | GND | 地 | ❌ 没接地 |
| CE | PB0 | 使能 | ❌ 接错引脚 |
| CSN | PA4 | 片选 | ❌ 接错引脚 |
| SCK | PA5 | SPI时钟 | ❌ 松动 |
| MOSI | PA7 | 主出从入 | ❌ 和MISO接反了! |
| MISO | PA6 | 主入从出 | ❌ 和MOSI接反了! |

---

## 💡 快速排查步骤

### 步骤1: 确认供电
用万用表测量NRF模块的VCC和GND引脚:
- 应该有3.3V左右的电压
- 如果是5V,NRF可能已损坏
- 如果是0V,说明供电没接好

### 步骤2: 确认SPI通信
看串口/OLED输出的寄存器值:
- 如果全是0xFF → NRF没连接
- 如果全是0x00 → SPI接线错误
- 如果是正常值 → 尝试通信测试

### 步骤3: 试试交换MOSI/MISO
这是最常见的错误!
- MOSI (PA7) 和 MISO (PA6) 线交换
- 重新上电测试

### 步骤4: 更换NRF模块
如果以上都不行:
- 可能NRF模块损坏 (用5V直供会烧坏)
- 换一个新的NRF24L01模块试试

---

## 📊 成功案例参考

**正常工作时的输出:**

Start节点串口:
```
Testing NRF...
CONFIG=0x08
STATUS=0x0E
SETUP_AW=0x03
RF_CH=0x02
NRF: OK!
```

Finish节点OLED:
```
CFG:08 ST:0E
AW:03
  ↓
NRF: OK
  ↓
Ready!
Wait for GO...
```

---

## 🚨 重要提示

1. **两个节点都要显示 "NRF: OK"** 才能通信
2. **寄存器值不完全相同没关系**,只要不是全0xFF或全0x00
3. **MOSI/MISO是最容易接错的**,先检查这个!

---

## 📞 测试后告诉我

重新烧录后,告诉我:

**Start节点串口输出:**
- CONFIG=?
- STATUS=?
- SETUP_AW=?
- 显示OK还是ERROR?

**Finish节点OLED显示:**
- CFG:?? ST:??
- 显示OK还是ERROR?

根据这些信息,我能准确判断问题!

