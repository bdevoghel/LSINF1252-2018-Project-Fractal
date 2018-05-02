#include <stdlib.h>
#include <CUnit/Basic.h>
#include "../libfractal/fractal.h"
#include "../stack/stack.h"

void testFractalesGetNames(void) {
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
