#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "board_config.h"

#ifdef CONFIG_NO_SDRAM_ENABLE					/* 封装不带sdram就打开该宏 */
#undef __SDRAM_SIZE__
#define __SDRAM_SIZE__    (0 * 1024 * 1024)
#endif

#define CONFIG_DATABASE_2_FLASH                 /* 系统配置存flash */
#define CONFIG_DEBUG_ENABLE                     /* 打印开关 */
#define CONFIG_RTC_ENABLE					/*RTC使能*/
//#define CONFIG_SYS_VDD_CLOCK_ENABLE			//系统可使用动态电源、时钟配置
//#define CONFIG_IPMASK_ENABLE				//系统使用不可屏蔽中断

#define CONFIG_PCM_DEC_ENABLE
#define CONFIG_PCM_ENC_ENABLE
// #define CONFIG_AEC_ENC_ENABLE
// #define CONFIG_DNS_ENC_ENABLE

#ifdef CONFIG_NET_ENABLE
#define CONFIG_CTP_ENABLE
#define CONFIG_WIFI_ENABLE  					/* 无线WIFI */
#ifdef CONFIG_NO_SDRAM_ENABLE
#define CONFIG_RF_TRIM_CODE_MOVABLE //把RF TRIM 的运行代码动态加载到ram运行(节省4K ram内存), 防止RF TRIM 期间500ms大电流访问flash造成flash挂掉持续大电流
#else
#define CONFIG_RF_TRIM_CODE_AT_RAM //把RF TRIM 的运行代码定死到ram运行(浪费4K ram内存,否则若动态加载到sdram需清cache), 防止RF TRIM 期间500ms大电流访问flash造成flash挂掉持续大电流
#endif
// #define CONFIG_IPERF_ENABLE  				// iperf测试
#endif

//#define CONFIG_OSD_ENABLE			/* 视频OSD时间戳开关 */
#define CONFIG_VIDEO_REC_PPBUF_MODE	/*视频使用乒乓BUF模式(图传<=20帧),关闭则用lbuf模式(图传>20帧和写卡录像),缓冲区大小配置video_buf_config.h*/
//#define CONFIG_VIDEO_SPEC_DOUBLE_REC_MODE	/* 视频支持双路莫模式(一路实时流、一路录SD卡)*/

#ifndef TCFG_SD0_ENABLE
#define TCFG_SD0_ENABLE		0
#endif
#ifndef TCFG_SD1_ENABLE
#define TCFG_SD1_ENABLE		0
#endif

#define RTOS_STACK_CHECK_ENABLE //是否启用定时检查任务栈
//#define MEM_LEAK_CHECK_ENABLE	//是否启用内存泄漏检查(需要包含mem_leak_test.h头文件)


/*升级版本号*/
/*
版本    说明
主版本  用于重大升级，例如整体架构，接口都发生重大改变，导致版本不兼容，此时应该升级主版本
次版本  用于功能增加及升级，整体架构不变，接口完全兼容。次版本必须往下兼容，即1.5.x 必须兼容1.4.x、1.3.x、1.2.x、1.1.x、1.0.x
修订版本    用于内部发布及BUG修复
*/

#define OTA_MAJOR   1
#define OTA_MINOR   3
#define OTA_PATCH   0

#define CONFIG_REC_DIR_0       "DCIM/1/"
#define CONFIG_REC_DIR_1       "DCIM/2/"
#ifndef CONFIG_VIDEO1_ENABLE
#define CONFIG_REC_DIR_2       "DCIM/2/"
#else
#define CONFIG_REC_DIR_2       "DCIM/3/"
#endif

#if TCFG_SD0_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd0"
#define SDX_DEV					"sd0"
#endif

#if TCFG_SD1_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd1"
#define SDX_DEV					"sd1"
#endif

#ifndef CONFIG_STORAGE_PATH
#define CONFIG_STORAGE_PATH		"no_sd_card" //不使用SD定义对应别的路径，防止编译出错
#define SDX_DEV					"no_sd"
#endif

#define CONFIG_ROOT_PATH     	CONFIG_STORAGE_PATH"/C/"
#define CONFIG_REC_PATH_0       CONFIG_STORAGE_PATH"/C/"CONFIG_REC_DIR_0
#define CONFIG_REC_PATH_1       CONFIG_STORAGE_PATH"/C/"CONFIG_REC_DIR_1
#define CONFIG_REC_PATH_2       CONFIG_STORAGE_PATH"/C/"CONFIG_REC_DIR_2

