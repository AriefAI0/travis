# Database Setup

## Purpose

This document covers how to set up the database tooling for this project:

- Flyway for schema migration
- SchemaSpy for ERD generation

## Flyway local development setup

Windows steps:

1. Open the Flyway installer page:
   https://documentation.red-gate.com/fd/installers-172490864.html
2. Choose Windows and install Flyway Desktop.
3. Open Flyway Desktop.
4. Create a new project.
5. Set the project name.
6. Set the project path to:

```text
C:\Users\arief\OneDrive\Desktop\travis-v2\travis\database\flyway_runtime
```

7. Choose the database type: `SQLite`.
8. Finish the project creation.

Result:

- Flyway generates `database/flyway_runtime/flyway.toml`
- Flyway generates `database/flyway_runtime/migrations/`
- Flyway generates `database/flyway_runtime/schema-model/`

If you also want the Flyway CLI bundled in the repo:

1. Download the Flyway command-line Windows zip.
2. Extract it into this project at `database/tools/flyway`.
3. Verify this file exists:

```text
database/tools/flyway/flyway.cmd
```

4. From the project root, verify:

```powershell
.\database\tools\flyway\flyway.cmd -v
```

5. Use the CLI from inside the Flyway project folder:

```powershell
cd .\database\flyway_runtime
..\tools\flyway\flyway.cmd info
..\tools\flyway\flyway.cmd migrate
```

Official references:

- Flyway command-line overview: https://documentation.red-gate.com/fd/command-line-277579359.html
- Flyway command-line parameters: https://documentation.red-gate.com/fd/command-line-parameters-277578836.html
- Flyway SQLite support: https://documentation.red-gate.com/flyway/reference/database-driver-reference/sqlite

## Flyway production bundling

Production should bundle Flyway inside the shipped app or update package.

Recommended approach:

1. Place Flyway in a packaged tools directory, for example `app/tools/flyway/`.
2. Include:
   - Flyway executable files
   - production Flyway config or project configuration
   - SQL migrations from `database/flyway_runtime/migrations`
3. During startup or update:
   - resolve the installed database path
   - call bundled Flyway
   - run `migrate`
4. Continue into Qt repositories/services only after migration succeeds.

Example command shape:

```powershell
app\tools\flyway\flyway.cmd migrate
```

## SchemaSpy setup

SchemaSpy will be used after the first migration exists and `database/travis.db` can be generated.

Official references:

- SchemaSpy home: https://schemaspy.org/
- SchemaSpy documentation: https://schemaspy.readthedocs.io/
