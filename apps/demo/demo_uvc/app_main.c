#include "system/includes.h"
#include "action.h"
#include "app_core.h"
#include "event/key_event.h"
#include "event/device_event.h"
#include "app_config.h"
#include "storage_device.h"
#include "generic/log.h"
#include "device/gpio.h"
#include "asm/p33.h"
#include "os/os_api.h"


/*中断列表 */
const struct irq_info irq_info_table[] = {
    //中断号   //优先级0-7   //注册的cpu(0或1)
#ifdef CONFIG_IPMASK_ENABLE
    //不可屏蔽中断方法：支持写flash，但中断函数和调用函数和const要全部放在内部ram
    { IRQ_SOFT5_IDX,      6,   0    }, //此中断强制注册到cpu0
    { IRQ_SOFT4_IDX,      6,   1    }, //此中断强制注册到cpu1
#if 0 //如下，SPI1使用不可屏蔽中断设置
    { IRQ_SPI1_IDX,      7,   1    },//中断强制注册到cpu0/1
#endif
#endif

#if CPU_CORE_NUM == 1
    { IRQ_SOFT5_IDX,      7,   0    }, //此中断强制注册到cpu0
    { IRQ_SOFT4_IDX,      7,   1    }, //此中断强制注册到cpu1
    { -2,     			-2,   -2   },//如果加入了该行, 那么只有该行之前的中断注册到对应核, 其他所有中断强制注册到CPU0
#endif

    { -1,     			-1,   -1   },
};

/*创建使用 os_task_create_static 或者task_create 接口的 静态任务堆栈*/
#define SYS_TIMER_STK_SIZE 512
#define SYS_TIMER_Q_SIZE 128
static u8 sys_timer_tcb_stk_q[sizeof(StaticTask_t) + SYS_TIMER_STK_SIZE * 4 + sizeof(struct task_queue) + SYS_TIMER_Q_SIZE] ALIGNE(4);

#define SYSTIMER_STK_SIZE 256
static u8 systimer_tcb_stk_q[sizeof(StaticTask_t) + SYSTIMER_STK_SIZE * 4] ALIGNE(4);

#define SYS_EVENT_STK_SIZE 512
static u8 sys_event_tcb_stk_q[sizeof(StaticTask_t) + SYS_EVENT_STK_SIZE * 4] ALIGNE(4);

#define APP_CORE_STK_SIZE 1024
#define APP_CORE_Q_SIZE 1024
static u8 app_core_tcb_stk_q[sizeof(StaticTask_t) + APP_CORE_STK_SIZE * 4 + sizeof(struct task_queue) + APP_CORE_Q_SIZE] ALIGNE(4);

/*创建使用  thread_fork 接口的 静态任务堆栈*/
#define WIFI_TASKLET_STK_SIZE 1400
static u8 wifi_tasklet_tcb_stk_q[sizeof(struct thread_parm) + WIFI_TASKLET_STK_SIZE * 4] ALIGNE(4);

#define WIFI_CMDQ_STK_SIZE 300
static u8 wifi_cmdq_tcb_stk_q[sizeof(struct thread_parm) + WIFI_CMDQ_STK_SIZE * 4] ALIGNE(4);

#define WIFI_MLME_STK_SIZE 700
static u8 wifi_mlme_tcb_stk_q[sizeof(struct thread_parm) + WIFI_MLME_STK_SIZE * 4] ALIGNE(4);

#define WIFI_RX_STK_SIZE 256
static u8 wifi_rx_tcb_stk_q[sizeof(struct thread_parm) + WIFI_RX_STK_SIZE * 4] ALIGNE(4);

