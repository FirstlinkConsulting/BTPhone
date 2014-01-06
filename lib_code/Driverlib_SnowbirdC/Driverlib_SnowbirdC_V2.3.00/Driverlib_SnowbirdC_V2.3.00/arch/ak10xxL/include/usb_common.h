
#ifndef __USB_COMMON_H__
#define __USB_COMMON_H__

#include "anyka_cpu.h"
#include "anyka_types.h"

/** @{@name USB Define
 *    Define the USB character
 */
#define USB_ENDPOINT_MAX_NUM      3     //end point max num
#define EP0_MAX_PAK_SIZE        64      //end point 0 FIFO size
#define EP_BULK_MAX_PAK_SIZE    512      //end point bulk FIFO size

#define EP_BULK_HIGHSPEED_MAX_PAK_SIZE 512
#define EP_BULK_FULLSPEED_MAX_PAK_SIZE 64

#define EP_ISO_FULLSPEED_MAX_PAK_SIZE 0xc0
#define EP_ISO_HIGHSPEED_MAX_PAK_SIZE 512

/** @{@name Buffer length define
 *  Define buffer length of EPx
 */
#define     EP0_IBUF_MAX_LEN    64  //the max packet size end point 0 in buffer can take
#define     EP0_OBUF_MAX_LEN    64  //the max packet size end point 0 out buffer can take
#define     EP1_BUF_MAX_LEN     EP_BULK_MAX_PAK_SIZE   //the max packet size end point 1 buffer can take
#define     EP2_BUF_MAX_LEN     EP_BULK_MAX_PAK_SIZE  //the max packet size end point 2 buffer can take
#define     EP3_BUF_MAX_LEN     EP_BULK_MAX_PAK_SIZE  //the max packet size end point 3 buffer can take

#define USB_SUS_EN                  (1 << 2)
#define USB_REG_EN                  (1 << 1)

#define USB_EP0_INDEX             0
#define USB_EP1_INDEX             (1 << 0)
#define USB_EP2_INDEX             (1 << 1)
#define USB_EP3_INDEX             ((1 << 1)|(1 << 0))
#define USB_EP4_INDEX             (1 << 2)
#define USB_EP5_INDEX             ((1 << 2)|(1 << 0))
#define USB_EP6_INDEX             ((1 << 2)|(1 << 1))
#define USB_EP7_INDEX             ((1 << 2)|(1 << 1)|(1 << 0))

/* usb control and status register */
#define USB_REG_RXCSR1_RXSTALL    (1 << 6)
#define USB_REG_RXCSR1_REQPKT     (1 << 5)

#define USB_TXCSR_AUTOSET               0x80
#define USB_TXCSR_ISO                   0x40
#define USB_TXCSR_MODE1                 0x20
#define USB_TXCSR_DMAREQENABLE          0x10
#define USB_TXCSR_FRCDATATOG            0x8
#define USB_TXCSR_DMAREQMODE1           0x4
#define USB_TXCSR_DMAREQMODE0           0x0


#define USB_RXCSR_AUTOSET               0x80
#define USB_RXCSR_ISO                   0x40
#define USB_RXCSR_DMAREQENAB            0x20
#define USB_RXCSR_DISNYET               0x10
#define USB_RXCSR_DMAREQMODE1           0x8
#define USB_RXCSR_DMAREQMODE0           0x0
#define USB_RXCSR_INCOMPRX              0x1

/**  USB DMA */
#define USB_DMA_INTR                    (USB_MODULE_BASE_ADDR + 0x0200)
#define USB_DMA_CNTL_1                  (USB_MODULE_BASE_ADDR + 0x0204)
#define USB_DMA_ADDR_1                  (USB_MODULE_BASE_ADDR + 0x0208)
#define USB_DMA_COUNT_1                 (USB_MODULE_BASE_ADDR + 0x020c)
#define USB_DMA_CNTL_2                  (USB_MODULE_BASE_ADDR + 0x0214)
#define USB_DMA_ADDR_2                  (USB_MODULE_BASE_ADDR + 0x0218)
#define USB_DMA_COUNT_2                 (USB_MODULE_BASE_ADDR + 0x021c)

// Power Control  Register ~ 0x04070001
#define USB_POWER_ENSUSPEND         (1 << 0)
#define USB_POWER_SUSPENDM          (1 << 1)
#define USB_POWER_RESUME            (1 << 2)
#define USB_POWER_RESET             (1 << 3)
#define USB_POWER_HSMODE            (1 << 4)
#define USB_POWER_HSENABLE          (1 << 5)

//Interrupt Register ~
#define USB_INTR_EP0              (1 << 0)
#define USB_INTR_EP1              (1 << 1)
#define USB_INTR_EP2              (1 << 2)
#define USB_INTR_EP3              (1 << 3)

// Interrupt Enable Register for Endpoint ~ 
#define USB_EP0_ENABLE              (1 << 0)
#define USB_EP1_TX_ENABLE           (1 << 1)
#define USB_EP2_TX_ENABLE           (1 << 2)
#define USB_EP3_TX_ENABLE           (1 << 3)

