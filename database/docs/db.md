# Database

This project uses:

- Qt SQL for runtime database access
- SQLite for the local database file
- Flyway for schema migrations
- SchemaSpy for ERD generation

## Structure

- `flyway_runtime/`: Flyway project created from Flyway Desktop
- `flyway_runtime/migrations/`: versioned Flyway SQL migrations
- `flyway_runtime/schema-model/`: Flyway schema model files
- `docs/`: generated database documentation output
- `scripts/`: local helper scripts for migration commands

## Rules

- SQL migrations are the source of truth for schema changes
- Qt code must not create or alter tables automatically
- Development uses local Flyway migrations against `database/travis.db`
- Production uses only tested versioned migrations

## Naming

- Initial schema: `V1__init_schema.sql`
- Later changes: `V2__description.sql`, `V3__description.sql`

## Local database

- Database target is managed through the Flyway Desktop project configuration
- Flyway history table is managed inside the SQLite database by Flyway

## References

- [tools.md](/C:/Users/arief/OneDrive/Desktop/travis-v2/travis/database/docs/tools.md)
- [setupdb.md](/C:/Users/arief/OneDrive/Desktop/travis-v2/travis/database/docs/setupdb.md)
