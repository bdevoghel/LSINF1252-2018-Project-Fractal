#include <stdlib.h>
#include <CUnit/Basic.h>
#include "../libfractal/fractal.h"
#include "../stack/stack.h"

void testGetNames(void) {
    struct fractal * test = fractal_new("fractale", 80, 40, 0.80, -0.8);
    CU_ASSERT_STRING_EQUAL(name, fractal_get_name(test));
    fractal_free(test);
}

void testGetAndSetValue(void) {
    struct fractal * test = fractal_new("fractale", 80, 40, 0.80, -0.8);
    int x = 20;
    fractal_set_value(test, 80, 40, x);
    CU_ASSERT_EQUAL(x, fractal_get_value(test, 80, 40));
    fractal_free(test);
}

void testGetHeight(void) {
    struct fractal * test = fractal_new("fractale", 100, 80, 0.8, -0.8);
    CU_ASSERT_EQUAL(80, fractal_get_height(test));
    fractal_free(test);
}

void testGetWidth(void) {
    struct fractal * test = fractal_new("fractale", 100, 80, 0.8, -0.8);
    CU_ASSERT_EQUAL(100, fractal_get_width(test));
    fractal_free(test);
}

void testGetA(void) {
    struct fractal * test = fractal_new("fractale", 100, 50, 0.8, -0.8);
    CU_ASSERT_EQUAL(0.8, fractal_get_a(test));
    fractal_free(test);
}

void testGetB(void) {
    struct fractal * test = fractal_new("fractale", 100, 50, 0.8, -0.8);
    CU_ASSERT_EQUAL(-0.8, fractal_get_b(test));
    fractal_free(test);
}


int main(int argc, char const *argv[]) {
    CU_pSuite pSuite;
    if (CUE_SUCCESS != CU_initialize_registry()){
        return CU_get_error();
    }
    pSuite = NULL;
    pSuite = CU_add_suite("Collection de tests sur les fractales", setup, teardown);
    if (pSuite==NULL) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (
            CU_add_test(pSuite, "test de la librairie fractale, nom", testGetNames)==NULL ||
            CU_add_test(pSuite, "test get/set value", testGetAndSetValue)==NULL ||
            CU_add_test(pSuite, "test de la librairie fractale, largeur", testGetHeight)==NULL ||
            CU_add_test(pSuite, "test de la librairie fractale, hauteur", testGetWidth)==NULL ||
            CU_add_test(pSuite, "test de la librairie fractale, a", testGetA)==NULL ||
            CU_add_test(pSuite, "test de la librairie fractale, b", testGetB)==NULL ){

        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_basic_show_failures(CU_get_failure_list());
    CU_cleanup_registry();

    printf("\n\n");
    return 0;
}