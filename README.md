博客连接：[https://blog.csdn.net/weixin_44139651/article/details/107395361](https://blog.csdn.net/weixin_44139651/article/details/107395361)

一种扫码和自动打印标签的装置能把产品信息采集下来，并且与不同的信息组合生成新的一维码或者二维码，贴到产品或者工作记录上。
扫码枪是直接淘宝采购的扫码枪，通过485接到STM32串口2，打印机也是淘宝采购精普小型打印机，可以打印各种便签，并且只要输入相应的指令和ASCII码内容就能自动转换成一维码或者二维码，非常的方便。打印机通过232接串口3，DMA方式接收数据(DMA1通道2)。LCD为TFTLCD_HX8352C。

![](_v_images/20200717113808290_5959.png)