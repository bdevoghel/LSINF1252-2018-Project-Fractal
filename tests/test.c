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
