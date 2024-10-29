#include "app_config.h"
#ifdef CONFIG_UI_ENABLE //上电执行则打开app_config.h TCFG_DEMO_UI_RUN = 1

#include "device/device.h"//u8
#include "storage_device.h"//SD
#include "server/video_dec_server.h"//fopen
#include "system/includes.h"//GPIO
#include "lcd_drive.h"
#include "sys_common.h"
#include "yuv_soft_scalling.h"
#include "asm/jpeg_codec.h"
#include "get_yuv_data.h"
#include "lcd_config.h"
#include "ui_demo.h"


/************视频部分*将摄像头数据显示到lcd上**********************************/
/* 开关打印提示 */
#if 1
#define log_info(x, ...)    printf("\n[camera_to_lcd_test]>>>>>>>>>>>>>>>>>>>###" x " \n", ## __VA_ARGS__)
#else
#define log_info(...)
#endif

u8 get_lcd_turn_mode(void);

static void Calculation_frame(void)
{
    static u32 tstart = 0, tdiff = 0;
    static u8 fps_cnt = 0;

    fps_cnt++ ;

    if (!tstart) {
        tstart = timer_get_ms();
    } else {
        tdiff = timer_get_ms() - tstart;

        if (tdiff >= 1000) {
            printf("\n [MSG]fps_count = %d\n", fps_cnt *  1000 / tdiff);
            tstart = 0;
            fps_cnt = 0;
        }
    }
}
/********保存JPG图片到SD卡中************/
static void save_jpg_ontime(u8 *buf, u32 len)
{
    if (storage_device_ready()) {//自动重命名文件数量限制配置搜索 FILE_AUTO_RENAME_NUM
        char file_name[64];//定义路径存储
        snprintf(file_name, 64, CONFIG_ROOT_PATH"JPG/JPG****.JPG");
        FILE *fd = fopen(file_name, "wb+");
        printf("[msg]>>>>>>>>>>>file_name=%s", file_name);
        fwrite(buf, 1, len, fd);
        fclose(fd);
        log_info("JPG save ok name=JPG\r\n");
    }
}
/********保存RGB数据到SD卡中************/
static void get_RGB_save_to_SD(char *RGB, u32 len)
{
    if (storage_device_ready()) {
        char file_name[64];//定义路径存储
        snprintf(file_name, 64, CONFIG_ROOT_PATH"rgb/RGB***.bin");
        FILE *fd = fopen(file_name, "wb+");
        fwrite(RGB, 1, len, fd);
        fclose(fd);
        log_info("RGB save ok name=RGB\r\n");
    }
}
/********保存YUV数据到SD卡中************/
static void save_YUV_date_ontime(u8 *buf, u32 len)
{
    if (storage_device_ready()) {
        char file_name[64];//定义路径存储
        snprintf(file_name, 64, CONFIG_ROOT_PATH"YUV/YUV***.bin");
        FILE *fd = fopen(file_name, "wb+");
        fwrite(buf, 1, len, fd);
        fclose(fd);
        log_info("YUV save ok name=YUV\r\n");
    }
}

/********读取JPG数据并保存在SD卡中该数据经过缩放保存大小1280*720********/
#define yuv_out_w   1280
#define yuv_out_h   720
#define in_w 		640
#define in_h 		480
static void get_JPG_save_to_SD(char *yuv_img_buf)
{
    struct jpeg_encode_req req = {0};
    u32 buf_len =  yuv_out_w * yuv_out_h * 3 / 2 ;
    u8 *yuv_out_buf =  malloc(buf_len);
    u8 err = 0;

    if (!yuv_out_buf) {
        log_info("jpeg buf not enough err!!!\n");
        return;
    }

    YUV420p_Soft_Scaling(yuv_img_buf, yuv_out_buf, in_w, in_h, yuv_out_w, yuv_out_h);
    //q >=2 , size  = w*h*q*0.32
    req.q = 13;	//jepg编码质量(范围0-13),0最小 质量越好,下面的编码buf需要适当增大
    req.format = JPG_SAMP_FMT_YUV420;
    req.data.buf = (yuv_out_buf + 1024);// 防止做转换的时候第一行数据追尾
    req.data.len = buf_len;
    req.width = yuv_out_w ;
    req.height =  yuv_out_h ;
    req.y = yuv_out_buf;
    req.u = req.y + (yuv_out_w * yuv_out_h);
    req.v = req.u + (yuv_out_w * yuv_out_h) / 4;
    err = jpeg_encode_one_image(&req);//获取一张JPG图片

    if (!err) {
        save_jpg_ontime(req.data.buf, buf_len); //保存图片到SD卡
    } else {
        log_info("get_jpg err!!!\n");
    }
    free(yuv_out_buf);
}

