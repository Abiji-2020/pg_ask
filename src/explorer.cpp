extern "C" {
#include "postgres.h"
#include "c.h"
#include "catalog/pg_namespace.h"
#include "catalog/pg_class.h"
#include "catalog/pg_attribute.h"
#include "access/htup_details.h"
#include "access/table.h"
#include "access/genam.h"
#include "utils/fmgroids.h"
#include "utils/builtins.h"
#include "utils/lsyscache.h"
#include "access/xact.h"
#include "utils/snapmgr.h"
#include "utils/inval.h"
}

#include <string>
#include <sstream>
#include <vector>
#include "explorer.h"

static bool isSystemSchema(const std::string& s, const std::vector<std::string>& ignore) {
    for (const auto& bad : ignore) {
        if (s == bad)
            return true;
    }
    return false;
}

static void listColumns(Oid tableOid, std::ostringstream& out, Snapshot snapshot = NULL) {
    Relation attr_rel = table_open(AttributeRelationId, AccessShareLock);

    ScanKeyData key;
    ScanKeyInit(&key, Anum_pg_attribute_attrelid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(tableOid));

    SysScanDesc scan = systable_beginscan(attr_rel, AttributeRelidNumIndexId, true, snapshot, 1, &key);

    HeapTuple tup;

    while ((tup = systable_getnext(scan)) != NULL) {
        Form_pg_attribute att = (Form_pg_attribute)GETSTRUCT(tup);

        if (att->attisdropped)
            continue;

        if (att->attnum <= 0)
            continue;

        std::string col = NameStr(att->attname);
        char* col_type_ptr = format_type_with_typemod(att->atttypid, att->atttypmod);
        std::string col_type = std::string(col_type_ptr);
        pfree(col_type_ptr);
        out << "     Column:  " << col << " | Type: " << col_type << "\n";
    }

    systable_endscan(scan);
    table_close(attr_rel, AccessShareLock);
}

static void listTables(Oid nsp_oid, std::ostringstream& out, Snapshot snapshot = NULL) {
    Relation rel_rel = table_open(RelationRelationId, AccessShareLock);

    ScanKeyData key;
    ScanKeyInit(&key, Anum_pg_class_relnamespace, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(nsp_oid));

    SysScanDesc scan = systable_beginscan(rel_rel, ClassNameNspIndexId, true, snapshot, 1, &key);

    HeapTuple tup;

    while ((tup = systable_getnext(scan)) != NULL) {

        Form_pg_class cls = (Form_pg_class)GETSTRUCT(tup);

        if (cls->relkind != RELKIND_RELATION)
            continue;
        std::string table = NameStr(cls->relname);

        out << "  Table: " << table << "\n";
        listColumns(cls->oid, out, snapshot);
        out << "\n";
    }

    systable_endscan(scan);
    table_close(rel_rel, AccessShareLock);
}

std::string buildDatabaseMap() {
    std::ostringstream out;

    InvalidateSystemCaches();
    InvalidateCatalogSnapshot();
    CommandCounterIncrement();
    Snapshot snapshot = GetLatestSnapshot();

    std::vector<std::string> system_schemas = {
        "pg_catalog",
        "pg_toast",
        "information_schema",
        "pg_toast_temp",
    };

    Relation nsp_rel = table_open(NamespaceRelationId, AccessShareLock);

    SysScanDesc nsp_scan = systable_beginscan(nsp_rel, NamespaceNameIndexId, true, snapshot, 0, NULL);

    HeapTuple nsp_tup;

    while ((nsp_tup = systable_getnext(nsp_scan)) != NULL) {
        Form_pg_namespace nsp = (Form_pg_namespace)GETSTRUCT(nsp_tup);
        std::string schema = NameStr(nsp->nspname);

        if (isSystemSchema(schema, system_schemas))
            continue;
        out << "Schema: " << schema << "\n";
        listTables(nsp->oid, out, snapshot);
        out << "\n";
    }
    systable_endscan(nsp_scan);
    table_close(nsp_rel, AccessShareLock);
    return out.str();
}

std::string formatSchema(const std::string& raw) {
    std::istringstream in(raw);
    std::ostringstream out;

    std::string line;
    std::string schema;
    std::string table;

    std::vector<std::pair<std::string, std::string>> columns;

    auto trim = [](const std::string s) {
        auto start = s.find_first_not_of(" \t");
        auto end = s.find_last_not_of(" \t");
        if (start == std::string::npos)
            return std::string();
        return s.substr(start, end - start + 1);
    };

    auto flush = [&]() {
        if (schema.empty() || table.empty())
            return;
        out << schema << "." << table << " (";

        for (size_t i = 0; i < columns.size(); i += 1) {
            out << columns[i].first << " " << columns[i].second;
            if (i + 1 < columns.size())
                out << ", ";
        }
        out << ")\n";
        columns.clear();
    };

    while (std::getline(in, line)) {
        line = trim(line);
        if (line.rfind("Schema:", 0) == 0) {
            flush();
            schema = trim(line.substr(strlen("Schema:")));
            table.clear();
        } else if (line.rfind("Table:", 0) == 0) {
            flush();
            table = trim(line.substr(strlen("Table:")));
        } else if (line.rfind("Column:", 0) == 0) {
            std::string rest = line.substr(strlen("Column:"));
            rest = trim(rest);

            auto sep = rest.find(" | Type: ");
            if (sep == std::string::npos) {
                continue;
            }

            std::string col = trim(rest.substr(0, sep));
            std::string typ = trim(rest.substr(sep + strlen(" | Type: ")));
            columns.emplace_back(col, typ);
        }
    }
    flush();
    return out.str();
}
