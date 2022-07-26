// queue.c

#include "queue.h"
#include <stdlib.h>
#include <string.h>
#include <applibs/log.h>

#define printf Log_Debug

// queue UART

void qUART_init(str_qUART* device, unsigned char* buf, unsigned int depth)
{
    device->pool = buf;
    device->wp = 0;
    device->ovr = 0;
    device->rp = 0;
    device->depth = depth;
    
    memset(device->pool, 0, (device->depth));
}

unsigned int qUART_getlen(str_qUART* device)
{
    unsigned int wp = device->wp;
    unsigned int over = device->ovr;
    unsigned int rp = device->rp;
    unsigned int len;
    
    if (over)
    {
        len = (device->depth - rp) + wp;
    }
    else
    {
        len = wp - rp;
    }
    
    return len;
}

int qUART_1enqueue(str_qUART* device, unsigned char* input)
{
    if(device->depth == qUART_getlen(device))
    {
        // dequeue 1
        
        unsigned char temp;
        
        if(0 == qUART_dequeue(device, &temp, 1))
        {
            return -1;
        }
    }
    
    device->pool[device->wp++] = *input;
    
    if (device->wp == device->depth)
    {
        device->wp = 0;
        device->ovr += 1;
    }
    
    return 0;
}

int qUART_nenqueue(str_qUART* device, unsigned char* input, unsigned int len)
{
    // empty count
    unsigned int empty = device->depth - qUART_getlen(device);
    
    if(empty < len)
    {
        // dequeue len-empty
        
        unsigned int i;
        unsigned char temp;
        
        for(i=0; i<len - empty; i++)
        {
            if(0 == qUART_dequeue(device, &temp, 1))
            {
                return -1;
            }
        }
    }
    
    // check queue will overwrap
    if(device->wp + len > device->depth)
    {
        unsigned int rlen;

        rlen = device->depth - device->wp;
        memcpy(&(device->pool[device->wp]), input, rlen);

        memcpy(&(device->pool[0]), input+rlen, len-rlen);
        device->wp = len - rlen;
        device->ovr += 1;
    }
    else
    {
        memcpy(&(device->pool[device->wp]), input, len);
        device->wp += len;

        if (device->wp == device->depth)
        {
            device->wp = 0;
            device->ovr += 1;
        }
    }
    
    return 0;
}

unsigned int qUART_enqueue(str_qUART* device, unsigned char* input, unsigned int len)
{
    int ret;
    
    if(len == 1)
    {
        ret = qUART_1enqueue(device, input);
        
        if(ret != 0)
        {
            return 0;
        }
        return len;
    }
    
    if(len > device->depth)
    {
        unsigned int rlen = device->depth;
        unsigned int slen = 0;
        
        do
        {
            ret = qUART_nenqueue(device, input+slen, rlen);
            if(ret != 0)
            {
                return slen;
            }
            
            slen += rlen;
            
            len -= rlen;
            if(len > device->depth)
            {
                rlen = device->depth;
            }
            else
            {
                rlen = len;
            }
        }
        while(len != 0);
        
        return slen;
    }
    
    ret = qUART_nenqueue(device, input, len);
    if(ret != 0)
    {
        return 0;
    }
    
    return len;
}

void qUART_1dequeue(str_qUART* device, unsigned char* output)
{
    memcpy(output, &device->pool[device->rp++], (unsigned int)sizeof(device->pool[0]));
    
    if (device->rp == device->depth)
    {
        device->rp = 0;
        device->ovr -= 1;
    }
}

void qUART_ndequeue(str_qUART* device, unsigned char* output, unsigned int len)
{

    memcpy(output, &device->pool[device->rp], len);
    
    device->rp += len;
        
    if (device->rp == device->depth)
    {
        device->rp = 0;
        device->ovr -= 1;
    }
}

