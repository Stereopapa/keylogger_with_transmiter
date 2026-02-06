DROP TRIGGER IF EXISTS update_machine_last_seen;
DROP TRIGGER IF EXISTS update_user_last_seen;

DROP INDEX IF EXISTS idx_user_time;
DROP INDEX IF EXISTS idx_machine_guid;
DROP INDEX IF EXISTS idx_username;
DROP INDEX IF EXISTS idx_ipv4;

DROP TABLE IF EXISTS machines_ips;
DROP TABLE IF EXISTS logs;
DROP TABLE IF EXISTS users;
DROP TABLE IF EXISTS ip_addresses;
DROP TABLE IF EXISTS machines;
DROP TABLE IF EXISTS windows;


