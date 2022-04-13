


build: 
	docker-compose build

run:
	docker-compose up

build_notification_service: 
	docker build -t notification_service notification_service/

run_notification_service:
	docker run -it --rm --name notification_service notification_service

build_cascade:
	docker build -t cascade cascade_service/

run_cascade:
	docker run -it --rm --name cascade cascade


