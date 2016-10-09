@CODE_COVERAGE_RULES@
check_PROGRAMS=protocol
protocol_SOURCES=protocol.c
protocol_CFLAGS=$(CODE_COVERAGE_CFLAGS) -I../src/ -std=gnu99 -Wall -Werror
protocol_LDFLAGS=$(CODE_COVERAGE_LDFLAGS) -L../src/.libs -lulink
TESTS=$(check_PROGRAMS)
