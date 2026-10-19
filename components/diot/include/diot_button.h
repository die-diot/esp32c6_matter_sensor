#ifndef DIOT_BUTTON_H
#define DIOT_BUTTON_H

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_err.h"
#include "fsm.h"
#include "iot_button.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*diot_button_callback_t)(uint32_t duration_ms, void *arg);

typedef struct {
    fsm_t fsm;
    gpio_num_t gpio;
    bool active_low;
    uint32_t debounce_time_ms;
    uint32_t long_press_time_ms;
    uint32_t duration;
    volatile bool pending_event;
    volatile uint32_t pending_duration;
    button_handle_t handle;
    diot_button_callback_t callback;
    void *callback_arg;
} diot_button_fsm_t;

diot_button_fsm_t *diot_button_fsm_new(void);
void diot_button_fsm_destroy(diot_button_fsm_t *const p_button_fsm);
void diot_button_fsm_init(diot_button_fsm_t *const p_button_fsm);
void diot_button_fsm_register_callback(diot_button_fsm_t *const p_button_fsm,
                                       diot_button_callback_t callback, void *arg);

static inline void diot_button_fsm_fire(diot_button_fsm_t *const p_button_fsm)
{
    fsm_fire((fsm_t *)p_button_fsm);
}

#ifdef __cplusplus
}
#endif

#endif