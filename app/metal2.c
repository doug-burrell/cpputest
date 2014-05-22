/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
/* std c library */
#include <stdint.h>

/* project specific */
#include "spi_2.h"

#include "metal2.h"

/*****************************************************************************/
/* Defined and Enumerated Constants                                          */
/*****************************************************************************/
/* LDC1000 registers */
#define LDC1000_REG_REVID       0x00
#define LDC1000_REG_RPMAX       0x01
#define LDC1000_REG_RPMIN       0x02
#define LDC1000_REG_SENSORFREQ  0x03
#define LDC1000_REG_LDCCONFIG   0x04
#define LDC1000_REG_CLKCONFIG   0x05
#define LDC1000_REG_THRESHILSB  0x06
#define LDC1000_REG_THRESHIMSB  0x07
#define LDC1000_REG_THRESLOLSB  0x08
#define LDC1000_REG_THRESLOMSB  0x09
#define LDC1000_REG_INTCONFIG   0x0A
#define LDC1000_REG_PWRCONFIG   0x0B
#define LDC1000_REG_STATUS      0x20
#define LDC1000_REG_PROXLSB     0x21
#define LDC1000_REG_PROXMSB     0x22
#define LDC1000_REG_FREQCTRLSB  0x23
#define LDC1000_REG_FREQCTRMID  0x24
#define LDC1000_REG_FREQCTRMSB  0x25

#define CALIBRATION_RETRY_LIMIT (5)
#define AVERAGE_BUFFER_SIZE (16)
#define STD_DEV_BUFFER_SIZE (10)
#define FILTER_BUFFER_SIZE (64)
#define CALIBRATION_SAMPLE_COUNT (64)
#define METAL2_TIMER_SEED (10000)

typedef enum
{
    DISABLED = 0,
    ENABLED,
    CALIBRATION_FAILURE,
} metal2_state_t;
/*****************************************************************************/
/* User Defined Datatypes                                                    */
/*****************************************************************************/
/* ldc config register contents */
typedef union
{
    uint8_t data;

    struct
    {
        uint8_t resp_time : 3;
        uint8_t amplitude : 2;
        uint8_t unused :    3;
    } bits;
} ldc_config_t;

/* clock config register contents */
typedef union
{
    uint8_t data;

    struct
    {
        uint8_t clock_pd :  1;
        uint8_t clock_sel : 1;
        uint8_t unused :    6;
    } bits;
} clock_config_t;

/* interrupt pin config */
typedef union
{
    uint8_t data;

    struct
    {
        uint8_t mode :      3;
        uint8_t unused :    5;
    } bits;
} int_pin_config_t;

/* power config */
typedef union
{
    uint8_t data;

    struct
    {
        uint8_t mode :      1;
        uint8_t unused :    7;
    } bits;
} power_config_t;


/* ldc module state and register settings */
typedef struct
{
    /* ldc1000 ic register settings */
    uint8_t rev_id;
    uint8_t rp_max;
    uint8_t rp_min;
    uint8_t sensor_freq;
    uint8_t ldc_config;
    uint8_t clock_config;
    uint16_t comp_thresh_hi;
    uint16_t comp_thresh_lo;
    uint8_t int_pin_config;
    uint8_t power_config;
    uint8_t ldc_status;
    uint16_t prox_data;
    uint32_t freq_data;

    /* metal2 module settings */
    metal2_state_t state;
    uint32_t delay;
    uint16_t prox_range;
    uint8_t cal_retry_count;
    uint8_t cal_status;
    uint16_t debounce_counter;

    /* signal processing data */
    uint8_t metal_level;
    uint16_t prox_baseline;
    int32_t average;
    int16_t std_dev;
    int16_t baseline_diff;
} metal2_t;
/*****************************************************************************/
/* Private Prototypes                                                        */
/*****************************************************************************/
static void change_state(metal2_t *, metal2_state_t);
static void set_power_config(metal2_t *, uint8_t);
static void get_status(metal2_t *);
static void get_prox_data(metal2_t *);

static void enable_metal(metal2_t *);
static void disable_metal(metal2_t *);

/*****************************************************************************/
/* Private Variables                                                         */
/*****************************************************************************/

static uint32_t g_init_flag;
static metal2_t g_metal2;

static uint8_t g_metal2_state;
/*****************************************************************************/
/* Public Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Public Interface Functions                                                */
/*****************************************************************************/

/******************************************************************************

  FUNCTION:     metal2_create

  SCOPE:        private

  PARAMETERS:   void

  RETURNS:      void

  DESCRIPTION:

******************************************************************************/
void metal2_create(uint32_t init_flag)
{
    g_init_flag = init_flag;

}

/******************************************************************************

  FUNCTION:     metal2_get_level

  SCOPE:        private

  PARAMETERS:   void

  RETURNS:      void

  DESCRIPTION:

******************************************************************************/
uint8_t metal2_get_level(void)
{
    return g_metal2.metal_level;
}

