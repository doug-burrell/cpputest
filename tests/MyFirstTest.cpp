#include "CppUTest/TestHarness.h"
#include <stdint.h>

extern "C"
{
	#include "spi_2.h"

	#include "metal2.h"

    static uint8_t var = 0;

    void spi_2_send_spi_data(uint8_t *p_buf, uint8_t data, uint8_t addr, uint8_t length)
    {
        *p_buf = var;
    }
}

TEST_GROUP(MyCode)
{
    void setup()
    {
    }

    void teardown()
    {
    }
};

TEST(MyCode, test1)
{
	int level;

	level = metal2_get_level();
	LONGS_EQUAL(0, level);
}

TEST(MyCode, test2)
{
	int status;

	status = metal2_calibrate_baseline();
	LONGS_EQUAL(0, status);
}

TEST(MyCode, test3)
{
	int status;
	var = 0xFF;

	status = metal2_calibrate_baseline();
	LONGS_EQUAL(0, status);
}

