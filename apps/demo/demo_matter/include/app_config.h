#ifndef APP_CONFIG_H
#define APP_CONFIG_H



#define __FLASH_SIZE__    (4 * 1024 * 1024)
#define __SDRAM_SIZE__    (2 * 1024 * 1024)

#define CONFIG_CXX_SUPPORT


#define TCFG_ADKEY_ENABLE 1

#define CONFIG_WIFI_ENABLE  					/* 无线WIFI */
#ifdef CONFIG_NO_SDRAM_ENABLE
#define CONFIG_RF_TRIM_CODE_MOVABLE //把RF TRIM 的运行代码动态加载到ram运行(节省4K ram内存), 防止RF TRIM 期间500ms大电流访问flash造成flash挂掉持续大电流
#else
#define CONFIG_RF_TRIM_CODE_AT_RAM //把RF TRIM 的运行代码定死到ram运行(浪费4K ram内存,否则若动态加载到sdram需清cache), 防止RF TRIM 期间500ms大电流访问flash造成flash挂掉持续大电流
#endif
//#define CONFIG_ASSIGN_MACADDR_ENABLE        //第一次开机连上外网后，使用杰理服务器分配WIFI模块的MAC地址, 关闭则使用<蓝牙地址更新工具*.exe>或者随机数作为MAC地址
#define CONFIG_IPERF_ENABLE       				// iperf测试
#define  CONFIG_AIRKISS_NET_CFG  //AIRKISS配网
#define RTOS_STACK_CHECK_ENABLE
#define CONFIG_STATIC_IPADDR_ENABLE          //记忆路由器分配的IP,下次直接使用记忆IP节省DHCP时间
//*********************************************************************************//
//                                    电源配置                                     //
//*********************************************************************************//
//#define CONFIG_LOW_POWER_ENABLE              //WIFI节能模式开关
#define TCFG_LOWPOWER_BTOSC_DISABLE			0

#ifdef CONFIG_LOW_POWER_ENABLE
#define TCFG_LOWPOWER_LOWPOWER_SEL			RF_SLEEP_EN //配置仅WIFI RF休眠
#else
#define TCFG_LOWPOWER_LOWPOWER_SEL			0
#endif
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_32V       //强VDDIO电压档位，不要高于外部DCDC的电压
#define TCFG_LOWPOWER_VDDIOW_LEVEL			VDDIOW_VOL_21V       //弱VDDIO电压档位
#define VDC14_VOL_SEL_LEVEL			        VDC14_VOL_SEL_140V   //RF1.4V电压档位
#define SYSVDD_VOL_SEL_LEVEL				SYSVDD_VOL_SEL_126V  //内核电压档位值

//*********************************************************************************//
//                                  FCC测试相关配置                                //
//*********************************************************************************//
//#define RF_FCC_TEST_ENABLE//使能RF_FCC测试，详细配置见"apps/common/rf_fcc_tool/include/rf_fcc_main.h"


//*********************************************************************************//
//                                  BT_BLE配置                                     //
//*********************************************************************************//
#ifdef CONFIG_BT_ENABLE
#define CONFIG_BT_RX_BUFF_SIZE  0
#define CONFIG_BT_TX_BUFF_SIZE  0

#define TCFG_USER_BLE_ENABLE                      1     //BLE功能使能
#define TCFG_USER_BT_CLASSIC_ENABLE               0     //经典蓝牙功能

#if TCFG_USER_BLE_ENABLE
#define BT_NET_CFG_EN                             0     //从机 配网专用
#define XIAOMI_EN                                 0     //从机 mi_server
#define BT_NET_HID_EN                             0     //从机 hid
#define TCFG_BLE_SECURITY_EN                      0     //配对加密使能

#endif

#if TRANS_MULTI_BLE_EN
#define TRANS_MULTI_BLE_SLAVE_NUMS                2
#define TRANS_MULTI_BLE_MASTER_NUMS               2
#endif

#endif





#define CONFIG_DEBUG_ENABLE                     /* 打印开关 */

//#define SDTAP_DEBUG

#if !defined CONFIG_DEBUG_ENABLE || defined CONFIG_LIB_DEBUG_DISABLE
#define LIB_DEBUG    0
#else
#define LIB_DEBUG    1
#endif
#define CONFIG_DEBUG_LIB(x)         (x & LIB_DEBUG)


#endif


