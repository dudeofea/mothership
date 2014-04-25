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

//simple volume module
//buffer size is 20
void volume_effect(float *in, float *out, float *arg){
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
void add_3(float *in, float *out, float *arg){
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
   ms_init();
   effect_module e1 = {
      1, 1, 1,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect1",
      volume_effect
   };
   ms_add_effect(e1);
   CU_ASSERT_TRUE(ms_get_effect(0).inp_ports == 1);
   CU_ASSERT_TRUE(ms_get_effect(0).out_ports == 1);
   CU_ASSERT_TRUE(ms_get_effect(0).arg_ports == 1);
   CU_ASSERT_TRUE(ms_get_effect(0).inp_size == 20);
   CU_ASSERT_TRUE(ms_get_effect(0).out_size == 20);
   ms_exit();
}
void test_add2(void)
{
   ms_init();
   effect_module e1 = {
      1, 1, 1,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect1",
      volume_effect
   };
   ms_add_effect(e1);
   effect_module e2 = {
      1, 1, 0,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect2",
      add_3
   };
   ms_add_effect(e2);
   CU_ASSERT_TRUE(ms_get_effect(0).inp_ports == 1);
   CU_ASSERT_TRUE(ms_get_effect(0).out_ports == 1);
   CU_ASSERT_TRUE(ms_get_effect(0).arg_ports == 1);
   CU_ASSERT_TRUE(ms_get_effect(0).inp_size == 20);
   CU_ASSERT_TRUE(ms_get_effect(0).out_size == 20);

   CU_ASSERT_TRUE(ms_get_effect(1).inp_ports == 1);
   CU_ASSERT_TRUE(ms_get_effect(1).out_ports == 1);
   CU_ASSERT_TRUE(ms_get_effect(1).arg_ports == 0);
   CU_ASSERT_TRUE(ms_get_effect(1).inp_size == 20);
   CU_ASSERT_TRUE(ms_get_effect(1).out_size == 20);
   ms_exit();
}
void test_add3(void)
{
   ms_init();
   effect_module e1 = {
      1, 1, 1,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect1",
      volume_effect
   };
   ms_add_effect(e1);
   effect_module e2 = {
      1, 1, 0,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect2",
      add_3
   };
   ms_add_effect(e2);
   static wire w = {
      0,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w);
   w.inp[0] = JACKD_INPUT;
   w.arg[0] = NO_INPUT;
   ms_add_wire(w);
   static wire w2 = {
      1,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w2);
   w2.inp[0] = 0;
   w2.inp_ports[0] = 0;
   ms_add_wire(w2);
   ms_set_effect_arg(0, 0, 1.0f);

   //Check wire positions
   CU_ASSERT_TRUE(ms_get_wire(0).module == 0);
   CU_ASSERT_TRUE(ms_get_wire(1).module == 1);
   CU_ASSERT_TRUE(ms_get_wire(2).module == JACKD_OUTPUT);

   ms_exit();
}
void test_add4(void)
{
   ms_init();
   effect_module e1 = {
      1, 1, 1,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect1",
      volume_effect
   };
   ms_add_effect(e1);
   effect_module e2 = {
      1, 1, 0,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect2",
      add_3
   };
   ms_add_effect(e2);
   static wire w2 = {
      1,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w2);
   w2.inp[0] = 0;
   w2.inp_ports[0] = 0;
   ms_add_wire(w2);
   static wire w = {
      0,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w);
   w.inp[0] = JACKD_INPUT;
   w.arg[0] = NO_INPUT;
   ms_add_wire(w);
   ms_set_effect_arg(0, 0, 1.0f);

   //Check wire positions
   CU_ASSERT_TRUE(ms_get_wire(0).module == 0);
   CU_ASSERT_TRUE(ms_get_wire(1).module == 1);
   CU_ASSERT_TRUE(ms_get_wire(2).module == JACKD_OUTPUT);

   ms_exit();
}
void test_add5(void)
{
   ms_init();
   effect_module e1 = {
      1, 1, 1,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect1",
      volume_effect
   };
   ms_add_effect(e1);
   effect_module e2 = {
      1, 1, 0,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect2",
      add_3
   };
   ms_add_effect(e2);
   ms_add_effect(e2);
   static wire w = {
      0,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w);
   w.inp[0] = JACKD_INPUT;
   w.arg[0] = NO_INPUT;
   ms_add_wire(w);
   static wire w2 = {
      2,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w2);
   w2.inp[0] = 1;
   w2.inp_ports[0] = 0;
   ms_add_wire(w2);
   static wire w3 = {
      1,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w3);
   w3.inp[0] = 0;
   w3.inp_ports[0] = 0;
   ms_add_wire(w3);
   ms_set_effect_arg(0, 0, 1.0f);

   ms_sort_wires();

   //Check wire positions
   CU_ASSERT_TRUE(ms_get_wire(0).module == 0);
   CU_ASSERT_TRUE(ms_get_wire(1).module == 1);
   CU_ASSERT_TRUE(ms_get_wire(2).module == 2);
   CU_ASSERT_TRUE(ms_get_wire(3).module == JACKD_OUTPUT);

   ms_exit();
}

/*
*  Test wire functions
*/
void test_wire1(void)
{
   ms_init();
   effect_module e1 = {
      1, 1, 0,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect2",
      add_3
   };
   ms_add_effect(e1);
   ms_add_effect(e1);
   ms_add_effect(e1);
   static wire w = {
      0,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w);
   w.inp[0] = JACKD_INPUT;
   ms_add_wire(w);
   static wire w2 = {
      1,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w2);
   w2.inp[0] = JACKD_INPUT;
   ms_add_wire(w2);
   static wire w3 = {
      2,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w3);
   w3.inp[0] = JACKD_INPUT;
   ms_add_wire(w3);

   //Check wire positions
   CU_ASSERT_TRUE(ms_get_wire(0).module == 2);
   CU_ASSERT_TRUE(ms_get_wire(1).module == 1);
   CU_ASSERT_TRUE(ms_get_wire(2).module == 0);
   CU_ASSERT_TRUE(ms_get_wire(3).module == JACKD_OUTPUT);

   ms_remove_and_insert_wire(0, 3);

   //Check wire positions
   CU_ASSERT_TRUE(ms_get_wire(0).module == 1);
   CU_ASSERT_TRUE(ms_get_wire(1).module == 0);
   CU_ASSERT_TRUE(ms_get_wire(2).module == JACKD_OUTPUT);
   CU_ASSERT_TRUE(ms_get_wire(3).module == 2);

   ms_exit();
}
void test_wire2(void)
{
   ms_init();
   effect_module e1 = {
      1, 1, 0,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect2",
      add_3
   };
   ms_add_effect(e1);
   ms_add_effect(e1);
   ms_add_effect(e1);
   static wire w = {
      0,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w);
   w.inp[0] = JACKD_INPUT;
   ms_add_wire(w);
   static wire w2 = {
      1,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w2);
   w2.inp[0] = JACKD_INPUT;
   ms_add_wire(w2);
   static wire w3 = {
      2,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w3);
   w3.inp[0] = JACKD_INPUT;
   ms_add_wire(w3);

   //Check wire positions
   CU_ASSERT_TRUE(ms_get_wire(0).module == 2);
   CU_ASSERT_TRUE(ms_get_wire(1).module == 1);
   CU_ASSERT_TRUE(ms_get_wire(2).module == 0);
   CU_ASSERT_TRUE(ms_get_wire(3).module == JACKD_OUTPUT);

   ms_remove_wire(1);

   //Check wire positions
   CU_ASSERT_TRUE(ms_get_wire(0).module == 2);
   CU_ASSERT_TRUE(ms_get_wire(1).module == 0);
   CU_ASSERT_TRUE(ms_get_wire(2).module == JACKD_OUTPUT);

   ms_exit();
}
void test_wire3(void)
{
   ms_init();
   effect_module e1 = {
      1, 1, 0,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect2",
      add_3
   };
   ms_add_effect(e1);
   ms_add_effect(e1);
   ms_add_effect(e1);
   static wire w = {
      0,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w);
   w.inp[0] = JACKD_INPUT;
   ms_add_wire(w);
   static wire w2 = {
      1,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w2);
   w2.inp[0] = JACKD_INPUT;
   ms_add_wire(w2);
   static wire w3 = {
      2,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w3);
   w3.inp[0] = JACKD_INPUT;
   ms_add_wire(w3);

   //Check wire positions
   CU_ASSERT_TRUE(ms_get_wire(0).module == 2);
   CU_ASSERT_TRUE(ms_get_wire(1).module == 1);
   CU_ASSERT_TRUE(ms_get_wire(2).module == 0);
   CU_ASSERT_TRUE(ms_get_wire(3).module == JACKD_OUTPUT);

   ms_remove_and_insert_wire(2, 0);

   //Check wire positions
   CU_ASSERT_TRUE(ms_get_wire(0).module == 0);
   CU_ASSERT_TRUE(ms_get_wire(1).module == 2);
   CU_ASSERT_TRUE(ms_get_wire(2).module == 1);
   CU_ASSERT_TRUE(ms_get_wire(3).module == JACKD_OUTPUT);

   ms_exit();
}

/*
*  Test effect calculations
*/
void test_calc1(void)
{
   ms_init();
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
      NULL, "test_effect1",
      volume_effect
   };
   ms_add_effect(e1);
   effect_module e2 = {
      1, 1, 0,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect2",
      add_3
   };
   ms_add_effect(e2);
   static wire w = {
      0,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w);
   w.inp[0] = JACKD_INPUT;
   w.arg[0] = NO_INPUT;
   ms_add_wire(w);
   static wire w2 = {
      1,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w2);
   w2.inp[0] = 0;
   w2.inp_ports[0] = 0;
   ms_add_wire(w2);
   //set volume
   ms_set_effect_arg(0, 0, 2.0f);
   ms_set_output_module(1, 0);
   ms_refresh();
   ms_run_engine(in, out, 20);
   CU_ASSERT_TRUE(array_equal(out, ans, 20));
   ms_exit();
}
void test_calc2(void)
{
   ms_init();
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
      NULL, "test_effect1",
   volume_effect
   };
   ms_add_effect(e1);
   effect_module e2 = {
      1, 1, 0,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect2",
   add_3
   };
   ms_add_effect(e2);
   static wire w = {
      1,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w);
   w.inp[0] = JACKD_INPUT;
   ms_add_wire(w);
   static wire w2 = {
      0,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w2);
   w2.inp[0] = 1;
   w2.inp_ports[0] = 0;
   w2.arg[0] = NO_INPUT;
   ms_add_wire(w2);
   ms_set_effect_arg(0, 0, 2.0f);
   ms_refresh();
   ms_set_output_module(0, 0);
   ms_run_engine(in, out, 20);
   CU_ASSERT_TRUE(array_equal(out, ans, 20));
   ms_exit();
}
void test_calc3(void)
{
   ms_init();
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
      NULL, "test_effect1",
      volume_effect
   };
   ms_add_effect(e1);
   effect_module e2 = {
      1, 1, 0,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect2",
      add_3
   };
   ms_add_effect(e2);
   ms_add_effect(e2);
   static wire w = {
      1,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w);
   w.inp[0] = JACKD_INPUT;
   ms_add_wire(w);
   static wire w2 = {
      0,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w2);
   w2.inp[0] = 2;
   w2.inp_ports[0] = 0;
   w2.arg[0] = NO_INPUT;
   ms_add_wire(w2);
   static wire w3 = {
      2,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w3);
   w3.inp[0] = 1;
   w3.inp_ports[0] = 0;
   ms_add_wire(w3);
   //set volume
   ms_set_effect_arg(0, 0, 2.0f);
   ms_set_output_module(0, 0);
   ms_refresh();
   ms_run_engine(in, out, 20);
   CU_ASSERT_TRUE(array_equal(out, ans, 20));
   ms_exit();
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
   CU_add_test(pSuite, "test wire functions 1", test_wire1);
   CU_add_test(pSuite, "test wire functions 2", test_wire2);
   CU_add_test(pSuite, "test wire functions 3", test_wire3);
   CU_add_test(pSuite, "test add wire 1", test_add3);
   CU_add_test(pSuite, "test add wire 2", test_add4);
   CU_add_test(pSuite, "test add wire 3", test_add5);
   CU_add_test(pSuite, "test engine calc 1", test_calc1);
   CU_add_test(pSuite, "test engine calc 2", test_calc2);
   CU_add_test(pSuite, "test engine calc 3", test_calc3);

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}