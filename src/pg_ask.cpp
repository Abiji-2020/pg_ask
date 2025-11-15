extern "C" {
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/varlena.h"
#include "varatt.h"
}
#include <string>

#include "factory.hpp"
#include "prompt_builder.hpp"
#include "schema_loader.hpp"
#include "sql_executor.hpp"

// Mandatory module magic

extern "C" {
PG_MODULE_MAGIC;
// Mandatory function info macro — declares the function with C++ linkage
PG_FUNCTION_INFO_V1(pg_gen_query);

Datum pg_gen_query(PG_FUNCTION_ARGS) {
    text* input = PG_GETARG_TEXT_PP(0);

    std::string userQuery(VARDATA_ANY(input), VARSIZE_ANY_EXHDR(input));

    PG_RETURN_TEXT_P(cstring_to_text(userQuery.c_str()));
}
}