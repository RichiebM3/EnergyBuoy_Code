# EnergyBuoy_Code
A environmental energy buoy that stores Solar and Kinetic Energy
# Dual Solar Power Buoy Monitoring System

[![Arduino](https://img.shields.io/badge/Arduino-R4%20WiFi-00979D?style=flat&logo=arduino)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Status](https://img.shields.io/badge/Status-Active-success.svg)]()

A robust, dual-redundant solar power monitoring system designed for marine buoy applications. This system provides real-time monitoring of solar charging performance, battery status, and energy generation through serial communication.

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [System Architecture](#system-architecture)
- [Installation](#installation)
- [Wiring Diagram](#wiring-diagram)
- [Configuration](#configuration)
- [Usage](#usage)
- [Monitoring Output](#monitoring-output)
- [Troubleshooting](#troubleshooting)
- [Maintenance](#maintenance)
- [Contributing](#contributing)
- [License](#license)

## 🌊 Overview

This project implements a dual solar power management system for autonomous marine buoys. The system features redundant power generation and storage, ensuring continuous operation even if one solar system fails. Real-time monitoring provides critical data on solar panel performance, battery health, and energy generation.

### Key Capabilities

- **Dual Redundancy**: Two independent solar charging systems for reliability
- **Real-time Monitoring**: Continuous tracking of voltage, current, and power metrics
- **Energy Tracking**: Cumulative energy generation measurement in Watt-hours
- **Battery Management**: Intelligent charging with voltage and current monitoring
- **Serial Interface**: Easy monitoring through Arduino Serial Monitor

## ✨ Features

- ⚡ Dual solar power management with DFRobot SP110 controllers
- 🔋 Independent 3.7V Li-ion battery monitoring for each system
- 📊 Real-time power and energy calculations
- 🔍 Battery level percentage estimation
- 📈 Cumulative energy tracking (Wh)
- 🔄 Charging status indicators (Charging/Discharging/Idle)
- 📡 I2C communication with configurable addresses
- 🖥️ Clean, formatted serial output for easy monitoring
- ⏱️ Configurable update intervals

## 🛠️ Hardware Requirements

### Main Components

| Component | Quantity | Specifications | Purpose |
|-----------|----------|----------------|---------|
| Arduino R4 WiFi | 1 | Main microcontroller | System controller and data processor |
| DFRobot Solar Power Manager (SP110) | 2 | I2C interface | Solar charge controllers |
| 5V Solar Panels | 2 | 5V output, weather-resistant | Primary power generation |
| Li-ion Batteries | 2 | 3.7V nominal, recommended 2000-5000mAh | Energy storage |
| I2C Pull-up Resistors | 2 | 4.7kΩ (if not included on modules) | I2C communication reliability |

### Additional Materials

- **Enclosure**: Waterproof IP67+ rated enclosure for electronics
- **Connectors**: Marine-grade waterproof connectors
- **Wiring**: 18-22 AWG stranded wire, marine grade
- **Mounting Hardware**: Stainless steel or marine-grade fasteners
- **Cable Glands**: Waterproof cable entry points
- **Heat Shrink Tubing**: For connection protection
- **Silica Gel Packets**: Moisture control inside enclosure

### Optional Components

- **SD Card Module**: For data logging
- **RTC Module**: Real-time clock for timestamping
- **Temperature Sensors**: Monitor ambient and battery temperature
- **Status LEDs**: Visual system status indicators
- **Buzzer**: Audio alerts for critical conditions

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    SOLAR BUOY SYSTEM                     │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  ┌──────────────┐              ┌──────────────┐        │
│  │ Solar Panel 1│              │ Solar Panel 2│        │
│  │    (5V)      │              │    (5V)      │        │
│  └──────┬───────┘              └──────┬───────┘        │
│         │                              │                │
│         ▼                              ▼                │
│  ┌──────────────┐              ┌──────────────┐        │
│  │   SP110 #1   │              │   SP110 #2   │        │
│  │  (Addr 0x10) │              │  (Addr 0x11) │        │
│  └──────┬───────┘              └──────┬───────┘        │
│         │                              │                │
│         │         ┌──────────┐         │                │
│         ├─────────┤ Arduino  ├─────────┤                │
│         │   I2C   │ R4 WiFi  │   I2C   │                │
│         │         └────┬─────┘         │                │
│         │              │               │                │
│         ▼              ▼               ▼                │
│  ┌──────────────┐  Serial      ┌──────────────┐        │
│  │  Battery 1   │  Monitor     │  Battery 2   │        │
│  │   (3.7V)     │              │   (3.7V)     │        │
│  └──────────────┘              └──────────────┘        │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

## 📥 Installation

### 1. Software Setup

#### Install Arduino IDE
Download and install the latest Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software)

#### Install Required Libraries

Open Arduino IDE and navigate to **Tools → Manage Libraries**, then search and install:

1. **DFRobot_SP110** - Solar Power Manager library
 ```
 Library Manager → Search "DFRobot SP110" → Install
 ```

2. **Wire** - I2C communication (usually pre-installed)

#### Install Arduino R4 WiFi Board Support

1. Go to **Tools → Board → Boards Manager**
2. Search for "Arduino UNO R4"
3. Install "Arduino UNO R4 Boards"

### 2. Hardware Assembly

#### Step 1: Prepare the SP110 Modules

1. Configure I2C addresses:
 - SP110 #1: Leave at default address (0x10)
 - SP110 #2: Change address to 0x11 (refer to SP110 documentation)

#### Step 2: Connect I2C Bus

Connect both SP110 modules to the Arduino R4 WiFi I2C bus:

- **SDA**: Connect to Arduino SDA pin
- **SCL**: Connect to Arduino SCL pin
- **GND**: Connect to Arduino GND
- **VCC**: Connect to Arduino 5V (if needed for logic)

#### Step 3: Connect Solar Panels

- Connect Solar Panel 1 to SP110 #1 solar input
- Connect Solar Panel 2 to SP110 #2 solar input

#### Step 4: Connect Batteries

- Connect Battery 1 to SP110 #1 battery output
- Connect Battery 2 to SP110 #2 battery output

⚠️ **Important**: Ensure correct polarity for all connections!

### 3. Upload Code

1. Open the provided `.ino` file in Arduino IDE
2. Select **Tools → Board → Arduino UNO R4 WiFi**
3. Select the correct **Port** under **Tools → Port**
4. Click **Upload** button
5. Wait for "Done uploading" message

## 🔌 Wiring Diagram

### I2C Connections

```
Arduino R4 WiFi          SP110 #1              SP110 #2
┌──────────────┐      ┌──────────┐         ┌──────────┐
│              │      │          │         │          │
│     SDA ─────┼──────┤ SDA      │    ┌────┤ SDA      │
│              │      │          │    │    │          │
│     SCL ─────┼──────┤ SCL      │    │    │ SCL      │
│              │      │          │    │    │          │
│     GND ─────┼──────┤ GND      ├────┴────┤ GND      │
│              │      │          │         │          │
│     5V  ─────┼──────┤ VCC      ├─────────┤ VCC      │
│              │      │          │         │          │
└──────────────┘      └──────────┘         └──────────┘
```

### Power Connections

```
Solar Panel 1 ──→ SP110 #1 (Solar Input) ──→ Battery 1 (3.7V)
Solar Panel 2 ──→ SP110 #2 (Solar Input) ──→ Battery 2 (3.7V)
```

## ⚙️ Configuration

### Battery Parameters

The default configuration is set for 3.7V Li-ion batteries:

```cpp
manager.setBatteryChargeVoltage(4200);    // 4.2V max charge (mV)
manager.setConstantChargeCurrent(1000);   // 1A charge current (mA)
manager.setConstantVoltage(4200);         // 4.2V constant voltage (mV)
```

### Monitoring Interval

Default update rate is 5 seconds. To change:

```cpp
delay(5000); // Change value in milliseconds
```

### I2C Addresses

If you need different addresses:

```cpp
DFRobot_SP110_I2C solarManager1(&Wire, 0x10);  // Change 0x10
DFRobot_SP110_I2C solarManager2(&Wire, 0x11);  // Change 0x11
```

## 🖥️ Usage

### Starting the System

1. Connect Arduino R4 WiFi to computer via USB
2. Open **Tools → Serial Monitor** in Arduino IDE
3. Set baud rate to **115200**
4. System will initialize and begin displaying data

### Serial Monitor Output

The system displays comprehensive monitoring data every 5 seconds:

```
Dual Solar Power Buoy Monitoring System
=======================================
Solar Manager 1 initialized successfully!
Solar Manager 2 initialized successfully!
Time since start: 5 seconds

========================================
SOLAR SYSTEM 1
========================================
SOLAR PANEL:
Voltage: 5.12 V
Current: 245.3 mA
Power: 1.25 W

BATTERY:
Voltage: 3.85 V
Current: 230.1 mA
Level: 70.8%

ENERGY STATISTICS:
Total Energy: 0.002 Wh
Status: Charging

========================================
SOLAR SYSTEM 2
========================================
[Similar output for System 2]

========================================
COMBINED SYSTEM STATUS
========================================
Total Power Output: 2.48 W
Total Energy Generated: 0.004 Wh
```

## 📊 Monitoring Output

### Metrics Explained

| Metric | Description | Units |
|--------|-------------|-------|
| Solar Voltage | Voltage from solar panel | Volts (V) |
| Solar Current | Current from solar panel | Milliamps (mA) |
| Solar Power | Instantaneous power generation | Watts (W) |
| Battery Voltage | Current battery voltage | Volts (V) |
| Battery Current | Charging/discharging current | Milliamps (mA) |
| Battery Level | Estimated charge percentage | Percent (%) |
| Total Energy | Cumulative energy generated | Watt-hours (Wh) |

### Status Indicators

- **Charging**: Battery is receiving charge from solar panel
- **Discharging**: Battery is supplying power to load
- **Idle**: No significant current flow

## 🔧 Troubleshooting

### Common Issues

#### Solar Manager Initialization Failed

**Symptoms**: "Solar Manager X failed to initialize" message loops

**Solutions**:
1. Check I2C wiring connections (SDA, SCL, GND)
2. Verify I2C addresses are correct and unique
3. Ensure SP110 modules are powered
4. Check for loose connections
5. Try using external pull-up resistors (4.7kΩ) on SDA and SCL lines

#### No Solar Voltage Reading

**Symptoms**: Solar voltage shows 0.00V

**Solutions**:
1. Check solar panel connections to SP110
2. Verify solar panel is receiving light
3. Test solar panel output with multimeter
4. Check for damaged solar panel

#### Battery Not Charging

**Symptoms**: Battery current shows 0 or negative value

**Solutions**:
1. Verify battery connections to SP110
2. Check if battery is already fully charged (4.2V)
3. Ensure solar panel is generating sufficient voltage
4. Verify charging is enabled in code
5. Check battery health with multimeter

#### Incorrect Battery Percentage

**Symptoms**: Battery level doesn't match expected value

**Solutions**:
1. Calibrate voltage thresholds in code for your specific battery
2. Allow battery to fully charge and discharge for calibration
3. Adjust `calculateBatteryLevel()` function parameters

### Serial Monitor Issues

**Problem**: No output in Serial Monitor

**Solutions**:
1. Verify baud rate is set to 115200
2. Check USB cable connection
3. Ensure correct COM port is selected
4. Try different USB cable or port

**Problem**: Garbled text in Serial Monitor

**Solutions**:
1. Set baud rate to 115200
2. Reset Arduino after opening Serial Monitor

## 🔄 Maintenance

### Regular Checks (Weekly)

- [ ] Inspect solar panels for dirt, debris, or damage
- [ ] Check all cable connections for corrosion
- [ ] Verify enclosure seals are intact
- [ ] Monitor battery voltage levels
- [ ] Check for water intrusion in enclosure

### Periodic Maintenance (Monthly)

- [ ] Clean solar panels with fresh water
- [ ] Inspect all wiring for wear or damage
- [ ] Check battery health and capacity
- [ ] Verify mounting hardware is secure
- [ ] Replace silica gel packets if saturated
- [ ] Download and backup monitoring data

### Annual Maintenance

- [ ] Replace batteries if capacity degraded
- [ ] Apply anti-corrosion treatment to connections
- [ ] Inspect and replace worn cable glands
- [ ] Test system under various light conditions
- [ ] Update firmware if new version available

## 🌐 Future Enhancements

Potential upgrades for this system:

- **WiFi Data Logging**: Upload monitoring data to cloud services
- **SD Card Logging**: Local data storage for offline analysis
- **Web Dashboard**: Real-time monitoring via web interface
- **GPS Integration**: Location tracking for mobile buoys
- **Temperature Monitoring**: Battery and ambient temperature sensors
- **Alert System**: Email/SMS notifications for critical conditions
- **Load Management**: Automatic load switching based on battery level
- **MPPT Optimization**: Maximum Power Point Tracking algorithms

## 📝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for:

- Bug fixes
- Feature enhancements
- Documentation improvements
- Hardware compatibility updates

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 👥 Authors

- Initial development for marine buoy monitoring application
- Community contributions welcome

## 🙏 Acknowledgments

- DFRobot for SP110 Solar Power Manager hardware and libraries
- Arduino community for development tools and support
- Marine renewable energy community for application insights

## 📞 Support

For questions, issues, or suggestions:

- Open an issue on GitHub
- Check the troubleshooting section
- Consult DFRobot SP110 documentation
- Arduino R4 WiFi documentation

---

**⚠️ Safety Warning**: This system involves electrical components and batteries. Always follow proper safety procedures when working with solar panels and lithium batteries. Ensure proper ventilation and use appropriate protective equipment. For marine applications, follow all relevant maritime safety regulations.

**🌊 Marine Deployment Note**: Ensure all components are properly waterproofed and rated for marine environments. Regular maintenance is critical for reliable operation in harsh marine conditions.

---

*Last Updated: November 2025*
*Version: 1.0.0*
