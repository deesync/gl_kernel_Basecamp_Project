# GL Kernel BaseCamp Project

```mermaid
---
title: Project Design
---
classDiagram
    
    class Userspace {
        Monitoring
        Configuration
    }

    class `Business Logic Driver` {
        Manage Sensor Data
        Control Visual Data
    }

    class `Display Driver` {
        Interface for BLD
        Data for visualization
    }

    class `MPU Driver` {
        Interface for BLD
        Selected Sensor Data
    }

    class `Display Device` {
        Visualization
    }

    class `MPU Sensor Device` {
        Raw Data
    }

    Userspace <..> `Business Logic Driver` : sysfs
    %%Userspace <.. `Business Logic Driver` : io_ctl

    `Business Logic Driver` ..> `MPU Driver`
    `Business Logic Driver` <.. `Display Driver`

    `MPU Driver` --o `MPU Sensor Device` : Kernel I2C Driver
    `Display Driver` <|-- `Display Device` : Framebuffer
```

```mermaid
---
title: Project Design
---
flowchart
    mpu{{MPU6050 Sensor}}
    display{{Display}}
    
    subgraph uspace [user space]
      control(Control App)
    end

    subgraph modules [kernel]
      bl_drv{{Busines Logic Driver}}
      mpu_driver[MPU6050 Driver]
      display_driver[Display Driver]
      
      i2c_driver[I2C Bus Driver]
      fb[Framebuffer]
    end
    
    bl_drv -.-> mpu_driver -.-> i2c_driver --> mpu
    display --> fb -.-> display_driver -.-> bl_drv
    bl_drv -.-> control
    display_driver -.-> mpu_driver
```

