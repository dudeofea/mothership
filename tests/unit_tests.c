#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "CUnit/Basic.h"
#include "../engine.h"

bool array_equal(float *arr1, float *arr2, int size){
	for (int i = 0; i < size; ++i)
	{
		if(arr1[i] != arr2[i]){
			printf("%f not equal to %f at index %d\n", arr1[i], arr2[i], i);
			return false;
		}
	}
	return true;
}

void array_print(float* a1, int s){
	printf("[");
	for (int i = 0; i < s; ++i)
	{
		if (i < s - 1)
		{
			printf("%.1f, ", a1[i]);
		}else{
			printf("%.1f", a1[i]);
		}
	}
	printf("]\n");
}

//simple volume module
//buffer size is 20
void volume_effect(float *in, float *out, float *arg, void* aux){
	int buf_len = 20;
	//printf("out1: ");
	for (int i = 0; i < buf_len; ++i)
	{
		out[i] = arg[0]*in[i];
		//printf("%f ", out[i]);
	}
	//printf("\n");
}

//Adds 3 to output
void add_3(float *in, float *out, float *arg, void* aux){
	int buf_len = 20;
	//printf("in2: \n");
	for (int i = 0; i < buf_len; ++i)
	{
		//printf("%f ", in[i]);
		out[i] = in[i] + 3.0f;
	}
	//printf("\n");
}

/*
*  Test adding effects
*/
void test_add1(void)
{
	engine_config config = ms_init();
	effect_module e1 = {
		1, 1, 1,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect1",
		volume_effect
	};
	ms_add_effect(e1, &config);
	CU_ASSERT_TRUE(config.effects[0].inp_ports == 1);
	CU_ASSERT_TRUE(config.effects[0].out_ports == 1);
	CU_ASSERT_TRUE(config.effects[0].arg_ports == 1);
	CU_ASSERT_TRUE(config.effects[0].inp_size == 20);
	CU_ASSERT_TRUE(config.effects[0].out_size == 20);
	ms_exit(&config);
}
void test_add2(void)
{
	engine_config config = ms_init();
	effect_module e1 = {
		1, 1, 1,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect1",
		volume_effect
	};
	ms_add_effect(e1, &config);
	effect_module e2 = {
		1, 1, 0,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect2",
		add_3
	};
	ms_add_effect(e2, &config);
	CU_ASSERT_TRUE(config.effects[0].inp_ports == 1);
	CU_ASSERT_TRUE(config.effects[0].out_ports == 1);
	CU_ASSERT_TRUE(config.effects[0].arg_ports == 1);
	CU_ASSERT_TRUE(config.effects[0].inp_size == 20);
	CU_ASSERT_TRUE(config.effects[0].out_size == 20);

	CU_ASSERT_TRUE(config.effects[1].inp_ports == 1);
	CU_ASSERT_TRUE(config.effects[1].out_ports == 1);
	CU_ASSERT_TRUE(config.effects[1].arg_ports == 0);
	CU_ASSERT_TRUE(config.effects[1].inp_size == 20);
	CU_ASSERT_TRUE(config.effects[1].out_size == 20);
	ms_exit(&config);
}
void test_add3(void)
{
	engine_config config = ms_init();
	effect_module e1 = {
		1, 1, 1,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect1",
		volume_effect
	};
	ms_add_effect(e1, &config);
	effect_module e2 = {
		1, 1, 0,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect2",
		add_3
	};
	ms_add_effect(e2, &config);
	static wire w = {
		0,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w, &config);
	w.inp[0] = JACKD_INPUT;
	w.arg[0] = NO_INPUT;
	ms_add_wire(w, &config);
	static wire w2 = {
		1,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w2, &config);
	w2.inp[0] = 0;
	w2.inp_ports[0] = 0;
	ms_add_wire(w2, &config);
	ms_set_effect_arg(0, 0, 1.0f, &config);

	//Check wire positions
	CU_ASSERT_TRUE(config.run_order[0].module == 0);
	CU_ASSERT_TRUE(config.run_order[1].module == 1);
	CU_ASSERT_TRUE(config.run_order[2].module == JACKD_OUTPUT);

	ms_exit(&config);
}
void test_add4(void)
{
	engine_config config = ms_init();
	effect_module e1 = {
		1, 1, 1,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect1",
		volume_effect
	};
	ms_add_effect(e1, &config);
	effect_module e2 = {
		1, 1, 0,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect2",
		add_3
	};
	ms_add_effect(e2, &config);
	static wire w2 = {
		1,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w2, &config);
	w2.inp[0] = 0;
	w2.inp_ports[0] = 0;
	ms_add_wire(w2, &config);
	static wire w = {
		0,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w, &config);
	w.inp[0] = JACKD_INPUT;
	w.arg[0] = NO_INPUT;
	ms_add_wire(w, &config);
	ms_set_effect_arg(0, 0, 1.0f, &config);

	//Check wire positions
	CU_ASSERT_TRUE(config.run_order[0].module == 0);
	CU_ASSERT_TRUE(config.run_order[1].module == 1);
	CU_ASSERT_TRUE(config.run_order[2].module == JACKD_OUTPUT);

	ms_exit(&config);
}
void test_add5(void)
{
	engine_config config = ms_init();
	effect_module e1 = {
		1, 1, 1,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect1",
		volume_effect
	};
	ms_add_effect(e1, &config);
	effect_module e2 = {
		1, 1, 0,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect2",
		add_3
	};
	ms_add_effect(e2, &config);
	ms_add_effect(e2, &config);
	static wire w = {
		0,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w, &config);
	w.inp[0] = JACKD_INPUT;
	w.arg[0] = NO_INPUT;
	ms_add_wire(w, &config);
	static wire w2 = {
		2,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w2, &config);
	w2.inp[0] = 1;
	w2.inp_ports[0] = 0;
	ms_add_wire(w2, &config);
	static wire w3 = {
		1,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w3, &config);
	w3.inp[0] = 0;
	w3.inp_ports[0] = 0;
	ms_add_wire(w3, &config);
	ms_set_effect_arg(0, 0, 1.0f, &config);

	ms_sort_wires(&config);

	//Check wire positions
	CU_ASSERT_TRUE(config.run_order[0].module == 0);
	CU_ASSERT_TRUE(config.run_order[1].module == 1);
	CU_ASSERT_TRUE(config.run_order[2].module == 2);
	CU_ASSERT_TRUE(config.run_order[3].module == JACKD_OUTPUT);

	ms_exit(&config);
}

