# Tools

## Flyway

Flyway is the schema migration tool for this project.

Current project layout uses a Flyway Desktop project here:

- `database/flyway_runtime`

### Development

Development uses the Flyway Desktop project:

- project folder: `database/flyway_runtime`
- migration source: `database/flyway_runtime/migrations`
- schema model source: `database/flyway_runtime/schema-model`

In development, `migrate` means:

- create or update the target SQLite database
- apply pending migration SQL files
- keep your local schema aligned with the current migration history

### Manual CLI

If you use the Flyway CLI directly, run it against the Flyway project folder:

```powershell
.\database\tools\flyway\flyway.cmd -environment=development info
.\database\tools\flyway\flyway.cmd -environment=development migrate
.\database\tools\flyway\flyway.cmd -environment=development validate
.\database\tools\flyway\flyway.cmd -environment=development repair
```

Run those commands from:

- `database/flyway_runtime`

What each command does:

- `info`: shows current migration status and which versions are applied
- `migrate`: applies pending SQL migrations to the configured SQLite database
- `validate`: checks whether applied migrations match the local migration files
- `repair`: fixes Flyway metadata when history state needs cleanup after a failed change

### Production

Production uses the same Flyway migration concept, but with the Flyway CLI bundled inside the shipped application or update package.

In production, the expected approach is:

- keep using versioned SQL migrations from `database/flyway_runtime/migrations`
- bundle the Flyway CLI with the application or updater
- run `flyway -environment=production migrate` automatically against the installed user database
- use a production-specific Flyway config if the DB path differs from local development
- trigger migration before the app starts using repositories and services

Simple rule:

- development uses the Flyway Desktop project in `database/flyway_runtime`
- production should use Flyway too, but with a bundled CLI and production-safe configuration

## Notes

- Flyway Desktop generated the project folders automatically
- setup details live in [setupdb.md](/C:/Users/arief/OneDrive/Desktop/travis-v2/travis/database/docs/setupdb.md)
