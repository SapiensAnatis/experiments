FROM postgres:18

COPY --chmod=444 --chown=postgres:postgres ["init.sql", "/docker-entrypoint.initdb.d/init.sql"]
COPY --chmod=444 --chown=postgres:postgres ["mock_data.csv", "postgresql.conf", "/etc/postgresql/"]
