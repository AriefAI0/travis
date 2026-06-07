# Tools

## Flyway

Run local migrations:

```powershell
powershell -ExecutionPolicy Bypass -File .\database\scripts\run-flyway.ps1 -Command migrate
```

If Flyway is not on `PATH`, pass the executable path:

```powershell
powershell -ExecutionPolicy Bypass -File .\database\scripts\run-flyway.ps1 -Command migrate -FlywayPath "C:\path\to\flyway.cmd"
```

## Notes

- Flyway's official SQLite JDBC URL format is `jdbc:sqlite:database`
- SchemaSpy setup will be added after the first migration exists
