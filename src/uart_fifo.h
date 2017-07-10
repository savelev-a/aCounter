/*
 * uart_fifo.h
 *
 *  Created on: 14 июн. 2017 г.
 *      Author: alchemist
 */

#ifndef UART_FIFO_H_
#define UART_FIFO_H_

#define FIFO( size )\
  struct {\
    unsigned char buf[size];\
    unsigned char tail;\
    unsigned char head;\
  }

#define FIFO_IS_FULL(fifo)   ((fifo.head-fifo.tail)==sizeof(fifo.buf))

#define FIFO_IS_EMPTY(fifo)  (fifo.tail==fifo.head)

#define FIFO_COUNT(fifo)     (fifo.head-fifo.tail)

#define FIFO_SIZE(fifo)      ( (sizeof(fifo.buf)))

#define FIFO_SPACE(fifo)     (FIFO_SIZE(fifo)-FIFO_COUNT(fifo))

#define FIFO_PUSH(fifo, byte) \
  {\
    fifo.buf[fifo.head & (sizeof(fifo.buf)-1)]=byte;\
    fifo.head++;\
  }

#define FIFO_FRONT(fifo) (fifo.buf[(fifo).tail & (sizeof(fifo.buf)-1)])

#define FIFO_POP(fifo)   \
  {\
      fifo.tail++; \
  }

#define FIFO_FLUSH(fifo)   \
  {\
    fifo.tail=0;\
    fifo.head=0;\
  }

#endif /* UART_FIFO_H_ */
