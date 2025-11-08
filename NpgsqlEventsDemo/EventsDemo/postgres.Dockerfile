FROM docker.io/library/postgres:17

COPY <<EOF /docker-entrypoint-initdb.d/create_job_table.sql
CREATE TABLE jobs (
    id INTEGER PRIMARY KEY,
    name VARCHAR(255)
);
EOF