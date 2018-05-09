#include <stdlib.h>
#include <CUnit/Basic.h>
#include "../libfractal/fractal.h"
#include "../libstack/stack.h"

void testGetNames(void) {
    struct fractal * test = fractal_new("fractal", 80, 40, 0.80, -0.8);
    CU_ASSERT_STRING_EQUAL("fractal", fractal_get_name(test));
    fractal_free(test);
     }

void testGetAndSetValue(void) {
    struct fractal * test = fractal_new("fractal", 80, 40, 0.80, -0.8);
    int x = 20;
    fractal_set_value(test, 80, 40, x);
    CU_ASSERT_EQUAL(x, fractal_get_value(test, 80, 40));
    fractal_free(test);
}

void testGetHeight(void) {
    struct fractal * test = fractal_new("fractal", -100, 80, 0.8, -0.8);
    CU_ASSERT_EQUAL(80, fractal_get_height(test));
    fractal_free(test);
}

void testGetWidth(void) {
    struct fractal * test = fractal_new("fractal", 100, 80, 0.8, -0.8);
    CU_ASSERT_EQUAL(100, fractal_get_width(test));
    fractal_free(test);
}

void testGetA(void) {
    struct fractal * test = fractal_new("fractal", 100, 50, 0.8, -0.8);
    CU_ASSERT_EQUAL(0.8, fractal_get_a(test));
    fractal_free(test);
}

void testGetB(void) {
    struct fractal * test = fractal_new("fractal", 100, 50, 0.8, -0.8);
    CU_ASSERT_EQUAL(-0.8, fractal_get_b(test));
    fractal_free(test);
}

void testComputeAndGetAverage(void){
    struct fractal * test = fractal_new("fractal", 100, 50, 0.75, -0.7);
    fractal_compute_average(test);
    double moyenne = 2.221400;
    CU_ASSERT_EQUAL(moyenne,fractal_get_average(test));
    fractal_free(test);
}


void testPushAndPop(void){
    node_t *nodes;
    struct fractal * test1 = fractal_new("fractal", 90, 10, 1, 0.6);
    stack_push(&nodes, test1);
    struct fractal * test2 = stack_pop(&nodes);
    CU_ASSERT_EQUAL(fractal_get_name(test1), fractal_get_name(test2));
    CU_ASSERT_EQUAL(fractal_get_height(test1), fractal_get_height(test2));
    CU_ASSERT_EQUAL(fractal_get_width(test1), fractal_get_width(test2));
    CU_ASSERT_EQUAL(fractal_get_b(test1), fractal_get_b(test2));
    CU_ASSERT_EQUAL(test1, test2);
    fractal_free(test1);
    fractal_free(test2);
    stack_free(nodes);
}

static FILE* temp_file = NULL;

int init_suite(void)
{
   if (NULL == (temp_file = fopen("temp.txt", "w+"))) {
      return -1;
   }
   else {
      return 0;
   }
}

int clean_suite(void)
{
   if (0 != fclose(temp_file)) {
      return -1;
   }
   else {
      temp_file = NULL;
      return 0;
   }
}

int main() {

  CU_pSuite pSuite = NULL;

    if (CUE_SUCCESS != CU_initialize_registry()){
        return CU_get_error();
    }

    pSuite = CU_add_suite("liste_de_test", init_suite, clean_suite);    

     if (pSuite==NULL) {
        CU_cleanup_registry();
        return CU_get_error();
     }

    if (
            CU_add_test(pSuite, "test sur le nom", testGetNames)==NULL ||
            CU_add_test(pSuite, "test get/set value", testGetAndSetValue)==NULL ||
            CU_add_test(pSuite, "test sur la largeur", testGetHeight)==NULL ||
            CU_add_test(pSuite, "test sur la hauteur", testGetWidth)==NULL ||
            CU_add_test(pSuite, "test sur a", testGetA)==NULL ||
            CU_add_test(pSuite, "test sur b", testGetB)==NULL ||
            CU_add_test(pSuite, "test sur compute/get de la moyenne", testComputeAndGetAverage) ||
            CU_add_test(pSuite, "test sur la push et pop", testPushAndPop)){

        CU_cleanup_registry();
        fprintf(stderr, "Testing ended with error.\n");
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_basic_show_failures(CU_get_failure_list());
    CU_cleanup_registry();


    printf("Testing successful.\n");
    return 0;
}
