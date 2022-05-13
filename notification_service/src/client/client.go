package main

import (
	"bufio"
	"context"
	"flag"
	"fmt"
	"log"
	pb "notification_service/notification"
	"os"
	"time"

	"google.golang.org/grpc"
)

func printCarReply(client pb.NotificationClient, notification *pb.CarNotification) {
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()
	reply, err := client.CarAlert(ctx, notification)
	if err != nil {
		log.Fatalf("client.GetFeature failed: %v", err)
	}
	log.Println(reply)
}

func encode_image() []byte {
	fileToBeUploaded := "ken.jpeg"
	file, err := os.Open(fileToBeUploaded)

	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	defer file.Close()

	fileInfo, _ := file.Stat()
	var size int64 = fileInfo.Size()
	bytes := make([]byte, size)

	// read file into bytes
	buffer := bufio.NewReader(file)
	_, err = buffer.Read(bytes) // <--------------- here!

	return bytes
}

func main() {
	flag.Parse()

	conn, err := grpc.Dial("localhost:50051", grpc.WithInsecure())
	if err != nil {
		log.Fatalf("fail to dial: %v", err)
	}
	defer conn.Close()
	client := pb.NewNotificationClient(conn)

	// Looking for a valid feature
	printCarReply(client, &pb.CarNotification{Image: encode_image(), UserName: "Tedi"})
}
