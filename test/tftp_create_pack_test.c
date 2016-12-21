#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "tftp.h"

/*
 * Setup and teardown for create message tests.
 */
static int setup(void **state) {
  apr_pool_t *mp;

  apr_initialize();
  apr_pool_create(&mp, NULL);

  *state = mp;

  return 0;
}

static int teardown(void **state) {
  apr_pool_destroy(*state);
  apr_terminate();
  return 0;
}

/*
 * Testing functions.
 */

/* Test create RRQ tftp packet. */
// ----------------------------------
static void create_rrq_pack_test (void **state)
{
  assert_int_equal (10, 10);
}

/*
 * Run all tests.
 */
int main(void)
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test_setup_teardown (create_rrq_pack_test, setup, teardown),
  };
  return cmocka_run_group_tests_name("tftpclient library tests", tests, NULL, NULL);
}
