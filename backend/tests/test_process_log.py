

def test_log_processor():
    from server import log_processor
    from app.data_transfer_objects import CreateLogCommand, LogEntryDTO

    from datetime import datetime
    mock_command = CreateLogCommand(
        machine_guid="e4b1a8d2-7f3c-4b91-a1e5-6d8c92b4f0a1",
        username="john_doe",
        ip="192.168.1.100",
        logs=[
            LogEntryDTO(
                window_title="Visual Studio Code - main.py",
                window_class="Chrome_WidgetWin_1",
                window_process="Code.exe",
                timestamp=int(datetime(2023, 12, 16, 14, 44, 0).timestamp()),
                keystrokes="def hello_world():[ENTER]    print('Hello')"
            ),
            LogEntryDTO(
                window_title="Google Chrome - Stack Overflow",
                window_class="Chrome_WidgetWin_1",
                window_process="chrome.exe",
                timestamp=int(datetime(2023, 12, 16, 14, 44, 15).timestamp()),
                keystrokes="how to fix sqlalchemy[ENTER]"
            ),
            LogEntryDTO(
                window_title="Google Chrome - Stack Overflow",
                window_class="Chrome_WidgetWin_1",
                window_process="chrome.exe",
                timestamp=int(datetime(2023, 12, 16, 14, 44, 0).timestamp()),
                keystrokes="how to fix sqlalchemy[ENTER]"
            )
        ]
    )
    log_processor.process_log(mock_command)