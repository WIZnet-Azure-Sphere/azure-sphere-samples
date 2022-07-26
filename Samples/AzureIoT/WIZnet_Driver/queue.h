// queue.h

#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef struct _str_qUART
{
    unsigned char* pool;
    unsigned int wp;
    unsigned int ovr;
    unsigned int rp;
    unsigned int depth;
}str_qUART;

typedef struct _str_Element {
    #if 1
    unsigned char element[sizeof("{\"accel_amp_x\": 0.0044, \"accel_amp_y\": 0.0023, \"accel_amp_z\": 0.0065, \"temperature\": 28.7593}")];
    #else
    unsigned char element[sizeof("{\
        \"accel_amp_x\" : 0.0044,\
        \"accel_amp_y\" : 0.0023,\
        \"accel_amp_z\" : 0.0065,\
        \"temperature\" : 28.7593\
    } ")];
    #endif
} str_Element;

typedef struct _str_qElement {
    str_Element *pool;
    unsigned int wp;
    unsigned int ovr;
    unsigned int rp;
    unsigned int depth;
} str_qElement;

//#define DEBUG_QUART_INIT
//#define DEBUG_QUART_ENQUEUE
//#define DEBUG_QUART_DEQUEUE
//#define DEBUG_QUART_DISP

// queue UART

void qUART_init(str_qUART* device, unsigned char* buf, unsigned int depth);

unsigned int qUART_getlen(str_qUART* device);

int qUART_1enqueue(str_qUART* device, unsigned char* input);
int qUART_nenqueue(str_qUART* device, unsigned char* input, unsigned int len);
unsigned int qUART_enqueue(str_qUART* device, unsigned char* input, unsigned int len);

void qUART_1dequeue(str_qUART* device, unsigned char* output);
void qUART_ndequeue(str_qUART* device, unsigned char* output, unsigned int len);
unsigned int qUART_dequeue(str_qUART* device, unsigned char* output, unsigned int len);

int qUART_ncp(str_qUART* device, unsigned char* output, unsigned int len, unsigned int from);
unsigned int qUART_cp(str_qUART* device, unsigned char* output, unsigned int len);

void qUART_disp(str_qUART* device);

// queue Element
void qElement_init(str_qElement *device, str_Element *buf, unsigned int depth);

unsigned int qElement_getlen(str_qElement *device);

int qElement_1enqueue(str_qElement *device, str_Element *input);
int qElement_nenqueue(str_qElement *device, str_Element *input, unsigned int len);
unsigned int qElement_enqueue(str_qElement *device, str_Element *input, unsigned int len);

void qElement_1dequeue(str_qElement *device, str_Element *output);
void qElement_ndequeue(str_qElement *device, str_Element *output, unsigned int len);
unsigned int qElement_dequeue(str_qElement *device, str_Element *output, unsigned int len);

int qElement_ncp(str_qElement *device, str_Element *output, unsigned int len, unsigned int from);
unsigned int qElement_cp(str_qElement *device, str_Element *output, unsigned int len);

unsigned int qElement_increase(str_qElement *device);

void qElement_disp(str_qElement *device);
#endif
