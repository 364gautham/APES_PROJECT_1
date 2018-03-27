/********************************************************************************************************
*
* UNIVERSITY OF COLORADO BOULDER
*
* @file unit_test.c
* @brief cmocka testing 
* 
* Several process talks in a synchronised way to achieve a common goal
*
* @author Kiran Hegde and Gautham K A
* @date  3/15/2018
* @tools vim editor
*
********************************************************************************************************/


/********************************************************************************************************
*
* Header Files
*
********************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include "temp.h"

#define LIGHT
#define TEMP

#ifdef TEMP
void test_readtempdata()
{
	float c,k,f;
	int ans = 170;
	int tfd1, result;
	result = read_tempdata(tfd1, &c, &k, &f);
	assert_int_equal(c, ans);	
}

void test_readtlowreg()
{
	int test, result;
	float var;
	result = read_tlowreg(test, &var);
	assert_int_equal(result, SUCCESS);
}

void test_write_config_sh()
{
        int test, result;
        int var;
        result = read_config_em(test, &var);
        assert_int_equal(result, SUCCESS);
}
#endif
#ifdef LIGHT
void test_int_thres_read()
{
        int test, result;
        uint16_t var, val;
        result = int_thres_read(test, &var, &val);
        assert_int_equal(result, SUCCESS);
}

void test_LOG()
{
        int test, result;
        LOG(test, result, "Hey", NULL);
}

void test_lum_data()
{
	int test, result;
        int var, ans;
        result = lum_data(test, &var);
        assert_int_equal(result, SUCCESS);
}
#endif
int main()
{

	const struct CMUnitTest tests[] =
	{	
	#ifdef TEMP 
		cmocka_unit_test(test_readtempdata),
		cmocka_unit_test(test_readtlowreg),
		cmocka_unit_test(test_write_config_sh),
	#endif
	#ifdef LIGHT
		cmocka_unit_test(test_int_thres_read),
		cmocka_unit_test(test_lum_data),
		cmocka_unit_test(test_LOG),
	#endif
	};

	return cmocka_run_group_tests(tests, NULL, NULL);

}

