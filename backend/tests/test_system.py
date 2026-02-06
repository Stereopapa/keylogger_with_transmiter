import pytest
from flask import Request, request, current_app
from utils.http_utils import get_ip_addr


# @pytest.mark.connection
# def test_localhost_server_conn():
#     from server import app
#
#     @app.route("/test/collect", methods=["POST"])
#     def col():
#         ip = get_ip_addr(request)
#         print(f"IP: {ip}\n payload={request.get_json()}")
#
#         return 'OK', 200
#
#
#     app.run(host="127.0.0.1", port=80)

@pytest.mark.request_validation
def test_localhost_request_validation():
    from server import app, log_controller

    @app.route("/test/collect", methods=["POST"])
    def col():
        ip = get_ip_addr(request)
        print(request.get_json())
        res = log_controller.add_log(request.get_json(), ip)
        return res

    app.run(host="127.0.0.1", port=80)