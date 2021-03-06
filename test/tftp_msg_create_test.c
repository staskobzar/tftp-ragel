#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "tftp_msg.h"

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
  char *buf = apr_palloc(*state, DATA_SIZE + 4);
  struct pack_rq rrq = {
    .filename = "bar.txt",
    .len_filename = strlen("bar.txt"),
    .mode = MODE_ASCII,
    .len_mode = strlen(MODE_ASCII),
    .e_mode = E_ASCII
  };
  apr_size_t len = tftp_create_rrq (buf, &rrq);
  tftp_pack *pack = tftp_packet_read(buf, len, *state);
  assert_int_equal (pack->opcode, E_RRQ);
  assert_string_equal (pack->data->rq.filename, "bar.txt");
  assert_int_equal (pack->data->rq.e_mode, E_ASCII);
}

/* Test create WRQ tftp packet. */
// ----------------------------------
static void create_wrq_pack_test (void **state)
{
  char *buf = apr_palloc(*state, DATA_SIZE + 4);
  struct pack_rq wrq = {
    .filename = "xyz.zip",
    .len_filename = strlen("xyz.zip"),
    .mode = MODE_OCTET,
    .len_mode = strlen(MODE_OCTET),
    .e_mode = E_ASCII
  };
  apr_size_t len = tftp_create_wrq (buf, &wrq);
  tftp_pack *pack = tftp_packet_read(buf, len, *state);
  assert_int_equal (pack->opcode, E_WRQ);
  assert_string_equal (pack->data->rq.filename, "xyz.zip");
  assert_int_equal (pack->data->rq.e_mode, E_OCTET);
}

/* Test create DATA tftp packet. */
// ----------------------------------
static void create_data_pack_test (void **state)
{
  apr_size_t len = 0;
  char *buf = apr_palloc(*state, DATA_SIZE + 4);
  tftp_pack *pack;
  const char data[] = "The licenses for most software and other practical works are designed\n"
                      "to take away your freedom to share and change the works.  By contrast,\n"
                      "the GNU General Public License is intended to guarantee your freedom to\n"
                      "share and change all versions of a program--to make sure it remains free\n"
                      "software for all its users.  We, the Free Software Foundation, use the\n"
                      "GNU General Public License for most of our software; it applies also to\n"
                      "any other work released this way by its authors.  You can apply it to\n"
                      "your programs";
  struct pack_data pdata = {
    .block = 12345,
    .length = strlen(data)
  };

  apr_cpystrn(pdata.data, data, pdata.length + 1);

  len = tftp_create_data (buf, &pdata);
  pack = tftp_packet_read(buf, len, *state);
  assert_int_equal (pack->opcode, E_DATA);
  assert_string_equal (pack->data->data.data, data);
  assert_int_equal (pack->data->data.block, 12345);
}

/* Test convert CR, NUL -> CR and CR, LF -> LF in tftp packet. */
// ----------------------------------
static void convert_str_ntoh_test (void **state)
{
  char *str_to_conv = "This is test \r\nstring\r\n to \nconvert";
  char *str_to_test = "This is test \nstring\n to \nconvert";
  apr_size_t len = strlen(str_to_conv);
  apr_size_t new_len = strlen(str_to_test);
  apr_size_t test_len = 0;

  char *buf = apr_pstrmemdup(*state, str_to_conv, len);

  test_len = tftp_str_ntoh (*state, buf, len);
  assert_int_equal (test_len, new_len);
  assert_string_equal (str_to_test, buf);
}

/* Test convert CR -> CR, NUL and LF -> CR, LF in tftp packet. */
// ----------------------------------
static void convert_str_hton_test (void **state)
{
  char *str_to_conv = "This is test \nstring\n to \nconvert";
  char *str_to_test = "This is test \r\nstring\r\n to \r\nconvert";
  apr_size_t len = strlen(str_to_conv);
  apr_size_t new_len = strlen(str_to_test);
  apr_size_t test_len = 0;

  char *buf = apr_pstrmemdup(*state, str_to_conv, len);

  test_len = tftp_str_hton (*state, buf, len);
  assert_int_equal (test_len, new_len);
  assert_string_equal (str_to_test, buf);
}


/* Test create ACK tftp packet. */
// ----------------------------------
static void create_ack_pack_test (void **state)
{
  char *buf = apr_palloc(*state, DATA_SIZE);
  int block = 4585;
  apr_size_t len = tftp_create_ack (buf, block);
  tftp_pack *pack = tftp_packet_read(buf, len, *state);

  assert_int_equal (pack->opcode, E_ACK);
  assert_int_equal (pack->data->ack.block, block);
}

/* Test create ERROR tftp packet. */
// ----------------------------------
static void create_error_pack_test (void **state)
{
  apr_size_t len = 0;
  tftp_pack *pack;
  char *msg = "Disk full.";
  int msg_len = strlen (msg);
  char *buf = apr_palloc(*state, DATA_SIZE);
  struct pack_error err_pack = {
    .ercode = ERR_DISKFULL,
    .msg = msg,
    .msg_len = msg_len
  };

  len = tftp_create_error (buf, &err_pack);
  pack = tftp_packet_read(buf, len, *state);
  assert_int_equal (pack->opcode, E_ERROR);
  assert_string_equal (pack->data->error.msg, msg);
  assert_int_equal (pack->data->error.msg_len, msg_len);
}

/*
 * Run all tests.
 */
int main(void)
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test_setup_teardown (create_rrq_pack_test, setup, teardown),
    cmocka_unit_test_setup_teardown (create_wrq_pack_test, setup, teardown),
    cmocka_unit_test_setup_teardown (create_data_pack_test, setup, teardown),
    cmocka_unit_test_setup_teardown (create_ack_pack_test, setup, teardown),
    cmocka_unit_test_setup_teardown (create_error_pack_test, setup, teardown),
    cmocka_unit_test_setup_teardown (convert_str_ntoh_test, setup, teardown),
    cmocka_unit_test_setup_teardown (convert_str_hton_test, setup, teardown),
  };

  return cmocka_run_group_tests_name("tftpclient library tests", tests, NULL, NULL);
}
