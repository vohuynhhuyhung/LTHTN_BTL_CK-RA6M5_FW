/* generated HAL source file - do not edit */
#include "hal_data.h"
/* I2C Communication Device */
rm_comms_i2c_instance_ctrl_t g_comms_i2c_device0_ctrl;

/* Lower level driver configuration */
const i2c_master_cfg_t g_comms_i2c_device0_lower_level_cfg =
{ .slave = 0x32, .addr_mode = I2C_MASTER_ADDR_MODE_7BIT, .p_callback = rm_comms_i2c_callback, };

const rm_comms_cfg_t g_comms_i2c_device0_cfg =
{ .semaphore_timeout = 0xFFFFFFFF, .p_lower_level_cfg = (void*) &g_comms_i2c_device0_lower_level_cfg, .p_extend =
          (void*) &g_comms_i2c_bus0_extended_cfg,
  .p_callback = rm_zmod4xxx_comms_i2c_callback,
#if defined(g_zmod4xxx_sensor0_ctrl)
    .p_context          = g_zmod4xxx_sensor0_ctrl,
#else
  .p_context = (void*) &g_zmod4xxx_sensor0_ctrl,
#endif
        };

const rm_comms_instance_t g_comms_i2c_device0 =
{ .p_ctrl = &g_comms_i2c_device0_ctrl, .p_cfg = &g_comms_i2c_device0_cfg, .p_api = &g_comms_on_comms_i2c, };
zmod4xxx_dev_t g_zmod4xxx_sensor0_dev;
iaq_2nd_gen_handle_t g_zmod4xxx_sensor0_lib_handle;
iaq_2nd_gen_results_t g_zmod4xxx_sensor0_lib_results;
uint8_t g_zmod4xxx_sensor0_product_data[7];
extern rm_zmod4xxx_api_t const g_zmod4xxx_on_zmod4410_iaq_2nd_gen;
extern zmod4xxx_conf g_zmod4410_iaq_2nd_gen_sensor_type[];
rm_zmod4xxx_lib_extended_cfg_t g_zmod4xxx_sensor0_extended_cfg =
{ .lib_type = RM_ZMOD4410_LIB_TYPE_IAQ_2ND_GEN,
  .product_id = 0x2310,
  .p_api = (void*) &g_zmod4xxx_on_zmod4410_iaq_2nd_gen,
  .p_data_set = (void*) g_zmod4410_iaq_2nd_gen_sensor_type,
  .p_product_data = g_zmod4xxx_sensor0_product_data,
  .p_device = (void*) &g_zmod4xxx_sensor0_dev,
  .p_handle = (void*) &g_zmod4xxx_sensor0_lib_handle,
  .p_results = (void*) &g_zmod4xxx_sensor0_lib_results, };
rm_zmod4xxx_instance_ctrl_t g_zmod4xxx_sensor0_ctrl;
const rm_zmod4xxx_cfg_t g_zmod4xxx_sensor0_cfg =
{ .p_comms_instance = &g_comms_i2c_device0,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_irq_instance = NULL,
  .p_irq_callback = NULL,
#else
    .p_irq_instance = &RA_NOT_DEFINED,
    .p_irq_callback = zmod4xxx_irq_callback,
#endif
#undef RA_NOT_DEFINED
  .p_comms_callback = zmod4xxx_comms_i2c_callback,
#if defined(NULL)
    .p_context           = NULL,
#else
  .p_context = (void*) &NULL,
#endif
  .p_extend = (void*) &g_zmod4xxx_sensor0_extended_cfg, };

const rm_zmod4xxx_instance_t g_zmod4xxx_sensor0 =
{ .p_ctrl = &g_zmod4xxx_sensor0_ctrl, .p_cfg = &g_zmod4xxx_sensor0_cfg, .p_api = &g_zmod4xxx_on_zmod4xxx, };
sci_uart_instance_ctrl_t g_uart3_ctrl;

baud_setting_t g_uart3_baud_setting =
        {
        /* Baud rate calculated with 0.469% error. */.semr_baudrate_bits_b.abcse = 0,
          .semr_baudrate_bits_b.abcs = 0, .semr_baudrate_bits_b.bgdm = 1, .cks = 0, .brr = 53, .mddr = (uint8_t) 256, .semr_baudrate_bits_b.brme =
                  false };

/** UART extended configuration for UARTonSCI HAL driver */
const sci_uart_extended_cfg_t g_uart3_cfg_extend =
{ .clock = SCI_UART_CLOCK_INT, .rx_edge_start = SCI_UART_START_BIT_FALLING_EDGE, .noise_cancel =
          SCI_UART_NOISE_CANCELLATION_DISABLE,
  .rx_fifo_trigger = SCI_UART_RX_FIFO_TRIGGER_MAX, .p_baud_setting = &g_uart3_baud_setting, .flow_control =
          SCI_UART_FLOW_CONTROL_RTS,
#if 0xFF != 0xFF
                .flow_control_pin       = BSP_IO_PORT_FF_PIN_0xFF,
                #else
  .flow_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
  .rs485_setting =
  { .enable = SCI_UART_RS485_DISABLE, .polarity = SCI_UART_RS485_DE_POLARITY_HIGH,
#if 0xFF != 0xFF
                    .de_control_pin = BSP_IO_PORT_FF_PIN_0xFF,
                #else
    .de_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
          },
  .irda_setting =
  { .ircr_bits_b.ire = 0, .ircr_bits_b.irrxinv = 0, .ircr_bits_b.irtxinv = 0, }, };

/** UART interface configuration */
const uart_cfg_t g_uart3_cfg =
{ .channel = 3, .data_bits = UART_DATA_BITS_8, .parity = UART_PARITY_OFF, .stop_bits = UART_STOP_BITS_1, .p_callback =
          NULL,
  .p_context = NULL, .p_extend = &g_uart3_cfg_extend,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_tx = NULL,
#else
                .p_transfer_tx       = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_rx = NULL,
#else
                .p_transfer_rx       = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
  .rxi_ipl = (12),
  .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (12),
#if defined(VECTOR_NUMBER_SCI3_RXI)
                .rxi_irq             = VECTOR_NUMBER_SCI3_RXI,
#else
  .rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI3_TXI)
                .txi_irq             = VECTOR_NUMBER_SCI3_TXI,
#else
  .txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI3_TEI)
                .tei_irq             = VECTOR_NUMBER_SCI3_TEI,
#else
  .tei_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI3_ERI)
                .eri_irq             = VECTOR_NUMBER_SCI3_ERI,
#else
  .eri_irq = FSP_INVALID_VECTOR,
#endif
        };

/* Instance structure to use this module. */
const uart_instance_t g_uart3 =
{ .p_ctrl = &g_uart3_ctrl, .p_cfg = &g_uart3_cfg, .p_api = &g_uart_on_sci };
void g_hal_init(void)
{
    g_common_init ();
}
