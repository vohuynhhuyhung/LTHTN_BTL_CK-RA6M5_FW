#include "s_i2c.h"
#include "hal_data.h"

void i2c0_register_init(void) {
    // 3. Cấu hình các chân P400, P401 về chế độ I2C (Quan trọng!)
    R_IOPORT_PinCfg(&g_ioport_ctrl, BSP_IO_PORT_04_PIN_00,
                    (uint32_t)(IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_IIC | IOPORT_CFG_NMOS_ENABLE));
    R_IOPORT_PinCfg(&g_ioport_ctrl, BSP_IO_PORT_04_PIN_01,
                    (uint32_t)(IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_IIC | IOPORT_CFG_NMOS_ENABLE));

    // 1. Cấp xung cho IIC0
    R_BSP_MODULE_START(FSP_IP_IIC, 0);

    // 2. Reset module
    R_IIC0->ICCR1 = 0x00;
    R_IIC0->ICCR1 = 0x40; // I2C reset
    for(volatile int i=0; i<100; i++);
    R_IIC0->ICCR1 = 0xC0; // Internal reset

    // 3. Thiết lập tốc độ 100kHz (Giả định PCLKB = 50MHz)
    R_IIC0->ICMR1 = 0x30; // CKS = 3
    R_IIC0->ICBRH = 28;
    R_IIC0->ICBRL = 28;

    // 4. Bật module
    R_IIC0->ICCR1 = 0x80;
}

void i2c0_write_reg(uint8_t slave_addr, uint8_t reg, uint8_t data) {
    // 1. Tạo điều kiện START
    R_IIC0->ICCR2 |= 0x02;

    // 2. Gửi địa chỉ Slave (Write: bit cuối = 0)
    while(!(R_IIC0->ICSR2 & 0x80)); // Đợi ICDRT trống (TDRE)
    R_IIC0->ICDRT = (uint8_t)(slave_addr << 1);

    // 3. Gửi địa chỉ thanh ghi (Register)
    while(!(R_IIC0->ICSR2 & 0x80));
    R_IIC0->ICDRT = reg;

    // 4. Gửi dữ liệu cần ghi
    while(!(R_IIC0->ICSR2 & 0x80));
    R_IIC0->ICDRT = data;

    // 5. Đợi truyền xong và tạo điều kiện STOP
    while(!(R_IIC0->ICSR2 & 0x40)); // Đợi TEND (Truyền xong hoàn toàn)
    R_IIC0->ICCR2 |= 0x08;         // STOP
    while(R_IIC0->ICCR2 & 0x08);   // Đợi lệnh STOP thực hiện xong
}

uint8_t i2c0_read_reg(uint8_t slave_addr, uint8_t reg) {
    uint8_t receive_val;

    // Bước A: Write address (Write phase)
    R_IIC0->ICCR2 |= 0x02;
    while(!(R_IIC0->ICSR2 & 0x80));
    R_IIC0->ICDRT = (uint8_t)(slave_addr << 1);
    while(!(R_IIC0->ICSR2 & 0x80));
    R_IIC0->ICDRT = reg;
    while(!(R_IIC0->ICSR2 & 0x40)); // Đợi truyền xong địa chỉ thanh ghi

    // Bước B: Restart (Read phase)
    R_IIC0->ICSR2_b.TEND = 0;
    // Restart
    R_IIC0->ICCR2_b.RS = 1;
    while(R_IIC0->ICCR2_b.RS);
    // Slave + Read
    while(!(R_IIC0->ICSR2 & 0x80));
    R_IIC0->ICDRT = (uint8_t)((slave_addr << 1) | 0x01);

    // Bước C: Đọc dữ liệu (Dummy Read trước)
    while(!(R_IIC0->ICSR2_b.RDRF)); // Đợi ICDRR có dữ liệu (RDRF)

    // Trước khi đọc byte cuối, phải set NACK và STOP
    R_IIC0->ICMR3_b.WAIT = 1; // Đặt WAIT (Dừng clock sau khi nhận)
    R_IIC0->ICMR3_b.ACKWP = 1; // Cho phép sửa bit ACKBT
    R_IIC0->ICMR3_b.ACKBT = 1; // Gửi NACK (Báo cho DS3231 là tôi đủ rồi)

    (void)R_IIC0->ICDRR;   // ĐỌC DUMMY (Bắt đầu lấy dữ liệu thật)

    while(!(R_IIC0->ICSR2_b.RDRF)); // Đợi dữ liệu thật về
    R_IIC0->ICCR2_b.SP = 1;          // STOP
    receive_val = R_IIC0->ICDRR;    // Đọc giá trị thật

    return receive_val;
}

