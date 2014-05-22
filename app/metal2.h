
#ifndef METAL2_H
#define METAL2_H

/*****************************************************************************/
/* Defined and Enumerated Constants                                          */
/*****************************************************************************/
#define ENABLE_METAL_SIG        BIT0
#define DISABLE_METAL_SIG       BIT1
#define METAL2_SPI2_THREAD_DEPENDENCY_SIG   BIT2

typedef enum
{
    METAL_LEVEL_NO_METAL,
    METAL_LEVEL_WEAK_METAL,
    METAL_LEVEL_STRONG_METAL
} metal_level_t;
/*****************************************************************************/
/* User Defined Datatypes                                                    */
/*****************************************************************************/

/*****************************************************************************/
/* Public Prototypes                                                         */
/*****************************************************************************/
extern void metal2_create(uint32_t);
extern uint8_t metal2_get_level(void);
extern uint8_t metal2_calibrate_baseline(void);

/*****************************************************************************/
/* Public Variables                                                          */
/*****************************************************************************/

#endif

