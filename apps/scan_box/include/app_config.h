#ifndef APP_CONFIG_H
#define APP_CONFIG_H


#define __FLASH_SIZE__    (4 * 1024 * 1024)
#define __SDRAM_SIZE__    (2 * 1024 * 1024)     //NO_SDRAM 一定要配置成0
#define TCFG_SD0_ENABLE     1                  // SD卡选择


#define CONFIG_DEBUG_ENABLE                  //打印开关
#define CONFIG_RTC_ENABLE                    //RTC开关

#define CONFIG_RTOS_AND_MM_LIB_CODE_SECTION_IN_SDRAM

#ifdef CONFIG_NO_SDRAM_ENABLE
#undef __SDRAM_SIZE__
#define __SDRAM_SIZE__    (0 * 1024 * 1024)
#endif

#ifdef CONFIG_NET_ENABLE
#define CONFIG_WIFI_ENABLE  					/* 无线WIFI */
#ifdef CONFIG_NO_SDRAM_ENABLE
#define CONFIG_RF_TRIM_CODE_MOVABLE //把RF TRIM 的运行代码动态加载到ram运行(节省4K ram内存), 防止RF TRIM 期间500ms大电流访问flash造成flash挂掉持续大电流
#else
#define CONFIG_RF_TRIM_CODE_AT_RAM //把RF TRIM 的运行代码定死到ram运行(浪费4K ram内存,否则若动态加载到sdram需清cache), 防止RF TRIM 期间500ms大电流访问flash造成flash挂掉持续大电流
#endif
// #define CONFIG_IPERF_ENABLE  				// iperf测试
// #define CONFIG_MASS_PRODUCTION_ENABLE //启用产测模式
#endif

//#define CONFIG_OSD_ENABLE			/* 视频OSD时间戳开关 */
#define CONFIG_VIDEO_REC_PPBUF_MODE	/*视频使用乒乓BUF模式(图传<=20帧),关闭则用lbuf模式(图传>20帧和写卡录像),缓冲区大小配置video_buf_config.h*/
//#define CONFIG_VIDEO_SPEC_DOUBLE_REC_MODE	/* 视频支持双路莫模式(一路实时流、一路录SD卡)*/

#ifdef CONFIG_MASS_PRODUCTION_ENABLE
#define ROUTER_SSID     "test"    //量产模式的路由器名称
#define ROUTER_PWD      "12345678"  //量产模式的路由器密码
//#define CONFIG_PRODUCTION_IO_PORT			IO_PORTB_01 //配置进入量产莫模式的IO
//#define CONFIG_PRODUCTION_IO_STATE		0 			//配置进入量产莫模式的IO状态：0低电平，1高电平
#endif

#ifndef TCFG_SD0_ENABLE
#define TCFG_SD0_ENABLE		0
#endif
#ifndef TCFG_SD1_ENABLE
#define TCFG_SD1_ENABLE		0
#endif

#define RTOS_STACK_CHECK_ENABLE //是否启用定时检查任务栈
//#define MEM_LEAK_CHECK_ENABLE	//是否启用内存泄漏检查(需要包含mem_leak_test.h头文件)


#if TCFG_SD0_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd0"
#define SDX_DEV					"sd0"
#endif

#if TCFG_SD1_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd1"
#define SDX_DEV					"sd1"
#endif

#ifndef CONFIG_STORAGE_PATH
#define CONFIG_STORAGE_PATH		"storage/sdx"
#define SDX_DEV					"sdx"
#endif

#define CONFIG_REC_DIR_0       "DCIM/1/"
#define CONFIG_REC_PATH_0       CONFIG_STORAGE_PATH"/C/"CONFIG_REC_DIR_0
#define CONFIG_ROOT_PATH     	CONFIG_STORAGE_PATH"/C/"



//*********************************************************************************//
//                             编码图片分辨率                                      //
//*********************************************************************************//
//#define CONFIG_VIDEO_720P
#ifdef CONFIG_VIDEO_720P
#define CONFIG_VIDEO_IMAGE_W    1280
#define CONFIG_VIDEO_IMAGE_H    720
#else
#define CONFIG_VIDEO_IMAGE_W    640
#define CONFIG_VIDEO_IMAGE_H    480
#endif

//*********************************************************************************//
//                             视频流相关配置                                      //
//*********************************************************************************//
#define VIDEO_REC_AUDIO_SAMPLE_RATE		0    //视频流的音频采样率,注意：硬件没MIC则为0
#define VIDEO_REC_FPS 					20   //录像SD卡视频帧率设置,0为默认

#define CONFIG_USR_VIDEO_ENABLE		//用户VIDEO使能
#ifdef CONFIG_USR_VIDEO_ENABLE
#define CONFIG_USR_PATH 	"192.168.1.1:8000" //用户路径，可随意设置，video_rt_usr.c的init函数看进行读取
#endif



//*********************************************************************************//
//                        升级：单双备份和名称路径配置                             //
//*********************************************************************************//
#define CONFIG_DOUBLE_BANK_ENABLE			1//双备份方式升级
#define CONFIG_UPGRADE_FILE_NAME			"update.ufw"
#define CONFIG_UPGRADE_PATH       	        CONFIG_ROOT_PATH\
											CONFIG_UPGRADE_FILE_NAME	//备份方式升级