#define CONFIG_UDISK_STORAGE_PATH	"storage/udisk0"
#define CONFIG_UDISK_ROOT_PATH     	CONFIG_UDISK_STORAGE_PATH"/C/"

#ifdef CONFIG_EMR_DIR_ENABLE
#define CONFIG_EMR_REC_DIR         "EMR/"
#define CONFIG_EMR_REC_DIR_0       "DCIM/1/"CONFIG_EMR_REC_DIR
#define CONFIG_EMR_REC_DIR_1       "DCIM/2/"CONFIG_EMR_REC_DIR
#ifndef CONFIG_VIDEO1_ENABLE
#define CONFIG_EMR_REC_DIR_2       "DCIM/2/"CONFIG_EMR_REC_DIR
#else
#define CONFIG_EMR_REC_DIR_2       "DCIM/3/"CONFIG_EMR_REC_DIR
#endif
#define CONFIG_EMR_REC_PATH_0   CONFIG_STORAGE_PATH"/C/"CONFIG_EMR_REC_DIR_0
#define CONFIG_EMR_REC_PATH_1   CONFIG_STORAGE_PATH"/C/"CONFIG_EMR_REC_DIR_1
#define CONFIG_EMR_REC_PATH_2   CONFIG_STORAGE_PATH"/C/"CONFIG_EMR_REC_DIR_2
#endif


#define CAMERA0_CAP_PATH        CONFIG_REC_PATH_0
#define CAMERA1_CAP_PATH        CONFIG_REC_PATH_1
#define CAMERA2_CAP_PATH        CONFIG_REC_PATH_2

#define CONFIG_DEC_ROOT_PATH 	CONFIG_ROOT_PATH"DCIM/"
#define CONFIG_DEC_PATH_1 	    CONFIG_REC_PATH_0
#define CONFIG_DEC_PATH_2 	    CONFIG_REC_PATH_1
#define CONFIG_DEC_PATH_3 	    CONFIG_REC_PATH_2

#define CONFIG_MUSIC_PATH      CONFIG_ROOT_PATH"MUSIC/"

#define CONFIG_MUSIC_PATH_SD        CONFIG_ROOT_PATH
#define CONFIG_MUSIC_PATH_UDISK     CONFIG_UDISK_ROOT_PATH

#define MAX_FILE_NAME_LEN       64
#define FILE_SHOW_NUM           12  /* 一页显示文件数 */

#if (defined CONFIG_VIDEO1_ENABLE) && (defined CONFIG_VIDEO2_ENABLE)
#define THREE_WAY_ENABLE		1
#define CONFIG_VIDEO_REC_NUM    3
#else
#define THREE_WAY_ENABLE		0
#define CONFIG_VIDEO_REC_NUM    3
#endif

#define VM_MAGIC_VERSION 0X129
#define VM_STORAGE_MODE  0 //2:双64K,1:64K+4K,0:双4K
#define FLASH_END_RESERVE_SPACE  ((VM_STORAGE_MODE==0?8*1024:(VM_STORAGE_MODE==1?68*1024:128*1024))+4*1024) //FLASH末尾预留空间不纳入文件系统管理,VM区域加上其他用到区域例如4K profile, 4k mac

//*********************************************************************************//
//                            AUDIO_ADC应用的通道配置                              //
//*********************************************************************************//
#define AUDIO_ENC_SAMPLE_SOURCE_MIC         0
#define AUDIO_ENC_SAMPLE_SOURCE_PLNK0       1
#define AUDIO_ENC_SAMPLE_SOURCE_PLNK1       2
#define AUDIO_ENC_SAMPLE_SOURCE_IIS0        3
#define AUDIO_ENC_SAMPLE_SOURCE_IIS1        4
#define AUDIO_ENC_SAMPLE_SOURCE_LINEIN      5

//#define CONFIG_ASR_ALGORITHM ROOBO_ALGORITHM
#ifdef CONFIG_ASR_ALGORITHM
#define CONFIG_ALL_ADC_CHANNEL_OPEN_ENABLE          //四路ADC硬件全开
#endif

