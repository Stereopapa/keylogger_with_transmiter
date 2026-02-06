from ipaddress import ip_address

from sqlalchemy import select, insert, update
from database.db import get_db_session
from database.models import Machine, Log, User, MachineIp, Window, IpAddress


class IpRepository:
    def get_or_crete_ip(self, ipv_4: str) -> int:
        with get_db_session() as session:
            querry = select(IpAddress).where(IpAddress.ipv4 == ipv_4)
            ip_address = session.execute(querry).scalar_one_or_none()
            if not ip_address:
                ip_address = IpAddress(ipv4=ipv_4)
                session.add(ip_address)
                session.flush()
            return ip_address.id # type: ignore



class MachinesRepository:
    def get_or_crete_machine(self, machine_guid: str) -> int:
        with get_db_session() as session:
            querry = select(Machine).where(Machine.guid == machine_guid )
            machine = session.execute(querry).scalar_one_or_none()
            if not machine:
                machine = Machine(guid=machine_guid)
                session.add(machine)
                session.flush()
            return  machine.id # type: ignore

class UsersRepository:
    def get_or_crete_user(self, username: str, machine_id: int) -> int:
        with get_db_session() as session:
            querry = select(User).where(
                User.username == username,
                User.machine_id == machine_id
            )
            user = session.execute(querry).scalar_one_or_none()
            if not user:
                user = User(username=username, machine_id=machine_id)
                session.add(user)
                session.flush()
            return user.id # type: ignore


class WindowsRepository:
    def get_or_crete_window(self, title: str, process_name: str, class_name: str) -> int:
        with get_db_session() as session:
            querry = select(Window).where(
                Window.name == title,
                Window.process_name == process_name,
                Window.class_name == class_name
            )
            window = session.execute(querry).scalar_one_or_none()
            if not window:
                window = Window(name=title, class_name=class_name, process_name=process_name)
                session.add(window)
                session.flush()

            return window.id # type: ignore

class MachineIpRepository:
    def get_or_crete_ip_machine(self, ip_id: int, machine_id: int) -> MachineIp:
        with get_db_session() as session:
            querry = select(MachineIp).where(
                MachineIp.machine_id == machine_id,
                MachineIp.ip_id == ip_id
            )
            mach_ip = session.execute(querry).scalar_one_or_none()
            if not mach_ip:
                mach_ip = MachineIp(machine_id=machine_id, ip_id=ip_id)
                session.add(mach_ip)
                session.flush()

            return mach_ip.id # type: ignore

class LogRepository:
    def update_or_add_log(self, in_log: Log) -> None:
        with get_db_session() as session:
            querry = select(Log).where(
                Log.timestamp == in_log.timestamp,
                Log.machine_id == in_log.machine_id,
                Log.user_id == in_log.user_id,
                Log.window_id == in_log.window_id,
            )
            out_log: Log = session.execute(querry).scalar_one_or_none()
            if out_log:
                out_log.keystrokes += in_log.keystrokes
            else:
                session.add(in_log)
                session.flush()



