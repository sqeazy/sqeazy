
FROM sqy/trusty/encoders-static:latest
FROM sqy/trusty/utils-static:latest

FROM gradle:4.0.1-jdk8

WORKDIR /home/gradle/

# build sqeazy
RUN git clone https://github.com/sqeazy/sqeazy.git

# Run app.py when the container launches
CMD ["bash"]