void i2c1_register_init(void) {
    R_IOPORT_PinCfg(&g_ioport_ctrl, BSP_IO_PORT_05_PIN_12,
        (uint32_t)(IOPORT_CFG_PERIPHERAL_PIN |
                   IOPORT_PERIPHERAL_IIC |
                   IOPORT_CFG_NMOS_ENABLE |
                   IOPORT_CFG_PULLUP_ENABLE));

    R_IOPORT_PinCfg(&g_ioport_ctrl, BSP_IO_PORT_05_PIN_11,
        (uint32_t)(IOPORT_CFG_PERIPHERAL_PIN |
                   IOPORT_PERIPHERAL_IIC |
                   IOPORT_CFG_NMOS_ENABLE |
                   IOPORT_CFG_PULLUP_ENABLE));

    // 1. Cấp xung cho IIC1
    R_BSP_MODULE_START(FSP_IP_IIC, 1);

    // 2. Reset module
    R_IIC1->ICCR1 = 0x00;
    R_IIC1->ICCR1 = 0x40; // I2C reset
    for(volatile int i=0; i<100; i++);
    R_IIC1->ICCR1 = 0xC0; // Internal reset
    for(volatile int i=0; i<100; i++);

    /* * 3. Thiết lập tốc độ cực thấp (~10kHz)
     * PCLKB = 50MHz. CKS = 5 (Chia 32) => Clock nội bộ = 1.5625 MHz
     * Tổng số chu kỳ cần: 1.5625 MHz / 10 kHz = 156 chu kỳ
     * Chia đôi cho High và Low: 156 / 2 = 78
     */
    R_IIC1->ICMR1 = 0x50;        // CKS = 5 (Bit 6-4)
    R_IIC1->ICBRH = 77;          // ICBRH + 1 = 78
    R_IIC1->ICBRL = 77;          // ICBRL + 1 = 78

    // 4. Bật module
    R_IIC1->ICCR1 = 0x80;        // Enable IIC
}

void i2c1_write_reg(uint8_t slave_addr, uint8_t reg, uint8_t data) {
    // 1. Tạo điều kiện START
    R_IIC1->ICCR2_b.ST = 1;

    // 2. Gửi địa chỉ Slave (Write: bit cuối = 0)
    while(!(R_IIC1->ICSR2 & 0x80)); // Đợi ICDRT trống (TDRE)
    R_IIC1->ICDRT = (uint8_t)(slave_addr << 1);

    // 3. Gửi địa chỉ thanh ghi (Register)
    while(!(R_IIC1->ICSR2 & 0x80));
    R_IIC1->ICDRT = reg;

    // 4. Gửi dữ liệu cần ghi
    while(!(R_IIC1->ICSR2 & 0x80));
    R_IIC1->ICDRT = data;

    // 5. Đợi truyền xong và tạo điều kiện STOP
    while(!(R_IIC1->ICSR2 & 0x40)); // Đợi TEND (Truyền xong hoàn toàn)
    R_IIC1->ICCR2 |= 0x08;         // STOP
    while(R_IIC1->ICCR2 & 0x08);   // Đợi lệnh STOP thực hiện xong
}

