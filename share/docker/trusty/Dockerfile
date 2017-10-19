ARG SQY_VERSION=latest

FROM sqy/trusty/base:${SQY_VERSION}
FROM sqy/trusty/encoders:${SQY_VERSION} as encoders
FROM sqy/trusty/utils:${SQY_VERSION} as utils

#COPY --from=base /usr/ /usr/
COPY --from=encoders /usr/ /usr/
COPY --from=utils /usr/ /usr/

WORKDIR /build

# build sqeazy
RUN git clone https://github.com/sqeazy/sqeazy.git

# Run app.py when the container launches
CMD ["bash"]
