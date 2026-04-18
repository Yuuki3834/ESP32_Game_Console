# ESP32_Game_Console
使用VSCode + PlatformIO制作的纯文字游戏【纵横四海】
使用ESP32S3+2.8寸240*320触摸屏就可以游玩。
<img width="612" height="655" alt="image" src="https://github.com/user-attachments/assets/3fc3d667-a864-449d-b69d-e3123bfeebfc" />
我使用的是这一款开发板，价格70元左右。
游戏内容参考wap时代纯文字网页游戏纵横四海，包含跑商，打怪升级养成等内容，纯单机游玩，包含7大区30+城市，数百种技能，自行扩展地图，装备，技能都可以很容易做到。
空间还剩不少，天机了北京浮生记这个纯文字游戏和纯文字魔塔游戏。
如何直接使用固件？：
去 Espressif（乐鑫）官网下载：Flash Download Tools（一个免安装的绿色软件）。
打开软件，选择：
Chip Type:  ESP32-S3
WorkMode: Develop
软件打开后，在顶部的白框里选择你刚才提取的三个文件，并在右侧填入对应的烧录地址：
勾选第一行 👉 选择 bootloader.bin 👉 地址填 @ 0x0000 (注：ESP32-S3/C3 填 0x0)
勾选第二行 👉 选择 partitions.bin 👉 地址填 @ 0x8000
勾选第三行 👉 选择 firmware.bin 👉 地址填 @ 0x10000
在右下角选择端口（COM口）和波特率（选 115200 或 921600）。
点击 START，等待绿色进度条跑完，显示 FINISH 即可！
