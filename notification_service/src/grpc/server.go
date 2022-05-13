package grpc

import (
	"context"
	"fmt"
	"log"
	"net"
	"notification_service/email_service"

	pb "notification_service/notification"

	"google.golang.org/grpc"
)

type notificationServer struct {
	pb.UnimplementedNotificationServer
}

// CarAlert sends an email with the offending car and then returns a
// CarReply or error.
func (s *notificationServer) CarAlert(ctx context.Context, car_notification *pb.CarNotification) (*pb.CarReply, error) {
	// img, _, err := image.Decode(bytes.NewReader(car_notification.Image))
	// if err != nil {
	// 	log.Fatalln(err)
	// }

	// out, _ := os.Create("./kenny.jpeg")
	// defer out.Close()

	// var opts jpeg.Options
	// opts.Quality = 1

	// err = jpeg.Encode(out, img, &opts)
	// //jpeg.Encode(out, img, nil)
	// if err != nil {
	// 	log.Println(err)
	// }

	email_service.SendEmail(email_service.Email{
		Subject: "Car Alert",
		Message: "Car was detected in parking spot.",
	})

	return &pb.CarReply{}, nil
}

func Init() {
	lis, err := net.Listen("tcp", fmt.Sprintf("localhost:%d", 50051))
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	grpcServer := grpc.NewServer()
	pb.RegisterNotificationServer(grpcServer, &notificationServer{})
	grpcServer.Serve(lis)
}
