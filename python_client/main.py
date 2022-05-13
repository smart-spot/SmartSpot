import notification_pb2_grpc
import __future__
import notification_pb2
import grpc


def main():
    with grpc.insecure_channel('localhost:50051', options=(('grpc.enable_http_proxy', 0),)) as channel:
        stub = notification_pb2_grpc.NotificationStub(channel)
        notification = notification_pb2.CarNotification(image=b'123', userName="Tedi")
        reply = stub.CarAlert(notification)


main()