/*
*  Test wire functions
*/
void test_wire1(void)
{
	engine_config config = ms_init();
	effect_module e1 = {
		1, 1, 0,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect2",
		add_3
	};
	ms_add_effect(e1, &config);
	ms_add_effect(e1, &config);
	ms_add_effect(e1, &config);
	static wire w = {
		0,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w, &config);
	w.inp[0] = JACKD_INPUT;
	ms_add_wire(w, &config);
	static wire w2 = {
		1,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w2, &config);
	w2.inp[0] = JACKD_INPUT;
	ms_add_wire(w2, &config);
	static wire w3 = {
		2,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w3, &config);
	w3.inp[0] = JACKD_INPUT;
	ms_add_wire(w3, &config);

	//Check wire positions
	CU_ASSERT_TRUE(config.run_order[0].module == 2);
	CU_ASSERT_TRUE(config.run_order[1].module == 1);
	CU_ASSERT_TRUE(config.run_order[2].module == 0);
	CU_ASSERT_TRUE(config.run_order[3].module == JACKD_OUTPUT);

	ms_remove_and_insert_wire(0, 3, &config);

	//Check wire positions
	CU_ASSERT_TRUE(config.run_order[0].module == 1);
	CU_ASSERT_TRUE(config.run_order[1].module == 0);
	CU_ASSERT_TRUE(config.run_order[2].module == JACKD_OUTPUT);
	CU_ASSERT_TRUE(config.run_order[3].module == 2);

	ms_exit(&config);
}
void test_wire2(void)
{
	engine_config config = ms_init();
	effect_module e1 = {
		1, 1, 0,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect2",
		add_3
	};
	ms_add_effect(e1, &config);
	ms_add_effect(e1, &config);
	ms_add_effect(e1, &config);
	static wire w = {
		0,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w, &config);
	w.inp[0] = JACKD_INPUT;
	ms_add_wire(w, &config);
	static wire w2 = {
		1,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w2, &config);
	w2.inp[0] = JACKD_INPUT;
	ms_add_wire(w2, &config);
	static wire w3 = {
		2,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w3, &config);
	w3.inp[0] = JACKD_INPUT;
	ms_add_wire(w3, &config);

	//Check wire positions
	CU_ASSERT_TRUE(config.run_order[0].module == 2);
	CU_ASSERT_TRUE(config.run_order[1].module == 1);
	CU_ASSERT_TRUE(config.run_order[2].module == 0);
	CU_ASSERT_TRUE(config.run_order[3].module == JACKD_OUTPUT);

	ms_remove_wire(1, &config);

	//Check wire positions
	CU_ASSERT_TRUE(config.run_order[0].module == 2);
	CU_ASSERT_TRUE(config.run_order[1].module == 0);
	CU_ASSERT_TRUE(config.run_order[2].module == JACKD_OUTPUT);

	ms_exit(&config);
}
void test_wire3(void)
{
	engine_config config = ms_init();
	effect_module e1 = {
		1, 1, 0,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect2",
		add_3
	};
	ms_add_effect(e1, &config);
	ms_add_effect(e1, &config);
	ms_add_effect(e1, &config);
	static wire w = {
		0,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w, &config);
	w.inp[0] = JACKD_INPUT;
	ms_add_wire(w, &config);
	static wire w2 = {
		1,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w2, &config);
	w2.inp[0] = JACKD_INPUT;
	ms_add_wire(w2, &config);
	static wire w3 = {
		2,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w3, &config);
	w3.inp[0] = JACKD_INPUT;
	ms_add_wire(w3, &config);

	//Check wire positions
	CU_ASSERT_TRUE(config.run_order[0].module == 2);
	CU_ASSERT_TRUE(config.run_order[1].module == 1);
	CU_ASSERT_TRUE(config.run_order[2].module == 0);
	CU_ASSERT_TRUE(config.run_order[3].module == JACKD_OUTPUT);

	ms_remove_and_insert_wire(2, 0, &config);

	//Check wire positions
	CU_ASSERT_TRUE(config.run_order[0].module == 0);
	CU_ASSERT_TRUE(config.run_order[1].module == 2);
	CU_ASSERT_TRUE(config.run_order[2].module == 1);
	CU_ASSERT_TRUE(config.run_order[3].module == JACKD_OUTPUT);

	ms_exit(&config);
}

