#include "server/audio_server.h"
#include "server/server_core.h"
#include "app_config.h"
#include "syscfg/syscfg_id.h"
#include "system/app_core.h"
#include "event/key_event.h"
#include "fs/fs.h"
#define CONFIG_LOCAL_MUSIC_MODE_ENABLE
#define CONFIG_NET_MUSIC_MODE_ENABLE
#define CONFIG_RECORDER_MODE_ENABLE
struct audio_app_t {
    const char *tone_file_name;
    const char *app_name;
};

static const struct audio_app_t audio_app_table[] = {
#ifdef CONFIG_LOCAL_MUSIC_MODE_ENABLE
    {"FlashMusic.mp3", "local_music"},        //FlashMusic     Live 
#endif
#ifdef CONFIG_NET_MUSIC_MODE_ENABLE
    {"NetMusic.mp3", "net_music"  },
#endif
#ifdef CONFIG_RECORDER_MODE_ENABLE
    {"Recorder.mp3", "recorder"},
#endif
#ifdef CONFIG_ASR_ALGORITHM_ENABLE
    {"AiSpeaker.mp3", "ai_speaker"},
#endif
};

static u8 mode_index = 0;

static void dec_server_event_handler(void *priv, int argc, int *argv)
{
    union audio_req r = {0};

    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
        log_i("tone: AUDIO_SERVER_EVENT_ERR\n");
    case AUDIO_SERVER_EVENT_END:
        log_i("tone: AUDIO_SERVER_EVENT_END\n");
        r.dec.cmd = AUDIO_DEC_STOP;
        server_request(priv, AUDIO_REQ_DEC, &r);
        server_close(priv); //priv是server_register_event_handler_to_task的priv参数
        fclose((FILE *)argv[1]); //argv[1]是解码开始时传递进去的文件句柄
        key_event_enable();
        //等提示音播完了再切换模式
        struct intent it;
        init_intent(&it);
        it.name = audio_app_table[mode_index++].app_name;
        start_app(&it);
        break;
    case AUDIO_SERVER_EVENT_CURR_TIME:
        log_i("play_time: %d\n", argv[1]);
        break;
    default:
        break;
    }
}

//播放提示音
static int app_play_tone_file(const char *path)
{
    printf("app play tone file  begin \n ");
    int err = 0;
    union audio_req req = {0};

    log_d("play tone file : %s\n", path);

    FILE *file = fopen(path, "r");
    if (!file) {
        printf(" !file  -1 \n ");
        return -1;
    }
    printf("file = fopen  \n ");
    void *dec_server = server_open("audio_server", "dec");
    if (!dec_server) {
        goto __err;
    }
    printf("server_open(audio_server, dec   \n ");
    server_register_event_handler_to_task(dec_server, dec_server, dec_server_event_handler, "app_core");

    req.dec.cmd             = AUDIO_DEC_OPEN;
    req.dec.volume          = 50;
    req.dec.output_buf_len  = 4 * 1024;
    req.dec.file            = file;
    req.dec.sample_source   = "dac";
    syscfg_read(CFG_MUSIC_VOL, &req.dec.volume, sizeof(req.dec.volume));

    err = server_request(dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        goto __err;
    }
    printf("  ---------------------------------------------------------------------- \n");
    printf("  kingkong app_play_tone_file err:%d \n",err);
    printf("  ---------------------------------------------------------------------- \n");
    req.dec.cmd = AUDIO_DEC_START;

    err = server_request(dec_server, AUDIO_REQ_DEC, &req);
    if (err) {
        goto __err;
    }

    key_event_disable();

    return 0;

__err:

    if (dec_server) {
        server_close(dec_server);
    }
    if (file) {
        fclose(file);
    }

    return err;
}

int audio_demo_mode_switch(void)
{
    printf("  ---------------------------------------------------------------------- \n");
    printf("  kingkong main mode_index :%d \n",mode_index);
    printf("  ---------------------------------------------------------------------- \n");
    if (mode_index >= ARRAY_SIZE(audio_app_table)) {
        mode_index = 0;
    }
    struct intent it;

    if (get_current_app()) {
        init_intent(&it);
        it.name = audio_app_table[mode_index].app_name;
        it.action = ACTION_STOP;    //退出当前模式
        start_app(&it);
    }

    char path[64];

    sprintf(path, "%s%s", CONFIG_ROOT_PATH, "wert.mp3");  //CONFIG_VOICE_PROMPT_FILE_PATH   audio_app_table[mode_index].tone_file_name
    printf("  ---------------------------------------------------------------------- \n");
    printf("  kingkong path :%s \n",path);  //https://doc.zh-jieli.com/AC79/zh-cn/master/module_example/system/index.html
    printf("  kingkong CONFIG_VOICE_PROMPT_FILE_PATH :%s \n",CONFIG_ROOT_PATH);    //    mnt/sdfile/res/audlogo/   CONFIG_VOICE_PROMPT_FILE_PATH
    printf("  kingkong tone_file_name :%s \n",audio_app_table[mode_index].tone_file_name); 
    printf("  ---------------------------------------------------------------------- \n");

    // FILE *file = fopen(CONFIG_ROOT_PATH"ee.mp3", "r");
    // if (!file) {
    //     printf(" !file  -1   ee.mp3\n ");
    //     return -1;
    // }
    // else
    // {
    //     printf("file = fopen  ee.mp3\n ");
    // }

    // puts("-------sd_fs_test--------");

    // extern int storage_device_ready(void);

    // while (!storage_device_ready()) {//等待sd文件系统挂载完成
    //     printf(" storage device ready ing ");
    //     os_time_dly(2);
    // }

    // //读写测试
    // u8 *test_data = "sd test 123456";
    // u8 read_buf[32];
    // int wlen;
    // int rlen;
    // int ret;
    // FILE *newFile = NULL; //保存移动文件操作成功后文件在新路径的文件流指针

    // memset(read_buf, 0, sizeof(read_buf));

    // //1.创建文件
    // FILE *fd = fopen(CONFIG_ROOT_PATH"111/test.txt", "w+");
    // if (fd) {
    //     fwrite(test_data, 1, strlen(test_data), fd);
    //     wlen = flen(fd);
    //     fclose(fd);
    //     printf("write file len: %d \n", wlen);
    // }

    // //2.读取文件
    // fd = fopen(CONFIG_ROOT_PATH"111/test.txt", "r");
    // if (fd) {
    //     rlen = flen(fd);//获取整个文件大小
    //     fread(read_buf, 1, rlen, fd);
    //     fclose(fd);
    //     printf("read file : %s  rlen : %d\n", read_buf, rlen);
    // }

    return app_play_tone_file(path);
}