/******************************************************************************

  FUNCTION:     metal2_calibrate_baseline

  SCOPE:        private

  PARAMETERS:   void

  RETURNS:      void

  DESCRIPTION:

******************************************************************************/
uint8_t metal2_calibrate_baseline(void)
{
    metal2_state_t previous_state;
    uint32_t sum;
    uint16_t max;
    uint16_t min;
    uint8_t count;
    metal2_t * me = &g_metal2;

    /* if current state is disabled then enable to run calibration */
    previous_state = me->state;
    if (me->state != ENABLED)
    {
        change_state(me, ENABLED);
    }

    // TODO: figure out a way to remove this delay, currently it's here to
    // allow the proximity data to settle after the infeed paddle has moved

    me->cal_retry_count = 0;
    do
    {
        sum = 0;
        max = 0;
        min = 0xFFFF;
        count = 0;
        // TODO: can we decrease this count to save time?
        while (count < CALIBRATION_SAMPLE_COUNT)
        {
            get_status(me);

            if (me->ldc_status == 0)
            {
                get_prox_data(me);
                count++;
                sum += me->prox_data;

                if (me->prox_data > max)
                {
                    max = me->prox_data;
                }
                if (me->prox_data < min)
                {
                    min = me->prox_data;
                }
            }
            else
            {
                if (me->ldc_status == 2)
                {
                    /* oscillator is dead, restart device */
                    change_state(me, DISABLED);
                    change_state(me, ENABLED);
                }

                if (me->cal_retry_count < CALIBRATION_RETRY_LIMIT)
                {
                    me->cal_retry_count++;
                }
                else
                {
                    count = CALIBRATION_SAMPLE_COUNT;
                    max = 0xFFFF;
                    min = 0;
                }
            }
        }

        /* calculate average and range */
        me->prox_baseline = sum / CALIBRATION_SAMPLE_COUNT;
        me->prox_range = max - min;

        if (me->prox_range < 0x0200)
        {
            me->cal_status = 0x00;
        }
        else
        {
            me->cal_status = 0xFF;
            me->cal_retry_count++;
        }
    } while ((me->cal_retry_count < CALIBRATION_RETRY_LIMIT) &&
             (me->cal_status != 0x00));

    /* if metal was previously disabled then restore state */
    if (previous_state != ENABLED)
    {
        change_state(me, DISABLED);
    }

    me->metal_level = METAL_LEVEL_NO_METAL;

    if (me->cal_status == 0xFF)
    {
        change_state(me, CALIBRATION_FAILURE);
    }

    return me->cal_status;
}

/*****************************************************************************/
/* Private Helper Functions                                                  */
/*****************************************************************************/

/******************************************************************************

  FUNCTION:     change_state

  SCOPE:        private

  PARAMETERS:   me - pointer to the metal2_t struct

  RETURNS:      void

  DESCRIPTION:  implement exit and entry actions

******************************************************************************/
static void change_state(metal2_t * me, metal2_state_t state)
{
    /* exit actions */
    switch (me->state)
    {
        case ENABLED:
        case DISABLED:
        case CALIBRATION_FAILURE:
        default:
            break;
    }

    me->state = state;

    /* entry actions */
    switch (state)
    {
        case ENABLED:
            enable_metal(me);
            break;
        case DISABLED:
            disable_metal(me);
            break;
        case CALIBRATION_FAILURE:
            break;
        default:
            break;
    }

    g_metal2_state = me->state;
}

/******************************************************************************

  FUNCTION:     set_power_config

  SCOPE:        private

  PARAMETERS:   void

  RETURNS:      void

  DESCRIPTION:

******************************************************************************/
static void set_power_config(metal2_t * me, uint8_t power_config)
{
    spi_2_send_spi_data(NULL, power_config, LDC1000_REG_PWRCONFIG, 0);
    spi_2_send_spi_data(&me->power_config, 0, LDC1000_REG_PWRCONFIG, 1);

    if (me->power_config == 0)
    {
        me->state = DISABLED;
        me->delay = 1000;
    }
    else
    {
        me->state = ENABLED;
        me->delay = 2;
    }
}

/******************************************************************************

  FUNCTION:     get_status

  SCOPE:        private

  PARAMETERS:   void

  RETURNS:      void

  DESCRIPTION:

******************************************************************************/
static void get_status(metal2_t * me)
{
    spi_2_send_spi_data(&me->ldc_status, 0, LDC1000_REG_STATUS, 1);
}

/******************************************************************************

  FUNCTION:     get_prox_data

  SCOPE:        private

  PARAMETERS:   void

  RETURNS:      void

  DESCRIPTION:

******************************************************************************/
static void get_prox_data(metal2_t * me)
{
    uint8_t temp_buf[5];

    spi_2_send_spi_data(temp_buf, 0, LDC1000_REG_PROXLSB, 5);

    me->prox_data = temp_buf[0];
    me->prox_data |= (temp_buf[1] << 8) & 0xFF00;

    me->freq_data = temp_buf[2];
    me->freq_data |= (temp_buf[3] << 8) & 0xFF00;
    me->freq_data |= (temp_buf[4] << 16) & 0xFF0000;
}

/******************************************************************************

  FUNCTION:     enable_metal

  SCOPE:        private

  PARAMETERS:   void

  RETURNS:      void

  DESCRIPTION:

******************************************************************************/
static void enable_metal(metal2_t * me)
{
    power_config_t power_config;

    power_config.bits.mode = 1; /* enabled */
    power_config.bits.unused = 0;
    set_power_config(me, power_config.data);

    me->debounce_counter = 750;
}

/******************************************************************************

  FUNCTION:     disable_metal

  SCOPE:        private

  PARAMETERS:   void

  RETURNS:      void

  DESCRIPTION:

******************************************************************************/
static void disable_metal(metal2_t * me)
{
    power_config_t power_config;

    power_config.bits.mode = 0; /* disabled */
    power_config.bits.unused = 0;
    set_power_config(me, power_config.data);
}

