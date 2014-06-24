#include <kilolib.h>

#define PERIOD 32
uint8_t modulo_clock;
message_t msgs[PERIOD];
uint8_t offsets[PERIOD];
uint8_t total;
uint16_t average;

// send modulo_clock message
message_t *message_tx() {
    return &msgs[modulo_clock];
}

// receive modulo_clock message
void message_rx(message_t *msg, distance_measurement_t *d) {
    // store clock offset
    if (modulo_clock > msg->data[0]) {
        if (modulo_clock - msg->data[0] < PERIOD/2)
            offsets[modulo_clock-msg->data[0]]++;
    } else {
        if (msg->data[0] - modulo_clock > PERIOD/2)
            offsets[modulo_clock + (32-msg->data[0])]++;
    }
}

void setup() {
    for (int i = 0; i < PERIOD; i++) {
        msgs[i].data[0] = i;
        msgs[i].type = NORMAL;
        msgs[i].crc = message_crc(&msgs[i]);
        offsets[i] = 0;
    }
}

void loop() {
    modulo_clock = (kilo_ticks>>2)%PERIOD;
    if (!modulo_clock) {
        // blink LED
        set_color(RGB(0,1,0));
        delay(100);
        set_color(RGB(0,0,0));
        // compute averge offset
        total = 0;
        average = 0;
        for (int i = 0; i < PERIOD; i++) {
            average += i*offsets[i];
            total += offsets[i];
            offsets[i] = 0;
        }
        // adjust clock by average offset
        if (total > 0) {
            average /= total;
            kilo_ticks -= (average<<2);
        }
    }
}


int main() {
    // initialize hardware
    kilo_init();
    // register message callbacks
    kilo_message_rx = message_rx;
    kilo_message_tx = message_tx;
    // register your program
    kilo_start(setup, loop);

    return 0;
}
