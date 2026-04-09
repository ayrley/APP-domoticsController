#ifndef __READER_TYPES_H_
#define __READER_TYPES_H_

enum readerLocationType {
    RDR_LOC_LOCAL = 0,
    RDR_LOC_IP
};

enum readerType {
    RDR_WIEGAND = 0,
    RDR_OSDP
};

enum ledColor {
    LED_NONE = 0,
    LED_RED,
    LED_GREEN,
    LED_YELLOW
};

#endif
