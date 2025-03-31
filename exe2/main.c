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

volatile bool timer_fired1 = false;
volatile bool timer_fired2 = false;
volatile uint32_t start_us1;
volatile uint32_t stop_us1; 
volatile uint32_t start_us2; 
volatile uint32_t stop_us2;

alarm_id_t alarm1;
alarm_id_t alarm2;

void pulso_trigger(int TRIGGER){
    gpio_put(TRIGGER, 1);
    sleep_us(10);
    gpio_put(TRIGGER, 0);
    sleep_us(2);
}

void echo_callback(uint gpio, uint32_t events) {
    if (gpio == ECHO1){
        if (events == 0x4) { 
            stop_us1 = get_absolute_time();
        } else if (events == 0x8) { 
            start_us1 = get_absolute_time();
            cancel_alarm(alarm1);
        }
    } 
    
    if (gpio == ECHO2){
        if (events == 0x4) { 
            stop_us2 = get_absolute_time();
        } else if (events == 0x8) { 
            start_us2 = get_absolute_time();
            cancel_alarm(alarm2);
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

    gpio_init(TRIGGER1);
    gpio_set_dir(TRIGGER1, GPIO_OUT);
    gpio_put(TRIGGER1, 0);

    gpio_init(TRIGGER2);
    gpio_set_dir(TRIGGER2, GPIO_OUT);
    gpio_put(TRIGGER2, 0);

    gpio_init(ECHO1);
    gpio_set_dir(ECHO1, GPIO_IN);
    gpio_set_irq_enabled_with_callback(ECHO1, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &echo_callback);

    gpio_init(ECHO2);
    gpio_set_dir(ECHO2, GPIO_IN);
    gpio_set_irq_enabled_with_callback(ECHO2, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &echo_callback);

    while (true) {
        int dist1;
        int dist2;

        timer_fired1 = false;    
        timer_fired2 = false;
        stop_us1 = 0;
        stop_us2 = 0;
        start_us1 = 0;
        start_us2 = 0;

        sleep_us(100);

        pulso_trigger(TRIGGER1);
        pulso_trigger(TRIGGER2);

        alarm1 = add_alarm_in_ms(20, alarm1_callback, NULL, false);
        alarm2 = add_alarm_in_ms(20, alarm2_callback, NULL, false);

        while(stop_us1 == 0 && timer_fired1 == false){}

        while(stop_us2 == 0 && timer_fired2 == false){}

        if (timer_fired1 == false){
            dist1 = (int)((stop_us1 - start_us1)*0.0343)/2;
            printf("Sensor 1 - %d cm\n", dist1);
        } else {
            printf("Sensor 1 - dist: erro\n");
        }

        if (timer_fired2 == false){
            dist2 = (int)((stop_us2 - start_us2)*0.0343)/2;
            printf("Sensor 2 - %d cm\n", dist2);
        } else {
            printf("Sensor 2 - dist: erro\n");
        }

        cancel_alarm(alarm1);
        cancel_alarm(alarm2);
    }

    return 0;
}
