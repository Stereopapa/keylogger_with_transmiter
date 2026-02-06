from database.repository import (IpRepository, UsersRepository,
                                 MachinesRepository, WindowsRepository,
                                 LogRepository, MachineIpRepository)
from app.data_transfer_objects import CreateLogCommand, LogEntryDTO
from database.models import Machine, Log, User, MachineIp, Window, IpAddress

class LogProcessor:
    def __init__(self,log_repo: LogRepository, ip_repo: IpRepository, usr_repo: UsersRepository,
                 machine_repo: MachinesRepository, window_repo: WindowsRepository,
                 mach_ip_repo: MachineIpRepository):

        self._log_repo = log_repo
        self._ip_repo = ip_repo
        self._usr_repo = usr_repo
        self._mach_repo = machine_repo
        self._win_repo = window_repo
        self._mach_ip_repo = mach_ip_repo


    def process_log(self, cmd: CreateLogCommand):
        m_id = self._mach_repo.get_or_crete_machine(cmd.machine_guid)
        u_id = self._usr_repo.get_or_crete_user(cmd.username, m_id)

        ip_id: int | None
        if not cmd.ip: ip_id = None
        else: ip_id = self._ip_repo.get_or_crete_ip(cmd.ip)

        mi_ip: int = self._mach_ip_repo.get_or_crete_ip_machine(ip_id, m_id)

        for in_log in cmd.logs:
            w_id: int = self._win_repo.get_or_crete_window(in_log.window_title,
                                                                in_log.window_process,
                                                                in_log.window_class)
            out_log = Log(
                timestamp= (in_log.timestamp//60)*60,
                keystrokes= in_log.keystrokes,
                machine_id= m_id,
                user_id= u_id,
                ip_id= ip_id,
                window_id= w_id
            )
            self._log_repo.update_or_add_log(out_log)