#define USB_EP1_RX_ENABLE           (1 << 1)
#define USB_EP2_RX_ENABLE           (1 << 2)
#define USB_EP3_RX_ENABLE           (1 << 3)

// Common USB Interrupt
#define USB_INTR_SUSPEND          (1 << 0)
#define USB_INTR_RESUME           (1 << 1)
#define USB_INTR_RESET            (1 << 2)
#define USB_INTR_SOF              (1 << 3)
#define USB_INTR_CONNECT          (1 << 4)
#define USB_INTR_DISCONNECT       (1 << 5)

// Interrupt Enable for common usb interrupt
#define USB_INTR_SUSPEND_ENA      (1 << 0)
#define USB_INTR_RESUME_ENA       (1 << 1)
#define USB_INTR_RESET_ENA        (1 << 2)
#define USB_INTR_SOF_ENA          (1 << 3)
#define USB_INTR_CONNECT_ENA      (1 << 4)
#define USB_INTR_DISCONNECT_ENA   (1 << 5)

//Write Buffer Forbidden Register
#define USB_EP0_FORBID_WRITE        (1 << 0)
#define USB_EP1_FORBID_WRITE        (1 << 1)
#define USB_EP2_FORBID_WRITE        (1 << 2)

//Pre-read Data Start Register
#define USB_EP0_START_PRE_READ      (1 << 0)
#define USB_EP1_START_PRE_READ      (1 << 1)
#define USB_EP2_START_PRE_READ      (1 << 2)


//Control and Status Register

//ep 0
#define USB_CSR0_RXPKTRDY         (1 << 0)
#define USB_CSR0_TXPKTRDY         (1 << 1)

#define USB_CSR0_P_SENTSTALL      (1 << 2)
#define USB_CSR0_P_DATAEND        (1 << 3)
#define USB_CSR0_P_SETUPEND       (1 << 4)
#define USB_CSR0_P_SENDSTALL      (1 << 5)
#define USB_CSR0_P_SVDRXPKTRDY    (1 << 6)
#define USB_CSR0_P_SVDSETUPEND    (1 << 7)

#define USB_CSR0_FLUSHFIFO        0x1

//ep 1&2
//TX
#define USB_TXCSR1_TXPKTRDY       (1 << 0)
#define USB_TXCSR1_FIFONOTEMPTY   (1 << 1)
#define USB_TXCSR1_FLUSHFIFO      (1 << 3)
#define USB_TXCSR1_CLRDATATOG     (1 << 6)

#define USB_TXCSR1_P_UNDERRUN     (1 << 2)
#define USB_TXCSR1_P_SENDSTALL    (1 << 4)
#define USB_TXCSR1_P_SENTSTALL    (1 << 5)

#define USB_TXCSR2_FRCDT          (1 << 3)
#define USB_TXCSR2_DMAMODE        (1 << 2)
#define USB_TXCSR2_DMAENAB        (1 << 4)
#define USB_TXCSR2_MODE           (1 << 5)
#define USB_TXCSR2_ISO            (1 << 6)
#define USB_TXCSR2_AUTOSET        (1 << 7)


//RX
#define USB_RXCSR1_RXPKTRDY       (1 << 0)
#define USB_RXCSR1_FIFOFULL       (1 << 1)
#define USB_RXCSR1_NAKTIMEOUT     (1 << 3)
#define USB_RXCSR1_FLUSHFIFO      (1 << 4)
#define USB_RXCSR1_CLRDATATOG     (1 << 7)

#define USB_RXCSR1_P_OVERRUN      (1 << 2) //0x04
#define USB_RXCSR1_P_SENDSTALL    (1 << 5) //0x20
#define USB_RXCSR1_P_SENTSTALL    (1 << 6) //0x40

#define USB_RXCSR2_AUTOCLEAR      (1 << 7)
#define USB_RXCSR2_DMAENAB        (1 << 5)
#define USB_RXCSR2_DISNYET        (1 << 4)
#define USB_RXCSR2_DMAMODE        (1 << 3)
#define USB_RXCSR2_INCOMPRX       (1 << 0)


//DMA interrupt status
#define DMA_CHANNEL1_INT            (1 << 0)
#define DMA_CHANNEL2_INT            (1 << 1)

#define USB_ENABLE_DMA                  1
#define USB_DIRECTION_RX                (0<<1)
#define USB_DIRECTION_TX                (1<<1)
#define USB_DMA_MODE1                   (1<<2)
#define USB_DMA_MODE0                   (0<<2)
#define USB_DMA_INT_ENABLE              (1<<3)
#define USB_DMA_INT_DISABLE             (0<<3)
#define USB_DMA_BUS_ERROR               (1<<8)
#define USB_DMA_BUS_MODE0               (0<<9)
#define USB_DMA_BUS_MODE1               (1<<9)
#define USB_DMA_BUS_MODE2               (2<<9)
#define USB_DMA_BUS_MODE3               (3<<9)


#endif

