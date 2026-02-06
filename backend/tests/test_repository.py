from database.repository import MachinesRepository, UsersRepository
from database.models import Machine, User

def test_into_machine():
    machine_repo = MachinesRepository()
    usr_repo = UsersRepository()
    machine_id: int = machine_repo.get_or_crete_machine("e4b1a812-7f3c-4b91-a1e5-6d8c92b4f0a1")
    print(machine_id)
    # user: User = usr_repo.get_or_crete_user("admin", machine.id)