uint8_t i2c1_read_reg(uint8_t slave_addr, uint8_t reg) {
    uint8_t receive_val;

    // Bước A: Write address (Write phase)
    R_IIC1->ICCR2 |= 0x02;
    while(!(R_IIC1->ICSR2 & 0x80));
    R_IIC1->ICDRT = (uint8_t)(slave_addr << 1);
    while(!(R_IIC1->ICSR2 & 0x80));
    R_IIC1->ICDRT = reg;
    while(!(R_IIC1->ICSR2 & 0x40)); // Đợi truyền xong địa chỉ thanh ghi

    // Bước B: Restart (Read phase)
    R_IIC1->ICSR2_b.TEND = 0;
    // Restart
    R_IIC1->ICCR2_b.RS = 1;
    while(R_IIC1->ICCR2_b.RS);
    // Slave + Read
    while(!(R_IIC1->ICSR2 & 0x80));
    R_IIC1->ICDRT = (uint8_t)((slave_addr << 1) | 0x01);

    // Bước C: Đọc dữ liệu (Dummy Read trước)
    while(!(R_IIC1->ICSR2_b.RDRF)); // Đợi ICDRR có dữ liệu (RDRF)

    // Trước khi đọc byte cuối, phải set NACK và STOP
    R_IIC1->ICMR3_b.WAIT = 1; // Đặt WAIT (Dừng clock sau khi nhận)
    R_IIC1->ICMR3_b.ACKWP = 1; // Cho phép sửa bit ACKBT
    R_IIC1->ICMR3_b.ACKBT = 1; // Gửi NACK (Báo cho DS3231 là tôi đủ rồi)

    (void)R_IIC1->ICDRR;   // ĐỌC DUMMY (Bắt đầu lấy dữ liệu thật)

    while(!(R_IIC1->ICSR2_b.RDRF)); // Đợi dữ liệu thật về
    R_IIC1->ICCR2_b.SP = 1;          // STOP
    receive_val = R_IIC1->ICDRR;    // Đọc giá trị thật

    return receive_val;
}

void i2c1_read_mult_reg(uint8_t slave_addr, uint8_t start_reg, uint8_t *p_data, uint8_t len) {
    // Phase Ghi địa chỉ bắt đầu (Giữ nguyên)
    R_IIC1->ICCR2_b.ST = 1;
    while(!(R_IIC1->ICSR2_b.TDRE));
    R_IIC1->ICDRT = (uint8_t)(slave_addr << 1);
    while(!(R_IIC1->ICSR2_b.TDRE));
    R_IIC1->ICDRT = start_reg;
    while(!(R_IIC1->ICSR2_b.TEND));

    // Phase Restart để đọc
    R_IIC1->ICCR2_b.RS = 1;
    while(R_IIC1->ICCR2_b.RS);
    while(!(R_IIC1->ICSR2_b.TDRE));
    R_IIC1->ICDRT = (uint8_t)((slave_addr << 1) | 0x01);

    // Đợi Slave phản hồi địa chỉ (ACK cho byte 0xD1)
    while(!(R_IIC1->ICSR2_b.RDRF));

    // --- BẮT ĐẦU ĐOẠN SỬA ---

    // 1. Bật WAIT để kiểm soát bus tốt hơn
    R_IIC1->ICMR3_b.WAIT = 1;

    // 2. Nếu chỉ đọc 1 byte duy nhất (len = 1)
    if (len == 1) {
        R_IIC1->ICMR3_b.ACKWP = 1; // Mở khóa cho phép ghi ACKBT
        R_IIC1->ICMR3_b.ACKBT = 1; // Chuẩn bị gửi NACK
        (void)R_IIC1->ICDRR;       // Dummy read để nhận byte duy nhất
        while(!(R_IIC1->ICSR2_b.RDRF));
        R_IIC1->ICCR2_b.SP = 1;    // Gửi STOP
        p_data[0] = R_IIC1->ICDRR;
    }
    // 3. Nếu đọc nhiều byte (len > 1)
    else {
        (void)R_IIC1->ICDRR;       // Dummy read để bắt đầu nhận byte thứ 1

        for (uint8_t i = 0; i < len; i++) {
            while(!(R_IIC1->ICSR2_b.RDRF)); // Đợi byte thứ i về

            if (i == (len - 2)) {
                // Khi đang ở byte GẦN CUỐI, phải set NACK ngay để áp dụng cho byte cuối
                R_IIC1->ICMR3_b.ACKWP = 1;
                R_IIC1->ICMR3_b.ACKBT = 1; // Byte tiếp theo sẽ là NACK
            }

            if (i == (len - 1)) {
                // Đang ở byte cuối cùng
                R_IIC1->ICCR2_b.SP = 1;    // Gửi STOP
            }

            p_data[i] = R_IIC1->ICDRR;     // Đọc data ra (giải phóng bus nhận byte tiếp theo)
        }
    }

    // Kết thúc: Tắt WAIT và đợi bus rảnh hoàn toàn
    R_IIC1->ICMR3_b.WAIT = 0;
    while(R_IIC1->ICCR2_b.BBSY); // Đợi bus hết bận
}