/*
*  Test effect calculations
*/
void test_calc1(void)
{
	engine_config config = ms_init();
	float in[20], out[20];
	float ans[20];
	for (int i = 0; i < 20; ++i)
	{
		in[i] = i;
		ans[i] = i*2.0f + 3.0f;
	}
	effect_module e1 = {
		1, 1, 1,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect1",
		volume_effect
	};
	ms_add_effect(e1, &config);
	effect_module e2 = {
		1, 1, 0,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect2",
		add_3
	};
	ms_add_effect(e2, &config);
	static wire w = {
		0,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w, &config);
	w.inp[0] = JACKD_INPUT;
	w.arg[0] = NO_INPUT;
	ms_add_wire(w, &config);
	static wire w2 = {
		1,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w2, &config);
	w2.inp[0] = 0;
	w2.inp_ports[0] = 0;
	ms_add_wire(w2, &config);
	//set volume
	ms_set_effect_arg(0, 0, 2.0f, &config);
	ms_set_output_module(1, 0, &config);
	ms_refresh(&config);
	ms_run_engine(in, out, 20, &config);
	CU_ASSERT_TRUE(array_equal(out, ans, 20));
	ms_exit(&config);
}
void test_calc2(void)
{
	engine_config config = ms_init();
	float in[20], out[20];
	float ans[20];
	for (int i = 0; i < 20; ++i)
	{
		in[i] = i;
		ans[i] = (i+3.0f)*2.0f;
	}
	effect_module e1 = {
		1, 1, 1,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect1",
	volume_effect
	};
	ms_add_effect(e1, &config);
	effect_module e2 = {
		1, 1, 0,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect2",
	add_3
	};
	ms_add_effect(e2, &config);
	static wire w = {
		1,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w, &config);
	w.inp[0] = JACKD_INPUT;
	ms_add_wire(w, &config);
	static wire w2 = {
		0,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w2, &config);
	w2.inp[0] = 1;
	w2.inp_ports[0] = 0;
	w2.arg[0] = NO_INPUT;
	ms_add_wire(w2, &config);
	ms_set_effect_arg(0, 0, 2.0f, &config);
	ms_refresh(&config);
	ms_set_output_module(0, 0, &config);
	ms_run_engine(in, out, 20, &config);
	CU_ASSERT_TRUE(array_equal(out, ans, 20));
	ms_exit(&config);
}
void test_calc3(void)
{
	engine_config config = ms_init();
	float in[20], out[20];
	float ans[20];
	for (int i = 0; i < 20; ++i)
	{
		in[i] = i;
		ans[i] = (i+3.0f+3.0f)*2.0f;
	}
	effect_module e1 = {
		1, 1, 1,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect1",
		volume_effect
	};
	effect_module e2 = {
		1, 1, 0,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect2",
		add_3
	};

	//effect 1: add 3
	ms_add_effect(e2, &config);
	static wire w = {
		0,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w, &config);
	w.inp[0] = JACKD_INPUT;
	ms_add_wire(w, &config);

	//effect 2: add 3
	ms_add_effect(e2, &config);
	static wire w2 = {
		1,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w2, &config);
	w2.inp[0] = 0;
	w2.inp_ports[0] = 0;
	ms_add_wire(w2, &config);

	//effect 3: multiply
	ms_add_effect(e1, &config);
	static wire w3 = {
		2,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w3, &config);
	w3.inp[0] = 1;
	w3.inp_ports[0] = 0;
	ms_add_wire(w3, &config);
	//set volume
	ms_set_effect_arg(2, 0, 2.0f, &config);
	ms_set_output_module(2, 0, &config);
	ms_refresh(&config);
	ms_run_engine(in, out, 20, &config);
	CU_ASSERT_TRUE(array_equal(out, ans, 20));
	ms_exit(&config);
}

