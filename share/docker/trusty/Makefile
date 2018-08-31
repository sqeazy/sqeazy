SQY_VERSION ?= $(shell git describe --tags --abbrev=0)

############## base
build-base: Dockerfile.base
	docker build -t sqy/trusty/base:$(SQY_VERSION) -f $< .

push-base: build-base
	docker tag sqy/trusty/base:$(SQY_VERSION) psteinb/sqy_trusty_base:$(SQY_VERSION)
	docker push psteinb/sqy_trusty_base 

############### static/deployment images

build-encoders-static: Dockerfile.encoders-static build-base
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/encoders-static:$(SQY_VERSION) -f $< .

build-utils-static: Dockerfile.utils-static build-base
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/utils-static:$(SQY_VERSION) -f $< .

push-utils-static: build-utils-static
	docker tag sqy/trusty/utils-static:$(SQY_VERSION) psteinb/sqy_trusty_utils_static:$(SQY_VERSION)
	docker push psteinb/sqy_trusty_utils_static

push-encoders-static: build-encoders-static
	docker tag sqy/trusty/encoders-static:$(SQY_VERSION) psteinb/sqy_trusty_encoders_static:$(SQY_VERSION)
	docker push psteinb/sqy_trusty_encoders_static

build-static: Dockerfile.static build-utils-static build-encoders-static
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t psteinb/sqeazy:$(SQY_VERSION) -f $< .

static: build-static

push-static : push-encoders-static push-utils-static push-base
	docker push psteinb/sqeazy:$(SQY_VERSION)

############### shared/ci images

build-encoders: Dockerfile.encoders build-base
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/encoders:$(SQY_VERSION) -f $< .

push-encoders: build-encoders
	docker tag sqy/trusty/encoders:$(SQY_VERSION) psteinb/sqy_trusty_encoders:$(SQY_VERSION)
	docker push psteinb/sqy_trusty_encoders 

build-utils: Dockerfile.utils build-base
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/utils:$(SQY_VERSION) -f $< .

push-utils: build-utils
	docker tag sqy/trusty/utils:$(SQY_VERSION) psteinb/sqy_trusty_utils:$(SQY_VERSION)
	docker push psteinb/sqy_trusty_utils 

build-shared: Dockerfile build-utils build-encoders
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t psteinb/sqeazy-testing:$(SQY_VERSION) -f $< .

shared : build-shared
testing: build-shared

push-shared : push-encoders push-utils push-base
	docker push psteinb/sqeazy-testing:$(SQY_VERSION)

#when the shared library container is built, we want to update the one on dockerhub right away
push : push-shared push-static

build: push

################# pull targets

pull-testing:
	docker pull psteinb/sqeazy-testing:$(SQY_VERSION)

pull-static:
	docker pull psteinb/sqeazy:$(SQY_VERSION)

################# run targets

run-base: build-base
	docker run -it --rm sqy/trusty/base:$(SQY_VERSION) bash

# run-encoders-static: build-encoders-static
# 	docker run -it --rm sqy/trusty/encoders-static:$(SQY_VERSION) bash

run-static: pull-static
	docker run -e ORG_GRADLE_PROJECT_bintray_user=psteinb -e ORG_GRADLE_PROJECT_bintray_key=${MY_BINTRAY_KEY} -it --rm psteinb/sqeazy:$(SQY_VERSION)

run: pull-testing
	docker run -e ORG_GRADLE_PROJECT_bintray_user=psteinb -e ORG_GRADLE_PROJECT_bintray_key=${MY_BINTRAY_KEY} -it --rm psteinb/sqeazy-testing:$(SQY_VERSION)

run-shared: run
