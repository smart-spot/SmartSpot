# Generated by the gRPC Python protocol compiler plugin. DO NOT EDIT!
"""Client and server classes corresponding to protobuf-defined services."""
import grpc

import notification_pb2 as notification__pb2


class NotificationStub(object):
    """Missing associated documentation comment in .proto file."""

    def __init__(self, channel):
        """Constructor.

        Args:
            channel: A grpc.Channel.
        """
        self.CarAlert = channel.unary_unary(
                '/Notification/CarAlert',
                request_serializer=notification__pb2.CarNotification.SerializeToString,
                response_deserializer=notification__pb2.CarReply.FromString,
                )


class NotificationServicer(object):
    """Missing associated documentation comment in .proto file."""

    def CarAlert(self, request, context):
        """Missing associated documentation comment in .proto file."""
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')


def add_NotificationServicer_to_server(servicer, server):
    rpc_method_handlers = {
            'CarAlert': grpc.unary_unary_rpc_method_handler(
                    servicer.CarAlert,
                    request_deserializer=notification__pb2.CarNotification.FromString,
                    response_serializer=notification__pb2.CarReply.SerializeToString,
            ),
    }
    generic_handler = grpc.method_handlers_generic_handler(
            'Notification', rpc_method_handlers)
    server.add_generic_rpc_handlers((generic_handler,))


 # This class is part of an EXPERIMENTAL API.
class Notification(object):
    """Missing associated documentation comment in .proto file."""

    @staticmethod
    def CarAlert(request,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.unary_unary(request, target, '/Notification/CarAlert',
            notification__pb2.CarNotification.SerializeToString,
            notification__pb2.CarReply.FromString,
            options, channel_credentials,
            insecure, call_credentials, compression, wait_for_ready, timeout, metadata)