SQY_VERSION ?= $(shell git describe --tags --abbrev=0)


build-base: Dockerfile.base
	docker build -t sqy/trusty/base:$(SQY_VERSION) -f $< .

build-encoders-static: Dockerfile.encoders-static build-base
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/encoders-static:$(SQY_VERSION) -f $< .

build-utils-static: Dockerfile.utils-static build-base
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/utils-static:$(SQY_VERSION) -f $< .

build-static: Dockerfile.static build-utils-static build-encoders-static
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/static:$(SQY_VERSION) -f $< .

build-encoders: Dockerfile.encoders build-base
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/encoders:$(SQY_VERSION) -f $< .

build-utils: Dockerfile.utils build-base
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/utils:$(SQY_VERSION) -f $< .

build-shared: build-utils build-encoders
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/shared:$(SQY_VERSION) .

#when the shared library container is built, we want to update the one on dockerhub right away
to_dockerhub : build-shared build-static
	docker tag sqy/trusty/shared:$(SQY_VERSION) psteinb/sqeazy-testing:$(SQY_VERSION)
	docker tag sqy/trusty/shared:$(SQY_VERSION) psteinb/sqeazy-testing:latest
	docker push psteinb/sqeazy-testing
	docker tag sqy/trusty/static:$(SQY_VERSION) psteinb/sqeazy:$(SQY_VERSION)
	docker tag sqy/trusty/static:$(SQY_VERSION) psteinb/sqeazy:latest
	docker push psteinb/sqeazy

build: build-static to_dockerhub

run-base: build-base
	docker run -it --rm sqy/trusty/base:$(SQY_VERSION) bash

run-encoders-static: build-encoders-static
	docker run -it --rm sqy/trusty/encoders-static:$(SQY_VERSION) bash

run-static: build-static
	docker run -e ORG_GRADLE_PROJECT_bintray_user=psteinb -e ORG_GRADLE_PROJECT_bintray_key=${MY_BINTRAY_KEY} -it --rm sqy/trusty/static:$(SQY_VERSION)

run: build-shared
	docker run -e ORG_GRADLE_PROJECT_bintray_user=psteinb -e ORG_GRADLE_PROJECT_bintray_key=${MY_BINTRAY_KEY} -it --rm sqy/trusty/shared:$(SQY_VERSION)

run-shared: run
