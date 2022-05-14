package main

import (
	"log"
	"notification_service/grpc"
)

func main() {
	log.Printf("Server Running")
	grpc.Init()
}