unsigned int qUART_dequeue(str_qUART* device, unsigned char* output, unsigned int len)
{
    unsigned int rlen;
    
    if (output == NULL)
    {
        // Error return 0
        return 0;
    }
    
    rlen = qUART_getlen(device);
    
    if (len > rlen)
    {
        len = rlen;
    }
    rlen = 0;
    
    if(device->ovr)
    {
        rlen = (device->depth)-(device->rp);
        
        if(len > rlen)
        {
            // dequeue from device->rp to device->depth
            qUART_ndequeue(device, output, rlen);
            
            // dequeue from 0 to device->wp
            qUART_ndequeue(device, output+rlen, len-rlen);
        }
        else
        {
            rlen = 0;
        }
    }
    
    if(rlen == 0)
    {
        qUART_ndequeue(device, output, len);
    }
    
    return len;
}

int qUART_ncp(str_qUART* device, unsigned char* output, unsigned int len, unsigned int from)
{
    if(from + len > device->depth)
    {
        return -1;
    }
    
    memcpy(output, &device->pool[from], len);
    
    return 0;
}

unsigned int qUART_cp(str_qUART* device, unsigned char* output, unsigned int len)
{
    unsigned int rlen;
    
    if (output == NULL)
    {
        // Error return 0
        return 0;
    }
        
    rlen = qUART_getlen(device);
    
    if (len > rlen)
    {
        len = rlen;
    }
    rlen = 0;
    
    if(device->ovr)
    {
        rlen = (device->depth)-(device->rp);
        
        if(len > rlen)
        {
            // dequeue from device->rp to device->depth
            qUART_ncp(device, output, rlen, device->rp);
            
            // dequeue from 0 to device->wp
            qUART_ncp(device, output+rlen, len-rlen, 0);
        }
        else
        {
            rlen = 0;
        }
    }
    
    if(rlen == 0)
    {
        qUART_ncp(device, output, len, device->rp);
    }
    
    return len;
}

void qUART_disp(str_qUART* device)
{
    unsigned int i;
    
    for(i=0; i<device->depth; i++)
    {
        if(i%16 == 0)
        {
            printf("0x%.8x : ", i);
        }
        #if 0
        printf("%c ", device->pool[i]);
        #else
        printf("0x%.2x ", device->pool[i]);
        #endif
        if(i!=0 && i%16 == 15)
        {
            printf("\r\n");
        }
    }
    printf("\r\n");
}

void qElement_init(str_qElement *device, str_Element *buf, unsigned int depth)
{
    device->pool = buf;
    device->wp = 0;
    device->ovr = 0;
    device->rp = 0;
    device->depth = depth;

    memset(device->pool, 0, (device->depth) * sizeof(str_Element));
}

unsigned int qElement_getlen(str_qElement *device)
{
    unsigned int wp = device->wp;
    unsigned int over = device->ovr;
    unsigned int rp = device->rp;
    unsigned int len;

    if (over) {
        len = (device->depth - rp) + wp;
    } else {
        len = wp - rp;
    }

    return len;
}

int qElement_1enqueue(str_qElement *device, str_Element *input)
{
    if (device->depth == qElement_getlen(device)) {
        // dequeue 1

        str_Element temp;

        if (0 == qElement_dequeue(device, &temp, 1)) {
            return -1;
        }
    }

    device->pool[device->wp++] = *input;

    if (device->wp == device->depth) {
        device->wp = 0;
        device->ovr += 1;
    }

    return 0;
}

int qElement_nenqueue(str_qElement *device, str_Element *input, unsigned int len)
{
    // empty count
    unsigned int empty = device->depth - qElement_getlen(device);

    if (empty < len) {
        // dequeue len-empty

        unsigned int i;
        str_Element temp;

        for (i = 0; i < len - empty; i++) {
            if (0 == qElement_dequeue(device, &temp, 1)) {
                return -1;
            }
        }
    }

    // check queue will overwrap
    if (device->wp + len > device->depth) {
        unsigned int rlen;

        rlen = device->depth - device->wp;
        memcpy((unsigned char *)&(device->pool[device->wp]), (unsigned char *)input,
               rlen * sizeof(str_Element));

        memcpy((unsigned char *)&(device->pool[0]), (unsigned char *)input + rlen,
               (len - rlen) * sizeof(str_Element));
        device->wp = len - rlen;
        device->ovr += 1;
    } else {
        memcpy((unsigned char *)&(device->pool[device->wp]), (unsigned char *)input,
               len * sizeof(str_Element));
        device->wp += len;

        if (device->wp == device->depth) {
            device->wp = 0;
            device->ovr += 1;
        }
    }

    return 0;
}

