/*
 * Copyright (c)
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date        Author    Email                    Notes
 * 2019-07-16  Kevin.Liu kevin.liu.mchp@gmail.com First Release
 */

#include <rtthread.h>
#include <rtdevice.h>

#include <atmel_start.h>

/* SAM MCU serial device */
static struct rt_serial_device sam_serial;

/**
 * @brief Configure serial port
 *
 * This function will configure UART baudrate, parity and so on.
 *
 * @return RT_EOK.
 */
static rt_err_t serial_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct usart_sync_descriptor* desc;

    RT_ASSERT(serial != RT_NULL);

    desc = (struct usart_sync_descriptor *)serial->parent.user_data;

    RT_ASSERT(desc != RT_NULL);
    RT_ASSERT(cfg  != RT_NULL);

    usart_sync_disable(desc);

    /* Set baudrate */
    usart_sync_set_baud_rate(desc, (const uint32_t)cfg->baud_rate);

    /* Set stop bit */
    if (cfg->stop_bits == STOP_BITS_1)
        usart_sync_set_stopbits(desc, USART_STOP_BITS_ONE);
    else if (cfg->stop_bits == STOP_BITS_2)
        usart_sync_set_stopbits(desc, USART_STOP_BITS_TWO);

    if (cfg->bit_order == BIT_ORDER_LSB)
        usart_sync_set_data_order(desc, USART_DATA_ORDER_LSB);
    else if (cfg->bit_order == BIT_ORDER_MSB)
        usart_sync_set_data_order(desc, USART_DATA_ORDER_MSB);

    /* Set character size */
    switch (cfg->data_bits)
    {
        case DATA_BITS_5:
            usart_sync_set_character_size(desc, USART_CHARACTER_SIZE_5BITS);
            break;
        case DATA_BITS_6:
            usart_sync_set_character_size(desc, USART_CHARACTER_SIZE_6BITS);
            break;
        case DATA_BITS_7:
            usart_sync_set_character_size(desc, USART_CHARACTER_SIZE_7BITS);
            break;
        case DATA_BITS_8:
            usart_sync_set_character_size(desc, USART_CHARACTER_SIZE_8BITS);
            break;
        case DATA_BITS_9:
            usart_sync_set_character_size(desc, USART_CHARACTER_SIZE_9BITS);
            break;
        default:
            break;
    }

    if (cfg->parity == PARITY_NONE)
        usart_sync_set_parity(desc, USART_PARITY_NONE);
    else if (cfg->parity == PARITY_ODD)
        usart_sync_set_parity(desc, USART_PARITY_ODD);
    else if (cfg->parity == PARITY_EVEN)
        usart_sync_set_parity(desc, USART_PARITY_EVEN);

    usart_sync_enable(desc);

    return RT_EOK;
}

/**
 * @brief Control serial port
 *
 * This function provide UART enable/disable control.
 *
 * @return RT_EOK.
 */
static rt_err_t serial_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct usart_sync_descriptor* desc;

    RT_ASSERT(serial != RT_NULL);
    desc = (struct usart_sync_descriptor *)serial->parent.user_data;

    RT_ASSERT(desc != RT_NULL);

    switch (cmd)
    {
        /* disable interrupt */
        case RT_DEVICE_CTRL_CLR_INT:
            usart_sync_disable(desc);
            break;
        /* enable interrupt */
        case RT_DEVICE_CTRL_SET_INT:
            usart_sync_enable(desc);
            break;
        /* UART config */
        case RT_DEVICE_CTRL_CONFIG :
            break;
    }

    return RT_EOK;
}

/**
 * @brief Serial sends a char
 *
 * This function will send a char to the UART
 *
 * @return 1.
 */
static int serial_putc(struct rt_serial_device *serial, char c)
{
    struct usart_sync_descriptor* desc;

    RT_ASSERT(serial != RT_NULL);
    desc = (struct usart_sync_descriptor *)serial->parent.user_data;

    RT_ASSERT(desc != RT_NULL);

    io_write(&desc->io, (const uint8_t *)&c, 1);

    return 1;
}

/**
 * @brief Serial gets a char
 *
 * This function will get a char from the UART
 *
 * @return received char character or -1 if no char received.
 */
static int serial_getc(struct rt_serial_device *serial)
{
    char c;
    int ch;
    struct usart_sync_descriptor* desc;

    RT_ASSERT(serial != RT_NULL);
    desc = (struct usart_sync_descriptor *)serial->parent.user_data;

    RT_ASSERT(desc != RT_NULL);

    ch = -1;
    if (usart_sync_is_rx_not_empty(desc))
    {
        io_read(&desc->io, (uint8_t *)&c, 1);;
        ch = c & 0xff;
    }

    return ch;
}

static const struct rt_uart_ops sam_serial_ops =
{
    serial_configure,
    serial_control,
    serial_putc,
    serial_getc,
};

/**
 * @brief Initialize the UART
 *
 * This function initialize the UART
 *
 * @return None.
 */
int rt_hw_uart_init(void)
{
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    sam_serial.ops       = &sam_serial_ops;
    sam_serial.config    = config;
    sam_serial.serial_rx = RT_NULL;
    sam_serial.serial_rx = RT_NULL;
    rt_hw_serial_register(&sam_serial, "uart0",
                          RT_DEVICE_FLAG_RDWR, (void *)&TARGET_IO);

    return 0;
}

/*@}*/
