FROM sqy/trusty/base:latest as base
FROM sqy/trusty/encoders-static:latest as encoders
FROM sqy/trusty/utils-static:latest as utils

COPY --from=base /usr/local /usr/local
COPY --from=encoders /usr/local /usr/local
COPY --from=utils /usr/local /usr/local

WORKDIR /build

# build sqeazy
RUN git clone https://github.com/sqeazy/sqeazy.git

# Run app.py when the container launches
CMD ["bash"]