/* Additional Engine Tests for strange dependencies */
void test_loop1(void){
	//init everything
	engine_config config = ms_init();
	float in[20], out[20];
	float ans[20];
	//the first module is the output, but it's run first
	//so after running the engine twice, it should give
	//<out> = <inp> * 4.5 + 3
	//the run order is [add_3, mul_4.5, JACKD_OUT]
	for (int i = 0; i < 20; ++i)
	{
		in[i] = i * i;	//why not?
		ans[i] = (in[i] * 4.5) + 3;
	}
	//add effects
	ms_create_effect(1, 1, 0, 20, 20, "add_3", add_3, &config);
	ms_create_effect(1, 1, 1, 20, 20, "mult", volume_effect, &config);
	//set patches
	wire w1 = ms_create_wire(0, &config);
	w1.inp[0] = 1;
	w1.inp_ports[0] = 0;
	ms_add_wire(w1, &config);
	wire w2 = ms_create_wire(1, &config);
	w2.inp[0] = JACKD_INPUT;
	ms_add_wire(w2, &config);
	ms_set_effect_arg(1, 0, 4.5f, &config);
	ms_set_output_module(0, 0, &config);
	//run engine
	ms_run_engine(in, out, 20, &config);
	ms_run_engine(in, out, 20, &config);
	//test
	CU_ASSERT_TRUE(array_equal(out, ans, 20));
	ms_exit(&config);
}

/* Test MIDI creation tool */
void test_MIDI1(void)
{
	engine_config config = ms_init();
	midi_sample sample = ms_create_midi("../samples/cello_C2.mp3", 0.0, 0.0);
	printf("attack length: %d\n", sample.sample_l);
	ms_exit(&config);
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{
	CU_pSuite pSuite = NULL;

	/* initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	/* add a suite to the registry */
	pSuite = CU_add_suite("Suite_1", NULL, NULL);
	if (NULL == pSuite) {
		CU_cleanup_registry();
		return CU_get_error();
	}
	/* add the tests to the suite */
	/* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
	CU_add_test(pSuite, "test add effect 1", test_add1);
	CU_add_test(pSuite, "test add effect 2", test_add2);
	// CU_add_test(pSuite, "test wire functions 1", test_wire1);
	// CU_add_test(pSuite, "test wire functions 2", test_wire2);
	// CU_add_test(pSuite, "test wire functions 3", test_wire3);
	// CU_add_test(pSuite, "test add wire 1", test_add3);
	// CU_add_test(pSuite, "test add wire 2", test_add4);
	// CU_add_test(pSuite, "test add wire 3", test_add5);
	CU_add_test(pSuite, "test engine calc 1", test_calc1);
	CU_add_test(pSuite, "test engine calc 2", test_calc2);
	CU_add_test(pSuite, "test engine calc 3", test_calc3);
	CU_add_test(pSuite, "test engine calc 4", test_loop1);
	CU_add_test(pSuite, "test MIDI sample creation 1", test_MIDI1);

	/* Run all tests using the CUnit Basic interface */
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}