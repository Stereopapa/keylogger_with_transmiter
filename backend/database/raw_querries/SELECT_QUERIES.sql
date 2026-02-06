
/*
FILTER:
	1. (Keystrokes have <string>)
	2. date_hour day is today or month is january
	3. window name == <string>,
	4. class name == <string>,
	5. process name == <string>,
	4. ip_address == <string>,
	5. machine_guid == <string>,
	6. username == <string>,
	7. user created_at == in this month
	8. user last_seen 
	
SORT:
	2. date_hour
	3. window name,
	4. class name,
	5. process name,
	4. ip_address,
	5. machine_guid,
	6. username,
	7. last_seen
	8. created_at  (less common and users machines will be small tables)
WHERE order (best performance)
date_hour_minute, machine_id, user_id, window_id

user_id, date_hour_minute
username, machine_id
name, class, process
*/

SELECT id FROM machines WHERE guid = "<>"
SELECT id FROM users WHERE username = "<>" AND machine_id = "<>"
SELECT id FROM windows WHERE name = "<>" AND process = "<>" AND class = ""
SELECT id FROM ip_addresses WHERE ipv4 = "<>"
SELECT id FROM machines_ips WHERE ip_id = "<>" AND machine_id = "<>"

SELECT id, keystrokes FROM logs
	INNER JOIN machines ON machines.id = machine_id  
	INNER JOIN users ON users.id = user_id
	INNER JOIN windows ON windows.id = window_id
	LEFT JOIN ip_addresses ON ip_addresses.id = ip_id
WHERE machine_id = 0 AND user_id = 0 AND window_id = 0 AND ip_id = 0
	AND timestamp = "<>"