#ifdef CONFIG_ALL_ADC_CHANNEL_OPEN_ENABLE
#define CONFIG_AISP_DIFFER_MIC_REPLACE_LINEIN       //用差分mic代替aec回采
#define CONFIG_ASR_CLOUD_ADC_CHANNEL        1		//云端识别mic通道
#define CONFIG_VOICE_NET_CFG_ADC_CHANNEL    1		//声波配网mic通道
#define CONFIG_AISP_MIC0_ADC_CHANNEL        1		//本地唤醒左mic通道
#define CONFIG_AISP_MIC_ADC_GAIN            80		//本地唤醒mic增益
#define CONFIG_AISP_LINEIN_ADC_CHANNEL      3		//本地唤醒LINEIN回采DAC通道
#define CONFIG_AISP_MIC1_ADC_CHANNEL        0		//本地唤醒右mic通道
#define CONFIG_REVERB_ADC_CHANNEL           1		//混响mic通道
#define CONFIG_PHONE_CALL_ADC_CHANNEL       1		//通话mic通道
#define CONFIG_UAC_MIC_ADC_CHANNEL          1		//UAC mic通道
#define CONFIG_AISP_LINEIN_ADC_GAIN         10		//本地唤醒LINEIN增益
#endif


//*********************************************************************************//
//                                  USB配置                                        //
//*********************************************************************************//
//1.主机UVC模式
/* #define CONFIG_UVC_VIDEO2_ENABLE */
#ifdef CONFIG_UVC_VIDEO2_ENABLE /*打开主机模式*/
#define TCFG_HOST_UVC_ENABLE               	1   //打开USB主机UVC
#define TCFG_LCD_USB_SHOW_COLLEAGUE         1   //UVC显示屏 和 wifi出图同时显示
#endif

//2.主机读取U盘功能
#define TCFG_UDISK_ENABLE     				0	//打开读写U盘功能

//3.从机连接PC模式下：电脑盘符功能、UVC功能
#define TCFG_PC_ENABLE						0    //打开usb从机功能(打开连接PC电脑模式)
#if TCFG_PC_ENABLE
#define USB_PC_NO_APP_MODE					2   //不用APP状态机接收消息
#define USB_MALLOC_ENABLE                   1   //buff采用申请方式
//#define USB_DEVICE_CLASS_CONFIG (UVC_CLASS)     //从机UVC电脑查看功能选择
#define USB_DEVICE_CLASS_CONFIG (MASSSTORAGE_CLASS) //从机电脑盘符功能选择
#if ((USB_DEVICE_CLASS_CONFIG & UVC_CLASS) == UVC_CLASS)
#define CONFIG_USR_VIDEO_ENABLE
#endif
#endif

//4.主机无线网卡LTE
#ifdef CONFIG_LTE_PHY_ENABLE
#define TCFG_HOST_WIRELESS_ENABLE           1
//#define TCFG_ADB_ENABLE						1//手机USB_4G网络共享使能
#endif

#if (TCFG_HOST_UVC_ENABLE && TCFG_USB_SLAVE_ENABLE)
#error "no support USB HOST_MODE and USB SLAVE_MODE"
#endif

#include "usb_std_class_def.h"
#include "usb_common_def.h"


//*********************************************************************************//
//                                  FCC测试相关配置                                //
//*********************************************************************************//
//#define RF_FCC_TEST_ENABLE//使能RF_FCC测试，详细配置见"apps/common/rf_fcc_tool/include/rf_fcc_main.h"

//*********************************************************************************//
//                                PRODUCT测试相关配置                              //
//*********************************************************************************//
//#define PRODUCT_TEST_ENABLE//使能厂测模式，详细配置见"apps/common/product_test_tool/includes/product_main.h"
#ifdef PRODUCT_TEST_ENABLE
//#define		PRODUCT_NET_CLIENT_ENABLE
#define 	CONFIG_USR_VIDEO_ENABLE
#endif

//*********************************************************************************//
//                                  网络配置                                       //
//*********************************************************************************//
// #define CONFIG_RTSP_TEST_ENABLE

// #define CONFIG_MASS_PRODUCTION_ENABLE //启用产测模式
// #define CONFIG_AUTO_PRODUCTION_ENABLE	//启用自动化产测模式

#ifdef CONFIG_MASS_PRODUCTION_ENABLE
//#define CONFIG_PRODUCTION_IO_PORT			IO_PORTB_01 //配置进入量产莫模式的IO
//#define CONFIG_PRODUCTION_IO_STATE		0 			//配置进入量产莫模式的IO状态：0低电平，1高电平
#endif

#define ACCESS_NUM 				1
//#define CONFIG_ENABLE_VLIST		//支持SD卡录像下的APP文件缩略图、文件列表检索

//AP模式的热点名称和密码
#define AP_WIFI_CAM_PREFIX    "wifi_camera_wl8x_"
#define AP_WIFI_CAM_WIFI_PWD  "12345678"

