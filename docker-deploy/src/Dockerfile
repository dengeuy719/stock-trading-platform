FROM ubuntu:20.04

RUN mkdir /src
WORKDIR /src
RUN apt-get -y update && apt-get install -y libpqxx-dev postgresql libxerces-c-dev libtinyxml-dev

CMD ["./main"]