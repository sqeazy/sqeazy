SQY_VERSION ?= $(shell git describe --tags --abbrev=0)


build: Dockerfile
	docker build -t sqy/trusty-test:$(SQY_VERSION) -f $< .

run:
	docker run -it --rm sqy/trusty-test:$(SQY_VERSION)
