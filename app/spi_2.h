
#ifndef SPI_2_H
#define SPI_2_H

/*****************************************************************************/
/* Defined and Enumerated Constants                                          */
/*****************************************************************************/
#define CMD_COMPLETE_SIG	BIT0

/* ------ SIGNAL_NAME --- IO_PORT --- IO_PIN --- */
#define SPI2_GPIO_TABLE(ENTRY)              \
    ENTRY(SCK2,             GPIOB,      10) \
    ENTRY(MOSI2,            GPIOB,      15) \
    ENTRY(MISO2,            GPIOB,      14)

/* this table defines the slaves connected to the SPI2 bus */
/* ------ SIGNAL_NAME --- IO_PORT --- IO_PIN --- */
#define CS_TABLE(ENTRY)                \
    ENTRY(DAC_ONE,          GPIOB,      9) \
    ENTRY(DAC_TWO,          GPIOB,      9) \
    ENTRY(LDC1000,          GPIOA,      1)

typedef enum
{
    WRITE_16_CMD,
    WRITE_2X8_CMD,
    READ_8_CMD
} spi_2_rw_cmd_t;
/*****************************************************************************/
/* User Defined Datatypes                                                    */
/*****************************************************************************/
/* this enum contains the slaves connected to the SPI2 bus */
#define EXPAND_AS_ENUM(a,b,c) a,
typedef enum
{
    CS_TABLE(EXPAND_AS_ENUM)
    NUM_SLAVES
} spi_2_slave_id_t;

typedef struct
{
    uint8_t addr;
    uint16_t data;
    uint8_t *p_buf;
    uint8_t read_length;
    spi_2_slave_id_t slave_id;
    uint32_t signal;
    spi_2_rw_cmd_t command;
} spi_2_mail_t;

/*****************************************************************************/
/* Public Prototypes                                                         */
/*****************************************************************************/
extern void spi_2_create(uint32_t);
extern void spi_2_send_spi_data(uint8_t *, uint8_t, uint8_t, uint8_t);

/*****************************************************************************/
/* Public Variables                                                          */
/*****************************************************************************/

#endif