//*********************************************************************************//
//                        SD 配置（暂只支持打开一个SD外设）                        //
//*********************************************************************************//
//SD0 	cmd,  clk,  data0, data1, data2, data3
//A     PB6   PB7   PB5    PB5    PB3    PB2
//B     PA7   PA8   PA9    PA10   PA5    PA6
//C     PH1   PH2   PH0    PH3    PH4    PH5
//D     PC9   PC10  PC8    PC7    PC6    PC5

//SD1 	cmd,  clk,  data0, data1, data2, data3
//A     PH6   PH7   PH5    PH4    PH3    PH2
//B     PC0   PC1   PC2    PC3    PC4    PC5

#if (TCFG_SD0_ENABLE || TCFG_SD1_ENABLE)
#define TCFG_SD_PORTS                      'D'			//SD0/SD1的ABCD组
#define TCFG_SD_DAT_WIDTH                  1			//1:单线模式, 4:四线模式
#define TCFG_SD_DET_MODE                   SD_CLK_DECT	//检测模式
#define TCFG_SD_DET_IO                     IO_PORTC_07	//SD_DET_MODE为SD_IO_DECT时有效
#define TCFG_SD_DET_IO_LEVEL               0			//IO检卡上线的电平(0/1),SD_DET_MODE为SD_IO_DECT时有效
#define TCFG_SD_CLK                        24000000		//SD时钟
#endif


//*********************************************************************************//
//                                  USB配置                                        //
//*********************************************************************************//
#define TCFG_PC_ENABLE                      1
#define USB_PC_NO_APP_MODE                  2
#define USB_MALLOC_ENABLE                   1
#define USB_DEVICE_CLASS_CONFIG             (HID_CLASS | UVC_CLASS)
#define TCFG_UDISK_ENABLE                   0
#define TCFG_HOST_AUDIO_ENABLE              0

#include "usb_std_class_def.h"
#include "usb_common_def.h"

#if TCFG_USB_SLAVE_HID_ENABLE
#define USB_HID_KEYBOARD_ENABLE             1
#define USB_HID_POS_ENABLE                  0

#if (USB_HID_KEYBOARD_ENABLE + USB_HID_POS_ENABLE > 1)
#error "they can not enable at the same time,just select one!!!"
#endif
#endif

//*********************************************************************************//
//                                  低功耗配置                                     //
//*********************************************************************************//
#define CONFIG_LOW_POWER_ENABLE            			//低功耗睡眠使能
//#define CONFIG_INTERNAL_VDDIO_POWER_SUPPLY_ENABLE //实际PCB设计如果采用内部VDDIO供电，必须定义该宏
#define TCFG_LOWPOWER_BTOSC_DISABLE			0
#ifdef CONFIG_LOW_POWER_ENABLE
#define TCFG_LOWPOWER_LOWPOWER_SEL			0
#else
#define TCFG_LOWPOWER_LOWPOWER_SEL			0
#endif
#ifdef CONFIG_INTERNAL_VDDIO_POWER_SUPPLY_ENABLE
#ifdef CONFIG_NO_SDRAM_ENABLE
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_33V       //强VDDIO电压档位
#else
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_35V       //强VDDIO电压档位，采用内部VDDIO供电，带sdram的封装实际工作电压至少要满足3.3V
#endif
#else
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_32V       //强VDDIO电压档位，不要高于外部DCDC的电压
#endif
#define TCFG_LOWPOWER_VDDIOW_LEVEL			VDDIOW_VOL_21V
#define VDC14_VOL_SEL_LEVEL					VDC14_VOL_SEL_140V
#define SYSVDD_VOL_SEL_LEVEL				SYSVDD_VOL_SEL_126V



//*********************************************************************************//
//                                  BT_BLE配置                                     //
//*********************************************************************************//
#ifdef CONFIG_BT_ENABLE

#define CONFIG_BT_RX_BUFF_SIZE  0
#define CONFIG_BT_TX_BUFF_SIZE  0

#define TCFG_USER_BT_CLASSIC_ENABLE               0     //经典蓝牙功能
#define TCFG_USER_BLE_ENABLE                      1     //BLE功能使能

#if TCFG_USER_BLE_ENABLE

#define TCFG_BLE_SECURITY_EN                      0     //配对加密使能

#define TRANS_DATA_EN                             0     //从机 传输数据
#define BT_NET_CENTRAL_EN                         0     //主机 client角色
#define BT_NET_HID_EN                             1     //从机 hid
#define TRANS_MULTI_BLE_EN                        0     //多机通讯
#define APP_NONCONN_24G				  			  0		//2.4g加密通讯

#if (TRANS_MULTI_BLE_EN + BT_NET_CFG_EN + BT_NET_HID_EN + TRANS_DATA_EN + APP_NONCONN_24G > 1)
#error "they can not enable at the same time,just select one!!!"
#endif
#endif

#if TRANS_MULTI_BLE_EN
#define TRANS_MULTI_BLE_SLAVE_NUMS                2
#define TRANS_MULTI_BLE_MASTER_NUMS               2
#endif

#endif


#if !defined CONFIG_DEBUG_ENABLE || defined CONFIG_LIB_DEBUG_DISABLE
#define LIB_DEBUG    0
#else
#define LIB_DEBUG    1
#endif

#define CONFIG_DEBUG_LIB(x)         (x & LIB_DEBUG)

#include "video_buf_config.h"

#endif

