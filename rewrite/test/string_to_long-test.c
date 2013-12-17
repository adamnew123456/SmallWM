#include <assert.h>

#include "../util.h"

int main()
{
    status_t status;
    unsigned long long result;

    result = string_to_long("102838", &status);
    assert(result == 102838 && status == SUCCESS);

    result = string_to_long("0xbeefface", &status);
    assert(result == 0xbeefface && status == SUCCESS);

    result = string_to_long("12 ugly men", &status);
    assert(status == FAIL);
}
