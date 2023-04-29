# GL Kernel BaseCamp Project

### Project Design

```mermaid
---
title: Project Design Diagram
---
flowchart
    subgraph hardware [Hardware]
        sensor{{MPU6050 Sensor}}
        display{{Display}}
    end

    subgraph modules [Kernel Space]
        blm[Busines Logic Module]
        sensor_driver[Inclinometer Driver]
        display_driver[Display Driver]
        
        subgraph kernel [Kernel Subsystems]
            i2c[[MPU6050 I2C Driver]]
            fb[[Framebuffer]]
        end
    end

    subgraph uspace [User Space]
        control([Control/Config Application])
    end


control -. sysfs / ioctl .-> blm

blm -.-> sensor_driver -.-> i2c --> sensor
blm -.-> display_driver -.-> fb --> display
```

```mermaid
---
title: Project Design Diagram
---
classDiagram
   
    class `User Space` {
        Monitoring
        Configuration
    }

    class `Business Logic Module` {
        Manage Sensor Data
        Control Visual Data
    }
    %%link `User Space` "https://www.github.com/deesync"

    class `Display Driver` {
        Interface for BLM
        Data for visualization
    }

    class `Inclinometer Driver` {
        Interface for BLM
        Selected Sensor Data
    }

    class `Display Device` {
        Visualization
    }

    class `MPU6050 Sensor Device` {
        Raw Data
    }

    `User Space` ..> `Business Logic Module` : sysfs

    `Business Logic Module` ..> `Inclinometer Driver`
    `Business Logic Module` ..> `Display Driver`

    `Inclinometer Driver` --> `MPU6050 Sensor Device` : I2C/SMBus Subsystem
    `Display Driver` --> `Display Device` : Framebuffer
```

### Inclinometer Driver Interface

Data to be exported:

```c
struct inclinometer_data {
	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;
};

int16_t inclinometer_get_gyro_x();
int16_t inclinometer_get_gyro_y();
int16_t inclinometer_get_gyro_z();
```

