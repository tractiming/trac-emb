#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#define TESTING
#include "rfid.c"

void __wrap_WriteCoreTimer(int n) {}
int __wrap_ReadCoreTimer(void) {return mock_type(unsigned int);}
void __wrap_write_string(UART_MODULE id, char *string) {}
void __wrap_delay_ms(unsigned int msec) {}

/* Test clearing the queue of splits. */
static void test_clear_queue(void **state)
{
        SplitQueue q;
        q.head = 2;
        q.tail = 1;
        clear_queue(&q);

        assert_int_equal(q.head, 0);
        assert_int_equal(q.tail, 0);

        (void) state;
}

/* Test parsing a message from the RFID reader. */
static void test_parse_splits(void **state)
{
        int err;
        char msg[] = "Tag:ABCD 1234, Last:2016/02/21 12:34:56.123, Ant:0";
        Split s;

        err = parse_split_data(msg, &s);

        assert_int_equal(err, 0);
        assert_string_equal(s.tag_id, "ABCD 1234");
        assert_string_equal(s.time, "2016/02/21 12:34:56.123");
        assert_string_equal(s.ant, "0");

        (void) state;
}

/* Test parsing an invalid message from the RFID reader. */
static void test_parse_splits_bad_msg(void **state)
{
        int err;
        char msg[] = "Tag:ABCD 1234, Ant:0"; // message missing a time
        Split s;

        err = parse_split_data(msg, &s);

        assert_int_equal(err, -1);

        (void) state;
}

/* Test adding a character to the RFID string buffer. */
static void test_add_to_buffer(void **state)
{
        LineBuffer b;
        clear_buffer(&b);

        rfid_add_to_buffer(&b, 'a');
        rfid_add_to_buffer(&b, 'b');
        rfid_add_to_buffer(&b, 'c');
        rfid_add_to_buffer(&b, '\n');
        rfid_add_to_buffer(&b, 'd');
        rfid_add_to_buffer(&b, 'e');
        rfid_add_to_buffer(&b, 'f');
        rfid_add_to_buffer(&b, '\n');

        assert_string_equal(b.buf[0], "abc");
        assert_string_equal(b.buf[1], "def");
        assert_int_equal(b.head, 2);
        assert_int_equal(b.tail, 0);
        assert_int_equal(b.indx, 0);

        (void) state;
}

/* Test getting an update message from the queue of splits. */
void test_get_update_msg(void **state)
{
        int cnt;
        char msg[150];

        SplitQueue q;
        Split s1 = {
                .tag_id = "ABCD 1234",
                .time = "2016/02/21 12:34:56.123",
                .ant = "0"
        };
        Split s2 = {
                .tag_id = "ABCD 1235",
                .time = "2016/02/21 12:34:57.124",
                .ant = "0"
        };
        q.queue[0] = s1;
        q.queue[1] = s2;
        q.tail = 0;
        q.head = 2;

        const char msg_chk[] =
                "[{\"tag\": \"ABCD 1234\", "
                "\"time\": \"2016/02/21 12:34:56.123\", "
                "\"reader\": \"A1010\", \"sessions\": [], "
                "\"athlete\": null},"
                "{\"tag\": \"ABCD 1235\", "
                "\"time\": \"2016/02/21 12:34:57.124\", "
                "\"reader\": \"A1010\", \"sessions\": [], "
                "\"athlete\": null}]";
        cnt = get_update_msg(&q, "A1010", msg);

        assert_int_equal(cnt, 2);
        assert_string_equal(msg, msg_chk);
}

int main(void)
{
        const struct CMUnitTest tests[] = {
                cmocka_unit_test(test_clear_queue),
                cmocka_unit_test(test_parse_splits),
                cmocka_unit_test(test_parse_splits_bad_msg),
                cmocka_unit_test(test_add_to_buffer),
                cmocka_unit_test(test_get_update_msg),
        };

        return cmocka_run_group_tests(tests, NULL, NULL);
}