//STA模式的路由器名称和密码
//注意：STA模式需要换:wl_wifi_ap.a换wl_wifi_sta.a; 或wl_wifi_ap_sfc.a换wl_wifi_sta_sfc.a; 同时加上：wpasupplicant.a
#define STA_WIFI_SSID		"GJ1"			//也为量产模式的路由器名称
#define STA_WIFI_PWD		"8888888899"    //也为量产模式的路由器密码
//#define CONFIG_WIFI_STA_MODE				//打开:STA连接固定路由器

#ifdef CONFIG_WIFI_STA_MODE
#define CONFIG_STATIC_IPADDR_ENABLE         //使用静态IP实现STA快速连接
#endif

#ifdef CONFIG_MASS_PRODUCTION_ENABLE /*量产模式打开STA模式*/
#define CONFIG_WIFI_STA_MODE	/*打开:STA连接固定路由器,否则:默认AP模式*/
#define CONFIG_USR_VIDEO_ENABLE		//用户VIDEO使能
#endif

//默认网络视频为JPEG格式
#define CONFIG_NET_JPEG

//#define CONFIG_NET_TCP_ENABLE
#define CONFIG_NET_UDP_ENABLE

/*-------------网络端口----------------*/
#define CTP_CTRL_PORT   	3333
#define CDP_CTRL_PORT   	2228
#define NET_SCR_PORT    	2230
#define VIDEO_PREVIEW_PORT 	2226
#define VIDEO_PLAYBACK_PORT 2223
#define HTTP_PORT           8080
#define RTSP_PORT           554

/*--------视频库内使用----------------*/
#define _DUMP_PORT          2229
#define _FORWARD_PORT    	2224
#define _BEHIND_PORT     	2225

#define CONFIG_NET_PKG_JPEG

//*********************************************************************************//
//                         录像和实时流帧率配置                                    //
//*********************************************************************************//
#define NET_VIDEO_SHARE_CHANNEL			1 //1:开启录像和实时流使用共用通道，可节省内存

//实时流BUFF允许缓存帧数(延时大则需要打开，例如录像中720P),写0无效,注意：该宏大于1时，会引起实时流实际帧率比配置低，但是实时性好
#define  NET_VIDEO_BUFF_FRAME_CNT		2

//实时流前后视帧率设置
#define  NET_VIDEO_REC_FPS0   			20  //不使用0作为默认值，写具体数值
#define  NET_VIDEO_REC_FPS1   			20   //不使用0作为默认值，写具体数值

//录像模式实时流丢帧配置(单路720录像且实时流720或双路录像有效),只能配置以下3个宏数值>=1其中一个,配置必须小于等于摄像头输出帧率,全0则输出录像帧率
#define  NET_VIDEO_REC_DROP_REAl_FP		15	//(实际输出帧率，单路720录像且实时流720或双路录像有效,写0无效,不录像请用NET_VIDEO_REC_FPS0,NET_VIDEO_REC_FPS1)
#define  NET_VIDEO_REC_LOSE_FRAME_CNT	0	//(每隔n帧丢一帧,单路720录像且实时流720或双路录像有效,写0无效)30p摄像头,需求>=15p/s则配置>=1;25p摄像头,需求>=12p/s则配置>=1.
#define  NET_VIDEO_REC_SEND_FRAME_CNT	0	//(每隔n帧发一帧,单路720录像且实时流720或双路录像有效,写0无效)

//RTSP实时流帧率设置
#define  STRM_VIDEO_REC_FPS0   			5  //不使用0作为默认值，写具体数值
#define  STRM_VIDEO_REC_DROP_REAl_FP 	5  //(实际输出帧率，单路720录像且实时流720或双路录像有效,写0无效)

//RTSP实时流BUFF允许缓存帧数(延时大则需要打开，例如录像中720P),写0无效,注意：该宏大于1时，会引起实时流实际帧率比配置低，但是实时性好
#define  STRM_VIDEO_BUFF_FRAME_CNT		2
/*--------------------------------------*/


//*********************************************************************************//
//                             编码图片分辨率                                      //
//*********************************************************************************//
//#define CONFIG_VIDEO_720P
#ifdef CONFIG_VIDEO_720P
#define CONFIG_VIDEO_IMAGE_W    1280 //前视镜头的宽
#define CONFIG_VIDEO_IMAGE_H    720 //前视镜头的高
#else
#define CONFIG_VIDEO_IMAGE_W    640 //前视镜头的宽
#define CONFIG_VIDEO_IMAGE_H    480 //前视镜头的高
#endif