/*********摄像头应用部分***********/
static void test_main(u8 *buf, u32 len, int yuv_in_w, int yuv_in_h)
{
#if CONFIG_LCD_QR_CODE_ENABLE
    char *ssid = NULL;
    char *pwd = NULL;
    qr_code_get_one_frame_YUV_420(buf, yuv_in_w, yuv_in_h);
    qr_get_ssid_pwd(&ssid, &pwd);
    printf(">>>>>>>>>>>>>>>sssid = %s, pwd = %s", ssid, pwd);
#endif
    if (get_take_photo_status()) {
        get_JPG_save_to_SD(buf);
        set_take_photo_ok();
    }
    lcd_show_frame(buf, len, yuv_in_w, yuv_in_h); //240*320*2=153600
}

void set_video_rt_cb(u32(*cb)(void *, u8 *, u32), void *priv);
int user_net_video_rec_open(char forward);
int user_net_video_rec_close(char forward);
int jpeg2yuv_open(void);
void jpeg2yuv_yuv_callback_register(void (*cb)(u8 *data, u32 len, int width, int height));
int jpeg2yuv_jpeg_frame_write(u8 *buf, u32 len);
void jpeg2yuv_close(void);
static void uvc_video_test(void);

void get_yuv_init(void (*cb)(u8 *data, u32 len, int width, int height));

void camera_to_lcd_init(char camera_ID)//摄像头选择 并初始化
{
#ifdef CONFIG_UVC_VIDEO2_ENABLE

    printf("\n [ERROR] %s -[yuyu] %d\n", __FUNCTION__, __LINE__);
    uvc_video_test();
#else
    get_yuv_init(test_main);
#endif
}

void camera_to_lcd_uninit(void)
{
    void get_yuv_uninit(void);
    get_yuv_uninit();
}
int isc_log_en()//如果定义该函数丢帧信息屏蔽
{
    return 0;
}

static void uvc_jpeg_cb(void *hdr, void *data, u32 len)
{
#define JPEG_HEAD 0xE0FFD8FF
#define JPEG_HEAD1 0xC0FFD8FF
    u32 *head = (u32 *)data;
    if (*head == JPEG_HEAD || *head == JPEG_HEAD1) {
        //video
        jpeg2yuv_jpeg_frame_write((u8 *)data, len);

        if (get_take_photo_status()) {
            /* get_JPG_save_to_SD(buf); */
            save_jpg_ontime((u8 *)data, len);
            set_take_photo_ok();
        }
    } else {
        //audio
    }
}

static void camera_show_lcd(u8 *buf, u32 size, int width, int height)
{
    char *ssid = NULL;
    char *pwd = NULL;
#if CONFIG_LCD_QR_CODE_ENABLE
    qr_code_get_one_frame_YUV_420(buf, width, height);
    qr_get_ssid_pwd(&ssid, &pwd);
    printf(">>>>>>>>>>>>>>>sssid = %s, pwd = %s", ssid, pwd);
#endif
    lcd_show_frame(buf, size, width, height);
    Calculation_frame();
}

static void uvc_video_task(void *priv)
{
    int ret;
    static void *user_video0;

start:
    //0.等待UVC上线
    while (!dev_online("uvc")) {
        os_time_dly(10);
    }
    //1.打开jpeg解码YUV
    ret = jpeg2yuv_open();
    if (ret) {
        return;
    }
    //2.注册YUV数据回调函数
    jpeg2yuv_yuv_callback_register(camera_show_lcd);

#if (APP_VIDEO_REC_RUN)
    /* extern OS_SEM video_sem; */
    /* os_sem_set(&video_sem, 0); */
    /* os_sem_pend(&video_sem, 0); */
#endif

    //3.打开UVC实时流
    ret = user_video_rec0_open();
    if (ret) {
        jpeg2yuv_close();
        return;
    }
    //3.注册jpeg数据回调函数
    set_video_rt_cb(uvc_jpeg_cb, NULL);

#if	TCFG_LCD_USB_SHOW_COLLEAGUE
    /* void set_tcp_uvc_rt_cb(u32(*cb)(void *priv, void *data, u32 len)); */
    /* void set_udp_uvc_rt_cb(u32(*cb)(void *priv, void *data, u32 len)); */
    /* set_udp_uvc_rt_cb(uvc_jpeg_cb); //wifi udp */
    /* set_tcp_uvc_rt_cb(uvc_jpeg_cb); //wifi tcp */
#endif

    //4.检查是否掉线
    struct lbuf_test_head *rbuf = NULL;

    while (dev_online("uvc")) {
        os_time_dly(5);
    }
#if (!APP_VIDEO_REC_RUN)
    //5.关闭UVC实时流
    /* user_net_video_rec_close(1); */
    //6.关闭jpeg解码YUV
    user_video_rec0_close();
    jpeg2yuv_close();
#endif
    goto start;

}
static void uvc_video_test(void)
{
    thread_fork("uvc_video_task", 5, 1024, 0, 0, uvc_video_task, NULL);
}
#endif

