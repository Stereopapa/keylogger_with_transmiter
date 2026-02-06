from dataclasses import dataclass
from typing import List


@dataclass(frozen=True)
class LogEntryDTO:
    window_title: str
    window_class: str
    window_process: str
    timestamp: int
    keystrokes: str


@dataclass(frozen=True)
class CreateLogCommand:
    machine_guid: str
    username: str
    ip: str
    logs: List[LogEntryDTO]