#ifdef CONFIG_VIDEO1_ENABLE
#define CONFIG_VIDEO1_IMAGE_W    320 //后视视镜头的宽
#define CONFIG_VIDEO1_IMAGE_H    240 //后视视镜头的高
#endif

//*********************************************************************************//
//                             视频流相关配置                                      //
//*********************************************************************************//
#define VIDEO_REC_AUDIO_SAMPLE_RATE		8000 //视频流的音频采样率,注意：硬件没MIC则为0
#define VIDEO_REC_FPS 					20   //录像SD卡视频帧率设置,0为默认

//#define CONFIG_USR_VIDEO_ENABLE		//用户VIDEO使能
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
#define CONFIG_NO_BACKUP_UPGRADE_PATH     	CONFIG_ROOT_PATH\
											CONFIG_UPGRADE_FILE_NAME	//不备份的网络固件升级专用，建议采用备份方式升级


//*********************************************************************************//
//                                  低功耗配置                                     //
//*********************************************************************************//
//#define CONFIG_LOW_POWER_ENABLE            			//低功耗睡眠使能
//#define CONFIG_INTERNAL_VDDIO_POWER_SUPPLY_ENABLE //实际PCB设计如果采用内部VDDIO供电，必须定义该宏
#define TCFG_LOWPOWER_BTOSC_DISABLE			0
#ifdef CONFIG_LOW_POWER_ENABLE
#define TCFG_LOWPOWER_LOWPOWER_SEL			(RF_SLEEP_EN|RF_FORCE_SYS_SLEEP_EN|SYS_SLEEP_EN) //该宏在睡眠低功耗才用到
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
#define TCFG_LOWPOWER_VDDIOW_LEVEL			VDDIOW_VOL_21V //软关机或睡眠的内部vddio最低电压值
#define VDC14_VOL_SEL_LEVEL					VDC14_VOL_SEL_140V //内部的1.4V默认1.4V
#define SYSVDD_VOL_SEL_LEVEL				SYSVDD_VOL_SEL_126V //系统内核电压，默认1.26V

//*********************************************************************************//
//                                  拍照配置                                         //
//*********************************************************************************//
#define PROCESS_EFFECT                      1//万花筒
#define PHOTO_BOOTH                         1//大头贴
#define MULTIPLE							1//多图像合成

//*********************************************************************************//
//                                  UI配置                                         //
//*********************************************************************************//
#ifdef CONFIG_UI_ENABLE
//资源存放在预留区请查阅cpu/tool/isd_config_rule.c进行配置资源文件地址以及大小
// #define UI_USE_WIFI_CAMERA_PROJECT          //使用wifi_camera_ui工程 该宏会在download.c进行调用 批处理自动生成UI资源文件
#define CONFIG_UI_RES_SAVE_EXTFLASH         1  //资源文件存放在flash最后不进行备份使用这个可以提高编译下载速度，需要根据提示自行解决下载问题 具体阅读isd_config_rule.c
#define CONFIG_VIDEO_DEC_ENABLE             1  //打开视频解码器
#define TCFG_LCD_ENABLE                     1  //使用lcd屏幕mcu屏//否则使用RGB屏8bit
#define TCFG_DEMO_UI_RUN                    1  //开机运行ui_demo.c
#define CONFIG_MP3_DEC_ENABLE               1
#define CONFIG_LCD_QR_CODE_ENABLE           0  //1检测二维码 2检测条形码 0关闭检测
#define CONFIG_PLAY_JPG_ENABLE              0
#if TCFG_DEMO_UI_RUN
#define NO_UI_LCD_TEST                      0  //开机运行camera_lcd_only_show_demo.c 不带UI 直推到屏
#define APP_VIDEO_REC_RUN					0  //SD卡录制(video_rec.c)和屏显(ui_demo.c)同时运行，需将此宏置1
#endif

#define TCFG_USE_SD_ADD_UI_FILE             0  //使用SD卡加载资源文件

#if TCFG_LCD_ENABLE
#define TCFG_LCD_ILI9481_ENABLE             1
#define TCFG_LCD_ILI9488_ENABLE             1
#define TCFG_LCD_TM9486X_ENABLE             0
#define TCFG_LCD_ILI9341_ENABLE	            0
#else
#define TCFG_LCD_480x272_8BITS              1
#endif

