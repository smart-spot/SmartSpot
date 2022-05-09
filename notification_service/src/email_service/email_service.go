// package email_service provides the email service which
// utilizes the smtp protocol to deliver emails from Smart Spot
package email_service

import (
	"fmt"
	"log"
	"net/smtp"
	"os"

	"github.com/joho/godotenv"
)

type Email struct {
	Message string
	Subject string
}

// init_dotenv initializes the dot environment
func initDotenv() {
	err := godotenv.Load()
	if err != nil {
		log.Fatalf("Error loading .env file")
	}
}

// send_email sends an email from the Smart Spot email address to
// za44@cornell.edu and tmb42@cornell.edu given a type Email object.
func SendEmail(email Email) {
	// initialize dot env for email and passwords
	initDotenv()

	// Sender data.
	from := os.Getenv("SMTP_EMAIL")
	password := os.Getenv("SMTP_PASSWORD")

	// Receiver email address.
	to := []string{
		"za44@cornell.edu",
		"tbm42@cornell.edu",
	}

	// smtp server configuration.
	const smtpHost = "smtp.gmail.com"
	const smtpPort = "587"

	// Message.
	messageBytes := []byte("Subject: " + email.Subject + "\r\n\r\n" +
		email.Message + "\r\n")

	// Authentication.
	auth := smtp.PlainAuth("", from, password, smtpHost)

	// Sending email.
	err := smtp.SendMail(smtpHost+":"+smtpPort, auth, from, to, messageBytes)
	if err != nil {
		log.Fatalf("%s", err)
	}
	fmt.Println("Email Sent Successfully!")
}
