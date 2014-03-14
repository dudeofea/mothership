#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "CUnit/Basic.h"
#include "../engine.h"

bool array_equal(float *arr1, float *arr2, int size){
   for (int i = 0; i < size; ++i)
   {
      if(arr1[i] != arr2[i]){
         return false;
      }
   }
   return true;
}

//simple volume module
//buffer size is 20
void volume_effect(float *in, float *out, float *arg){
   int buf_len = 20;
   for (int i = 0; i < buf_len; ++i)
   {
      out[i] = arg[0]*in[i];
   }
}

//Adds 3 to output
void add_3(float *in, float *out, float *arg){
   int buf_len = 20;
   for (int i = 0; i < buf_len; ++i)
   {
      out[i] = in[i] + 3.0f;
   }
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
   add_effect(e1);
   CU_ASSERT_TRUE(get_effect(0).inp_ports == 1);
   CU_ASSERT_TRUE(get_effect(0).out_ports == 1);
   CU_ASSERT_TRUE(get_effect(0).arg_ports == 1);
   CU_ASSERT_TRUE(get_effect(0).inp_size == 20);
   CU_ASSERT_TRUE(get_effect(0).out_size == 20);
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
   add_effect(e1);
   effect_module e2 = {
      1, 1, 0,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect2",
      add_3
   };
   add_effect(e2);
   CU_ASSERT_TRUE(get_effect(0).inp_ports == 1);
   CU_ASSERT_TRUE(get_effect(0).out_ports == 1);
   CU_ASSERT_TRUE(get_effect(0).arg_ports == 1);
   CU_ASSERT_TRUE(get_effect(0).inp_size == 20);
   CU_ASSERT_TRUE(get_effect(0).out_size == 20);

   CU_ASSERT_TRUE(get_effect(1).inp_ports == 1);
   CU_ASSERT_TRUE(get_effect(1).out_ports == 1);
   CU_ASSERT_TRUE(get_effect(1).arg_ports == 0);
   CU_ASSERT_TRUE(get_effect(1).inp_size == 20);
   CU_ASSERT_TRUE(get_effect(1).out_size == 20);
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
   add_effect(e1);
   effect_module e2 = {
      1, 1, 0,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect2",
   add_3
   };
   add_effect(e2);
   //set volume
   set_effect_arg(0, 0, 2.0f);
   static wire w = {
      0,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w);
   w.inp[0] = JACKD_INPUT;
   w.arg[0] = NO_INPUT;
   add_wire(w);
   static wire w2 = {
      1,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w2);
   w2.inp[0] = 0;
   w2.inp_ports[0] = 0;
   add_wire(w2);
   set_output_module(1, 0);
   run_engine(in, out, 20);
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
   add_effect(e1);
   effect_module e2 = {
      1, 1, 0,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect2",
   add_3
   };
   add_effect(e2);
   static wire w = {
      1,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w);
   w.inp[0] = JACKD_INPUT;
   add_wire(w);
   static wire w2 = {
      0,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w2);
   w2.inp[0] = 1;
   w2.inp_ports[0] = 0;
   w2.arg[0] = NO_INPUT;
   add_wire(w2);
   set_effect_arg(0, 0, 2.0f);
   set_output_module(0, 0);
   run_engine(in, out, 20);
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
   add_effect(e1);
   effect_module e2 = {
      1, 1, 0,
      20, 20,
      NULL, NULL, NULL,
      NULL, "test_effect2",
      add_3
   };
   add_effect(e2);
   add_effect(e2);
   //set volume
   set_effect_arg(0, 0, 2.0f);
   static wire w = {
      1,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w);
   w.inp[0] = JACKD_INPUT;
   add_wire(w);
   static wire w2 = {
      0,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w2);
   w2.inp[0] = 2;
   w2.inp_ports[0] = 0;
   w2.arg[0] = NO_INPUT;
   add_wire(w2);
   static wire w3 = {
      2,NULL,NULL,NULL,NULL
   };
   ms_wire_alloc(&w3);
   w3.inp[0] = 1;
   add_wire(w3);
   set_output_module(0, 0);
   run_engine(in, out, 20);
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
   CU_add_test(pSuite, "test engine calc 1", test_calc1);
   CU_add_test(pSuite, "test engine calc 2", test_calc2);
   CU_add_test(pSuite, "test engine calc 3", test_calc3);

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}