/*任务列表 */
const struct task_info task_info_table[] = {
    {"init",                25,   512,   256   },
    {"app_core",            22,   APP_CORE_STK_SIZE,	APP_CORE_Q_SIZE,  app_core_tcb_stk_q },
    {"sys_event",           25,   SYS_EVENT_STK_SIZE,  			  0,  sys_event_tcb_stk_q},
    {"systimer",            16,   SYSTIMER_STK_SIZE,  			  0,  systimer_tcb_stk_q },
    {"sys_timer",           10,   SYS_TIMER_STK_SIZE, SYS_TIMER_Q_SIZE, sys_timer_tcb_stk_q},
    {"audio_server",        16,   1024,   256  },
    {"audio_mix",           27,    512,   64   },
    {"audio_decoder",       30,   1024,   64   },
    {"audio_encoder",       14,   1024,   64   },
    {"speex_encoder",       10,   1024,   0    },
    {"opus_encoder",        10,   1536,   0    },
    {"vir_dev_task",         9,   1024,   0    },
    {"wechat_task",         18,   2048,   64   },
    {"amr_encoder",         16,   1024,   0    },
    {"usb_server",          20,   1024,   64   },
    {"usb_msd",             25,   1024,   32   },
    {"update",      		21,    512,   32   },
    {"dw_update",      		21,    512,   32   },

#ifdef CONFIG_WIFI_ENABLE
    {"tasklet",             28,   WIFI_TASKLET_STK_SIZE, 0,		 wifi_tasklet_tcb_stk_q	 },//通过调节任务优先级平衡WIFI收发占据总CPU的比重
    {"RtmpMlmeTask",        27,   WIFI_MLME_STK_SIZE,  	 0, 	 wifi_mlme_tcb_stk_q	 },
    {"RtmpCmdQTask",        28,   WIFI_CMDQ_STK_SIZE,    0,  	 wifi_cmdq_tcb_stk_q	 },
    {"wl_rx_irq_thread",    28,   WIFI_RX_STK_SIZE,  	 0,		 wifi_rx_tcb_stk_q		 },
#endif

    {"ai_server",			15,	  1024,	  64   },
    {"asr_server",			15,	  1024,	  64   },
    {"wake-on-voice",		 7,	  1024,	  0    },
    {"resample_task",		 8,	  1024,	  0    },
    {"vad_encoder",         16,   1024,   0    },
    {"video_server",        26,    800,   1024 },
    {"vpkg_server",         26,    512,   512  },
    {"jpg_dec",             26,   1024,   32   },
    {"jpg_spec_enc",        26,    800,   32   },
    {"dynamic_huffman0",    15,    256,   32   },
    {"video0_rec0",         25,    400,   512  },
    {"video0_rec1",         24,    400,   512  },
    {"video1_rec0",         25,    400,   512  },
    {"video1_rec1",         24,    400,   512  },
    {"video2_rec0",         24,    400,   512  },
    {"video2_rec1",         24,    400,   512  },
    {"video3_rec0",         24,    400,   512  },
    {"video3_rec1",         24,    400,   512  },
    {"audio_rec0",          22,    256,   256  },
    {"audio_rec1",          19,    256,   256  },
    {"avi0",                20,    320,   64   },
    {"avi1",                20,    320,   64   },
    {"avi2",                20,    320,   64   },

    {"ctp_server",          26,    500,   64   },
    {"net_video_server",    16,    256,   64   },
    {"net_avi0",            24,    512,   0    },
    {"net_avi1",            24,    512,   0    },
    {"net_avi2",            24,    512,   0    },
    {"net_avi3",            24,    512,   0    },
    {"stream_avi0",         18,    600,   0    },
    {"stream_avi1",         18,    600,   0    },
    {"stream_avi2",         18,    600,   0    },
    {"video_dec_server",    26,   1024,   1024 },
    {"vunpkg_server",       23,   1024,   128  },
    {"yuv_task",            22,   1024,   128  },
    {"wifi_put_rec",        15,    800,   0    },

#ifdef CONFIG_UI_ENABLE
    {"ui",                  21,      768,   256   },
    {"lcd_task_0",			8,	  1024,	  32   },
    {"lcd_task_1",			8,	  1024,	  32   },
    {"te_task",		    	9,	  1024,	  32   },
#endif

#if CPU_CORE_NUM > 1
    {"#C0usb_msd0",          1,    512,   128  },
#else
    {"usb_msd0",             1,    512,   128  },
#endif
    {"usb_msd1",             1,    512,   128  },
    {"uac_play0",           26,    512,   32   },
    {"uac_play1",           26,    512,   32   },
    {"uac_record0",         26,    512,   0    },
    {"uac_record1",         26,    512,   0    },
    {"dw_update",           21,    512,   32   },

    {"stream_media_server", 18,   4096,   128  },

    {0, 0},
};



/*
 * 默认的系统事件处理函数
 * 当所有活动的app的事件处理函数都返回false时此函数会被调用
 */
void app_default_event_handler(struct sys_event *event)
{
    printf("\n [ERROR] %s -[yuyu] %d\n", __FUNCTION__, __LINE__);
    switch (event->type) {
    case SYS_KEY_EVENT:
#ifdef CONFIG_UI_ENABLE
        camera_ui_key_event_handler((struct key_event *)event->payload);
#endif

        break;
    case SYS_TOUCH_EVENT:
        break;
    case SYS_DEVICE_EVENT:
        break;
    case SYS_BT_EVENT:
        break;
    default:
        ASSERT(0, "unknow event type: %s\n", __func__);
        break;
    }
}


/*
 * 应用程序主函数
 */
void app_main()
{
    struct intent it;

    puts("------------- wifi_camera app main-------------\n");
#ifdef PRODUCT_TEST_ENABLE
    u8 product_main(void);
    if (product_main()) {
        //进入产测模式后，将不再运行APP状态机
        return;
    }
#endif
#if 0
    init_intent(&it);

    it.name	= "net_video_rec";//APP状态机在：net_video_rec.c
#ifdef CONFIG_UI_ENABLE
#if APP_VIDEO_REC_RUN
    it.name	= "video_rec";//APP状态机在：video_rec.c
#endif
#endif
    it.action = ACTION_VIDEO_REC_MAIN;
    start_app(&it);
#endif
    //extern int prodct_auto_test_task_create(void);
    //prodct_auto_test_task_create();//自动量产测试:测试前后视摄像头
}

