ARG SQY_VERSION=latest

FROM psteinb/sqy_trusty_encoders:${SQY_VERSION} as encoders
FROM psteinb/sqy_trusty_utils:${SQY_VERSION} as utils
FROM psteinb/sqy_trusty_base:${SQY_VERSION}

#COPY --from=base /usr/ /usr/
COPY --from=encoders /usr/ /usr/
COPY --from=utils /usr/ /usr/

WORKDIR /build

# build sqeazy
RUN git clone https://github.com/sqeazy/sqeazy.git; \
    cd sqeazy ;\
    git remote add psteinb https://github.com/psteinb/sqeazy.git

# Run app.py when the container launches
CMD ["bash"]
