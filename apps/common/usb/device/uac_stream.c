#include "app_config.h"
#include "system/includes.h"
#include "usb/usb_config.h"
#include "usb/device/usb_stack.h"
#include "event/device_event.h"

#if TCFG_USB_SLAVE_AUDIO_ENABLE
#include "usb/device/uac_audio.h"
#include "uac_stream.h"

#ifndef APP_AUDIO_STATE_MUSIC
#define APP_AUDIO_STATE_MUSIC 0
#endif

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[UAC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define UAC_DEBUG_ECHO_MODE 0

static u8 speaker_stream_is_open[USB_MAX_HW_NUM];
static spinlock_t uac_lock;

struct uac_speaker_handle {
    cbuffer_t cbuf;
    volatile u8 need_resume;
    u8 channel;
    u8 alive;
    void *buffer;
};

static void (*uac_rx_handler[USB_MAX_HW_NUM])(void *, void *, int);
static void *uac_rx_priv[USB_MAX_HW_NUM];

#ifndef UAC_BUFFER_SIZE
#if (SOUNDCARD_ENABLE)
#define UAC_BUFFER_SIZE     (4 * 1024)
#else
#define UAC_BUFFER_SIZE     (2 * 1024)
#endif
#endif

#define UAC_BUFFER_MAX		(UAC_BUFFER_SIZE * 50 / 100)

static struct uac_speaker_handle *uac_speaker[USB_MAX_HW_NUM];

#if USB_MALLOC_ENABLE
#else
static struct uac_speaker_handle uac_speaker_handle[USB_MAX_HW_NUM] SEC(.uac_var);
static u8 uac_rx_buffer[USB_MAX_HW_NUM][UAC_BUFFER_SIZE] ALIGNED(4) SEC(.uac_rx);
#endif

#if 0
u32 uac_speaker_stream_length(void)
{
    return UAC_BUFFER_SIZE;
}

u32 uac_speaker_stream_size(void)
{
    u32 size = 0;

    spin_lock(&uac_lock);

    if (speaker_stream_is_open && uac_speaker) {
        size = cbuf_get_data_size(&uac_speaker->cbuf);
    }

    spin_unlock(&uac_lock);

    return size;
}

u32 uac_speaker_get_alive(void)
{
    if (uac_speaker) {
        return uac_speaker->alive;
    }
    return 0;
}

void uac_speaker_set_alive(u8 alive)
{
    spin_lock(&uac_lock);
    if (uac_speaker) {
        uac_speaker->alive = alive;
    }
    spin_unlock(&uac_lock);
}

void uac_speaker_stream_buf_clear(void)
{
    spin_lock(&uac_lock);
    if (speaker_stream_is_open) {
        cbuf_clear(&uac_speaker->cbuf);
    }
    spin_unlock(&uac_lock);
}
#endif

void set_uac_speaker_rx_handler(const usb_dev usb_id, void *priv, void (*rx_handler)(void *, void *, int))
{
    uac_rx_handler[usb_id] = rx_handler;
    uac_rx_priv[usb_id] = priv;
}

int uac_speaker_stream_sample_rate(void)
{
    return SPK_AUDIO_RATE;
}

void uac_speaker_stream_write(const usb_dev usb_id, const u8 *obuf, u32 len)
{
    if (speaker_stream_is_open[usb_id]) {
        //write dac
        int wlen = cbuf_write(&uac_speaker[usb_id]->cbuf, (void *)obuf, len);
        if (wlen != len) {
            //putchar('W');
        }
        //if (uac_speaker[usb_id]->rx_handler) {
        if (uac_rx_handler[usb_id]) {
            /* if (uac_speaker[usb_id]->cbuf.data_len >= UAC_BUFFER_MAX) { */
            // 马上就要满了，赶紧取走
            uac_speaker[usb_id]->need_resume = 1; //2020-12-22注:无需唤醒
            /* } */
            if (uac_speaker[usb_id]->need_resume) {
                uac_speaker[usb_id]->need_resume = 0;
                uac_rx_handler[usb_id](uac_rx_priv[usb_id], (void *)obuf, len);
                //uac_speaker[usb_id]->rx_handler(0, (void *)obuf, len);
            }
        }
        uac_speaker[usb_id]->alive = 0;
    }
}

#if 0
int uac_speaker_read(void *priv, void *data, u32 len)
{
    int r_len;
    int err = 0;

    spin_lock(&uac_lock);

    if (!speaker_stream_is_open[usb_id]) {
        spin_unlock(&uac_lock);
        return 0;
    }

    r_len = cbuf_get_data_size(&uac_speaker->cbuf);
    if (r_len) {
        r_len = r_len > len ? len : r_len;
        r_len = cbuf_read(&uac_speaker->cbuf, data, r_len);
        if (!r_len) {
            putchar('U');
        }
    }

    if (r_len == 0) {
        uac_speaker->need_resume = 1;
    }

    spin_unlock(&uac_lock);

    return r_len;
}
#endif

void uac_speaker_stream_open(const usb_dev usb_id, u32 samplerate, u32 ch)
{
    if (speaker_stream_is_open[usb_id]) {
        return;
    }

    log_info("%s", __func__);

    if (!uac_speaker[usb_id]) {
#if USB_MALLOC_ENABLE
        uac_speaker[usb_id] = zalloc(sizeof(struct uac_speaker_handle));
        if (!uac_speaker[usb_id]) {
            return;
        }

        uac_speaker[usb_id]->buffer = malloc(UAC_BUFFER_SIZE);
        if (!uac_speaker[usb_id]->buffer) {
            free(uac_speaker[usb_id]);
            uac_speaker[usb_id] = NULL;
            goto __err;
        }
#else
        uac_speaker[usb_id] = &uac_speaker_handle[usb_id];
        memset(uac_speaker[usb_id], 0, sizeof(struct uac_speaker_handle));
        uac_speaker[usb_id]->buffer = uac_rx_buffer[usb_id];
#endif
        uac_speaker[usb_id]->channel = ch;
    }

    //uac_speaker[usb_id]->rx_handler = NULL;

    cbuf_init(&uac_speaker[usb_id]->cbuf, uac_speaker[usb_id]->buffer, UAC_BUFFER_SIZE);

    struct device_event event = {0};
    event.event = USB_AUDIO_PLAY_OPEN;
    event.value = (int)((ch << 24) | samplerate);
    event.arg = (void *)(int)usb_id;

#if !UAC_DEBUG_ECHO_MODE
    device_event_notify(DEVICE_EVENT_FROM_UAC, &event);
#endif

    speaker_stream_is_open[usb_id] = 1;

__err:
    return;
}

void uac_speaker_stream_close(const usb_dev usb_id)
{
    if (speaker_stream_is_open[usb_id] == 0) {
        return;
    }

    log_info("%s", __func__);

    spin_lock(&uac_lock);
    speaker_stream_is_open[usb_id] = 0;
    spin_unlock(&uac_lock);

    if (uac_speaker[usb_id]) {
#if USB_MALLOC_ENABLE
        if (uac_speaker[usb_id]->buffer) {
            free(uac_speaker[usb_id]->buffer);
        }
        free(uac_speaker[usb_id]);
#endif
        uac_speaker[usb_id] = NULL;
    }

    struct device_event event = {0};
    event.event = USB_AUDIO_PLAY_CLOSE;
    event.arg = (void *)(int)usb_id;
    device_event_notify(DEVICE_EVENT_FROM_UAC, &event);
}

int uac_get_spk_vol(void)
{
    extern u8 get_max_sys_vol(void);
    extern s8 app_audio_get_volume(u8 state);
    int max_vol = get_max_sys_vol();
    int vol = app_audio_get_volume(APP_AUDIO_STATE_MUSIC);
    if (vol * 100 / max_vol < 100) {
        return vol * 100 / max_vol;
    } else {
        return 99;
    }
    return 0;
}

static u8 mic_stream_is_open[USB_MAX_HW_NUM];

void uac_mute_volume(const usb_dev usb_id, u32 type, u32 l_vol, u32 r_vol)
{
    struct device_event event = {0};

    static u32 last_spk_l_vol = (u32) - 1, last_spk_r_vol = (u32) - 1;
    static u32 last_mic_vol = (u32) - 1;

    switch (type) {
    case MIC_FEATURE_UNIT_ID: //MIC
        if (mic_stream_is_open[usb_id] == 0) {
            return;
        }
        if (l_vol == last_mic_vol) {
            return;
        }
        last_mic_vol = l_vol;
        event.event = USB_AUDIO_SET_MIC_VOL;
        break;
    case SPK_FEATURE_UNIT_ID: //SPK
        if (speaker_stream_is_open[usb_id] == 0) {
            return;
        }
        if (l_vol == last_spk_l_vol && r_vol == last_spk_r_vol) {
            return;
        }
        last_spk_l_vol = l_vol;
        last_spk_r_vol = r_vol;
        event.event = USB_AUDIO_SET_PLAY_VOL;
        break;
    default:
        break;
    }

    event.value = (int)(r_vol << 16 | l_vol);
    event.arg = (void *)(int)usb_id;
    device_event_notify(DEVICE_EVENT_FROM_UAC, &event);
}

static int (*mic_tx_handler[USB_MAX_HW_NUM])(void *, void *, int);
static void *mic_tx_priv[USB_MAX_HW_NUM];

int uac_mic_stream_read(const usb_dev usb_id, u8 *buf, u32 len)
{
    if (mic_stream_is_open[usb_id] == 0) {
        return 0;
    }
#if 0//48K 1ksin
    const s16 sin_48k[] = {
        0, 2139, 4240, 6270, 8192, 9974, 11585, 12998,
        14189, 15137, 15826, 16244, 16384, 16244, 15826, 15137,
        14189, 12998, 11585, 9974, 8192, 6270, 4240, 2139,
        0, -2139, -4240, -6270, -8192, -9974, -11585, -12998,
        -14189, -15137, -15826, -16244, -16384, -16244, -15826, -15137,
        -14189, -12998, -11585, -9974, -8192, -6270, -4240, -2139
    };
    u16 *l_ch = (u16 *)buf;
    u16 *r_ch = (u16 *)buf;
    r_ch++;
    for (int i = 0; i < len / 2; i++) {
        *l_ch = sin_48k[i];
        *r_ch = sin_48k[i];
        l_ch += 1;
        r_ch += 1;
    }
    return len;
#elif   UAC_DEBUG_ECHO_MODE
    uac_speaker_read(NULL, buf, len);
#if MIC_CHANNEL == 2
    const s16 sin_48k[] = {
        0, 2139, 4240, 6270, 8192, 9974, 11585, 12998,
        14189, 15137, 15826, 16244, 16384, 16244, 15826, 15137,
        14189, 12998, 11585, 9974, 8192, 6270, 4240, 2139,
        0, -2139, -4240, -6270, -8192, -9974, -11585, -12998,
        -14189, -15137, -15826, -16244, -16384, -16244, -15826, -15137,
        -14189, -12998, -11585, -9974, -8192, -6270, -4240, -2139
    };
    u16 *r_ch = (u16 *)buf;
    r_ch++;
    for (int i = 0; i < len / 4; i++) {
        *r_ch = sin_48k[i];
        r_ch += 2;
    }
#endif
    return len;
#else
    if (mic_tx_handler[usb_id]) {
#if 1
        return mic_tx_handler[usb_id](mic_tx_priv[usb_id], buf, len);
#else
        //16bit ---> 24bit
        int rlen = mic_tx_handler[usb_id](mic_tx_priv[usb_id], tmp_buf, len / 3 * 2);
        rlen /= 2; //sampe point
        for (int i = 0 ; i < rlen ; i++) {
            buf[i * 3] = 0;
            buf[i * 3 + 1] = tmp_buf[i * 2];
            buf[i * 3 + 2] = tmp_buf[i * 2 + 1];
        }
#endif
    } else {
        putchar('N');
    }
    return 0;
#endif
    return 0;
}

void set_uac_mic_tx_handler(const usb_dev usb_id, void *priv, int (*tx_handler)(void *, void *, int))
{
    mic_tx_handler[usb_id] = tx_handler;
    mic_tx_priv[usb_id] = priv;
}

u32 uac_mic_stream_open(const usb_dev usb_id, u32 samplerate, u32 frame_len, u32 ch)
{
    if (mic_stream_is_open[usb_id]) {
        return 0;
    }

    log_info("%s", __func__);

    struct device_event event = {0};
    event.event = USB_AUDIO_MIC_OPEN;
    event.value = (int)((ch << 24) | samplerate);
    event.arg = (void *)(int)usb_id;

#if !UAC_DEBUG_ECHO_MODE
    device_event_notify(DEVICE_EVENT_FROM_UAC, &event);
#endif

    mic_stream_is_open[usb_id] = 1;

    return 0;
}

void uac_mic_stream_close(const usb_dev usb_id)
{
    if (mic_stream_is_open[usb_id] == 0) {
        return;
    }

    log_info("%s", __func__);

    struct device_event event = {0};
    event.event = USB_AUDIO_MIC_CLOSE;
    event.arg = (void *)(int)usb_id;
    mic_stream_is_open[usb_id] = 0;
    device_event_notify(DEVICE_EVENT_FROM_UAC, &event);
}

_WEAK_
s8 app_audio_get_volume(u8 state)
{
    return 88;
}
_WEAK_
void usb_audio_demo_exit(void)
{

}
_WEAK_
int usb_audio_demo_init(void)
{
    return 0;
}
_WEAK_
u8 get_max_sys_vol(void)
{
    return 100;
}
_WEAK_
void *audio_local_sample_track_open(u8 channel, int sample_rate, int period)
{
    return NULL;
}
_WEAK_
int audio_local_sample_track_in_period(void *c, int samples)
{
    return 0;
}
_WEAK_
void audio_local_sample_track_close(void *c)
{
}
#endif
