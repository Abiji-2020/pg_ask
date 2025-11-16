extern "C" {
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/varlena.h"
#include "varatt.h"
}
#include <string>
#include <explorer.h>

// Mandatory module magic

extern "C" {
PG_MODULE_MAGIC;
// Mandatory function info macro — declares the function with C++ linkage
PG_FUNCTION_INFO_V1(pg_gen_query);

Datum pg_gen_query(PG_FUNCTION_ARGS) {
    text* input = PG_GETARG_TEXT_PP(0);

    std::string userQuery(VARDATA_ANY(input), VARSIZE_ANY_EXHDR(input));
    std::string tablenames = buildDatabaseMap();
    std::string formatted_table = formatSchema(tablenames);
    PG_RETURN_TEXT_P(cstring_to_text(formatted_table.c_str()));
}
}