#include "init.h"
#include "asm/p33.h"
#include "asm/system_reset_reason.h"

static int sys_rst_rs = 0;//0-15为唤醒原因，16-23位IO唤醒组号
static int system_reset_reason_check(void)
{
    int reset_flag = JL_SYSTEM->RST_SRC;
    u8 p3_rst = p33_rx_1byte(P3_RST_SRC);
    u8 r3_wkup = p33_rx_1byte(R3_WKUP_SRC);
    u8 port_wkup = p33_rx_1byte(P3_WKUP_PND);
    u8 p3_pmu = p33_rx_1byte(P3_PMU_CON1);
    int rst_src = 0;
    char rsn[16];

#if 0
    printf("--->RST_SRC = 0x%x \n", reset_flag);
    printf("--->R3_WKUP_SRC = 0x%x \n", r3_wkup);
    printf("--->P3_RST_SRC = 0x%x \n", p3_rst);
    printf("--->P3_PMU_CON1 = 0x%x \n", p3_pmu);
    printf("--->P3_WKUP_PND = 0x%x \n", port_wkup);
#endif

    if (r3_wkup & BIT(0)) {
        rst_src |= SYS_RST_ALM_WKUP;
        strcpy(rsn, SYS_ALM_WAKUP);
    } else if ((r3_wkup & BIT(3)) || (port_wkup && port_wkup != 0XFF) || (reset_flag & BIT(6))) {
        rst_src |= SYS_RST_PORT_WKUP;
        rst_src |= (port_wkup && port_wkup != 0XFF) ? (port_wkup << 16) : 0;
        strcpy(rsn, SYS_PORT_WAKUP);
    } else if ((reset_flag & BIT(5)) || (p3_rst & BIT(6))) {
        rst_src |= SYS_RST_SOFT;
        strcpy(rsn, SYS_SOFT);
    } else {
        if (p3_rst & BIT(0)) {
            rst_src |= SYS_RST_VDDIO_PWR_ON;
            strcpy(rsn, SYS_POWER_ON);
        }
        if (p3_rst & BIT(1)) {
            rst_src |= SYS_RST_VDDIO_LOW_PWR;
            strcpy(rsn, SYS_LOW_POWER);
        }
        if (p3_rst & BIT(2)) {
            rst_src |= SYS_RST_WDT;
            strcpy(rsn, SYS_WDT);
        }
        if (p3_rst & BIT(3)) {
            rst_src |= SYS_RST_VCM;
            strcpy(rsn, SYS_VCM);
        }
        if (p3_rst & BIT(4)) {
            rst_src |= SYS_RST_LONG_PRESS;
            strcpy(rsn, SYS_LONG_PRESS);
        }
        if (p3_rst & BIT(5)) {
            rst_src |= SYS_RST_12V;
            strcpy(rsn, SYS_SYS_12V);
        }
    }
    p33_or_1byte(P3_PMU_CON1, BIT(6));//清空上电标记
    printf("\n========= system reset reason: %s ========= \n", rsn);
    sys_rst_rs = rst_src;
    return rst_src;
}

int system_reset_reason_get(void)
{
    if (!sys_rst_rs) {
        sys_rst_rs = system_reset_reason_check();
    }
    return sys_rst_rs & 0xFFFF;
}
int system_wakeup_reset(void)
{
    if (!sys_rst_rs) {
        sys_rst_rs = system_reset_reason_check();
    }
    if (sys_rst_rs & (SYS_RST_PORT_WKUP | SYS_RST_ALM_WKUP | SYS_RST_VDDIO_PWR_ON)) {
        return true;
    }
    return false;
}
int system_wakeup_port_get(void)
{
    if (!sys_rst_rs) {
        sys_rst_rs = system_reset_reason_check();
    }
    if (sys_rst_rs & SYS_RST_PORT_WKUP) {
        if (p33_rx_1byte(P3_PINR_CON) & BIT(0)) { //有开长按唤醒则IO唤醒从1-7
            return (sys_rst_rs >> 17) & 0xFF;
        } else {
            return (sys_rst_rs >> 16) & 0xFF;
        }
    }
    return -EINVAL;
}


