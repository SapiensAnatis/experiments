CREATE TABLE logs (
    id INTEGER PRIMARY KEY,
    time TIMESTAMP WITH TIME ZONE NOT NULL,
    message VARCHAR(255) NOT NULL
);

CREATE INDEX logs_time_id_idx ON logs(time, id);

COPY logs (id, time, message) FROM '/etc/postgresql/mock_data.csv' WITH (FORMAT csv, DELIMITER ',', HEADER true);