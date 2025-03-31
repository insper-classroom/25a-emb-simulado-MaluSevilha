/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int TRIGGER1 = 19;
const int ECHO1 = 18;
const int TRIGGER2 = 13;
const int ECHO2 = 12;

volatile bool timer_fired1 = false, timer_fired2 = false;
volatile uint32_t start_us1, stop_us1, start_us2, stop_us2;

void pulso_trigger(int TRIGGER){
    gpio_put(TRIGGER, 1);
    sleep_us(10);
    gpio_put(TRIGGER, 0);
    sleep_us(2);
}

void echo_callback(uint gpio, uint32_t events) {
    if (gpio == ECHO1){
        if (events == 0x4) { 
            stop_us1 = to_us_since_boot(get_absolute_time());
        } else if (events == 0x8) { 
            start_us1 = to_us_since_boot(get_absolute_time());
        }
    } else if (gpio == ECHO2){
        if (events == 0x4) { 
            stop_us2 = to_us_since_boot(get_absolute_time());
        } else if (events == 0x8) { 
            start_us2 = to_us_since_boot(get_absolute_time());
        }
    }
}

int64_t alarm1_callback(alarm_id_t id, void *user_data) {
    timer_fired1 = true;
    return 0;
}

int64_t alarm2_callback(alarm_id_t id, void *user_data) {
    timer_fired2 = true;
    return 0;
}

int main() {
    stdio_init_all();
    alarm_id_t alarm1, alarm2;

    gpio_init(TRIGGER1);
    gpio_set_dir(TRIGGER1, GPIO_OUT);

    gpio_init(TRIGGER2);
    gpio_set_dir(TRIGGER2, GPIO_OUT);

    gpio_init(ECHO1);
    gpio_set_dir(ECHO1, GPIO_IN);
    gpio_set_irq_enabled_with_callback(ECHO1, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &echo_callback);

    gpio_init(ECHO2);
    gpio_set_dir(ECHO2, GPIO_IN);
    gpio_set_irq_enabled_with_callback(ECHO2, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &echo_callback);

    while (true) {
        double dist1, dist2;

        pulso_trigger(TRIGGER1);
        alarm1 = add_alarm_in_ms(500, alarm1_callback, NULL, false);

        pulso_trigger(TRIGGER2);
        alarm2 = add_alarm_in_ms(500, alarm2_callback, NULL, false);

        while((stop_us1 == 0 || stop_us2 == 0) && timer_fired1 == false && timer_fired2 == false){}

        if (!timer_fired1 && !timer_fired2){
            dist1 = ((stop_us1 - start_us1)*0.0343)/2;
            dist2 = ((stop_us2 - start_us2)*0.0343)/2;

            cancel_alarm(alarm1);
            cancel_alarm(alarm2);

            printf("Sensor 1 - %.0f cm\n", dist1);
            printf("Sensor 2 - %.0f cm\n", dist2);
        } else {
            printf("Sensor 1 - falha \n");
            printf("Sensor 2 - falha \n");
        }

        timer_fired1 = false;    
        timer_fired2 = false;
        stop_us1 = 0;
        stop_us2 = 0;
    }

    return 0;
}