#if TCFG_LCD_ILI9341_ENABLE
#define TCFG_TOUCH_GT911_ENABLE             1
#else
#define TCFG_TOUCH_GT911_ENABLE             0
#endif

#if TCFG_LCD_ILI9481_ENABLE
#define TCFG_TOUCH_FT6236_ENABLE            0
#else
#define TCFG_TOUCH_FT6236_ENABLE            0
#endif

#if TCFG_LCD_480x272_8BITS || TCFG_LCD_ST7789V_ENABLE || TCFG_LCD_ILI9341_ENABLE
#define HORIZONTAL_SCREEN                   0//0为使用竖屏 这个指的是ui工程的横屏竖屏方向
#else
#define HORIZONTAL_SCREEN                   0//1为使能横屏配置 这个指的是ui工程的横屏竖屏方向
#endif

#if TCFG_LCD_ST7789S_ENABLE || TCFG_LCD_ILI9341_ENABLE || TCFG_LCD_ILI9481_ENABLE || TCFG_LCD_ILI9488_ENABLE || TCFG_LCD_TM9486X_ENABLE
#define USE_LCD_TE                          1
#else
#define USE_LCD_TE                          0
#endif



#if CONFIG_UI_RES_SAVE_EXTFLASH
#define CONFIG_UI_FILE_SAVE_IN_RESERVED_ZONE  //UI资源打包后放在预留区，可以通过升级预留区更新此资源，一般用于双备份时UI资源小于代码大小的方案
// #define CONFIG_UI_FILE_SAVE_IN_RESERVED_EXPAND_ZONE  //UI资源打包后放在扩展预留区，不可以通过升级更新此资源，一般用于UI不需要更新的方案
#define CONFIG_VOICE_PROMPT_FILE_SAVE_IN_RESERVED_ZONE  //提示音资源打包后放在预留区，可以通过升级预留区更新此资源，一般用于双备份时提示音资源小于代码大小的方案
// #define CONFIG_VOICE_PROMPT_FILE_SAVE_IN_RESERVED_EXPAND_ZONE  //提示音资源打包后放在扩展预留区，不可以通过升级更新此资源，一般用于提示音不需要更新的方案
#endif

#if CONFIG_MP3_DEC_ENABLE
#if defined CONFIG_VOICE_PROMPT_FILE_SAVE_IN_RESERVED_EXPAND_ZONE
#define CONFIG_VOICE_PROMPT_FILE_PATH       "mnt/sdfile/EXT_RESERVED/aupackres/tone/"
#elif defined CONFIG_VOICE_PROMPT_FILE_SAVE_IN_RESERVED_ZONE
#define CONFIG_VOICE_PROMPT_FILE_PATH       "mnt/sdfile/app/aupackres/tone/"
#else
#define CONFIG_VOICE_PROMPT_FILE_PATH       "mnt/sdfile/res/audlogo/"
#endif
#endif

#if CONFIG_DOUBLE_BANK_ENABLE
#if (__FLASH_SIZE__ <= 2 * 1024 * 1024) /*开启双备份时打开UI，flash大小需要大于2M,修改在board_791xx_cfg.h*/
#error "err in CONFIG_UI_ENABLE, flash size no enough in board_791xx_cfg.h"
#endif
#endif
#endif//CONFIG_UI_ENABLE

#if !defined CONFIG_DEBUG_ENABLE || defined CONFIG_LIB_DEBUG_DISABLE
#define LIB_DEBUG    0
#else
#define LIB_DEBUG    1
#endif
#define CONFIG_DEBUG_LIB(x)         (x & LIB_DEBUG)

#include "video_buf_config.h"

#ifdef CONFIG_ENABLE_VLIST
#ifdef CONFIG_NO_SDRAM_ENABLE
#error "err in no sdram, cannot support file list : CONFIG_ENABLE_VLIST"
#endif
#endif

#ifndef TCFG_ADKEY_ENABLE
#define TCFG_ADKEY_ENABLE             0         //AD按键
#endif

#ifndef TCFG_IOKEY_ENABLE
#define TCFG_IOKEY_ENABLE             0         //IO按键
#define TCFG_IO_MULTIPLEX_WITH_SD     0
#endif

#ifndef TCFG_IRKEY_ENABLE
#define TCFG_IRKEY_ENABLE             0         //红外遥控按键
#endif

#ifndef TCFG_RDEC_KEY_ENABLE
#define TCFG_RDEC_KEY_ENABLE          0         //旋转编码器
#endif

#endif

