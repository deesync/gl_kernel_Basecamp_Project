
## Project Design

This project uses three Linux kernel modules:

- **Sensor driver module** for reading information from the sensor
- **Display driver module** for displaying information on the screen
- And also a **Business logic module** for the linking these two modules and interaction with user space

_Note: modules should have as few dependencies as possible and interact with userspace directly. But the goal of this project is to get acquainted with writing Linux kernel driver modules, so this approach was chosen intentionally._

```mermaid
---
title: Project Design Diagram
---
flowchart
    subgraph hardware [Hardware]
        sensor{{MPU6050 Sensor Device}}
        display{{SSD1306 OLED Display}}
    end

    subgraph modules [Kernel Space]
        blm[Busines Logic Module]
        sensor_driver[Sensor Driver Module]
        display_driver[Display Driver Module]
        
        subgraph kernel [Kernel HAL]
            i2c-1[[I2C/SMBus Subsystem]]
            i2c-2[[I2C/SMBus Subsystem]]
        end
    end

    subgraph uspace [User Space]
        control([Control/Config Application])
    end


control -. sysfs .-> blm

blm -.-> sensor_driver -.-> i2c-1 --> sensor
blm -.-> display_driver -.-> i2c-2 --> display
```

```mermaid
---
title: Module Roles Diagram
---
classDiagram
   
    class `User Space` {
        Monitoring
        Configuration
    }

    class `Inclinometer (Business Logic Module)` {
        Connect all together
        Make sysfs interface to user space
        Register IRQ handlers
        Read sensor calibration information
        Receive and manage sensor data
        Select the information to display according to the selected mode
        Schedule the work loop
    }

    class `Display Driver` {
        Clear Display
        Print Custom Text Message on Display
    }

    class `Sensor Driver` {
        Poll Raw Accelerometer/Gyroscope Data
        Poll Raw Temperature Data
    }

    class `SSD1306 OLED Display` {
        Graphic Display Data
    }

    class `MPU6050 Sensor Device` {
        Raw Sensor Data
    }

    `User Space` ..> `Inclinometer (Business Logic Module)` : sysfs

    `Inclinometer (Business Logic Module)` ..> `Sensor Driver`
    `Inclinometer (Business Logic Module)` ..> `Display Driver`

    `Sensor Driver` --> `MPU6050 Sensor Device` : I2C/SMBus
    `Display Driver` --> `SSD1306 OLED Display` : I2C/SMBus
```
