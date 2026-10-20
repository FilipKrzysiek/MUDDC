# Manometers ED72 expansion module

## Frame

| Byte |            Description             |
|:----:|:----------------------------------:|
|  0   |          time to set * 10          |
|  1   | 1st byte od manometer 1 (uint16_t) |
|  2   | 2nd byte od manometer 1 (uint16_t) |
|  3   | 1st byte od manometer 2 (uint16_t) |
|  4   | 2nd byte od manometer 2 (uint16_t) |
|  5   | 1st byte od manometer 3 (uint16_t) |
|  6   | 2nd byte od manometer 3 (uint16_t) |
|  7   |                V-LV                |
|  8   |                V-HV                |
|  9   |                C-TE                |
|  10  |          1st byte od C-LV          |
|  11  |          2nd byte od C-LV          |

## Pinout

|  Desc  | Pin description | PIN | &nbsp;&nbsp;&nbsp; | PIN | Pin description |   Desc   |
|:------:|:---------------:|:---:|--------------------|:---:|:---------------:|:--------:|
|  SDA   |  GP0, I2C0 SDA  |  1  |                    | 40  |      VBUS       |          |
|  SCL   |  GP1, I2C0 SCL  |  2  |                    | 39  |      VSYS       |          |
|        |     **GND**     |  3  |                    | 38  |     **GND**     |          |
| M1-IN1 |       GP2       |  4  |                    | 37  |     3V3_EN      |          |
| M2-IN1 |       GP3       |  5  |                    | 36  |    3V3(OUT)     |          |
| M1-IN2 |       GP4       |  6  |                    | 35  |                 |          |
| M2-IN2 |       GP5       |  7  |                    | 34  |      GP28       |          |
|        |     **GND**     |  8  |                    | 33  |     **GND**     |          |
| M1-IN3 |       GP6       |  9  |                    | 32  |      GP27       |          |
| M2-IN3 |       GP7       | 10  |                    | 31  |      GP26       |          |
| M1-IN4 |       GP8       | 11  |                    | 30  |       RUN       |          |
| M2-IN4 |       GP9       | 12  |                    | 29  |      GP22       |          |
|        |     **GND**     | 13  |                    | 28  |     **GND**     |          |
| M3-IN1 |      GP10       | 14  |                    | 27  |      GP21       |          |
|  V-LV  |      GP11       | 15  |                    | 26  |      GP20       |          |
| M3-IN2 |      GP12       | 16  |                    | 25  |      GP19       |          |
|  C-LV  |      GP13,      | 17  |                    | 24  |      GP18       | C-LV.Ref |
|        |     **GND**     | 18  |                    | 23  |     **GND**     |          |
| M3-IN3 |      GP14       | 19  |                    | 22  |      GP17       |   C-TE   |
|  V-HV  |      GP15       | 20  |                    | 21  |      GP16       |  M3-IN4  |

* M1 - manometer 1 for brakes 
* M2 - manometer 2 for main pipe
* M3 - manometer 3 for tank pressure
* V-LV - voltmeter low voltage
* C-TE - ammeter traction engine
* V-HV - voltmeter high voltage
* C-LV - ammeter low voltage
* C-LV.Ref - reference for low voltage ammeter