unsigned int qElement_enqueue(str_qElement *device, str_Element *input, unsigned int len)
{
    int ret;

    if (len == 1) {
        ret = qElement_1enqueue(device, input);

        if (ret != 0) {
            return 0;
        }
        return len;
    }

    if (len > device->depth) {
        unsigned int rlen = device->depth;
        unsigned int slen = 0;

        do {
            ret = qElement_nenqueue(device, input + slen, rlen);
            if (ret != 0) {
                return slen;
            }

            slen += rlen;

            len -= rlen;
            if (len > device->depth) {
                rlen = device->depth;
            } else {
                rlen = len;
            }
        } while (len != 0);

        return slen;
    }

    ret = qElement_nenqueue(device, input, len);
    if (ret != 0) {
        return 0;
    }

    return len;
}

void qElement_1dequeue(str_qElement *device, str_Element *output)
{
    memcpy((unsigned char *)output, (unsigned char *)&device->pool[device->rp++],
           (unsigned int)sizeof(device->pool[0]));

    if (device->rp == device->depth) {
        device->rp = 0;
        device->ovr -= 1;
    }
}

void qElement_ndequeue(str_qElement *device, str_Element *output, unsigned int len)
{
    memcpy((unsigned char *)output, (unsigned char *)&device->pool[device->rp],
           len * sizeof(str_Element));

    device->rp += len;

    if (device->rp == device->depth) {
        device->rp = 0;
        device->ovr -= 1;
    }
}

unsigned int qElement_dequeue(str_qElement *device, str_Element *output, unsigned int len)
{
    unsigned int rlen;

    if (output == NULL) {
        // Error return 0
        return 0;
    }

    rlen = qElement_getlen(device);

    if (len > rlen) {
        len = rlen;
    }
    rlen = 0;

    if (device->ovr) {
        rlen = (device->depth) - (device->rp);

        if (len > rlen) {
            // dequeue from device->rp to device->depth
            qElement_ndequeue(device, output, rlen);

            // dequeue from 0 to device->wp
            qElement_ndequeue(device, output + rlen, len - rlen);
        } else {
            rlen = 0;
        }
    }

    if (rlen == 0) {
        qElement_ndequeue(device, output, len);
    }

    return len;
}

int qElement_ncp(str_qElement *device, str_Element *output, unsigned int len, unsigned int from)
{
    if (from + len > device->depth) {
        return -1;
    }

    memcpy((unsigned char *)output, (unsigned char *)&device->pool[from],
           len * sizeof(str_Element));

    return 0;
}

unsigned int qElement_cp(str_qElement *device, str_Element *output, unsigned int len)
{
    unsigned int rlen;

    if (output == NULL) {
        // Error return 0
        return 0;
    }

    rlen = qElement_getlen(device);

    if (len > rlen) {
        len = rlen;
    }
    rlen = 0;

    if (device->ovr) {
        rlen = (device->depth) - (device->rp);

        if (len > rlen) {
            // dequeue from device->rp to device->depth
            qElement_ncp(device, output, rlen, device->rp);

            // dequeue from 0 to device->wp
            qElement_ncp(device, output + rlen, len - rlen, 0);
        } else {
            rlen = 0;
        }
    }

    if (rlen == 0) {
        qElement_ncp(device, output, len, device->rp);
    }

    return len;
}

unsigned int qElement_increase(str_qElement *device)
{
    device->rp++;

    if (device->rp == device->depth) {
        device->rp = 0;
        device->ovr -= 1;
    }

    return device->rp;
}

void qElement_disp(str_qElement *device)
{
    //#define DBG_qElement_DISP_INFO

    #if 1
    unsigned int i, j;
    #endif

#ifdef DBG_qElement_DISP_INFO
    printf("Id : %s, ", device->id);
    printf("rp : %d, ", device->rp);
    printf("wp : %d\r\n", device->wp);
#endif

    str_Element *p;

    p = device->pool;
    p += device->rp;

    #if 1
    j = (device->rp);
    for (i = 0; i < qElement_getlen(device); i++) {
        printf("Enqueued %d : ", j + i);
        printf("%s,", p->element);
        p++;
    }
    #endif
}
