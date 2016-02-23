#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#define TESTING
#include "gsm.c"

void __wrap_write_string(UART_MODULE id, char *string) {}
void __wrap_delay_ms(unsigned int msec) {}
void __wrap_OpenTimer23(unsigned int config, unsigned int period) {}
void __wrap_CloseTimer23(void) {}
void __wrap_WriteCoreTimer(int n) {}
void __wrap_WriteTimer23(unsigned int value) {}
unsigned int __wrap_ReadCoreTimer(void) {return mock_type(unsigned int);}
unsigned int __wrap_ReadTimer23(void) {return mock_type(unsigned int);}


/* Test checking the GSM buffer for a token. */
void test_compare_response(void **state)
{
        int fnd;
        GsmState s;

        strcpy(s.buf, "RESPONSE OK\r\n");

        fnd = compare_response(&s, "OK");
        assert_int_equal(fnd, 1);
        fnd = compare_response(&s, "OOPS");
        assert_int_equal(fnd, 0);

        (void) state;
}

int main(void)
{
        const struct CMUnitTest tests[] = {
               cmocka_unit_test(test_compare_response),
        };

        return cmocka_run_group_tests(tests, NULL, NULL);
}
