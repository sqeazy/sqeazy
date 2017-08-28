FROM sqy/trusty/base:latest
FROM sqy/trusty/encoders:latest as encoders
FROM sqy/trusty/utils:latest as utils

#COPY --from=base /usr/local /usr/local

COPY --from=encoders /usr/local /usr/local
COPY --from=encoders /usr/lib /usr/lib
COPY --from=encoders /usr/include /usr/include

COPY --from=utils /usr/local /usr/local

WORKDIR /build

# build sqeazy
RUN git clone https://github.com/sqeazy/sqeazy.git

# Run app.py when the container launches
CMD ["bash"]
